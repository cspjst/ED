/**
* @url https://www.stanislavs.org/helppc/int_16.html
*
*   INT 16,0   Wait for keystroke and read
* 	INT 16,1   Get keystroke status
* 	INT 16,2   Read Keyboard Flags
* 	INT 16,3   Set keyboard typematic rate (AT+)
* 	INT 16,4   Keyboard click adjustment (AT+)
* 	INT 16,5   Keyboard buffer write  (AT,PS/2 enhanced keyboards)
* 	INT 16,10  Wait for keystroke and read  (AT,PS/2 enhanced keyboards)
* 	INT 16,11  Get keystroke status  (AT,PS/2 enhanced keyboards)
* 	INT 16,12  Get shift status  (AT,PS/2 enhanced keyboards)
*/
#ifndef BIOS_KEYBOARD_SERVICES_H
#define BIOS_KEYBOARD_SERVICES_H

#include "bios_keyboard_types.h"

void bios_wait_for_keystroke_and_read(bios_key_t* key);

void bios_get_keystroke_status(bios_key_status_t* status);

void bios_get_keyboard_flags(bios_keybd_info_t* flags);

// void bios_set_keyboard_typematic(uint8_t mode, uint8_t delay, uint8_t rate);
// void bios_keyboard_click_adjustment(bool state);
// bool bios_keyboard_buffer_write(bios_key_t key);
// bios_wait_for_keystroke_and_read
// bios_get_keystroke_status
// bios_get_shift_status

#endif
