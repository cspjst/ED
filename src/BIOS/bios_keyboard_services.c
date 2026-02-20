#include "bios_keyboard_services.h"
#include "bios_keyboard_constants.h"
#include "bios_keyboard_types.h"

/**
 * halts program until key with a scancode is pressed
 * AH = 00
 * on return:
 *	AH = keyboard scan code
 *	AL = ASCII character or zero if special function key
 */
void bios_wait_for_keystroke_and_read(bios_key_t* key) {
    __asm {
		.8086
		pushf               ; not all BIOS functions are well nehaved
        push    ds

        les     di, key
		mov		ah, BIOS_WAIT_FOR_KEYSTROKE_AND_READ
		int		BIOS_KEYBOARD_SERVICES
		stosw

		pop 	ds
		popf
	}
}

/**
 * @note data code is *not* removed from buffer
 * AH = 01
 *	on return:
 *	ZF = 0 if a key pressed (even Ctrl-Break)
 *	AX = 0 if no scan code is available
 *	AH = scan code
 *	AL = ASCII character or zero if special function key
 */
void bios_get_keystroke_status(bios_key_status_t* key_state) {
    __asm {
		.8086
		pushf
        push    ds

        les     di, key_state
		mov		ah, BIOS_GET_KEYSTROKE_STATUS
		int		BIOS_KEYBOARD_SERVICES
        stosw
        jz      ZERO
        mov     al, 1
        jmp     STORE
ZERO:   xor     al, al
STORE:  stosb

		pop 	ds
		popf
	}
}

/**
 * AH = 02
 * on return:
 * 	AL = BIOS keyboard flags (located in BIOS Data Area 40:17)
 *
 * 		|7|6|5|4|3|2|1|0|  AL or BIOS Data Area 40:17
 * 		 | | | | | | | `---- right shift key depressed
 * 		 | | | | | | `----- left shift key depressed
 * 		 | | | | | `------ CTRL key depressed
 * 		 | | | | `------- ALT key depressed
 * 		 | | | `-------- scroll-lock is active
 * 		 | | `--------- num-lock is active
 * 		 | `---------- caps-lock is active
 * 		 `----------- insert is active
 */
void bios_get_keyboard_flags(bios_keybd_info_t* flags) {
    __asm {
		.8086
		pushf
        push    ds

        les     di, flags
		mov		ah, BIOS_GET_SHIFT_STATUS
		int		BIOS_KEYBOARD_SERVICES
		mov     es:[di], al

		pop 	ds
		popf
	}
}
