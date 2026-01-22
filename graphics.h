#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <stdint.h>
#include <stddef.h>
#include "limine.h"

// macOS Big Sur/Monterey Color Palette
#define COLOR_WHITE 0xFFFFFFFF
#define COLOR_BLACK 0xFF000000
#define COLOR_TOPBAR 0xFFF5F5F7 // Light gray with subtle warmth
#define COLOR_TOPBAR_TEXT 0xFF1D1D1F // Near black for contrast
#define COLOR_DOCK_BG 0xE6FFFFFF // Semi-transparent white (90% opacity simulation)
#define COLOR_DOCK_BORDER 0xFFE5E5E5
#define COLOR_APPLE_BLUE 0xFF007AFF // macOS accent blue
#define COLOR_FINDER 0xFF3B99FC // Finder blue
#define COLOR_SAFARI 0xFF00C2FF // Safari blue
#define COLOR_TERMINAL 0xFF000000 // Terminal black
#define COLOR_SETTINGS 0xFF8E8E93 // Settings gray
#define COLOR_WINDOW_BG 0xFFFFFFFF // Pure white
#define COLOR_WINDOW_TITLE 0xFFFAFAFA // Subtle gray for title bar
#define COLOR_SHADOW_AMBIENT 0x40000000 // Soft ambient shadow
#define COLOR_SHADOW_DIRECT 0x60000000 // Stronger direct shadow
#define COLOR_TEXT_PRIMARY 0xFF1D1D1F
#define COLOR_TEXT_SECONDARY 0xFF8E8E93

void init_graphics(struct limine_framebuffer *fb);
void put_pixel(int x, int y, uint32_t color);
void draw_rect(int x, int y, int width, int height, uint32_t color);
void draw_char(int x, int y, char c, uint32_t color);
void draw_string(int x, int y, char* str, uint32_t color);
void draw_cursor(int x, int y);
void draw_desktop_background();
void draw_top_bar(char* time_str);
void draw_dock();
void draw_window(int x, int y, int width, int height, char* title);

#endif
