#include "manage_platform.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

char *dirname_registered = "registered_users";
char *dirname_active = "active_users";

// -1 : error
// 0 : directorio existe, archivo no
// 1 : directorio existe, archivo también
// 2 : directorio no existe (archivo tampoco)
int is_file_in_directory(char *dirname, char *filepath){
    // Directorio "active_users" existe?
    DIR* dir = opendir(dirname);
    int result;
    errno = 0;
    
    // Directorio existe
    if (dir) {
        // Existe usuario?
        if (access(filepath, F_OK) == 0){
            // Archivo existe
            result = 1;
        }
        else{
            // Archivo no existe
            result = 0;
        }
        // Cerrar directorio
        if (closedir(dir) == -1){
            result = -1;
        }
    } 
    // Directorio no existe
    else if (ENOENT == errno) {
        result = 2;
    } 
    // Otro error abriendo directorio
    else {
        result = -1;
    }

    return result;
}

// -1 : error
// 0 : no registrado
// 1 : registrado
int check_user_registered(char *user){
    char filepath_registered[300];
    // Nombre del archivo del usuario (en el caso de existir)
    if (sprintf(filepath_registered, "%s/%s", dirname_registered, user) < 0){
        return 3;
    }

    int is_user_registered = is_file_in_directory(dirname_registered, filepath_registered);
    
    if (is_user_registered == 1){
        return 1;
    }
    else if (is_user_registered == -1){
        return -1;
    }
    else{
        return 0;
    }
}

// -1 : error
// 0 : no registrado
// 1 : registrado
int check_user_connected(char *user){
    char filepath_active[300];
    // Nombre del archivo del usuario (en el caso de existir)
    if (sprintf(filepath_active, "%s/%s", dirname_active, user) < 0){
        return 3;
    }

    int is_user_active= is_file_in_directory(dirname_active, filepath_active);
    
    if (is_user_active == 1){
        return 1;
    }
    else if (is_user_active == -1){
        return -1;
    }
    else{
        return 0;
    }
}

int register_user(char* user){
    char filepath[300];
    sprintf(filepath, "%s/%s", dirname_registered, user);

    // verificar antes si el archivo user existe
    int dir_status = is_file_in_directory(dirname_registered, filepath);
    if (dir_status == -1){
        // error
        return 2;
    }

    if (dir_status == 2){
        // el directorio no existe, crearlo 
        if (mkdir(dirname_registered, 0755) == -1){
            return 2;
        }
    }

    if (dir_status == 2 || dir_status == 0){
        // el archivo no existe, crearlo

        FILE *user_file = fopen(filepath, "w");

        if (user_file == NULL){
            return 2;
        }

        if (fclose(user_file) < 0){
            return 2;   // error
        }
    }else if (dir_status == 1){
        // el usuario existe
        return 1;
    }

    return 0;
}

int unregister_user(char* user){
    char filepath[300];
    sprintf(filepath, "%s/%s", dirname_registered, user);

    // verificar antes si el archivo user existe
    int dir_status = is_file_in_directory(dirname_registered, filepath);
    if (dir_status == -1){
        return 2;   // error
    }else if(dir_status == 0 || dir_status == 2){
        // usuario (archivo) no existe
        return 1;
    }

    // El archivo (y el usuario) existe, eliminarlo
    if (unlink(filepath) != 0){
        // error eliminando el archivo
        return 2;   // error
    }

    return 0;
}

int connect_user(char* user, char* ip_user, int port_user){
    // 0 en caso de exito
    // 1 si el usuario no registrado
    // 2 si el usuario ya está conectado
    // 3 en cualquier otro caso
    
    // ==========MODIFICAR===========
    // (1) Usuario registrado?
    int is_user_registered = check_user_registered(user);
    // No registrado
    if (is_user_registered == 0){
        return 1;
    }
    // Error
    else if (is_user_registered == -1){
        return 3;
    }
    
    // (2) Usuario ya conectado?

    char filepath_active[300];
    // Nombre del archivo del usuario (en el caso de existir)
    if (sprintf(filepath_active, "%s/%s", dirname_active, user) < 0){
        return 3;
    }

    int is_user_connected = is_file_in_directory(dirname_active, filepath_active);

    // Error
    if (is_user_connected == -1){
        return 3;
    }
    // Usuario conectado
    else if (is_user_connected == 1){
        return 2;
    }
    // Directorio no existe, crearlo
    else if (is_user_connected == 2){
        if (mkdir(dirname_active, 0755) == -1){
            return 3;
        }
    }
    
    // (3) Añadir archivo correspondiente al directorio

    FILE *user_active_file  = fopen(filepath_active, "w+");
    if (user_active_file == NULL){
        return 3;
    }
    // Escribir IP y puerto
    if (fprintf(user_active_file, "%s/%d", ip_user, port_user) < 0){
        fclose(user_active_file);
        return 3;
    }
    if (fclose(user_active_file)){
        return 3;
    }

    return 0;
}

