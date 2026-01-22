#include "dock.h"
#include "graphics.h"
#include "window.h"

#define DOCK_WIDTH 400
#define DOCK_HEIGHT 90
#define BASE_ICON_SIZE 48
#define MAX_ICON_SIZE 72
#define ICON_GAP 16
#define NUM_ICONS 4
#define MAGNIFY_RANGE 80

static int dock_x, dock_y;
static dock_icon_t icons[NUM_ICONS];

extern struct limine_framebuffer* fb; // Access from graphics.c

// Simple square root approximation for distance calculation
static int isqrt(int n) {
    if (n < 2) return n;
    int x = n;
    int y = (x + 1) / 2;
    while (y < x) {
        x = y;
        y = (x + n / x) / 2;
    }
    return x;
}

void dock_init() {
    // Initialize icon positions and properties
    icons[0].color = 0xFF1E90FF;  // Finder (Blue)
    icons[0].label = "Finder";
    icons[0].is_running = false;
    
    icons[1].color = 0xFF87CEEB;  // Safari (Light Blue)
    icons[1].label = "Safari";
    icons[1].is_running = false;
    
    icons[2].color = 0xFF2C2C2C;  // Terminal (Dark Gray)
    icons[2].label = "Terminal";
    icons[2].is_running = false;
    
    icons[3].color = 0xFF9E9E9E;  // Settings (Gray)
    icons[3].label = "Settings";
    icons[3].is_running = false;
    
    // Set base sizes
    for (int i = 0; i < NUM_ICONS; i++) {
        icons[i].current_size = BASE_ICON_SIZE;
    }
}

void dock_update_magnification(int mouse_x, int mouse_y) {
    // Calculate dock position
    dock_x = (800 - DOCK_WIDTH) / 2;
    dock_y = 600 - DOCK_HEIGHT - 10;
    
    int start_x = dock_x + 24;
    int icon_y = dock_y + 20;
    
    // Check if mouse is near dock
    bool near_dock = (mouse_y >= dock_y - 20 && mouse_y <= dock_y + DOCK_HEIGHT);
    
    for (int i = 0; i < NUM_ICONS; i++) {
        int icon_x = start_x + i * (BASE_ICON_SIZE + ICON_GAP);
        int icon_center_x = icon_x + BASE_ICON_SIZE / 2;
        int icon_center_y = icon_y + BASE_ICON_SIZE / 2;
        
        icons[i].base_x = icon_x;
        icons[i].base_y = icon_y;
        
        if (near_dock) {
            // Calculate distance from mouse to icon center
            int dx = mouse_x - icon_center_x;
            int dy = mouse_y - icon_center_y;
            int distance = isqrt(dx * dx + dy * dy);
            
            if (distance < MAGNIFY_RANGE) {
                // Apply magnification based on distance
                int magnify = ((MAGNIFY_RANGE - distance) * (MAX_ICON_SIZE - BASE_ICON_SIZE)) / MAGNIFY_RANGE;
                icons[i].current_size = BASE_ICON_SIZE + magnify;
            } else {
                // Smooth return to base size
                if (icons[i].current_size > BASE_ICON_SIZE) {
                    icons[i].current_size -= 2;
                }
            }
        } else {
            // Return to base size when mouse is away
            if (icons[i].current_size > BASE_ICON_SIZE) {
                icons[i].current_size -= 2;
            }
        }
        
        // Clamp size
        if (icons[i].current_size < BASE_ICON_SIZE) {
            icons[i].current_size = BASE_ICON_SIZE;
        }
        if (icons[i].current_size > MAX_ICON_SIZE) {
            icons[i].current_size = MAX_ICON_SIZE;
        }
    }
}

void dock_render() {
    // Calculate dock position
    dock_x = (800 - DOCK_WIDTH) / 2;
    dock_y = 600 - DOCK_HEIGHT - 10;
    
    // Dock background with subtle transparency effect
    draw_rect(dock_x, dock_y, DOCK_WIDTH, DOCK_HEIGHT, 0xFFE5E5E5);
    
    // Subtle top border
    draw_rect(dock_x, dock_y, DOCK_WIDTH, 1, 0xFFCCCCCC);
    
    // Render icons
    for (int i = 0; i < NUM_ICONS; i++) {
        int size = icons[i].current_size;
        
        // Center the icon based on its current size
        int offset = (MAX_ICON_SIZE - size) / 2;
        int x = icons[i].base_x - (size - BASE_ICON_SIZE) / 2;
        int y = icons[i].base_y - (size - BASE_ICON_SIZE) + offset;
        
        // Draw icon shadow for depth
        draw_rect(x + 2, y + 2, size, size, 0x33000000);
        
        // Draw icon
        draw_rect(x, y, size, size, icons[i].color);
        
        // Draw running indicator (small dot below icon)
        if (icons[i].is_running) {
            int dot_x = icons[i].base_x + BASE_ICON_SIZE / 2 - 2;
            int dot_y = dock_y + DOCK_HEIGHT - 8;
            draw_rect(dot_x, dot_y, 4, 4, 0xFF4A4A4A);
        }
    }
}

void dock_handle_click(int x, int y) {
    // Check if click is within dock area
    if (y < dock_y || y > dock_y + DOCK_HEIGHT) {
        return;
    }
    
    // Check each icon
    for (int i = 0; i < NUM_ICONS; i++) {
        int size = icons[i].current_size;
        int icon_x = icons[i].base_x - (size - BASE_ICON_SIZE) / 2;
        int icon_y = icons[i].base_y - (size - BASE_ICON_SIZE);
        
        if (x >= icon_x && x < icon_x + size &&
            y >= icon_y && y < icon_y + size) {
            
            // Handle click based on icon
            switch (i) {
                case 0: // Finder
                    wm_create_window(200, 100, 450, 380, "Finder", WINDOW_FILE_BROWSER);
                    icons[i].is_running = true;
                    break;
                case 1: // Safari
                    wm_create_window(180, 120, 500, 400, "Safari", WINDOW_FILE_BROWSER);
                    icons[i].is_running = true;
                    break;
                case 2: // Terminal
                    wm_create_window(120, 150, 500, 320, "Terminal", WINDOW_TERMINAL);
                    icons[i].is_running = true;
                    break;
                case 3: // Settings
                    wm_create_window(250, 200, 380, 250, "About AquaOS", WINDOW_ABOUT);
                    icons[i].is_running = true;
                    break;
            }
            return;
        }
    }
}

void dock_set_app_running(int icon_index, bool running) {
    if (icon_index >= 0 && icon_index < NUM_ICONS) {
        icons[icon_index].is_running = running;
    }
}
