#include "mouse.h"
#include "io.h"

static mouse_state_t mouse_state;
static uint8_t mouse_cycle = 0;
static int8_t mouse_byte[3];

void mouse_wait(uint8_t type) {
    uint32_t timeout = 100000;
    if (type == 0) {
        // Wait for output buffer
        while (timeout--) {
            if ((inb(0x64) & 1) == 1) return;
        }
    } else {
        // Wait for input buffer
        while (timeout--) {
            if ((inb(0x64) & 2) == 0) return;
        }
    }
}

void mouse_write(uint8_t data) {
    mouse_wait(1);
    outb(0x64, 0xD4);
    mouse_wait(1);
    outb(0x60, data);
}

uint8_t mouse_read() {
    mouse_wait(0);
    return inb(0x60);
}

void mouse_init() {
    mouse_state.x = 400;
    mouse_state.y = 300;
    mouse_state.buttons = 0;
    
    // Enable auxiliary mouse device
    mouse_wait(1);
    outb(0x64, 0xA8);
    
    // Enable interrupts
    mouse_wait(1);
    outb(0x64, 0x20);
    uint8_t status = mouse_read() | 2;
    mouse_wait(1);
    outb(0x64, 0x60);
    mouse_wait(1);
    outb(0x60, status);
    
    // Use default settings
    mouse_write(0xF6);
    mouse_read();
    
    // Enable packet streaming
    mouse_write(0xF4);
    mouse_read();
}

void mouse_handle_interrupt() {
    uint8_t status = inb(0x64);
    if (!(status & 0x20)) return; // Not mouse data
    
    int8_t data = inb(0x60);
    
    switch (mouse_cycle) {
        case 0:
            mouse_byte[0] = data;
            if (mouse_byte[0] & 0x08) mouse_cycle++; // Valid packet
            break;
        case 1:
            mouse_byte[1] = data;
            mouse_cycle++;
            break;
        case 2:
            mouse_byte[2] = data;
            mouse_cycle = 0;
            
            // Process packet
            mouse_state.buttons = mouse_byte[0] & 0x07;
            
            int dx = mouse_byte[1];
            int dy = mouse_byte[2];
            
            // Handle sign extension
            if (mouse_byte[0] & 0x10) dx -= 256;
            if (mouse_byte[0] & 0x20) dy -= 256;
            
            mouse_state.x += dx;
            mouse_state.y -= dy; // Invert Y
            
            // Clamp to screen (assuming 800x600)
            if (mouse_state.x < 0) mouse_state.x = 0;
            if (mouse_state.x > 799) mouse_state.x = 799;
            if (mouse_state.y < 0) mouse_state.y = 0;
            if (mouse_state.y > 599) mouse_state.y = 599;
            break;
    }
}

mouse_state_t* mouse_get_state() {
    return &mouse_state;
}
