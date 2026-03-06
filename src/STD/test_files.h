#ifndef TEST_FILES_H
#define TEST_FILES_H

#include "dos_errno.h"
#include "dos_assert.h"
#include "dos_stdio.h"
#include "dos_string.h"
#include "../DOS/dos_file_services.h"

/*
#include <errno.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
*/

#define TEST_BUF_SIZE 64

static void test_file_cleanup(const char* filename) {
    //dos_delete_file(filename);  // Ignore error - file may not exist
}

static int test_file_setup(const char* filename, const char* content) {
    /*
    dos_file_handle_t handle = 0;
    dos_error_code_t err = DOS_SUCCESS;
    uint16_t written = 0;
    uint16_t len = 0;

    // Calculate string length manually (no strlen dependency)
    const char* p = content;
    while (*p++) len++;

    // Create file
    err = dos_create_file(filename, CREATE_READ_WRITE, &handle);
    if (err != DOS_SUCCESS)
        return -1;

    // Write content
    err = dos_write_file(handle, len, content, &written);
    dos_close_file(handle);

    return (err == DOS_SUCCESS && written == len) ? 0 : -1;
    */
}

static int test_file_readall(const char* filename, char* buffer, uint16_t bufsize) {
    /*
    dos_file_handle_t handle = 0;
    dos_error_code_t err = DOS_SUCCESS;
    uint16_t bytes_read = 0;

    err = dos_open_file(filename, ACCESS_READ_ONLY, &handle);
    if (err != DOS_SUCCESS)
        return -1;

    err = dos_read_file(handle, bufsize - 1, buffer, &bytes_read);
    dos_close_file(handle);

    if (err == DOS_SUCCESS) {
        buffer[bytes_read] = '\0';
        return (int)bytes_read;
    }
    return -1;
    */
}

void test_fopen(void) {
    const char* test_file = "fopen.001";
    test_file_cleanup(test_file);
    FILE* f = NULL;


    printf("fopen NULL mode\n");
    f = fopen(test_file, NULL);
    assert(f == NULL);
    perror("!");
    assert(errno != 0);

    printf("fopen \"\" mode\n");
    f = fopen(test_file, "");
    assert(f == NULL);
    perror("!");
    assert(errno != 0);

    printf("fopen \"\" name\n");
    f = fopen("", "w");
    assert(f == NULL);
    perror("!");

    printf("fopen NULL name\n");
    f = fopen(NULL, "w");
    assert(f == NULL);
    perror("!");

    printf("fopen mode r - non-existent file should fail\n");
    f = fopen(test_file, "r");
    perror("!");
    assert(f == NULL);
    assert(errno != 0);  // Should be set to ENOENT or similar

    printf("fopen mode w - create new file\n");
    f = fopen(test_file, "w");
    perror("!");
    assert(f != NULL);
    assert(errno == 0);

    printf("fopen mode r - open existing file\n");
    f = fopen(test_file, "r");
    assert(f != NULL);
    perror("!");
    test_file_cleanup(test_file);

    printf("fopen mode w+ - create with read-write\n");
    f = fopen(test_file, "w+");
    assert(f != NULL);
    perror("!");

    printf("fopen mode r+ - open existing with read-write\n");
    f = fopen(test_file, "r+");
    assert(f != NULL);
    perror("!");
    test_file_cleanup(test_file);

    printf("fopen mode r+ - non-existent should fail\n");
    f = fopen(test_file, "r+");
    assert(f == NULL);
    perror("!");

    printf("fopen mode a - append (creates if not exists)\n");
    f = fopen(test_file, "a");
    assert(f != NULL);
    perror("!");
    test_file_cleanup(test_file);

    printf("fopen mode a+ - append with read (creates if not exists)\n");
    f = fopen(test_file, "a+");
    assert(f != NULL);
    perror("!");
    test_file_cleanup(test_file);

    printf("fopen invalid mode should fail\n");
    f = fopen(test_file, "x");
    perror("!");
    assert(f == NULL);
    assert(errno == EINVAL);

    printf("fopen NULL mode should fail\n");
    f = fopen(test_file, NULL);
    perror("!");
    assert(f == NULL);
    assert(errno == EINVAL);

    printf("fopen() tests passed\n\n");
}

