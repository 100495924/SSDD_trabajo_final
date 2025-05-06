#ifndef MANAGE_PLATFORM_H
#define MANAGE_PLATFORM_H

#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>

char *dirname_registered = "registered_users";
char *dirname_active = "active_users";

struct ListUserInfo {
    char user[256];
    char ip_user[16];
    int port_user;
};

typedef struct ListContentInfo{
    char file_path[256];
    char description[256];
}ListContentInfo;

int is_file_in_directory(char *filepath, char *user);
int check_user_registered(char *user);
int check_user_connected(char *user);

int register_user(char* user);
int unregister_user(char* user);

int connect_user(char* user, char* ip_user, int port_user);
int disconnect_user(char *user);
int publish(char* user, char* file_path, char* description);
int delete(char* user, char* file_path);

/// @param connected_users pasado por referencia para 
//                         devolver los usuarios conectados
int list_users_check(char* user); 
int list_users_get_num(long* count);
int list_users_get_info(struct ListUserInfo *info);

/// @param contents_owner username del dueño de los contenidos
int list_content(char* user, char* contents_owner, struct ListContentInfo* contents);

#endif