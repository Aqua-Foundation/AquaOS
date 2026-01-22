#ifndef LOGIN_H
#define LOGIN_H

#include <stdbool.h>

void login_init();
void login_render();
void login_handle_key(char c);
void login_handle_click(int x, int y);
bool login_is_complete();

#endif
