#ifndef TEST_BIOS_H
#define TEST_BIOS_H


#ifdef POLICY_USE_DOS_STD
    #include "../STD/dos_stdio.h"
    #include "../STD/dos_assert.h"
#else
    #include <stdio.h>
    #include <assert.h>
#endif

#include "../BIOS/bios_memory_services.h"
#include "../BIOS/bios_keyboard_services.h"
#include "../BIOS/bios_keyboard_scan_codes.h"
#include "../BIOS/bios_keyboard_constants.h"

void test_bios_memory(void) {
    unsigned short base_memory;
    unsigned long total_bytes;

    // Get base memory size
    base_memory = bios_get_startup_memory_kb();

    // Display in various units
    printf("Base memory (conventional):\n");
    printf("  %u KB\n", base_memory);
    printf("  %lu bytes\n", (unsigned long)base_memory * 1024L);
    printf("  %u paragraphs\n", base_memory * 64);  // 64 paragraphs per KB

    // Sanity checks
    printf("\nSanity check: ");
    if(base_memory > 0 && base_memory <= 640) {
        printf("PASS (within 0-640K range)\n");
    } else if(base_memory > 640 && base_memory < 1024) {
        printf("WARNING: Unusual value (%u KB) - some systems report >640K\n", base_memory);
    } else {
        printf("FAIL: Impossible value (%u KB)\n", base_memory);
    }
}

void test_bios_keys() {
    printf("Test BIOS...\n");
    assert(sizeof(unsigned char) == 1);
    assert(sizeof(unsigned short) == 2);
    assert(sizeof(unsigned int) == 2);

    bios_key_t key;
    bios_key_status_t key_state;
    bios_keybd_info_t flags;
    int key_count = 0;

    printf("Press any key to test (ESC to quit, F1 for help)\n");
    printf("----------------------------------------\n");

    // Show initial shift states
    bios_get_keyboard_flags(&flags);
    printf("Initial flags: %s%s%s%s%s%s%s%s\n",
        (flags & BIOS_KEY_RIGHT_SHIFT) ? "RSHIFT " : "",
        (flags & BIOS_KEY_LEFT_SHIFT) ? "LSHIFT " : "",
        (flags & BIOS_KEY_CTRL) ? "CTRL " : "",
        (flags & BIOS_KEY_ALT) ? "ALT " : "",
        (flags & BIOS_KEY_SCROLL_LOCK) ? "SCROLL " : "",
        (flags & BIOS_KEY_NUM_LOCK) ? "NUM " : "",
        (flags & BIOS_KEY_CAPS_LOCK) ? "CAPS " : "",
        (flags & BIOS_KEY_INSERT) ? "INSERT " : "(none)");

    while(1) {
        // Show status (non-blocking peek)
        bios_get_keystroke_status(&key_state);

        // Get full key (blocking read)
        bios_wait_for_keystroke_and_read(&key);
        key_count++;

        // Get flags after keypress
        bios_get_keyboard_flags(&flags);

        // Decode and display key information
        printf("[%03d] ", key_count);

        // Show if it's a special or normal key
        if(key.parts.ascii >= 32 && key.parts.ascii <= 126) {
            // Printable ASCII
            printf("'%c' ", key.parts.ascii);
        } else if(key.parts.ascii == 0) {
            // Special function key
            printf("(special) ");
        } else {
            // Control character
            printf("(0x%02X) ", key.parts.ascii);
        }

        // Show scan code with name if known
        printf("scan=0x%02X ", key.parts.scan);
        switch(key.parts.scan) {
            case SCAN_ESC:      printf("(ESC)"); break;
            case SCAN_ENTER:    printf("(ENTER)"); break;
            case SCAN_BACKSPACE: printf("(BACKSPACE)"); break;
            case SCAN_TAB:      printf("(TAB)"); break;
            case SCAN_SPACE:    printf("(SPACE)"); break;
            case SCAN_F1:       printf("(F1)"); break;
            case SCAN_F2:       printf("(F2)"); break;
            case SCAN_F3:       printf("(F3)"); break;
            case SCAN_F4:       printf("(F4)"); break;
            case SCAN_F5:       printf("(F5)"); break;
            case SCAN_F6:       printf("(F6)"); break;
            case SCAN_F7:       printf("(F7)"); break;
            case SCAN_F8:       printf("(F8)"); break;
            case SCAN_F9:       printf("(F9)"); break;
            case SCAN_F10:      printf("(F10)"); break;
            case SCAN_UP:       printf("(UP)"); break;
            case SCAN_DOWN:     printf("(DOWN)"); break;
            case SCAN_LEFT:     printf("(LEFT)"); break;
            case SCAN_RIGHT:    printf("(RIGHT)"); break;
            case SCAN_HOME:     printf("(HOME)"); break;
            case SCAN_END:      printf("(END)"); break;
            case SCAN_PGUP:     printf("(PGUP)"); break;
            case SCAN_PGDN:     printf("(PGDN)"); break;
            case SCAN_INS:      printf("(INSERT)"); break;
            case SCAN_DEL:      printf("(DELETE)"); break;
        }

        // Show shift state
        printf(" [");
        if(flags & BIOS_KEY_RIGHT_SHIFT) printf("RSh ");
        if(flags & BIOS_KEY_LEFT_SHIFT) printf("LSh ");
        if(flags & BIOS_KEY_CTRL) printf("Ctrl ");
        if(flags & BIOS_KEY_ALT) printf("Alt ");
        if(flags & BIOS_KEY_SCROLL_LOCK) printf("ScrLk ");
        if(flags & BIOS_KEY_NUM_LOCK) printf("NumLk ");
        if(flags & BIOS_KEY_CAPS_LOCK) printf("Caps ");
        if(flags & BIOS_KEY_INSERT) printf("Ins ");
        if(flags == 0) printf("none");
        printf("]\n");

        // Help screen on F1
        if(key.parts.scan == SCAN_F1) {
            printf("ESC: Quit\n");
            printf("F1:  Show this help\n");
            printf("F2:  Show status info\n");
            printf("Any other key: Display key info\n");
        }

        // Show status info on F2
        if(key.parts.scan == SCAN_F2) {
            printf("Keys pressed: %d\n", key_count);
            printf("Key available flag: %d\n", key_state.pressed);
            printf("Flags register: 0x%02X\n", flags);
        }

        // Quit on ESC
        if(key.parts.scan == SCAN_ESC) {
            printf("\nTest complete. %d keys processed.\n", key_count);
            break;
        }
    }
}


#endif