int disconnect_user(char *user){
    // 0 en caso de exito
    // 1 si el usuario no registrado
    // 2 si el usuario no conectado
    // 3 en cualquier otro caso
    
    // ==========MODIFICAR===========
    // (1) Usuario registrado?

    int is_user_registered = check_user_registered(user);
    // No registrado
    if (is_user_registered == 0){
        return 1;
    }
    // Error
    else if (is_user_registered == -1){
        return 3;
    }
    
    // (2) Usuario conectado?

    int is_user_connected = check_user_connected(user);
    // Error
    if (is_user_connected == -1){
        return 3;
    }
    // Usuario no conectado
    else if (is_user_connected == 0){
        return 2;
    }
    
    // (3) Eliminar usuario del directorio "active_users"

    char filepath_active[300];
    // Nombre del archivo del usuario (en el caso de existir)
    if (sprintf(filepath_active, "%s/%s", dirname_active, user) < 0){
        return 3;
    }

    if (unlink(filepath_active) != 0){
        return 3; 
    }

    return 0;
}

int publish(char* user, char* file_path, char* description){
    // verificar que el usuario existe
    char user_filepath[300];
    sprintf(user_filepath, "%s/%s", dirname_registered, user);

    // verificar antes si el archivo user existe
    if (check_user_registered(user) < 0){
        return 4;   // error
    }else if (check_user_registered(user) == 0){
        return 1;   // user no registrado
    }

    // verificar que el usuario está conectado
    if (check_user_connected(user) < 0){
        return 4;   // error
    }else if (check_user_connected(user) == 0){
        return 2;   // user no conectado
    }

    // abrir archivo user
    FILE *user_file = fopen(user_filepath, "r+");

    if (user_file == NULL){
        return 4;   // error
    }

    // verificar si el archivo ya está publicado
    int file_found = 1;     // 0 => encontrado
    int check_eof = 0;
    char path_read[256];
    char find_line_break = 'A';

    while (file_found != 0 && check_eof != EOF){
        check_eof = fscanf(user_file, "%s ", path_read);   // leemos el path del archivo 
        // lo comparamos con el path que estamos buscando
        file_found = strcmp(path_read, file_path);
        // leemos hasta encontrar un line break solo si no hemos terminado
        if (file_found != 0 && check_eof != EOF){
            while(find_line_break != '\n'){
                find_line_break = fgetc(user_file);
            }
            find_line_break = 'A';
        }
        
    }

    if (check_eof == EOF && file_found != 0){   // archivo no encontrado
        // publicar archivo
        if (fprintf(user_file, "%s ", file_path) < 0){
            fclose(user_file);
            return 4;   // error
        }
        if (fprintf(user_file, "%s\n", description) < 0){
            fclose(user_file);
            return 4;   // error
        }
    }else{
        fclose(user_file);
        return 3;   // el fichero ya está publicado
    }

    // cerrar el archivo 
    if (fclose(user_file) < 0){
        return 4;   // error
    }

    return 0;
}

