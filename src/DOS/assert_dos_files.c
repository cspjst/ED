#include "assert_dos_files.h"
#include <string.h>

static uint16_t _test_failures = 0;
static uint8_t _test_id = 0;

/* Helper: record failure and trigger debug breakpoint */
static void _test_fail(uint8_t test_num) {
    _test_failures |= (1 << test_num);
    __asm__ volatile ("int3");  /* Breakpoint for debugger */
}

/* Helper: simple memory compare (avoid libc dependency) */
static uint8_t _mem_equal(const void* a, const void* b, uint16_t len) {
    const uint8_t* pa = (const uint8_t*)a;
    const uint8_t* pb = (const uint8_t*)b;
    while (len--) {
        if (*pa++ != *pb++) return 0;
    }
    return 1;
}

/* ============================================================================
 * TEST: dos_file_exists
 * ============================================================================ */
uint8_t test_file_exists(void) {
    const char* existing = "C:\\TEST.EXE";   /* Adjust for your env */
    const char* missing  = "C:\\NO_SUCH_FILE.DAT";
    
    if (!dos_file_exists(existing)) { _test_fail(0); return TEST_FAIL; }
    if (dos_file_exists(missing))   { _test_fail(0); return TEST_FAIL; }
    
    return TEST_PASS;
}

/* ============================================================================
 * TEST: dos_file_ext
 * ============================================================================ */
uint8_t test_file_ext(void) {
    DOS_ASSERT(dos_file_ext("FILE.TXT") != NULL);
    DOS_ASSERT(dos_strcmp(dos_file_ext("FILE.TXT"), ".TXT") == 0);
    DOS_ASSERT(dos_file_ext("NOEXT") == NULL);
    DOS_ASSERT(dos_file_ext("PATH\\FILE.C") != NULL);
    DOS_ASSERT(dos_strcmp(dos_file_ext("PATH\\FILE.C"), ".C") == 0);
    return TEST_PASS;
}

/* ============================================================================
 * TEST: Create → Write → Read → Close → Delete
 * ============================================================================ */
uint8_t test_file_create_open_close(void) {
    dos_file_handle_t fh;
    dos_error_code_t err;
    const char* path = "C:\\ED_TEST.DAT";
    
    /* Cleanup first */
    dos_delete_file(path);
    
    /* Create */
    err = dos_create_file(path, 0, &fh);
    if (err != DOS_ERR_SUCCESS) { _test_fail(2); return TEST_FAIL; }
    DOS_ASSERT(fh != 0xFF);
    
    /* Close */
    err = dos_close_file(fh);
    if (err != DOS_ERR_SUCCESS) { _test_fail(2); return TEST_FAIL; }
    
    /* Re-open */
    err = dos_open_file(path, 0x02, &fh);  /* Read-only */
    if (err != DOS_ERR_SUCCESS) { _test_fail(2); return TEST_FAIL; }
    
    /* Close again */
    err = dos_close_file(fh);
    if (err != DOS_ERR_SUCCESS) { _test_fail(2); return TEST_FAIL; }
    
    /* Cleanup */
    dos_delete_file(path);
    
    return TEST_PASS;
}

/* ============================================================================
 * TEST: Write data, read back, verify
 * ============================================================================ */
uint8_t test_file_read_write(void) {
    dos_file_handle_t fh;
    dos_error_code_t err;
    uint16_t written, read;
    const char* path = "C:\\ED_RW.DAT";
    const char* data_out = "Hello, 8086!";
    char data_in[32];
    
    dos_delete_file(path);  /* Cleanup */
    dos_memset(data_in, 0, sizeof(data_in));
    
    /* Create + Write */
    err = dos_create_file(path, 0, &fh);
    if (err != DOS_ERR_SUCCESS) { _test_fail(3); return TEST_FAIL; }
    
    err = dos_write_file(fh, 12, data_out, &written);
    if (err != DOS_ERR_SUCCESS || written != 12) { _test_fail(3); return TEST_FAIL; }
    
    err = dos_close_file(fh);
    if (err != DOS_ERR_SUCCESS) { _test_fail(3); return TEST_FAIL; }
    
    /* Open + Read */
    err = dos_open_file(path, 0x02, &fh);
    if (err != DOS_ERR_SUCCESS) { _test_fail(3); return TEST_FAIL; }
    
    err = dos_read_file(fh, 32, data_in, &read);
    if (err != DOS_ERR_SUCCESS || read != 12) { _test_fail(3); return TEST_FAIL; }
    
    /* Verify content */
    if (!_mem_equal(data_out, data_in, 12)) { _test_fail(3); return TEST_FAIL; }
    
    err = dos_close_file(fh);
    dos_delete_file(path);
    
    return TEST_PASS;
}

