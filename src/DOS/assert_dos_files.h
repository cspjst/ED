#ifndef ASSERT_DOS_FILES_H
#define ASSERT_DOS_FILES_H

#include "../STD/dos_stdint.h"
#include "dos_file_tools.h"
#include "dos_file_services.h"

/* Minimal assert for freestanding: triggers INT 3 (debugger breakpoint) on fail */
#define DOS_ASSERT(cond) do { \
    if (!(cond)) { \
        __asm__ volatile ("int3"); \
    } \
} while(0)

/* Test result codes */
#define TEST_PASS 1
#define TEST_FAIL 0

/* Individual test functions - return TEST_PASS or TEST_FAIL */
uint8_t test_file_exists(void);
uint8_t test_file_ext(void);
uint8_t test_file_create_open_close(void);
uint8_t test_file_read_write(void);
uint8_t test_file_size(void);
uint8_t test_file_eof(void);
uint8_t test_file_move_pointer(void);
uint8_t test_file_attributes(void);
uint8_t test_file_rename(void);
uint8_t test_disk_free_space(void);

/* Run all tests - returns bitmask of failures (0 = all pass) */
uint16_t dos_file_run_all_tests(void);

#endif
