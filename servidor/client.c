#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include "lines.h"

#define MAX_LINE 	256

int main(int argc, char *argv[])
{
    int sd;
    struct sockaddr_in server_addr;
    struct hostent *hp;

    if (argc != 3) {
        printf("Usage: client <serverAddress> <serverPort>\n");
        exit(0);
    }

	// the socket is create
    sd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sd < 0){
        perror("Error in socket");
        exit(1);
    }

	// Obtain Server address 
    bzero((char *)&server_addr, sizeof(server_addr));
    hp = gethostbyname (argv[1]);
    if (hp == NULL) {
        perror("Error en gethostbyname");
        exit(1);
    }// Obtain Server address 
    bzero((char *)&server_addr, sizeof(server_addr));
    hp = gethostbyname (argv[1]);
    if (hp == NULL) {
        perror("Error en gethostbyname");
        exit(1);
    }

	memcpy (&(server_addr.sin_addr), hp->h_addr, hp->h_length);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2]));

    // se establece la conexión
    if (connect(sd, (struct sockaddr * restrict) &server_addr, sizeof(server_addr)) < 0) {
        printf("Error en la conexión\n");
        return(0);
    }

    char *buffer_envio = "REGISTER";
    char *buffer_envio2 = "HOLAA";
    char buffer_recibido[MAX_LINE];
	int error = 0;
	int n;
    char *exit_str = "EXIT";

    //listen(sd, SOMAXCONN);

    /* n = readLine(0, buffer_envio, MAX_LINE);
    if (n == -1){
        return -1;
    } */

    printf("Enviando mensaje al servidor\n");
    sendMessage(sd, buffer_envio, strlen(buffer_envio)+1);
    sendMessage(sd, buffer_envio2, strlen(buffer_envio2)+1);

    /* //n = readLine(sd, buffer_recibido, MAX_LINE);
    n = recvMessage(sd, buffer_recibido, n+1);
    if (n == -1){
        return -1;
    }

    printf("Recibido: %s\n", buffer_recibido);

    if (strcmp(buffer_recibido, exit_str) == 0){
        close(sd);
        return 0;   
    } */

    close (sd);
    return(0);
}