/* ============================================================================
 * TEST: dos_file_size
 * ============================================================================ */
uint8_t test_file_size(void) {
    dos_file_handle_t fh;
    dos_error_code_t err;
    dos_file_size_t size;
    const char* path = "C:\\ED_SIZE.DAT";
    const char* data = "1234567890";  /* 10 bytes */
    uint16_t written;
    
    dos_delete_file(path);
    
    /* Create file with known size */
    err = dos_create_file(path, 0, &fh);
    if (err != DOS_ERR_SUCCESS) { _test_fail(4); return TEST_FAIL; }
    
    err = dos_write_file(fh, 10, data, &written);
    if (err != DOS_ERR_SUCCESS) { _test_fail(4); return TEST_FAIL; }
    
    err = dos_close_file(fh);
    if (err != DOS_ERR_SUCCESS) { _test_fail(4); return TEST_FAIL; }
    
    /* Re-open and check size */
    err = dos_open_file(path, 0x02, &fh);
    if (err != DOS_ERR_SUCCESS) { _test_fail(4); return TEST_FAIL; }
    
    err = dos_file_size(fh, &size);
    if (err != DOS_ERR_SUCCESS || size != 10) { _test_fail(4); return TEST_FAIL; }
    
    err = dos_close_file(fh);
    dos_delete_file(path);
    
    return TEST_PASS;
}

/* ============================================================================
 * TEST: dos_file_is_eof
 * ============================================================================ */
uint8_t test_file_eof(void) {
    dos_file_handle_t fh;
    dos_error_code_t err;
    const char* path = "C:\\ED_EOF.DAT";
    char buf[1];
    uint16_t read;
    
    dos_delete_file(path);
    
    /* Create 5-byte file */
    err = dos_create_file(path, 0, &fh);
    if (err != DOS_ERR_SUCCESS) { _test_fail(5); return TEST_FAIL; }
    dos_write_file(fh, 5, "XXXXX", &read);
    dos_close_file(fh);
    
    /* Open and read to EOF */
    err = dos_open_file(path, 0x02, &fh);
    if (err != DOS_ERR_SUCCESS) { _test_fail(5); return TEST_FAIL; }
    
    /* Should not be EOF initially */
    if (dos_file_is_eof(fh)) { _test_fail(5); return TEST_FAIL; }
    
    /* Read all 5 bytes */
    for (int i = 0; i < 5; i++) {
        dos_read_file(fh, 1, buf, &read);
    }
    
    /* Now should be EOF */
    if (!dos_file_is_eof(fh)) { _test_fail(5); return TEST_FAIL; }
    
    dos_close_file(fh);
    dos_delete_file(path);
    
    return TEST_PASS;
}

/* ============================================================================
 * TEST: dos_move_file_pointer
 * ============================================================================ */
uint8_t test_file_move_pointer(void) {
    dos_file_handle_t fh;
    dos_error_code_t err;
    dos_file_position_t pos;
    const char* path = "C:\\ED_SEEK.DAT";
    
    dos_delete_file(path);
    
    /* Create 20-byte file */
    err = dos_create_file(path, 0, &fh);
    if (err != DOS_ERR_SUCCESS) { _test_fail(6); return TEST_FAIL; }
    uint16_t written;
    dos_write_file(fh, 20, "01234567890123456789", &written);
    dos_close_file(fh);
    
    /* Open and seek */
    err = dos_open_file(path, 0x02, &fh);
    if (err != DOS_ERR_SUCCESS) { _test_fail(6); return TEST_FAIL; }
    
    /* Seek to byte 10 from start */
    err = dos_move_file_pointer(fh, 10, 0, &pos);  /* 0 = SEEK_SET */
    if (err != DOS_ERR_SUCCESS || pos != 10) { _test_fail(6); return TEST_FAIL; }
    
    /* Seek 5 bytes forward */
    err = dos_move_file_pointer(fh, 5, 1, &pos);   /* 1 = SEEK_CUR */
    if (err != DOS_ERR_SUCCESS || pos != 15) { _test_fail(6); return TEST_FAIL; }
    
    /* Seek 3 bytes from end */
    err = dos_move_file_pointer(fh, -3, 2, &pos);  /* 2 = SEEK_END */
    if (err != DOS_ERR_SUCCESS || pos != 17) { _test_fail(6); return TEST_FAIL; }
    
    dos_close_file(fh);
    dos_delete_file(path);
    
    return TEST_PASS;
}

