#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <float.h>
#include "protocol.h"
#include "lines.h"

int str_to_long(char *long_str, char **endptr, int *resultado){
    // Función para pasar de un string a un long haciendo las 
    // comprobaciones de errores pertinentes
    *resultado = strtol(long_str, endptr, 10);

    // En realidad vamos a usar esta función para transformar string en int
    if (*resultado > INT_MAX || *resultado < INT_MIN){
        perror("strtol");
        return -1;
    }
    if (*endptr == long_str){
        fprintf(stderr, "No digits were found\n");
        return -1;
    }

    return 0;
}

int read_to_pet(int socket, struct peticion *pet){

    // Leer el comando del socket
    if (readLine(socket, pet->command_str, MAX_MSG) < 0){
        perror("ERROR: no se ha leido \n");
        return -1;
    }

    // Leer primer argumento del comando (client_user_name)
    if (readLine(socket, pet->client_user_name, MAX_MSG) < 0){
        perror("ERROR: no se ha leido \n");
        return -1;
    }

    if (strcmp(pet->command_str, "CONNECT") == 0){
        // Leer segundo argumento (puerto)
        if (readLine(socket, pet->user_ip, MAX_MSG) < 0){
            perror("ERROR: no se ha leido \n");
            return -1;
        }
        
        if (readLine(socket, pet->user_port, MAX_MSG) < 0){
            perror("ERROR: no se ha leido \n");
            return -1;
        }
    }else if(strcmp(pet->command_str, "PUBLISH") == 0){
        // Leer segundo argumento (nombre de archivo)
        if (readLine(socket, pet->file_name, MAX_MSG) < 0){
            perror("ERROR: no se ha leido \n");
            return -1;
        }

        // Leer tercer argumento (descripción)
        if (readLine(socket, pet->description, MAX_MSG) < 0){
            perror("ERROR: no se ha leido \n");
            return -1;
        }
    }else if (strcmp(pet->command_str, "DELETE") == 0){
        // Leer segundo argumento (nombre de archivo)
        if (readLine(socket, pet->file_name, MAX_MSG) < 0){
            perror("ERROR: no se ha leido \n");
            return -1;
        }
    }else if (strcmp(pet->command_str, "LIST_CONTENT") == 0){
        // Leer segundo argumento (nombre de usuario del que se lista)
        if (readLine(socket, pet->user_name, MAX_MSG) < 0){
            perror("ERROR: no se ha leido \n");
            return -1;
        }
    }
    
    return 0;
}
