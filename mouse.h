#ifndef MOUSE_H
#define MOUSE_H

#include <stdint.h>

typedef struct {
    int x;
    int y;
    uint8_t buttons; // Bit 0: Left, Bit 1: Right, Bit 2: Middle
} mouse_state_t;

void mouse_init();
void mouse_wait(uint8_t type);
void mouse_write(uint8_t data);
uint8_t mouse_read();
void mouse_handle_interrupt();
mouse_state_t* mouse_get_state();

#endif