/* ============================================================================
 * TEST: File attributes (get/set)
 * ============================================================================ */
uint8_t test_file_attributes(void) {
    dos_error_code_t err;
    dos_file_attributes_t attr;
    const char* path = "C:\\ED_ATTR.DAT";
    
    dos_delete_file(path);
    
    /* Create file */
    dos_file_handle_t fh;
    err = dos_create_file(path, 0, &fh);
    if (err != DOS_ERR_SUCCESS) { _test_fail(7); return TEST_FAIL; }
    dos_close_file(fh);
    
    /* Get attributes (should be normal = 0x20 or 0x00) */
    err = dos_get_file_attributes(path, &attr);
    if (err != DOS_ERR_SUCCESS) { _test_fail(7); return TEST_FAIL; }
    
    /* Set read-only (0x01) */
    err = dos_set_file_attributes(path, 0x01);
    if (err != DOS_ERR_SUCCESS) { _test_fail(7); return TEST_FAIL; }
    
    /* Verify */
    err = dos_get_file_attributes(path, &attr);
    if (err != DOS_ERR_SUCCESS || (attr & 0x01) == 0) { _test_fail(7); return TEST_FAIL; }
    
    /* Clear read-only */
    err = dos_set_file_attributes(path, 0x20);  /* Normal */
    dos_delete_file(path);
    
    return TEST_PASS;
}

/* ============================================================================
 * TEST: dos_rename_file
 * ============================================================================ */
uint8_t test_file_rename(void) {
    dos_error_code_t err;
    const char* old_path = "C:\\ED_OLD.DAT";
    const char* new_path = "C:\\ED_NEW.DAT";
    
    dos_delete_file(old_path);
    dos_delete_file(new_path);
    
    /* Create original file */
    dos_file_handle_t fh;
    err = dos_create_file(old_path, 0, &fh);
    if (err != DOS_ERR_SUCCESS) { _test_fail(8); return TEST_FAIL; }
    dos_close_file(fh);
    
    /* Verify old exists, new doesn't */
    if (!dos_file_exists(old_path) || dos_file_exists(new_path)) {
        _test_fail(8); return TEST_FAIL;
    }
    
    /* Rename */
    err = dos_rename_file(old_path, new_path);
    if (err != DOS_ERR_SUCCESS) { _test_fail(8); return TEST_FAIL; }
    
    /* Verify swap */
    if (dos_file_exists(old_path) || !dos_file_exists(new_path)) {
        _test_fail(8); return TEST_FAIL;
    }
    
    /* Cleanup */
    dos_delete_file(new_path);
    
    return TEST_PASS;
}

/* ============================================================================
 * TEST: dos_get_disk_free_space
 * ============================================================================ */
uint8_t test_disk_free_space(void) {
    dos_error_code_t err;
    dos_file_disk_space_info_t info;
    
    /* Test drive 0 = default, 2 = C: */
    err = dos_get_disk_free_space(2, &info);
    if (err != DOS_ERR_SUCCESS) { _test_fail(9); return TEST_FAIL; }
    
    /* Sanity checks */
    if (info.sectors_per_cluster == 0) { _test_fail(9); return TEST_FAIL; }
    if (info.total_clusters == 0) { _test_fail(9); return TEST_FAIL; }
    
    return TEST_PASS;
}

/* ============================================================================
 * MASTER TEST RUNNER
 * ============================================================================ */
uint16_t dos_file_run_all_tests(void) {
    _test_failures = 0;
    _test_id = 0;
    
    /* Run tests - order matters for cleanup */
    test_file_ext();              /* 0 */
    test_file_exists();           /* 1 */
    test_file_create_open_close();/* 2 */
    test_file_read_write();       /* 3 */
    test_file_size();             /* 4 */
    test_file_eof();              /* 5 */
    test_file_move_pointer();     /* 6 */
    test_file_attributes();       /* 7 */
    test_file_rename();           /* 8 */
    test_disk_free_space();       /* 9 */
    
    return _test_failures;  /* 0 = all pass, bit N set = test N failed */
}
