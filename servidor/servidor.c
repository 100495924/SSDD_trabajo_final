#include "comm_sock.h"
#include "protocol.h"
#include "lines.h"
#include "manage_platform.h"
#include <stdio.h>
#include <pthread.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <netdb.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <ifaddrs.h>


/*
Protocolo:

REGISTER <user_name> ==> 1 byte (0, 1 o 2)
UNREGISTER <user_name> ==> 1 byte (0, 1 o 2)
CONNECT <user_name> (<ip>) (<port>) ==> 1 byte (0, 1, 2 o 3)
PUBLISH (<user_name>) <file_name> <description> ==> 1 byte (0, 1, 2, 3 o 4)
DELETE (<user_name>) <file_name> ==> 1 byte (0, 1, 2, 3 o 4)
LIST_USERS (<user_name>) ==> 1 byte (0, 1, 2 o 3) y USUARIOS 
LIST_CONTENT (<user_name>) <user_name> ==> 1 byte (0, 1, 2, 3 o 4) y FICHEROS DE UN USUARIO
DISCONNECT <user_name> ==> 1 byte (0, 1, 2 o 3)

GET_FILE <user_name> <remote_file_name> <local_file_name>
*/

#define MAX_THREADS 10
#define MAX_PETICIONES 1024

struct peticion buffer_peticiones[MAX_PETICIONES];      // Buffer

int n_elementos;    // n de elementos en el buffer de peticiones
int pos_servicio = 0;
pthread_mutex_t mutex;
pthread_cond_t no_lleno;
pthread_cond_t no_vacio;

pthread_mutex_t mfin;   // fin del programa 
int fin=false;

pthread_mutex_t mutex_tuples;

void tratar_peticion(struct peticion *pet);

void *servicio(){
    struct peticion pet; /* mensaje local */

    for(;;){
        pthread_mutex_lock(&mutex);
        while (n_elementos == 0) {
            if (fin==true) {
                fprintf(stderr,"Finalizando servicio\n");
                pthread_mutex_unlock(&mutex);
                pthread_exit(0);
            }
            pthread_cond_wait(&no_vacio, &mutex);
        }

        // Copiar una petición del buffer circular
        // a una variable local para trabajar con ella
        pet = buffer_peticiones[pos_servicio];
        pos_servicio = (pos_servicio + 1) % MAX_PETICIONES;
        n_elementos --;

        pthread_cond_signal(&no_lleno);
        pthread_mutex_unlock(&mutex);
        
        pthread_mutex_lock(&mutex_tuples);
        tratar_peticion(&pet);  // tratar petición
        pthread_mutex_unlock(&mutex_tuples);
    }   

    pthread_exit(0);
}

