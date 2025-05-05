#ifndef MANAGE_PLATFORM_H
#define MANAGE_PLATFORM_H

#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>

int is_file_in_directory(char *filepath, char *user);

int register_user(char* user);
int unregister_user(char* user);

int connect_user(char* user, char* ip_user, int port_user);
int disconnect_user(char *user);
int publish(char* user, char* file_path, char* description);
int delete(char* user, char* file_path);

/// @param connected_users pasado por referencia para 
//                         devolver los usuarios conectados
int list_users(char* user, char* connected_users); 

/// @param contents_owner username del dueño de los contenidos
int list_content(char* user, char* contents_owner);

#endif