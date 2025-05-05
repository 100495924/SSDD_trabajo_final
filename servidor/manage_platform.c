#include "manage_platform.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

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
    char *dirname_registered = "registered_users";
    char *filepath;
    filepath = malloc(sizeof(char) * (strlen(user) + strlen(dirname_registered) + 1));
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
    char *dirname_registered = "registered_users";
    char *filepath;
    filepath = malloc(sizeof(char) * (strlen(user) + strlen(dirname_registered) + 1));
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

int list_users(char* user, char* connected_users){
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

    // (3) Preparar lista

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

int main(int argc, char const *argv[])
{
    // char *usuario = "user/Francisco";
    // int res = unregister_user(usuario);

    // int res = connect_user("Francisco", "10.128.1.253", 5000);
    int res = disconnect_user("Francisco");

    printf("Res: %d\n", res);

    return 0;
}
