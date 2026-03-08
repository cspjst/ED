#ifndef TEST_STDIO_H
#define TEST_STDIO_H


#include "dos_stdio.h"
#include "dos_errno.h"
#include "dos_stdlib.h"
#include "dos_assert.h"
#include "dos_string.h"

/*
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>
*/

#define TEST_BUF_SIZE 64

void test_stdio_basic() {

    fputc(fputc('a', stdout) + 2, stdout);
    assert(putc('b', stdout) == 'b');
    fputc('e', stderr);
    putc('E', stderr);

    // Test basic character output
    putchar('A');
    putchar('B');
    putchar('C');
    putchar('\n');

    // Test numeric values
    for (int i = 0; i < 10; i++) {
        putchar('0' + i);
    }
    putchar('\n');

    // Test special characters
    putchar('\t');
    putchar('X');
    putchar('\n');

    // Test basic string output
    puts("Hello, DOS!");
    puts("");
    puts("This\nis\na\ntest\tstring");
    puts("Test 123");
    puts("Special chars: !@#$%^&*()");
    puts(NULL);
}
void test_printf_integers(void) {
    printf("Testing printf integers...\n");

    // Test basic signed integers
    printf("Signed: %d %d %d\n", 0, 42, -17);

    // Test unsigned integers
    printf("Unsigned: %u %u\n", 4294967254U, 12345U);

    // Test long integers
    printf("Long: %ld %ld\n", 123456L, -987654L);

    // Test zero and boundary values
    printf("Zero: %d %u\n", 0, 0U);
    printf("Max int: %d\n", 32767);  // Assuming 16-bit int
    printf("Min int: %d\n", -32768);

    printf("Integer test complete\n");
}

void test_printf_strings(void) {
    printf("Testing printf strings...\n");

    // Test basic string
    printf("String: %s\n", "Hello");

    // Test empty string
    printf("Empty: '%s'\n", "");

    // Test string with spaces
    printf("Spaces: '%s'\n", "hello world");

    // Test null string (if handled)
    char* null_str = NULL;
    printf("Null: %s\n", null_str ? null_str : "(null)");

    // Test string with special characters
    printf("Special: %s\n", "test!@#$%");

    printf("String test complete\n");
}

void test_printf_hex(void) {
    printf("Testing printf hex...\n");

    // Test lowercase hex
    printf("Lower hex: %x %x %x\n", 0, 255, 4096);

    // Test uppercase hex
    printf("Upper hex: %X %X %X\n", 0, 255, 4096);

    // Test mixed case
    printf("Mixed: %x %X\n", 0xDEAD, 0xBEEF);

    // Test boundary values
    printf("Boundaries: %x %X\n", 0xFFFF, 0x8000);

    // Test with long values
    printf("Long hex: %lx %lX\n", 0x12345678UL, 0x87654321UL);

    printf("Hex test complete\n");
}

void test_printf_recursive() {
    printf("Testing recursive number formatting...\n");

    // Test recursive depth with various numbers
    printf("1. Single digit: %d\n", 1);
    printf("2. Two digits: %d\n", 12);
    printf("3. Three digits: %d\n", 123);
    printf("4. Four digits: %d\n", 1234);
    printf("5. Many digits: %d\n", 1234567890);

    // Test base conversions
    printf("6. Binary pattern: %x\n", 0xAAAAAAAA);
    printf("7. Alternating bits: %o\n", 01234567);

    // Test recursive with different bases
    printf("8. Hex recursive: %x\n", 0x12345678);
    printf("9. Octal recursive: %o\n", 01234567);
    printf("10. Large unsigned: %u\n", 3000000000U);
}

void test_printf_error_conditions() {
    printf("Testing printf error conditions...\n");

    // Test 1: Unknown format specifiers
    printf("1. Unknown specifier: %q (should show %q)\n");
    printf("2. Incomplete format: % (should show % )\n");
    printf("3. Double percent: %%d should show % d literally\n");

    // Test 2: Empty format string
    printf("4. Empty format:\n", 42); // Extra arg should be ignored
    printf("5. Just text, no formats\n");

    // Test 3: Missing arguments (will read garbage - but shouldn't crash)
    printf("6. Missing string: %s\n"); // Will show garbage but not crash
    printf("7. Missing number: %d\n"); // Will show garbage but not crash
}

