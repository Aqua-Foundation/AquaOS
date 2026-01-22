#ifndef NANO_H
#define NANO_H

#include <stdbool.h>

void nano_init();
void nano_open(char* filename);
void nano_handle_key(char c);
void nano_render(int win_x, int win_y, int win_w, int win_h);
bool nano_needs_redraw();
bool nano_is_active();
void nano_close();

#endif
