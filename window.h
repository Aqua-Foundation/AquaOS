#ifndef WINDOW_H
#define WINDOW_H

#include <stdint.h>
#include <stdbool.h>

#define MAX_WINDOWS 10
#define WINDOW_TITLE_HEIGHT 30

typedef enum {
    WINDOW_TERMINAL,
    WINDOW_FILE_BROWSER,
    WINDOW_ABOUT,
    WINDOW_NANO
} window_type_t;

typedef enum {
    RESIZE_NONE,
    RESIZE_RIGHT,
    RESIZE_BOTTOM,
    RESIZE_BOTTOM_RIGHT
} resize_mode_t;

typedef struct window {
    int x, y;
    int width, height;
    char title[64];
    window_type_t type;
    bool is_active;
    bool is_dragging;
    bool is_resizing;
    resize_mode_t resize_mode;
    int drag_offset_x, drag_offset_y;
    int resize_start_width, resize_start_height;
} window_t;

void wm_init();
window_t* wm_create_window(int x, int y, int width, int height, char* title, window_type_t type);
void wm_render_all();
void wm_handle_mouse_down(int x, int y);
void wm_handle_mouse_up(int x, int y);
void wm_handle_mouse_move(int x, int y);
window_t* wm_get_active_window();

#endif