void test_fputc_file_ops(void) {
    // Test character output to stdout (handle 1)
    FILE* stdout_handle = (FILE*)(uintptr_t)1;
    fputc('A', stdout_handle);
    fputc('B', stdout_handle);
    fputc('\n', stdout_handle);

    // Test newline handling
    fputc('X', stdout_handle);
    fputc('\n', stdout_handle);

    // Test numeric character output
    for (int i = 0; i < 5; i++) {
        fputc('0' + i, stdout_handle);
    }
    fputc('\n', stdout_handle);

    printf("fputc test complete\n");
}

void test_fputs_file_ops(void) {
    printf("Testing fputs file operations...\n");

    // Test string output to stdout (handle 1)
    FILE* stdout_handle = (FILE*)(uintptr_t)1;
    fputs("Hello from fputs\n", stdout_handle);

    // Test empty string
    fputs("", stdout_handle);
    fputs("After empty\n", stdout_handle);

    // Test string with spaces
    fputs("Test string with spaces\n", stdout_handle);

    // Test short strings
    fputs("A\n", stdout_handle);
    fputs("AB\n", stdout_handle);
    fputs("ABC\n", stdout_handle);

    // Test special characters
    fputs("Special: !@#$%\n", stdout_handle);

    printf("fputs test complete\n");
}

#ifdef DOS_STDIO_PRINTF_FLOAT
void test_printf_float_basic(void) {
    // Test positive values
    printf("  %%f: %f\n", 0.0);
    printf("  %%f: %f\n", 1.0);
    printf("  %%f: %f\n", 123.456);
    printf("  %%f: %f\n", 0.123);
    printf("  %%f: %f\n", 0.0001);
    printf("  %%f: %f\n", 999.999);

    // Test negative values
    printf("  %%f: %f\n", -0.0);
    printf("  %%f: %f\n", -1.0);
    printf("  %%f: %f\n", -123.456);
    printf("  %%f: %f\n", -0.123);
    printf("  %%f: %f\n", -0.0001);

    // Test edge cases
    printf("  %%f: %f\n", 1.0 / 3.0); // Approx 0.333
    printf("  %%f: %f\n", 2.0 / 3.0); // Approx 0.667
    printf("  %%f: %f\n", 1.0 / 1000.0); // 0.001

    printf("printf %%f tests complete.\n\n");
}
#endif

#ifdef DOS_STDIO_PRINTF_SCIENTIFIC
void test_printf_scientific_basic(void) {
    printf("Testing printf %%e and %%E (basic)...\n");

    // Test positive values
    printf("  %%e: %e\n", 0.0);
    printf("  %%e: %e\n", 1.0);
    printf("  %%e: %e\n", 123.456);
    printf("  %%e: %e\n", 0.123);
    printf("  %%e: %e\n", 0.0001);
    printf("  %%e: %e\n", 999.999);
    printf("  %%e: %e\n", 1.0 / 3.0); // Approx 3.333e-1
    printf("  %%e: %e\n", 1.0 / 1000.0); // 1.000e-3

    // Test negative values
    printf("  %%e: %e\n", -0.0);
    printf("  %%e: %e\n", -1.0);
    printf("  %%e: %e\n", -123.456);
    printf("  %%e: %e\n", -0.123);
    printf("  %%e: %e\n", -0.0001);

    // Test large number
    printf("  %%e: %e\n", 1234567.0); // 1.235e+06

    // Test %%E
    printf("  %%E: %E\n", 0.0);
    printf("  %%E: %E\n", 1.0);
    printf("  %%E: %E\n", 123.456);
    printf("  %%E: %E\n", 0.123);
    printf("  %%E: %E\n", 0.0001);
    printf("  %%E: %E\n", 999.999);
    printf("  %%E: %E\n", 1.0 / 3.0); // Approx 3.333E-1
    printf("  %%E: %E\n", 1.0 / 1000.0); // 1.000E-3
    printf("  %%E: %E\n", 1234567.0); // 1.235E+06

    printf("printf %%e and %%E (basic) test complete.\n\n");
}

