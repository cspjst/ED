#ifndef BIOS_KEYBOARD_SCAN_CODES_H
#define BIOS_KEYBOARD_SCAN_CODES_H

typedef enum {
    // Function Keys
    SCAN_F1         = 0x3B,
    SCAN_F2         = 0x3C,
    SCAN_F3         = 0x3D,
    SCAN_F4         = 0x3E,
    SCAN_F5         = 0x3F,
    SCAN_F6         = 0x40,
    SCAN_F7         = 0x41,
    SCAN_F8         = 0x42,
    SCAN_F9         = 0x43,
    SCAN_F10        = 0x44,

    // Alphanumeric Keys (Main block)
    SCAN_ESC        = 0x01,
    SCAN_1          = 0x02,
    SCAN_2          = 0x03,
    SCAN_3          = 0x04,
    SCAN_4          = 0x05,
    SCAN_5          = 0x06,
    SCAN_6          = 0x07,
    SCAN_7          = 0x08,
    SCAN_8          = 0x09,
    SCAN_9          = 0x0A,
    SCAN_0          = 0x0B,
    SCAN_MINUS      = 0x0C,  // -
    SCAN_EQUALS     = 0x0D,  // =
    SCAN_BACKSPACE  = 0x0E,

    SCAN_TAB        = 0x0F,
    SCAN_Q          = 0x10,
    SCAN_W          = 0x11,
    SCAN_E          = 0x12,
    SCAN_R          = 0x13,
    SCAN_T          = 0x14,
    SCAN_Y          = 0x15,
    SCAN_U          = 0x16,
    SCAN_I          = 0x17,
    SCAN_O          = 0x18,
    SCAN_P          = 0x19,
    SCAN_LBRACKET   = 0x1A,  // [
    SCAN_RBRACKET   = 0x1B,  // ]
    SCAN_ENTER      = 0x1C,

    SCAN_A          = 0x1E,
    SCAN_S          = 0x1F,
    SCAN_D          = 0x20,
    SCAN_F          = 0x21,
    SCAN_G          = 0x22,
    SCAN_H          = 0x23,
    SCAN_J          = 0x24,
    SCAN_K          = 0x25,
    SCAN_L          = 0x26,
    SCAN_SEMICOLON  = 0x27,  // ;
    SCAN_APOSTROPHE = 0x28,  // '
    SCAN_GRAVE      = 0x29,  // `

    SCAN_BACKSLASH  = 0x2B,
    SCAN_Z          = 0x2C,
    SCAN_X          = 0x2D,
    SCAN_C          = 0x2E,
    SCAN_V          = 0x2F,
    SCAN_B          = 0x30,
    SCAN_N          = 0x31,
    SCAN_M          = 0x32,
    SCAN_COMMA      = 0x33,  // ,
    SCAN_PERIOD     = 0x34,  // .
    SCAN_SLASH      = 0x35,  // /

    // Special Keys
    SCAN_SPACE      = 0x39,
    SCAN_CAPS_LOCK  = 0x3A,

    // Number Pad (when Num Lock is off)
    SCAN_HOME       = 0x47,
    SCAN_UP         = 0x48,
    SCAN_PGUP       = 0x49,
    SCAN_LEFT       = 0x4B,
    SCAN_CENTER     = 0x4C,
    SCAN_RIGHT      = 0x4D,
    SCAN_END        = 0x4F,
    SCAN_DOWN       = 0x50,
    SCAN_PGDN       = 0x51,
    SCAN_INS        = 0x52,
    SCAN_DEL        = 0x53,

    // Modifier Keys
    SCAN_LSHIFT     = 0x2A,
    SCAN_RSHIFT     = 0x36,
    SCAN_LCTRL      = 0x1D,
    SCAN_RCTRL      = 0x1D,  // Same as left on original XT
    SCAN_LALT       = 0x38,
    SCAN_RALT       = 0x38,  // Same as left on original XT

    // Other
    SCAN_SCROLL_LOCK = 0x46,
    SCAN_PAUSE      = 0x45,
    SCAN_PRINT_SCREEN = 0x37
} keyboard_scan_code_t;

#endif
