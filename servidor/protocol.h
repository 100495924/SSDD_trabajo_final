#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "comm_sock.h"

#define MAX_MSG 2048

int read_to_pet(int socket, struct peticion *pet);

#endif