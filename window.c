#include "window.h"
#include "graphics.h"
#include "memory.h"
#include "shell.h"

static window_t* windows[MAX_WINDOWS];
static int window_count = 0;
static window_t* active_window = NULL;

// External string helpers
extern int strcmp(const char* s1, const char* s2);
extern void strcpy(char* dest, const char* src);

void wm_init() {
    window_count = 0;
    active_window = NULL;
}

window_t* wm_create_window(int x, int y, int width, int height, char* title, window_type_t type) {
    if (window_count >= MAX_WINDOWS) return NULL;
    
    window_t* win = (window_t*)malloc(sizeof(window_t));
    win->x = x;
    win->y = y;
    win->width = width;
    win->height = height;
    strcpy(win->title, title);
    win->type = type;
    win->is_active = true;
    win->is_dragging = false;
    win->is_resizing = false;
    win->resize_mode = RESIZE_NONE;
    
    windows[window_count++] = win;
    active_window = win;
    return win;
}

void wm_render_window(window_t* win) {
    if (!win || !win->is_active) return;
    
    // Use the refined draw_window function
    draw_window(win->x, win->y, win->width, win->height, win->title);
    
    // Content area based on type (render inside the window content area)
    int content_x = win->x + 1;
    int content_y = win->y + 28; // After title bar
    int content_w = win->width - 2;
    int content_h = win->height - 29;
    
    if (win->type == WINDOW_TERMINAL) {
        // Draw black background ONCE - it stays static
        draw_rect(content_x + 1, content_y + 1, content_w - 2, content_h - 2, 0xFF000000);
    } else if (win->type == WINDOW_NANO) {
        // Nano background is drawn by nano_render - don't draw here
    } else if (win->type == WINDOW_FILE_BROWSER) {
        // Simple file browser placeholder
        draw_rect(content_x + 8, content_y + 8, content_w - 16, content_h - 16, 0xFFFFFFFF);
        draw_string(content_x + 16, content_y + 16, "Files:", COLOR_TEXT_PRIMARY);
    } else if (win->type == WINDOW_ABOUT) {
        // About window
        draw_rect(content_x + 8, content_y + 8, content_w - 16, content_h - 16, 0xFFFFFFFF);
        draw_string(content_x + 16, content_y + 16, "AquaOS v1.0", COLOR_TEXT_PRIMARY);
        draw_string(content_x + 16, content_y + 32, "Professional macOS-like OS", COLOR_TEXT_SECONDARY);
    }
}

void wm_render_all() {
    for (int i = 0; i < window_count; i++) {
        wm_render_window(windows[i]);
    }
}

bool point_in_rect(int px, int py, int rx, int ry, int rw, int rh) {
    return px >= rx && px < rx + rw && py >= ry && py < ry + rh;
}

void wm_handle_mouse_down(int x, int y) {
    #define RESIZE_EDGE_SIZE 8
    
    // Check if clicking on any window (reverse order for z-index)
    for (int i = window_count - 1; i >= 0; i--) {
        window_t* win = windows[i];
        if (!win->is_active) continue;
        
        // Check for resize on edges/corners
        bool on_right_edge = (x >= win->x + win->width - RESIZE_EDGE_SIZE && x <= win->x + win->width);
        bool on_bottom_edge = (y >= win->y + win->height - RESIZE_EDGE_SIZE && y <= win->y + win->height);
        bool in_window = point_in_rect(x, y, win->x, win->y, win->width, win->height);
        
        if (in_window && (on_right_edge || on_bottom_edge)) {
            // Start resizing
            win->is_resizing = true;
            win->resize_start_width = win->width;
            win->resize_start_height = win->height;
            win->drag_offset_x = x;
            win->drag_offset_y = y;
            
            if (on_right_edge && on_bottom_edge) {
                win->resize_mode = RESIZE_BOTTOM_RIGHT;
            } else if (on_right_edge) {
                win->resize_mode = RESIZE_RIGHT;
            } else {
                win->resize_mode = RESIZE_BOTTOM;
            }
            
            active_window = win;
            return;
        }
        
        // Check title bar for dragging
        if (point_in_rect(x, y, win->x, win->y, win->width, WINDOW_TITLE_HEIGHT)) {
            // Start dragging
            win->is_dragging = true;
            win->drag_offset_x = x - win->x;
            win->drag_offset_y = y - win->y;
            active_window = win;
            return;
        }
    }
}

void wm_handle_mouse_up(int x, int y) {
    (void)x; (void)y; // Unused
    for (int i = 0; i < window_count; i++) {
        windows[i]->is_dragging = false;
        windows[i]->is_resizing = false;
        windows[i]->resize_mode = RESIZE_NONE;
    }
}

void wm_handle_mouse_move(int x, int y) {
    #define MIN_WINDOW_WIDTH 200
    #define MIN_WINDOW_HEIGHT 150
    
    for (int i = 0; i < window_count; i++) {
        window_t* win = windows[i];
        
        if (win->is_dragging) {
            win->x = x - win->drag_offset_x;
            win->y = y - win->drag_offset_y;
        }
        
        if (win->is_resizing) {
            int dx = x - win->drag_offset_x;
            int dy = y - win->drag_offset_y;
            
            if (win->resize_mode == RESIZE_RIGHT || win->resize_mode == RESIZE_BOTTOM_RIGHT) {
                win->width = win->resize_start_width + dx;
                if (win->width < MIN_WINDOW_WIDTH) win->width = MIN_WINDOW_WIDTH;
            }
            
            if (win->resize_mode == RESIZE_BOTTOM || win->resize_mode == RESIZE_BOTTOM_RIGHT) {
                win->height = win->resize_start_height + dy;
                if (win->height < MIN_WINDOW_HEIGHT) win->height = MIN_WINDOW_HEIGHT;
            }
        }
    }
}

window_t* wm_get_active_window() {
    return active_window;
}