void tratar_peticion(struct peticion *pet){
    int error, n_files;     // n_files para list_content_get_num
    int status;
    char status_str[2];
    long num_users = 0;  // para list_users_get_num
    struct ListContentInfo* contents;
    struct ListUserInfo* user_info;
    char port_user_string[10];

    //printf("%s %s %d", pet->command_str, pet->client_user_name, strcmp(pet->command_str, "REGISTER"));
    if (strcmp(pet->command_str, "REGISTER") == 0){
        // ejecutar REGISTER
        status = register_user(pet->client_user_name);
        
    }else if (strcmp(pet->command_str, "UNREGISTER") == 0){
        // ejecutar UNREGISTER
        status = unregister_user(pet->client_user_name);

    }else if (strcmp(pet->command_str, "CONNECT") == 0){
        // ejecutar CONNECT
        //printf("%s", pet->user_port);
        status = connect_user(pet->client_user_name, pet->user_ip, atoi(pet->user_port));

    }else if (strcmp(pet->command_str, "PUBLISH") == 0){
        // ejecutar PUBLISH
        //printf("%s %s", pet->file_name, pet->description);

    }else if (strcmp(pet->command_str, "DELETE") == 0){
        // ejecutar DELETE
        //printf("%s", pet->file_name);

    }else if (strcmp(pet->command_str, "LIST_USERS") == 0){
        list_users_check(pet->client_user_name);
        
        char connected_users[11];
        int return_value = list_users_get_num(&num_users);

        if (return_value != 0){
            status = return_value;
        }
        if (sprintf(connected_users, "%ld", num_users) < 0){
            status = 3;
        }

        user_info = (struct ListUserInfo*)malloc(num_users * sizeof(struct ListUserInfo));
        if (contents == NULL) {
            status = 3;
        }

        return_value = list_users_get_info(user_info);
        if (return_value != 0){
            free(user_info);
            status = return_value;
        }

        if (status != 0){
            free(user_info);
        }


    }else if (strcmp(pet->command_str, "LIST_CONTENT") == 0){
        // ejecutar LIST_CONTENT
        //printf("%s", pet->user_name);
        n_files = list_content_get_num(pet->user_name);  // contar el número de entradas que tenemos que devolver
        if (n_files == -1){
            // el usuario cuyo contenido se quiere conocer no existe
            status = 3;
        }else if (n_files == -2){
            status = 4; // error
        }else{
            contents = (struct ListContentInfo*) malloc(sizeof(struct ListContentInfo) * n_files);
            status = list_content(pet->client_user_name, pet->user_name, contents);
        }

    }else if (strcmp(pet->command_str, "DISCONNECT") == 0){
        // ejecutar DISCONNECT
        status = disconnect_user(pet->client_user_name);
    }

    sprintf(status_str, "%d", status);
    // enviar byte status
    /* if (write(pet->socket_pet, &status, sizeof(status)) < 0){
        perror("[ERROR] Error al enviar status\n");
        return;
    } */
    if (sendMessage(pet->socket_pet, status_str, 2) < 0){
        close(pet->socket_pet);
        return;
    }
    // si command_str es LIST_USERS y status es 0 enviar usuarios
    if (strcmp(pet->command_str, "LIST_USERS") == 0 && status == 0){
        for (int i=0; i<num_users; i++){
            if (sendMessage(pet->socket_pet, user_info[i].user, strlen(user_info[i].user)) < 0){
                free(user_info);
                close(pet->socket_pet);
                return;
            }
            if (sendMessage(pet->socket_pet, user_info[i].ip_user, strlen(user_info[i].ip_user)) < 0){
                free(user_info);
                close(pet->socket_pet);
                return;
            }
            if (sprintf(port_user_string, "%d", user_info[i].port_user) < 0){
                return;
            }
            if (sendMessage(pet->socket_pet, port_user_string, strlen(port_user_string)) < 0){
                free(user_info);
                close(pet->socket_pet);
                return;
            }
        }
    }

    // si command_str es LIST_CONTENT y status es 0 enviar contenido 
    if (strcmp(pet->command_str, "LIST_CONTENT") == 0 && status == 0){
        for (int i=0; i<n_files; i++){
            // enviar contents[i].file_path y contents[i].description
            if (sendMessage(pet->socket_pet, contents[i].file_path, strlen(contents[i].file_path)) < 0){
                free(contents);
                close(pet->socket_pet);
                return;
            }
            if (sendMessage(pet->socket_pet, contents[i].description, strlen(contents[i].description)) < 0){
                free(contents);
                close(pet->socket_pet);
                return;
            }
            //printf("%s %s", contents[i].file_path, contents[i].description);
        }
    }


    //printf("\n");

    close(pet->socket_pet);     // Cerramos el socket del cliente
}

