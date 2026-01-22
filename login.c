#include "login.h"
#include "graphics.h"
#include "auth.h"

// External string helpers
extern int strcmp(const char* s1, const char* s2);
extern void strcpy(char* dest, const char* src);
extern int strlen(const char* str);

#define MAX_INPUT 64

static char username_input[MAX_INPUT];
static char password_input[MAX_INPUT];
static int username_len = 0;
static int password_len = 0;
static bool on_password_field = false;
static bool login_complete = false;
static char error_message[128];

void login_init() {
    username_len = 0;
    password_len = 0;
    on_password_field = false;
    login_complete = false;
    username_input[0] = '\0';
    password_input[0] = '\0';
    error_message[0] = '\0';
}

void login_render() {
    // Center coordinates
    int center_x = 400;
    int center_y = 300;
    
    // User avatar circle (simplified as square)
    draw_rect(center_x - 40, center_y - 120, 80, 80, COLOR_APPLE_BLUE);
    
    // Username label
    draw_string(center_x - 100, center_y - 20, "Username:", COLOR_TEXT_PRIMARY);
    
    // Username input box
    draw_rect(center_x - 100, center_y, 200, 32, COLOR_WHITE);
    draw_rect(center_x - 100, center_y, 200, 1, COLOR_DOCK_BORDER); // Top border
    draw_rect(center_x - 100, center_y + 31, 200, 1, COLOR_DOCK_BORDER); // Bottom border
    
    if (!on_password_field) {
        // Active field indicator
        draw_rect(center_x - 100, center_y + 31, 200, 2, COLOR_APPLE_BLUE);
    }
    
    draw_string(center_x - 90, center_y + 10, username_input, COLOR_TEXT_PRIMARY);
    
    // Password label
    draw_string(center_x - 100, center_y + 50, "Password:", COLOR_TEXT_PRIMARY);
    
    // Password input box
    draw_rect(center_x - 100, center_y + 70, 200, 32, COLOR_WHITE);
    draw_rect(center_x - 100, center_y + 70, 200, 1, COLOR_DOCK_BORDER);
    draw_rect(center_x - 100, center_y + 101, 200, 1, COLOR_DOCK_BORDER);
    
    if (on_password_field) {
        // Active field indicator
        draw_rect(center_x - 100, center_y + 101, 200, 2, COLOR_APPLE_BLUE);
    }
    
    // Show password as asterisks
    char masked[MAX_INPUT];
    for (int i = 0; i < password_len && i < MAX_INPUT - 1; i++) {
        masked[i] = '*';
    }
    masked[password_len] = '\0';
    draw_string(center_x - 90, center_y + 80, masked, COLOR_TEXT_PRIMARY);
    
    // Login button
    draw_rect(center_x - 50, center_y + 120, 100, 36, COLOR_APPLE_BLUE);
    draw_string(center_x - 20, center_y + 130, "Login", COLOR_WHITE);
    
    // Error message
    if (error_message[0] != '\0') {
        draw_string(center_x - 80, center_y + 170, error_message, 0xFFFF3B30); // Red
    }
    
    // AquaOS branding
    draw_string(center_x - 40, center_y - 200, "AquaOS", COLOR_TEXT_PRIMARY);
}

void login_handle_key(char c) {
    if (c == '\n') {
        // Enter key
        if (!on_password_field) {
            // Move to password field
            on_password_field = true;
        } else {
            // Attempt login
            if (auth_verify_login(username_input, password_input)) {
                login_complete = true;
            } else {
                strcpy(error_message, "Invalid username or password");
                password_len = 0;
                password_input[0] = '\0';
            }
        }
    } else if (c == '\b') {
        // Backspace
        if (!on_password_field && username_len > 0) {
            username_len--;
            username_input[username_len] = '\0';
        } else if (on_password_field && password_len > 0) {
            password_len--;
            password_input[password_len] = '\0';
        }
    } else if (c == '\t') {
        // Tab to switch fields
        on_password_field = !on_password_field;
    } else if (c >= 32 && c < 127) {
        // Printable character
        if (!on_password_field && username_len < MAX_INPUT - 1) {
            username_input[username_len] = c;
            username_len++;
            username_input[username_len] = '\0';
        } else if (on_password_field && password_len < MAX_INPUT - 1) {
            password_input[password_len] = c;
            password_len++;
            password_input[password_len] = '\0';
        }
    }
}

void login_handle_click(int x, int y) {
    int center_x = 400;
    int center_y = 300;
    
    // Check if clicking on username field
    if (x >= center_x - 100 && x < center_x + 100 &&
        y >= center_y && y < center_y + 32) {
        on_password_field = false;
        return;
    }
    
    // Check if clicking on password field
    if (x >= center_x - 100 && x < center_x + 100 &&
        y >= center_y + 70 && y < center_y + 102) {
        on_password_field = true;
        return;
    }
    
    // Check if clicking on login button
    if (x >= center_x - 50 && x < center_x + 50 &&
        y >= center_y + 120 && y < center_y + 156) {
        // Attempt login
        if (auth_verify_login(username_input, password_input)) {
            login_complete = true;
        } else {
            strcpy(error_message, "Invalid username or password");
            password_len = 0;
            password_input[0] = '\0';
        }
    }
}

bool login_is_complete() {
    return login_complete;
}