int delete(char* user, char* file_path){
    // verificar que el usuario existe
    char user_filepath[300];
    sprintf(user_filepath, "%s/%s", dirname_registered, user);

    // verificar antes si el archivo user existe
    if (check_user_registered(user) < 0){
        return 4;   // error
    }else if (check_user_registered(user) == 0){
        return 1;   // user no registrado
    }

    // verificar que el usuario está conectado
    if (check_user_connected(user) < 0){
        return 4;   // error
    }else if (check_user_connected(user) == 0){
        return 2;   // user no conectado
    }

    // abrir archivo user
    FILE *user_file = fopen(user_filepath, "r+");

    if (user_file == NULL){
        return 4;   // error
    }

    // verificar si el archivo ya está publicado
    int file_found = 1;     // 0 => encontrado
    int check_eof = 0;
    char path_read[256];
    char find_line_break = 'A';

    while (file_found != 0 && check_eof != EOF){
        check_eof = fscanf(user_file, "%s ", path_read);   // leemos el path del archivo 
        // lo comparamos con el path que estamos buscando
        file_found = strcmp(path_read, file_path);
        // leemos hasta encontrar un line break solo si no hemos terminado
        if (file_found != 0 && check_eof != EOF){
            while(find_line_break != '\n'){
                find_line_break = fgetc(user_file);
            }
            find_line_break = 'A';
        }
        
    }

    if (check_eof == EOF && file_found != 0){   
        // archivo no encontrado
        fclose(user_file);
        return 3;
    }else{      // el fichero está publicado
        int is_last = 0;
        long key_pos;
        fseek(user_file, -strlen(path_read), SEEK_CUR);
        // estamos delante del fichero que hay que borrar
        key_pos = ftell(user_file);

        // avanzamos a la siguiente entrada
        check_eof = fscanf(user_file, "%s ", path_read);   // leemos el path del archivo 
        while(find_line_break != '\n'){
            find_line_break = fgetc(user_file);
        }
        find_line_break = 'A';
        // Ahora estamos al principio de la siguiente entrada después de la que hay que borrar

        // checkeamos si estamos al final del archivo o no
        check_eof = fscanf(user_file, "%s ", path_read);

        FILE *temp_file;
        if (check_eof != EOF){
            // No estamos al final del archivo, hacemos una copia 
            // del resto del archivo en un archivo temporal
            is_last = 0;
            temp_file = fopen("temp_copy", "w+");
            if (temp_file == NULL){
                return 4;   // error
            }

            // Metemos el path que habíamos leído antes
            fprintf(temp_file, "%s ", path_read);
            
            char buffer[1];
            while (fread(&buffer, sizeof(char), 1, user_file) > 0){
                fwrite(&buffer[0], 1, sizeof(buffer[0]), temp_file);
                
            }// Llegamos al final del archivo, habiendo hecho su copia.

            fflush(temp_file);
        }else{
            // estamos al final del archivo y no hace falta hacer copias
            is_last = 1;
        }
        fseek(user_file, key_pos-1, SEEK_SET);

        // truncamos el archivo
        ftruncate(user_file->_fileno, ftell(user_file));

        if (is_last == 0){
            // hacemos un MERGE de archivos
            char buffer[1];
            fseek(temp_file, 0, SEEK_SET);	// Volvemos al inicio en el archivo temporal.
            while (fread(&buffer, sizeof(char), 1, temp_file) > 0){
                fwrite(&buffer[0], 1, sizeof(buffer[0]), user_file);
            }
    
            fflush(user_file);
    
            if (fclose(temp_file) < 0){
                return 4;   // error
            }
            unlink("temp_copy");
        }
    }

    // cerrar el archivo 
    if (fclose(user_file) < 0){
        return 4;   // error
    }

    return 0;
}


int list_users_check(char* user){
    // 0 en caso de exito
    // 1 si el usuario no registrado
    // 2 si el usuario no conectado
    // 3 en cualquier otro caso

    // (1) Usuario registrado?

    int is_user_registered = check_user_registered(user);
    // No registrado
    if (is_user_registered == 0){
        return 1;
    }
    // Error
    else if (is_user_registered == -1){
        return 3;
    }
    
    // (2) Usuario conectado?

    int is_user_connected = check_user_connected(user);
    // Error
    if (is_user_connected == -1){
        return 3;
    }
    // Usuario no conectado
    else if (is_user_connected == 0){
        return 2;
    }

    return 0;
}

int list_users_get_num(long* count){
    DIR* dir = opendir(dirname_active);
    struct dirent * entry;
    errno = 0;

    if (dir) {
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_type == DT_REG) {
                // printf("%s - %d\n", entry->d_name, entry->d_type);
                count++;
            }
        }
        // Error de readdir()
        if (errno == EBADF){
            closedir(dir);
            return 3;
        }
        // Cerrar directorio
        if (closedir(dir) == -1){
            return 3;
        }
    } 
    // Error
    else {
        return 3;
    }

    return 0;
}


int list_users_get_info(struct ListUserInfo *info){
    DIR* dir = opendir(dirname_active);
    struct dirent * entry;
    errno = 0;
    FILE *user_active_file;
    char filepath_active[300];
    long count = 0;

    if (dir) {
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_type == DT_REG) {
                // user
                sprintf(info[count].user, "%s", entry->d_name);

                // Abrir archivo
                if (sprintf(filepath_active, "%s/%s", dirname_active, entry->d_name) < 0){
                    return 3;
                }
                user_active_file = fopen(filepath_active, "r");
                if (user_active_file == NULL){
                    return 3;
                }

                // ip_value
                // Calcular length del string en el archivo (leer caracter a caracter hasta "/")
                char ip_value[16];
                int count_ip = 0;
                char c = 'a';
                while (c != '/'){
                    c = fgetc(user_active_file);
                    count_ip++;
                }
                // Leer ese numero de bytes en el archivo
                fseek(user_active_file, 0, SEEK_SET); // principio del archivo
                fread(ip_value, sizeof(char), count_ip-1, user_active_file);
                ip_value[count_ip-1] = '\0';
                fseek(user_active_file, 1, SEEK_CUR); // saltarse "/"

                sprintf(info[count].ip_user, "%s", ip_value);

                // port_user
                if (fscanf(user_active_file, "%d", &info[count].port_user) == EOF){
                    fclose(user_active_file);
                    return 3;
                }

                // Cerrar archivo
                if (fclose(user_active_file) < 0){
                    return 3;
                }
                count++;
            }
        }
        // Error de readdir()
        if (errno == EBADF){
            closedir(dir);
            return 3;
        }
        // Cerrar directorio
        if (closedir(dir) == -1){
            return 3;
        }
    } 
    // Error
    else {
        return 3;
    }

    return 0;
}

