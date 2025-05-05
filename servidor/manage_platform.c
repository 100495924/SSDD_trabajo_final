#include "manage_platform.h"
#include <stdio.h>
#include <unistd.h>

// -1 : error
// 0 : directorio existe, archivo no
// 1 : directorio existe, archivo también
// 2 : directorio no existe (archivo tampoco)
int is_file_in_directory(char *dirname, char *filepath){
    // Directorio "active_users" existe?
    DIR* dir = opendir(dirname);
    int result;
    
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

int register_user(char* user){
    // verificar antes si el archivo user existe
    if (access(user, F_OK) == 0){
        // El archivo (y el usuario) ya existe
        return 1;   
    }

    // abrir un archivo en modo escritura con fopen() con el nombre
    // user

    FILE *user_file = fopen(user, "w");

    if (user_file == NULL){
        perror("[ERROR] Error al crear el archivo para user\n");
        return 2;
    }

    return 0;
}

int unregister_user(char* user){
    // verificar antes si el archivo user existe
    if (access(user, F_OK) == 0){
        // El archivo (y el usuario) existe, eliminarlo
        if (unlink(user) != 0){
            // error eliminando el archivo
            return 2;   // error
        }
        return 0;
           
    }else{
        // El archivo (usuario) no existe
        return 1;
    }
}

int connect_user(char* user, char* ip_user, int port_user){
    // 0 en caso de exito
    // 1 si el usuario no registrado
    // 2 si el usuario ya está conectado
    // 3 en cualquier otro caso
    
    // ==========MODIFICAR===========
    // (1) Usuario registrado?
    char *dirname_registered = "registered_users";
    char filepath_registered[300];
    // Nombre del archivo del usuario (en el caso de existir)
    if (sprintf(filepath_registered, "%s/%s", dirname_registered, user) < 0){
        return 3;
    }

    int is_user_registered = is_file_in_directory(dirname_registered, filepath_registered);
    
    if (is_user_registered == 0 || is_user_registered == 2){
        return 1;
    }
    
    // (2) Usuario ya conectado?

    char *dirname_active = "active_users";
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
    char *dirname_registered = "registered_users";
    char filepath_registered[300];
    // Nombre del archivo del usuario (en el caso de existir)
    if (sprintf(filepath_registered, "%s/%s", dirname_registered, user) < 0){
        return 3;
    }

    int is_user_registered = is_file_in_directory(dirname_registered, filepath_registered);
    
    if (is_user_registered == 0 || is_user_registered == 2){
        return 1;
    }
    
    // (2) Usuario conectado?

    char *dirname_active = "active_users";
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
    // Usuario no conectado
    else if (is_user_connected == 0 || is_user_connected == 2){
        return 2;
    }
    
    // (3) Eliminar usuario del directorio "active_users"
    if (unlink(filepath_active) != 0){
        return 3; 
    }

    return 0;
}

int main(int argc, char const *argv[])
{
    // char *usuario = "user/Francisco";
    // int res = unregister_user(usuario);

    // int res = connect_user("Francisco", "10.128.1.253", 5000);
    int res = disconnect_user("Francisco");

    printf("Res: %d\n", res);

    return 0;
}