void test_fopen_exclusive() {

    const char* test_file = "excl.txt";
    FILE* f = NULL;

    // Clean start
    dos_delete_file(test_file);

    // First create with "wx" should succeed
    f = fopen(test_file, "wx");
    assert(f != NULL);
    fputs("first\n", f);
    fclose(f);

    // Second create with "wx" should fail (file exists)
    f = fopen(test_file, "wx");
    assert(f == NULL);
    assert(errno == EEXIST);

    // "w+x" should also fail
    f = fopen(test_file, "w+x");
    assert(f == NULL);
    assert(errno == EEXIST);

    // "w" (non-exclusive) should succeed and truncate
    f = fopen(test_file, "w");
    assert(f != NULL);
    fclose(f);

    // Clean up
    dos_delete_file(test_file);
    printf("fopen() exclusive-create test passed\n\n");
}

void test_fputc_fgetc(void) {
    const char* test_file = "fputc.txt";
    FILE* f = NULL;
    int c = 0;

    test_file_cleanup(test_file);

    // Write characters
    f = fopen(test_file, "w");
    assert(f != NULL);
    assert(fputc('A', f) == 'A');
    assert(fputc('B', f) == 'B');
    assert(fputc('C', f) == 'C');
    assert(fputc('\n', f) == '\n');
    fclose(f);

    // Read characters back
    f = fopen(test_file, "r");
    assert(f != NULL);
    c = fgetc(f);
    assert(c == 'A');
    c = fgetc(f);
    assert(c == 'B');
    c = fgetc(f);
    assert(c == 'C');
    c = fgetc(f);
    assert(c == '\n');
    c = fgetc(f);
    assert(c == EOF);  // End of file
    fclose(f);

    // Test EOF on empty file
    f = fopen(test_file, "w");
    assert(f != NULL);
    fclose(f);
    f = fopen(test_file, "r");
    assert(f != NULL);
    c = fgetc(f);
    assert(c == EOF);
    fclose(f);

    test_file_cleanup(test_file);
    printf("fputc() / fgetc() tests passed\n\n");
}

void test_fputs_fgets(void) {
    const char* test_file = "fputs.txt";
    FILE* f = NULL;
    char buf[64];

    test_file_cleanup(test_file);

    // Write lines
    f = fopen(test_file, "w");
    assert(f != NULL);
    assert(fputs("Hello, DOS!\n", f) == 0);
    assert(fputs("Second line\n", f) == 0);
    assert(fputs("No newline", f) == 0);  // No trailing newline
    fclose(f);

    // Read lines back
    f = fopen(test_file, "r");
    assert(f != NULL);

    // Line 1
    assert(fgets(buf, sizeof(buf), f) != NULL);
    assert(strcmp(buf, "Hello, DOS!\n") == 0);

    // Line 2
    assert(fgets(buf, sizeof(buf), f) != NULL);
    assert(strcmp(buf, "Second line\n") == 0);

    // Line 3 (no newline)
    assert(fgets(buf, sizeof(buf), f) != NULL);
    assert(strcmp(buf, "No newline") == 0);

    // EOF
    assert(fgets(buf, sizeof(buf), f) == NULL);

    fclose(f);

    // Test size limit
    f = fopen(test_file, "w");
    assert(f != NULL);
    fputs("1234567890", f);
    fclose(f);

    f = fopen(test_file, "r");
    assert(f != NULL);
    assert(fgets(buf, 5, f) != NULL);  // Read max 4 chars + null
    assert(strlen(buf) == 4);
    assert(strcmp(buf, "1234") == 0);
    fclose(f);

    test_file_cleanup(test_file);
    printf("fputs() / fgets() tests passed\n\n");
}

