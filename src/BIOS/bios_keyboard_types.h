#ifndef BIOS_KEYBOARD_TYPES_H
#define BIOS_KEYBOARD_TYPES_H

typedef unsigned char bios_keybd_info_t;

#pragma pack(1)
typedef struct {
    unsigned char ascii;
    unsigned char scan;
} bios_key_parts_t;
#pragma pack()

typedef union {
    unsigned short combined;
    bios_key_parts_t parts;
} bios_key_t;

#pragma pack(1)
typedef struct {
    bios_key_t charscan;
    unsigned char pressed;
} bios_key_status_t;
#pragma pack()

#endif
