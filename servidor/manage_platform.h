#ifndef MANAGE_PLATFORM_H
#define MANAGE_PLATFORM_H

int register_user(char* user);
int unregister_user(char* user);

int connect_user(char* user, int port);
int disconnect_user(char *user);
int publish(char* user, char* file_path, char* description);
int delete(char* user, char* file_path);

/// @param connected_users pasado por referencia para 
//                         devolver los usuarios conectados
int list_users(char* user, char* connected_users); 

/// @param contents_owner username del dueño de los contenidos
int list_content(char* user, char* contents_owner);

#endif