void test_fwrite_fread(void) {
    const char* test_file = "fwrite.txt";
    FILE* f = NULL;
    char write_buf[] = "Binary test data 12345";
    char read_buf[64];
    size_t written = 0;
    size_t read_count = 0;

    test_file_cleanup(test_file);

    // Write binary data
    f = fopen(test_file, "wb");
    assert(f != NULL);
    written = fwrite(write_buf, 1, sizeof(write_buf) - 1, f);  // Exclude null
    assert(written == sizeof(write_buf) - 1);
    fclose(f);

    // Read binary data back
    f = fopen(test_file, "rb");
    assert(f != NULL);
    read_count = fread(read_buf, 1, sizeof(read_buf), f);
    assert(read_count == sizeof(write_buf) - 1);
    read_buf[read_count] = '\0';  // Null-terminate for comparison
    assert(strcmp(read_buf, write_buf) == 0);
    fclose(f);

    // Test struct write/read
    typedef struct {
        uint16_t id;
        uint16_t value;
    } test_record_t;

    test_record_t rec_out = { 0x1234, 0x5678 };
    test_record_t rec_in = { 0, 0 };

    f = fopen(test_file, "wb");
    assert(f != NULL);
    written = fwrite(&rec_out, sizeof(test_record_t), 1, f);
    assert(written == 1);
    fclose(f);

    f = fopen(test_file, "rb");
    assert(f != NULL);
    read_count = fread(&rec_in, sizeof(test_record_t), 1, f);
    assert(read_count == 1);
    assert(rec_in.id == rec_out.id);
    assert(rec_in.value == rec_out.value);
    fclose(f);

    test_file_cleanup(test_file);
    printf("fwrite() / fread() test passed\n\n");
}

void test_fseek_ftell(void) {
    const char* test_file = "fseek.txt";
    FILE* f = NULL;
    long pos = 0;
    char c = 0;

    test_file_cleanup(test_file);

    // Create test file with known content
    f = fopen(test_file, "w");
    assert(f != NULL);
    fputs("0123456789", f);  // 10 bytes
    fclose(f);

    // Test SEEK_SET
    f = fopen(test_file, "r");
    assert(f != NULL);
    pos = ftell(f);
    assert(pos == 0);  // Start at beginning

    fseek(f, 5, SEEK_SET);
    pos = ftell(f);
    assert(pos == 5);
    c = fgetc(f);
    assert(c == '5');

    fseek(f, 0, SEEK_SET);
    pos = ftell(f);
    assert(pos == 0);
    c = fgetc(f);
    assert(c == '0');

    fclose(f);

    // Test SEEK_CUR
    f = fopen(test_file, "r");
    assert(f != NULL);
    fseek(f, 3, SEEK_CUR);
    pos = ftell(f);
    assert(pos == 3);
    fseek(f, 2, SEEK_CUR);
    pos = ftell(f);
    assert(pos == 5);
    fseek(f, -2, SEEK_CUR);
    pos = ftell(f);
    assert(pos == 3);
    fclose(f);

    // Test SEEK_END
    f = fopen(test_file, "r");
    assert(f != NULL);
    fseek(f, 0, SEEK_END);
    pos = ftell(f);
    assert(pos == 10);  // File is 10 bytes
    fseek(f, -3, SEEK_END);
    pos = ftell(f);
    assert(pos == 7);
    c = fgetc(f);
    assert(c == '7');
    fclose(f);

    // Test append mode seeks to end
    f = fopen(test_file, "a");
    assert(f != NULL);
    pos = ftell(f);
    assert(pos == 10);  // Should be at end immediately
    fclose(f);

    test_file_cleanup(test_file);
    printf("fseek() / ftell() tests passed\n\n");
}