void test_printf_float_edge_cases(void) {

    // Very small positive number (should be 1.000e-04)
    printf("  %%f: %f\n", 0.00001); // 0.000
    printf("  %%e: %e\n", 0.00001); // 1.000e-04
    printf("  %%E: %E\n", 0.00001); // 1.000E-04

    // Very small negative number
    printf("  %%f: %f\n", -0.00001); // -0.000
    printf("  %%e: %e\n", -0.00001); // -1.000e-04
    printf("  %%E: %E\n", -0.00001); // -1.000E-04

    // Very large number (should be 1.235e+06)
    printf("  %%f: %f\n", 1234567.0); // 1234567.000
    printf("  %%e: %e\n", 1234567.0); // 1.235e+06
    printf("  %%E: %E\n", 1234567.0); // 1.235E+06

    // Zero with negative sign
    printf("  %%f: %f\n", -0.0); // -0.000
    printf("  %%e: %e\n", -0.0); // -0.000e+00
    printf("  %%E: %E\n", -0.0); // -0.000E+00

    // Large number with fractional part
    printf("  %%f: %f\n", 1234567.890); // 1234567.890
    printf("  %%e: %e\n", 1234567.890); // 1.235e+06
    printf("  %%E: %E\n", 1234567.890); // 1.235E+06

    printf("printf %%f, %%e, %%E (edge cases) test complete.\n\n");

}
#endif

void test_perror_strerror() {

    printf("\n--- strerror() return values ---\n");
    printf("errno=0:     '%s'\n", strerror(0));
    printf("errno=EPERM: '%s'\n", strerror(EPERM));
    printf("errno=ENOENT:'%s'\n", strerror(ENOENT));
    printf("errno=EIO:   '%s'\n", strerror(EIO));
    printf("errno=EBADF: '%s'\n", strerror(EBADF));
    printf("errno=ENOMEM:'%s'\n", strerror(ENOMEM));
    printf("errno=EACCES:'%s'\n", strerror(EACCES));
    printf("errno=EFAULT:'%s'\n", strerror(EFAULT));
    printf("errno=EEXIST:'%s'\n", strerror(EEXIST));
    printf("errno=EXDEV: '%s'\n", strerror(EXDEV));
    printf("errno=ENODEV:'%s'\n", strerror(ENODEV));
    printf("errno=EINVAL:'%s'\n", strerror(EINVAL));
    printf("errno=EMFILE:'%s'\n", strerror(EMFILE));
    printf("errno=ENOSYS:'%s'\n", strerror(ENOSYS));
    printf("errno=99:    '%s'\n", strerror(99));
    printf("errno=-1:    '%s'\n", strerror(-1));

    errno = 0;
    perror("errno=0");           // Should print: "errno=0: Unknown error"
    errno = ENOENT;
    perror("fopen");             // Should print: "fopen: No such file or directory"
    errno = EACCES;
    perror("");                  // Should print: "Permission denied" (no prefix)
    errno = ENOMEM;
    perror(NULL);                // Should print: "Cannot allocate memory" (no prefix)
    errno = ENOSYS;
    perror("syscall");           // Should print: "syscall: Function not implemented"
    errno = 99;                  // Invalid errno
    perror("bogus");             // Should print: "bogus: Unknown error"
    errno = EPERM;
    perror("test_perm");
    errno = EBADF;
    perror("test_badfd");
    errno = EIO;
    perror("test_io");

    printf("perror/strerror test complete n\n");
}


void test_fgets_stdin(void) {
    char buf[TEST_BUF_SIZE];
    printf("Enter a line (max %d chars): ", TEST_BUF_SIZE - 1);
    char* result = fgets(buf, TEST_BUF_SIZE, stdin);
    if (result == NULL) {
        printf("fgets returned NULL (EOF or error)\n");
        return;
    }
    printf("You entered: '%s'\n", buf);
    // Verify null termination
    assert(buf[TEST_BUF_SIZE - 1] == '\0' || strlen(buf) < TEST_BUF_SIZE);
    printf("stdin test passed\n\n");
}

void test_stdio() {

    test_stdio_basic();
    getchar();
    test_printf_integers();
    getchar();
    test_printf_strings();
    getchar();
    test_printf_hex();
    getchar();
    test_printf_recursive();
    getchar();
    test_printf_error_conditions();
    getchar();
    test_fputc_file_ops();
    getchar();
    test_fputs_file_ops();
    getchar();
    #ifdef DOS_STDIO_PRINTF_FLOAT
    test_printf_float_basic();
    getchar();
    #endif
    #ifdef DOS_STDIO_PRINTF_SCIENTIFIC
    test_printf_scientific_basic();
    getchar();
    test_printf_float_edge_cases();
    getchar();
    #endif
    test_perror_strerror();
    getchar();
    test_fgets_stdin();     // Interactive

}

#endif