int main(int argc, char const *argv[]){
    struct peticion pet;
    struct sockaddr_in server_addr, client_addr;
    // punteros para almacenar las IPs de  
    // las interfaces de la máquina
    struct ifaddrs *net_interfaces_addresses;  
    struct ifaddrs *temp_interface_ptr;  
    socklen_t socket_len;

    pthread_attr_t threads_attr;
    pthread_t threads[MAX_THREADS];

    int socket_servidor, socket_cliente;
    int val;
    int error;
    int pos = 0;  
    int port_tuplas;  

    char *end_pointer;  // necesario para str_to_long()
    char port_tuplas_str[MAX_MSG];
    char pet_str[MAX_MSG];

    if (argc != 3){
        // El servidor solo puede tener un argumento,
        // el cual debe ser el puerto que se usará
        perror("[ERROR] Argumentos inválidos: ./servidor -p <puerto> ");
        return -1;
    }

    strcpy(port_tuplas_str, argv[2]);
    // Convertir el primer argumento a double
    if (str_to_long(port_tuplas_str, &end_pointer, &port_tuplas) < 0){
        perror("[ERROR] Argumento puerto inválido, introduzca un entero entre 0 y 65535");
        return -1;
    }

    // Verificar que el puerto proporcionado está dentro del rango de puertos. 
    if (port_tuplas < 0 || port_tuplas > 65535){
        perror("[ERROR] Argumento puerto inválido, introduzca un entero entre 0 y 65535");
        return -1;
    }

    getifaddrs(&net_interfaces_addresses) < 0;
    temp_interface_ptr = net_interfaces_addresses;

    while (temp_interface_ptr){
        if (temp_interface_ptr->ifa_addr && temp_interface_ptr->ifa_addr->sa_family == AF_INET){
            struct sockaddr_in *pAddr = (struct sockaddr_in *)temp_interface_ptr->ifa_addr;
            if (strcmp(temp_interface_ptr->ifa_name, "lo") != 0){
                printf("init server %s:%s\n", inet_ntoa(pAddr->sin_addr), port_tuplas_str);
            }  
        }
        temp_interface_ptr = temp_interface_ptr->ifa_next;
    }

    freeifaddrs(net_interfaces_addresses);

    // Inicializar mutexes y variables condicionales
    pthread_mutex_init(&mutex,NULL);
    pthread_cond_init(&no_lleno,NULL);
    pthread_cond_init(&no_vacio,NULL);
    pthread_mutex_init(&mfin,NULL);
    pthread_mutex_init(&mutex_tuples,NULL);

    // Inicializar y crear pool de threads con sus atributos
    pthread_attr_init(&threads_attr);
    for (int i = 0; i < MAX_THREADS; i++){
        if (pthread_create(&threads[i], NULL, servicio, NULL) !=0){
            return -1;
        }
    }
        
    // Crear socket del servidor
    if ((socket_servidor = socket(AF_INET, SOCK_STREAM, 0))<0){
        printf ("SERVER: Error en el socket");
        return (0);
    }

    val = 1;
    error = setsockopt(socket_servidor, SOL_SOCKET, SO_REUSEADDR, (char *) &val, sizeof(int));

    if (error < 0) {
        perror("Error in opction");
        exit(1);
    }

    bzero((char *)&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port_tuplas);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // bindear la dirección y puerto al socket del servidor
    if (bind(socket_servidor,(struct sockaddr * restrict) &server_addr, sizeof(server_addr)) < 0){
        printf("Error en el bind\n");
        return(-1);
    }

    // Escuchar el socket del servidor
    listen(socket_servidor, SOMAXCONN);
    socket_len = sizeof(client_addr);

    for (;;){
        printf("Esperando una conexión\n");
        // Aceptar la conexión de un cliente
        socket_cliente = accept(socket_servidor, (struct sockaddr * restrict) &client_addr, &socket_len);
        
        if (socket_cliente < 0) {
            perror("Error en el accept");
            return(-1);
        }

        error = 0;
        printf("Atendiendo al cliente\n");
        // Atendiendo al cliente
        // receive mensaje por socket TCP
        error = read_to_pet(socket_cliente, &pet);

        // guardamos el socket del cliente para
        // pasarlo dentro de la petición y que 
        // el hilo que ejecute la petición tenga
        // acceso al socket para enviar la respuesta
        pet.socket_pet = socket_cliente;
        if (error == -1){
            perror("ERROR: formato de petición incorrecto \n");
            close(socket_cliente);
            break;
        }

        pthread_mutex_lock(&mutex);

        while (n_elementos == MAX_PETICIONES){
            pthread_cond_wait(&no_lleno, &mutex);
        }

        // Insertar una petición en el buffer circular
        // la cual será ejecutada por uno de los threads
        // consumidores
        buffer_peticiones[pos] = pet;
        pos = (pos+1) % MAX_PETICIONES;
        n_elementos++;
        pthread_cond_signal(&no_vacio);
        pthread_mutex_unlock(&mutex);
        
    }
    
    // Código a ejecutar al cerrar el servidor

    pthread_mutex_lock(&mfin);
    fin=true;
    pthread_mutex_unlock(&mfin);
    pthread_mutex_lock(&mutex);
    pthread_cond_broadcast(&no_vacio);
    pthread_mutex_unlock(&mutex);
    
    for (int i=0;i<MAX_THREADS;i++){
        pthread_join(threads[i],NULL);
    }
    
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&no_lleno);
    pthread_cond_destroy(&no_vacio);
    pthread_mutex_destroy(&mfin);

    close(socket_cliente);
    close(socket_servidor);
    
    return 0;
}