#ifndef DOCK_H
#define DOCK_H

#include <stdint.h>
#include <stdbool.h>

// Dock icon structure
typedef struct {
    int base_x;
    int base_y;
    int current_size;
    bool is_running;
    uint32_t color;
    char* label;
} dock_icon_t;

void dock_init();
void dock_render();
void dock_update_magnification(int mouse_x, int mouse_y);
void dock_handle_click(int x, int y);
void dock_set_app_running(int icon_index, bool running);

#endif