void test_fclose(void) {
    const char* test_file = "fclose.txt";
    FILE* f = NULL;
    int result = 0;

    test_file_cleanup(test_file);

    // Close after write
    f = fopen(test_file, "w");
    assert(f != NULL);
    fputs("data", f);
    result = fclose(f);
    assert(result == 0);

    // Close after read
    f = fopen(test_file, "r");
    assert(f != NULL);
    result = fclose(f);
    assert(result == 0);

    // Close NULL should fail
    result = fclose(NULL);
    assert(result == EOF);
    assert(errno == EBADF);

    // Double close should fail (handle invalid after first close)
    f = fopen(test_file, "r");
    assert(f != NULL);
    result = fclose(f);
    assert(result == 0);
    // Second close on same handle - behavior depends on implementation
    // Skipping this test as it's implementation-defined

    test_file_cleanup(test_file);
    printf("fclose() tests passed\n\n");
}

void test_fgets_file(void) {
    FILE* f = fopen("test.txt", "w");
    if (!f) {
        printf("Failed to create test.txt\n");
        return;
    }
    printf("char written %i\n",fputc('!', f));
    printf("bytes written %i\n",fputs("Hello, DOS!\nSecond line\n", f));
    fclose(f);

    f = fopen("test.txt", "r");
    if (!f) {
        printf("Failed to open test.txt for reading\n");
        return;
    }

    char buf[TEST_BUF_SIZE];
    printf("Testing fgets from file...\n");

    // First line
    char* result = fgets(buf, TEST_BUF_SIZE, f);
    printf("Line 1: '%s'\n", buf);
    assert(result != NULL);
    assert(strcmp(buf, "!Hello, DOS!\n") == 0);

    // Second line
    result = fgets(buf, TEST_BUF_SIZE, f);
    printf("Line 2: '%s'\n", buf);
    assert(result != NULL);
    assert(strcmp(buf, "Second line\n") == 0);

    // EOF
    result = fgets(buf, TEST_BUF_SIZE, f);
    assert(result == NULL);
    printf("File test passed\n\n");

    fclose(f);
}

void test_file_operations_integration(void) {
    const char* test_file = "integ.txt";
    FILE* f = NULL;
    char buf[128];

    test_file_cleanup(test_file);

    // Write multiple lines
    f = fopen(test_file, "w");
    assert(f != NULL);
    fputs("Line 1\n", f);
    fputs("Line 2\n", f);
    fputs("Line 3\n", f);
    fclose(f);

    // Read all lines
    f = fopen(test_file, "r");
    assert(f != NULL);

    assert(fgets(buf, sizeof(buf), f) != NULL);
    assert(strcmp(buf, "Line 1\n") == 0);

    assert(fgets(buf, sizeof(buf), f) != NULL);
    assert(strcmp(buf, "Line 2\n") == 0);

    assert(fgets(buf, sizeof(buf), f) != NULL);
    assert(strcmp(buf, "Line 3\n") == 0);

    assert(fgets(buf, sizeof(buf), f) == NULL);  // EOF

    fclose(f);

    // Rewrite with fseek
    f = fopen(test_file, "r+");
    assert(f != NULL);
    fseek(f, 0, SEEK_SET);
    fputs("NEW Line 1\n", f);
    fclose(f);

    // Verify rewrite
    f = fopen(test_file, "r");
    assert(f != NULL);
    assert(fgets(buf, sizeof(buf), f) != NULL);
    assert(strcmp(buf, "NEW Line 1\n") == 0);
    fclose(f);

    test_file_cleanup(test_file);
    printf("File operations (integration) tests passed\n\n");
}

void test_files(void) {

    test_fopen();
    test_fopen_exclusive();
    test_fputc_fgetc();
    test_fputs_fgets();
    test_fwrite_fread();
    test_fseek_ftell();
    test_fclose();
    test_fgets_file();
    test_file_operations_integration();

}

#endif
