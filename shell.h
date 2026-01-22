#ifndef SHELL_H
#define SHELL_H

#include <stdbool.h>

// Nano request globals
extern char nano_requested_file[256];
extern bool nano_requested;

void shell_init();
void shell_handle_key(char c);
void shell_update(int win_x, int win_y, int win_w, int win_h);
bool shell_needs_redraw();
void shell_set_dirty();

#endif
