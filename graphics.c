#include "graphics.h"

static struct limine_framebuffer *fb = NULL;

void init_graphics(struct limine_framebuffer *framebuffer) {
    fb = framebuffer;
}

void put_pixel(int x, int y, uint32_t color) {
    if (!fb) return;
    if (x < 0 || x >= (int)fb->width || y < 0 || y >= (int)fb->height) return;

    // Assuming 32-bit color (4 bytes per pixel)
    uint32_t *fb_ptr = (uint32_t*)fb->address;
    size_t index = y * (fb->pitch / 4) + x;
    fb_ptr[index] = color;
}

#include "font.h"

void draw_rect(int x, int y, int width, int height, uint32_t color) {
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            put_pixel(x + j, y + i, color);
        }
    }
}

void draw_char(int x, int y, char c, uint32_t color) {
    if (!fb) return;
    int idx = (unsigned char)c;
    if (idx > 127) idx = 127;

    for (int row = 0; row < 8; row++) {
        uint8_t line = font8x8_basic[idx][row];
        for (int col = 0; col < 8; col++) {
            // Check if bit (7-col) is set
            if ((line >> (7 - col)) & 1) {
                // Scale by 1 (or 2 for bigger font)
                put_pixel(x + col, y + row, color);
            }
        }
    }
}

void draw_string(int x, int y, char* str, uint32_t color) {
    int cur_x = x;
    while (*str) {
        draw_char(cur_x, y, *str, color);
        cur_x += 8; // Move 8 pixels to the right
        str++;
    }
}

// Draw a macOS Big Sur-style gradient background
void draw_desktop_background() {
    if (!fb) return;
    uint32_t *fb_ptr = (uint32_t*)fb->address;
    
    for (size_t y = 0; y < fb->height; y++) {
        for (size_t x = 0; x < fb->width; x++) {
            // Big Sur gradient: Soft pink/purple at top, deep blue/purple at bottom
            // Using integer math for smooth transitions
            uint32_t ny_val = (y * 255) / fb->height;
            uint32_t nx_val = (x * 255) / fb->width;
            
            // Top: Soft pink (255, 200, 220) -> Bottom: Deep purple/blue (100, 100, 200)
            uint8_t r = (uint8_t)(255 - (ny_val * 155) / 255 + (nx_val * 20) / 255);
            uint8_t g = (uint8_t)(200 - (ny_val * 100) / 255);
            uint8_t b = (uint8_t)(220 - (ny_val * 20) / 255 + (nx_val * 35) / 255);
            
            size_t index = y * (fb->pitch / 4) + x;
            fb_ptr[index] = (0xFF << 24) | (r << 16) | (g << 8) | b;
        }
    }
}

void draw_top_bar(char* time_str) {
    if (!fb) return;
    // Subtle light gray background
    draw_rect(0, 0, fb->width, 28, COLOR_TOPBAR);
    
    // Apple logo placeholder (refined black square)
    draw_rect(16, 6, 16, 16, COLOR_BLACK);
    
    // Draw time on the right side with proper spacing
    if (time_str) {
        int time_x = fb->width - 70;
        draw_string(time_x, 8, time_str, COLOR_TOPBAR_TEXT);
    }
}

void draw_dock() {
    if (!fb) return;
    int dock_width = 400;
    int dock_height = 70;
    int dock_x = (fb->width - dock_width) / 2;
    int dock_y = fb->height - dock_height - 16;

    // Refined dock background with subtle transparency simulation
    draw_rect(dock_x, dock_y, dock_width, dock_height, COLOR_DOCK_BG);
    
    // Subtle border
    draw_rect(dock_x, dock_y, dock_width, 1, COLOR_DOCK_BORDER);

    // Icons with proper spacing (8px grid)
    int icon_size = 48;
    int gap = 16;
    int start_x = dock_x + 24;
    int start_y = dock_y + 11;

    // Finder (Blue)
    draw_rect(start_x, start_y, icon_size, icon_size, COLOR_FINDER);
    
    // Safari/Browser (Light Blue)
    draw_rect(start_x + icon_size + gap, start_y, icon_size, icon_size, COLOR_SAFARI);
    
    // Terminal (Black)
    draw_rect(start_x + (icon_size + gap) * 2, start_y, icon_size, icon_size, COLOR_TERMINAL);
    
    // Settings (Gray)
    draw_rect(start_x + (icon_size + gap) * 3, start_y, icon_size, icon_size, COLOR_SETTINGS);
}

void draw_window(int x, int y, int width, int height, char* title) {
    // Multi-layer shadow for depth (Apple-style)
    // Ambient shadow (larger, softer)
    draw_rect(x - 4, y - 4, width + 8, height + 8, COLOR_SHADOW_AMBIENT);
    // Direct shadow (smaller, sharper)
    draw_rect(x - 2, y - 2, width + 4, height + 4, COLOR_SHADOW_DIRECT);

    // Main Window Body (pure white)
    draw_rect(x, y, width, height, COLOR_WINDOW_BG);

    // Title Bar (subtle gray)
    int title_bar_height = 28;
    draw_rect(x, y, width, title_bar_height, COLOR_WINDOW_TITLE);

    // Traffic Lights with proper macOS spacing and colors
    int btn_y = y + 7;
    int btn_size = 12;
    draw_rect(x + 12, btn_y, btn_size, btn_size, 0xFFFF5F56); // Close (Red)
    draw_rect(x + 32, btn_y, btn_size, btn_size, 0xFFFFBD2E); // Minimize (Yellow)
    draw_rect(x + 52, btn_y, btn_size, btn_size, 0xFF27C93F); // Maximize (Green)

    // Title text (centered in title bar)
    if (title) {
        draw_string(x + 80, y + 8, title, COLOR_TEXT_PRIMARY);
    }

    // Window Content Placeholder (subtle background)
    draw_rect(x + 1, y + title_bar_height, width - 2, height - title_bar_height - 1, 0xFFFAFAFA);
}

void draw_cursor(int x, int y) {
    // Simple arrow cursor (10x16 pixels)
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 10 - (i / 2); j++) {
            if (j < 2 || i < 2) {
                put_pixel(x + j, y + i, COLOR_WHITE);
            } else {
                put_pixel(x + j, y + i, COLOR_BLACK);
            }
        }
    }
}
