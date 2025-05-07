#ifndef COMM_SOCK_H
#define COMM_SOCK_H

#define MAX_STR 256
#define MAX_VECTOR 32

typedef struct peticion{

    // socket del cliente, usado para pasar 
    // el socket a un thread
    int socket_pet;

    // string que indica qué operación se va a hacer:
    // REGISTER, UNREGISTER, CONNECT, DISCONNECT, 
    // PUBLISH, PUBLISH, LIST_USERS y LIST_CONTENT
    char command_str[MAX_STR];  

    // Nombre de usuario que realiza la operación (TODAS)
    char client_user_name[MAX_STR];

    // ip del usuario que se conecta (CONNECT)
    char user_ip[MAX_STR];

    // puerto del usuario que se conecta (CONNECT)
    char user_port[MAX_STR];

    // descripción del archivo (PUBLISH)
    char description[MAX_STR];

    // path del archivo sobre el que se va a hacer
    // la operación (PUBLISH Y DELETE)
    char file_name[MAX_STR];

    // nombre de usuario del que se va a mostrar contenido (LIST_CONTENT)
    char user_name[MAX_STR];

}peticion;


#endif