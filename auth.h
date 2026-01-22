#ifndef AUTH_H
#define AUTH_H

#include <stdbool.h>

#define MAX_USERNAME 32
#define MAX_PASSWORD 64
#define MAX_USERS 10

typedef struct {
    char username[MAX_USERNAME];
    char password[MAX_PASSWORD];
    bool is_active;
} user_t;

void auth_init();
bool auth_create_user(char* username, char* password);
bool auth_verify_login(char* username, char* password);
char* auth_get_current_user();
bool auth_is_logged_in();
void auth_logout();

#endif
