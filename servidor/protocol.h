#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "../comm_sock.h"

#define MAX_MSG 2048

int str_to_long(char *long_str, char **endptr, int *resultado);

int read_to_pet(int socket, struct peticion *pet);

#endif