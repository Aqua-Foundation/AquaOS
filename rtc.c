#include "rtc.h"
#include "io.h"

#define CMOS_ADDRESS 0x70
#define CMOS_DATA    0x71

static uint8_t rtc_read_register(uint8_t reg) {
    outb(CMOS_ADDRESS, reg);
    return inb(CMOS_DATA);
}

static uint8_t bcd_to_binary(uint8_t bcd) {
    return ((bcd / 16) * 10) + (bcd & 0x0F);
}

void rtc_read_time(rtc_time_t* time) {
    // Read RTC registers
    time->second = rtc_read_register(0x00);
    time->minute = rtc_read_register(0x02);
    time->hour = rtc_read_register(0x04);
    time->day = rtc_read_register(0x07);
    time->month = rtc_read_register(0x08);
    time->year = rtc_read_register(0x09);
    
    // Check if BCD mode (bit 2 of register B)
    uint8_t registerB = rtc_read_register(0x0B);
    if (!(registerB & 0x04)) {
        // BCD mode, convert to binary
        time->second = bcd_to_binary(time->second);
        time->minute = bcd_to_binary(time->minute);
        time->hour = bcd_to_binary(time->hour);
        time->day = bcd_to_binary(time->day);
        time->month = bcd_to_binary(time->month);
        time->year = bcd_to_binary(time->year);
    }
    
    // Year is 2-digit, add 2000
    time->year += 2000;
}

void rtc_format_time(rtc_time_t* time, char* buffer) {
    // Format as "HH:MM"
    buffer[0] = '0' + (time->hour / 10);
    buffer[1] = '0' + (time->hour % 10);
    buffer[2] = ':';
    buffer[3] = '0' + (time->minute / 10);
    buffer[4] = '0' + (time->minute % 10);
    buffer[5] = '\0';
}
