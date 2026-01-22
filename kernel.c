#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "limine.h"

// Set the base revision to 2, this is recommended as this is the latest
// base revision described by the Limine boot protocol specification.
// See specification for further info.
__attribute__((used, section(".requests")))
static volatile LIMINE_BASE_REVISION(2);

// The Limine requests can be placed anywhere, but it is important that
// the compiler does not optimize them away, so, usually, they should
// be made volatile or equivalent.

__attribute__((used, section(".requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};

// Halts the CPU
static void hcf(void) {
    asm("cli");
    for (;;) {
        asm("hlt");
    }
}

// The following will be our kernel's entry point.
// If renaming _start() to something else, make sure to change the
// linker script accordingly.
#include "graphics.h"
#include "keyboard.h"
#include "shell.h"
#include "memory.h"
#include "mouse.h"
#include "rtc.h"
#include "window.h"
#include "dock.h"
#include "auth.h"
#include "login.h"
#include "nano.h"

// ... (Keep headers and Limine requests) ...

// Use the exact same Limine requests as before

// The following will be our kernel's entry point.
void _start(void) {
    // Ensure we got a framebuffer.
    if (framebuffer_request.response == NULL
     || framebuffer_request.response->framebuffer_count < 1) {
        hcf();
    }

    // Fetch the first framebuffer.
    struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];

    // Initialize Graphics Support
    init_graphics(framebuffer);
    
    // Initialize Memory Manager
    memory_init();
    
    // Initialize Authentication System
    auth_init();
    login_init();
    
    // Initialize Mouse (needed for login screen)
    mouse_init();
    
    // Draw login screen background
    draw_desktop_background();
    
    // Login loop
    uint8_t prev_login_buttons = 0;
    while (!login_is_complete()) {
        login_render();
        
        // Poll keyboard for login input
        char c = keyboard_read_char();
        if (c != 0) {
            login_handle_key(c);
        }
        
        // Poll mouse for login clicks
        mouse_handle_interrupt();
        mouse_state_t* mouse = mouse_get_state();
        
        uint8_t buttons = mouse->buttons;
        if ((buttons & 1) && !(prev_login_buttons & 1)) {
            // Left button pressed
            login_handle_click(mouse->x, mouse->y);
        }
        prev_login_buttons = buttons;
        
        // Draw cursor
        draw_cursor(mouse->x, mouse->y);
        
        // Small delay
        for(volatile int i=0; i<10000; i++);
    }
    
    // Login successful - continue to desktop
    
    // Mouse already initialized above
    // mouse_init(); // Remove duplicate
    
    // Initialize Window Manager
    wm_init();
    dock_init();
    
    // Initialize Shell
    shell_init();

    // Draw UI (Background, TopBar, Dock)
    draw_desktop_background();
    
    rtc_time_t current_time;
    char time_buffer[16];
    rtc_read_time(&current_time);
    rtc_format_time(&current_time, time_buffer);
    draw_top_bar(time_buffer);
    
    dock_render();
    
    // Auto-launch Terminal window after login
    window_t* terminal_win = wm_create_window(150, 120, 500, 350, "Terminal", WINDOW_TERMINAL);
    if (terminal_win) {
        dock_set_app_running(2, true); // Terminal is icon index 2
    }
    
    // Initial dock render
    dock_render();
    
    int frame_counter = 0;
    uint8_t prev_buttons = 0;
    bool desktop_needs_redraw = false;

    while (1) {
        // Check if nano was requested from shell
        if (nano_requested) {
            nano_open(nano_requested_file[0] != '\0' ? nano_requested_file : NULL);
            wm_create_window(100, 80, 600, 400, "Nano Editor", WINDOW_NANO);
            nano_requested = false;
            desktop_needs_redraw = true;
        }
        
        // Poll Keyboard
        char c = keyboard_read_char();
        if (c != 0) {
            // Route keyboard input to nano if active, otherwise to shell
            if (nano_is_active()) {
                nano_handle_key(c);
            } else {
                shell_handle_key(c);
            }
        }
        
        // Poll Mouse
        mouse_handle_interrupt();
        mouse_state_t* mouse = mouse_get_state();
        
        // Handle mouse button events
        uint8_t buttons = mouse->buttons;
        if ((buttons & 1) && !(prev_buttons & 1)) {
            // Left button pressed
            wm_handle_mouse_down(mouse->x, mouse->y);
            dock_handle_click(mouse->x, mouse->y);
        } else if (!(buttons & 1) && (prev_buttons & 1)) {
            // Left button released
            wm_handle_mouse_up(mouse->x, mouse->y);
        }
        
        if (buttons & 1) {
            // Button held - handle dragging
            wm_handle_mouse_move(mouse->x, mouse->y);
        }
        
        prev_buttons = buttons;
        
        // Update dock magnification based on mouse position
        dock_update_magnification(mouse->x, mouse->y);
        
        // Update clock every ~1000 frames
        if (frame_counter++ > 1000) {
            frame_counter = 0;
            rtc_read_time(&current_time);
            rtc_format_time(&current_time, time_buffer);
            draw_top_bar(time_buffer);
        }
        
        // Only redraw desktop if needed
        if (desktop_needs_redraw) {
            draw_desktop_background();
            draw_top_bar(time_buffer);
            desktop_needs_redraw = false;
        }
        
        // Redraw dock every frame for smooth magnification
        dock_render();
        
        // Render all windows
        wm_render_all();
        
        // Render shell or nano in active window
        window_t* active_win = wm_get_active_window();
        if (active_win) {
            if (active_win->type == WINDOW_TERMINAL) {
                // Always render shell to prevent flickering
                shell_update(active_win->x, active_win->y, active_win->width, active_win->height);
            } else if (active_win->type == WINDOW_NANO && nano_needs_redraw()) {
                nano_render(active_win->x, active_win->y, active_win->width, active_win->height);
            }
        }
        
        // Draw Cursor on top
        draw_cursor(mouse->x, mouse->y);
        
        // Small delay
        for(volatile int i=0; i<10000; i++); 
    }
}

// Nano save handler - called when user presses Ctrl+S in nano
void Save_text_nano(window_t* nano_window) {
    // Declare variables
    static bool control_s_pressed = false;
    static bool leave_nano_flag = false;
    
    if (nano_window != NULL && nano_window->type == WINDOW_NANO) {
        // Nano save is handled by nano.c itself via Ctrl+O
        // This function is for additional save logic if needed
        
        // Check if Ctrl+S was pressed (handled by nano_handle_key)
        if (control_s_pressed) {
            // Save operation already handled by nano.c
            control_s_pressed = false;
        }
        
        // Check if user wants to exit
        if (leave_nano_flag) {
            nano_close();
            leave_nano_flag = false;
        }
    }
}