int list_content_get_num(char *user){
    // verificar que el usuario existe
    char user_filepath[300];
    sprintf(user_filepath, "%s/%s", dirname_registered, user);

    // verificar antes si el archivo user existe
    if (check_user_registered(user) < 0){
        return -2;   // error
    }else if (check_user_registered(user) == 0){
        return -1;   // user no registrado
    }
    
    FILE* owner_file = fopen(user_filepath, "r+");

    if (owner_file == NULL){
        return -2;
    }

    // contar el número de lineas del archivo es igual a 
    // contar el número de entradas/archivos que ofrece
    // un usuario
    int n_files = 0;
    char check_line_break = 'A';

    fseek(owner_file, 0, SEEK_SET);
    while (check_line_break != EOF){
        check_line_break = fgetc(owner_file);
        if (check_line_break == '\n'){
            n_files++;
        }
    }

    if (fclose(owner_file) < 0){
        return -2;  // error
    }
    
    return n_files;
}

int list_content(char* user, char* contents_owner, struct ListContentInfo* contents){
    // verificar que el usuario existe
    char user_filepath[300];
    sprintf(user_filepath, "%s/%s", dirname_registered, contents_owner);

    // verificar antes si el archivo user existe
    if (check_user_registered(user) < 0){
        return 4;   // error
    }else if (check_user_registered(user) == 0){
        return 1;   // user no registrado
    }

    // verificar que el usuario está conectado
    if (check_user_connected(user) < 0){
        return 4;   // error
    }else if (check_user_connected(user) == 0){
        return 2;   // user no conectado
    }

    // abrir archivo del dueño del contenido
    FILE *user_file = fopen(user_filepath, "r+");

    if (user_file == NULL){
        return 4;   // error
    }

    // leer todo el archivo y guardar las entradas
    int check_eof = 0;
    char path_read[256];
    char find_line_break = 'A';
    char description_read[256];
    int entry = 0;
    int descr_char = 0;

    fseek(user_file, 0, SEEK_SET);
    while (check_eof != EOF){
        check_eof = fscanf(user_file, "%s ", path_read);   // leemos el path del archivo 

        // leemos hasta encontrar un line break solo si no hemos terminado
        if (check_eof != EOF){
            strcpy(contents[entry].file_path, path_read);
            descr_char = 0;
            while(find_line_break != '\n'){
                find_line_break = fgetc(user_file);
                description_read[descr_char] = find_line_break;
                descr_char++;
            }
            find_line_break = 'A';
            description_read[descr_char] = '\0';
            //check_eof = fscanf(user_file, "%s\n", description_read);
            strcpy(contents[entry].description, description_read);
        }
        entry++;
    }


    return 0;
}

/* int main(int argc, char const *argv[])
{
    // char *usuario = "user/Francisco";
    // int res = unregister_user(usuario);

    struct ListContentInfo* contents;
    int n_files, status;
    int res = register_user("aaaaaaa");
    printf("Res: %d\n", res);
    res = register_user("Paco");
    printf("Res: %d\n", res);
    res = connect_user("Paco", "10.128.1.253", 5000);
    //int res = unregister_user("Francisco");

    printf("Res: %d\n", res);

    res = publish("Francisco", "/usr/desktop/file_pataton", "Este archivo es una patata");
    res = publish("Francisco", "/usr/desktop/file_patatatita", "Este archivo es una patata");
    res = publish("Francisco", "/usr/desktop/file_patatatota", "Este archivo es una patata");
    //res = delete("Francisco", "/usr/desktop/file_pataton");

    // código para probar list_content
    n_files = list_content_get_num("Francisco");  // contar el número de entradas que tenemos que devolver
    if (n_files == -1){
        // el usuario cuyo contenido se quiere conocer no existe
        status = 3;
    }else if (n_files == -2){
        status = 4; // error
    }else{
        contents = (struct ListContentInfo*) malloc(sizeof(struct ListContentInfo) * n_files);
        status = list_content("Paco", "Francisco", contents);
    }

    if (status == 0){
        for (int i=0; i<n_files; i++){
            printf("%s %s", contents[i].file_path, contents[i].description);
        }
    }
    

    printf("status: %d\n", status);

    return 0;
} */


