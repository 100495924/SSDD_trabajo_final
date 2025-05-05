#include "manage_platform.h"
#include <stdio.h>
#include <unistd.h>

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

int main(int argc, char const *argv[])
{
    char *usuario = "user/Francisco";
    int res = unregister_user(usuario);

    printf("Res: %d\n", res);

    return 0;
}
