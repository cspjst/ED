#ifndef BIOS_KEYBOARD_CONSTANTS_H
#define BIOS_KEYBOARD_CONSTANTS_H

#define BIOS_KEYBOARD_SERVICES    16h

#define BIOS_WAIT_FOR_KEYSTROKE_AND_READ      0
#define BIOS_GET_KEYSTROKE_STATUS             1
#define BIOS_GET_SHIFT_STATUS                 2

#define BIOS_KEY_RIGHT_SHIFT    1
#define BIOS_KEY_LEFT_SHIFT     2
#define BIOS_KEY_CTRL           4
#define BIOS_KEY_ALT            8
#define BIOS_SCROLL_LOCK        16
#define BIOS_NUM_LOCK           32
#define BIOS_CAPS_LOCK          64
#define BIOS_KEY_INSERT         128

#endif
