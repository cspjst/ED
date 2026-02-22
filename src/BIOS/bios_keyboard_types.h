#ifndef BIOS_KEYBOARD_TYPES_H
#define BIOS_KEYBOARD_TYPES_H

#include "../STD/dos_stdint.h"

typedef uint8_t bios_keybd_info_t;

#pragma pack(1)
typedef struct {
    uint8_t ascii;
    uint8_t scan;
} bios_key_parts_t;
#pragma pack()

typedef union {
    uint16_t combined;
    bios_key_parts_t parts;
} bios_key_t;

#pragma pack(1)
typedef struct {
    bios_key_t charscan;
    uint8_t pressed;
} bios_key_status_t;
#pragma pack()

#endif
