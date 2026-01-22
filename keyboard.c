#include "io.h"
#include <stdint.h>

// Scancode Set 1 (US QWERTY)
static char scancode_map[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
  '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',   0,
   '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',   0, '*',
    0, ' ',   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
};

// Check if a key is waiting to be read
int keyboard_hit() {
    uint8_t status = inb(0x64);
    return (status & 1); // Bit 0 set means output buffer full
}

// Read the next character (blocking or non-blocking logic via polling)
// Returns 0 if no new key or non-printable scancode
char keyboard_read_char() {
    if (!keyboard_hit()) {
        return 0;
    }
    
    uint8_t scancode = inb(0x60);
    
    // Ignore key release (break codes have bit 7 set)
    if (scancode & 0x80) {
        return 0; 
    }
    
    // Only accept valid scancodes
    if (scancode >= 128) {
        return 0;
    }
    
    char c = scancode_map[scancode];
    
    // Only return printable characters, backspace, enter, tab
    if (c >= 32 && c < 127) {
        return c; // Printable
    } else if (c == '\n' || c == '\b' || c == '\t') {
        return c; // Special keys
    }
    
    return 0; // Ignore everything else
}
