#include "auth.h"
#include "memory.h"

// External string helpers
extern int strcmp(const char* s1, const char* s2);
extern void strcpy(char* dest, const char* src);
extern int strlen(const char* str);

static user_t users[MAX_USERS];
static int user_count = 0;
static char current_user[MAX_USERNAME];
static bool logged_in = false;

void auth_init() {
    user_count = 0;
    logged_in = false;
    current_user[0] = '\0';
    
    // Create default root user
    auth_create_user("root", "root");
}

bool auth_create_user(char* username, char* password) {
    if (user_count >= MAX_USERS) return false;
    if (strlen(username) == 0 || strlen(password) == 0) return false;
    
    // Check if user already exists
    for (int i = 0; i < user_count; i++) {
        if (strcmp(users[i].username, username) == 0) {
            return false; // User exists
        }
    }
    
    strcpy(users[user_count].username, username);
    strcpy(users[user_count].password, password);
    users[user_count].is_active = true;
    user_count++;
    return true;
}

bool auth_verify_login(char* username, char* password) {
    for (int i = 0; i < user_count; i++) {
        if (strcmp(users[i].username, username) == 0 &&
            strcmp(users[i].password, password) == 0 &&
            users[i].is_active) {
            strcpy(current_user, username);
            logged_in = true;
            return true;
        }
    }
    return false;
}

char* auth_get_current_user() {
    return current_user;
}

bool auth_is_logged_in() {
    return logged_in;
}

void auth_logout() {
    logged_in = false;
    current_user[0] = '\0';
}
