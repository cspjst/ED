#ifndef TEST_DOS_FILES_H
#define TEST_DOS_FILES_H

#include "dos_file_tools.h"
#include "dos_file_services.h"
#include "dos_error_types.h"
#include "dos_file_services.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

void test_dos_files() {
    printf("Testing DOS File Functions...\n");

    // Test 1: File creation and basic operations
    printf("1. Testing file creation...\n");
    dos_file_handle_t fh;
    dos_error_code_t err;

    err = dos_create_file("TESTFILE.TXT",CREATE_READ_WRITE , &fh);
    assert(err == 0);
    assert(fh != 0xFFFF);
    printf("File created successfully, handle: 0x%04X\n", fh);

    // Test 2: File writing
    printf("2. Testing file writing...\n");
    const char* test_data = "Hello, DOS World!\r\nLine 2\r\nLine 3\r\n";
    uint16_t bytes_to_write = strlen(test_data);
    uint16_t bytes_written = bytes_to_write;

    err = dos_write_file(fh, bytes_to_write, test_data, &bytes_written);
    assert(err == 0);
    printf("%u %u\n", bytes_to_write, bytes_written);
    assert(bytes_written == bytes_to_write);
    printf("Write successful, wrote %u bytes\n", bytes_written);

    // Test 3: File closing
    printf("3. Testing file closing...\n");
    err = dos_close_file(fh);
    assert(err == 0);
    printf("File closed successfully\n");

    // Test 4: File opening
    printf("4. Testing file opening...\n");
    err = dos_open_file("TESTFILE.TXT", ACCESS_READ_ONLY, &fh);
    assert(err == 0);
    assert(fh != 0xFFFF);
    printf("File opened successfully, handle: 0x%04X\n", fh);

    // Test 5: File reading
    printf("5. Testing file reading...\n");
    char buffer[128] = {0};
    uint16_t bytes_to_read = sizeof(buffer) - 1;
    uint16_t bytes_read = bytes_to_read;

    err = dos_read_file(fh, bytes_to_read, buffer, &bytes_read);
    assert(err == 0);
    assert(bytes_read > 0);
    assert(bytes_read <= bytes_to_read);
    assert(strncmp(buffer, test_data, bytes_read) == 0);
    printf("Read successful, read %u bytes\n", bytes_read);
    printf("Data integrity verified\n");

    // Test 6: File positioning
    printf("6. Testing file positioning...\n");
    dos_file_position_t new_pos;

    // Seek to beginning
    err = dos_move_file_pointer(fh, 0, FSEEK_SET, &new_pos); // SEEK_SET
    assert(err == 0);
    assert(new_pos == 0);
    printf("Seek to beginning successful\n");

    // Seek to position 7
    new_pos = 7;
    err = dos_move_file_pointer(fh, 7, FSEEK_SET, &new_pos);
    assert(err == 0);
    assert(new_pos == 7);
    printf("Seek to position 7 successful\n");

    new_pos = -2;
    err = dos_move_file_pointer(fh, -2, FSEEK_CUR, &new_pos);
    assert(err == 0);
    assert(new_pos == 5);
    printf("Seek to position 5 successful\n");

    // Read from new position
    bytes_to_read = 5;
    bytes_read = bytes_to_read;
    err = dos_read_file(fh, bytes_to_read, buffer, &bytes_read);
    assert(err == 0);
    assert(bytes_read == 5);
    buffer[bytes_read] = '\0';
    assert(strcmp(buffer, ", DOS") == 0);
    printf("Read from position 5 verified\n");

    // Test 7: File attributes
    printf("7. Testing file attributes...\n");
    dos_file_attributes_t attrs;
    err = dos_get_file_attributes("TESTFILE.TXT", &attrs);
    assert(err == 0);
    assert(attrs != 0xFFFF);
    printf("File attributes retrieved: 0x%02X\n", attrs);

    // Test 8: File renaming
    printf("8. Testing file renaming...\n");
    err = dos_rename_file("TESTFILE.TXT", "RENAMED.TXT");
    assert(err == 0);
    printf("File renamed successfully\n");

    // Verify old name doesn't exist
    attrs = 0;
    err = dos_get_file_attributes("TESTFILE.TXT", &attrs);
    assert(err != 0); // Should fail
    printf("Old filename correctly removed\n");

    // Verify new name exists
    err = dos_get_file_attributes("RENAMED.TXT", &attrs);
    assert(err == 0);
    printf("New filename verified\n");

    // Test 9: File deletion
    printf("9. Testing file deletion...\n");
    err = dos_close_file(fh);
    assert(err == 0);

    err = dos_delete_file("RENAMED.TXT");
    assert(err == 0);
    printf("File deleted successfully\n");

    // Verify deletion
    err = dos_get_file_attributes("RENAMED.TXT", &attrs);
    assert(err != 0);
    printf("File deletion verified\n");

    // Test 10: Error conditions
    printf("10. Testing error conditions...\n");

    // Try to open non-existent file
    err = dos_open_file("NONEXISTENT.FIL",ACCESS_READ_ONLY , &fh);
    assert(err != 0);
    printf("Non-existent file handling correct (error: %d)\n", err);

    // Try to delete non-existent file
    err = dos_delete_file("ALSONOTTHERE.FIL");
    assert(err != 0);
    printf("Non-existent file deletion handling correct (error: %d)\n", err);

    // Test 11: Disk free space
    printf("11. Testing disk free space...\n");
    dos_file_disk_space_info_t disk_info;
    err = dos_get_disk_free_space(0, &disk_info); // Default drive
    assert(err == 0);
    assert(disk_info.sectors_per_cluster != 0xFFFF);
    printf("Disk free space: %u sectors/cluster, %u bytes/sector\n",
           disk_info.sectors_per_cluster, disk_info.bytes_per_sector);
    printf("Available clusters: %u, Total clusters: %u\n",
           disk_info.available_clusters, disk_info.clusters_per_drive);

    // Test 12: Boundary conditions
    printf("12. Testing boundary conditions...\n");

    // Create empty file
    err = dos_create_file("EMPTY.TXT", CREATE_READ_WRITE, &fh);
    assert(err == 0);

    // Test write 0 bytes (should succeed)
    bytes_to_write = 0;
    bytes_written = 0;
    err = dos_write_file(fh, bytes_to_write, buffer, &bytes_written);
    assert(err == 0);
    assert(bytes_written == 0);
    printf("Zero-byte write successful\n");

    // Test read 0 bytes (should succeed)
    bytes_to_read = 0;
    bytes_read = 0;
    err = dos_read_file(fh, bytes_to_read, buffer, &bytes_read);
    assert(err == 0);
    assert(bytes_read == 0);
    printf("Zero-byte read successful\n");

    dos_close_file(fh);
    dos_delete_file("EMPTY.TXT");

    // Test 13: File attribute manipulation
    printf("13. Testing file attribute manipulation...\n");
    err = dos_create_file("ATTRTEST.TXT", CREATE_READ_WRITE, &fh);
    printf("err = %i\n", err);
    assert(err == 0);
    dos_close_file(fh);

    // Set read-only attribute
    err = dos_set_file_attributes("ATTRTEST.TXT", ATTR_READ_ONLY);
    assert(err == 0);
    printf("Read-only attribute set\n");

    // Verify attribute
    err = dos_get_file_attributes("ATTRTEST.TXT", &attrs);
    assert(err == 0);
    assert((attrs & ATTR_READ_ONLY) == 1); // Read-only bit set
    printf("Read-only attribute verified: 0x%02X\n", attrs);

    // Try to open read-only file for writing (should fail)
    // not work on DOSBOX
    //err = dos_open_file("ATTRTEST.TXT", ACCESS_WRITE_ONLY, &fh);
    //assert(err != 0);
    //printf("Read-only file write protection working (error: %d)\n", err);

    // Clear attributes and delete
    dos_set_file_attributes("ATTRTEST.TXT", 0);
    dos_delete_file("ATTRTEST.TXT");

    // Test 14: Large file operations
    printf("14. Testing large file operations...\n");
    err = dos_create_file("BOUNDARY.TXT", CREATE_READ_WRITE, &fh);
    assert(err == 0);

    // Write multiple chunks
    const char* chunk = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    for (int i = 0; i < 10; i++) {
        bytes_to_write = strlen(chunk);
        bytes_written = bytes_to_write;
        err = dos_write_file(fh, bytes_to_write, chunk, &bytes_written);
        assert(err == 0);
        assert(bytes_written == bytes_to_write);
    }
    printf("Multiple chunk writes successful\n");

    // Seek to various positions
    err = dos_move_file_pointer(fh, 100, SEEK_SET, &new_pos);
    assert(err == 0);
    assert(new_pos == 100);
    printf("Large file seek successful\n");

    dos_close_file(fh);
    dos_delete_file("BOUNDARY.TXT");

    // Test 15: Invalid parameter handling
    printf("15. Testing invalid parameter handling...\n");

    // Try operations with invalid handle
    bytes_to_read = 10;
    bytes_read = bytes_to_read;
    err = dos_read_file(0xFFFF, bytes_to_read, buffer, &bytes_read);
    assert(err != 0);
    printf("Invalid handle detection working (error: %d)\n", err);

    err = dos_close_file(0xFFFF);
    assert(err != 0);
    printf("Invalid handle close detection working (error: %d)\n", err);

    printf("16. Testing file tools...\n");
    dos_file_size_t size = 0xFFFF;
    uint16_t bytes = 0;
    err = dos_create_file("TOOLS.TXT", CREATE_READ_WRITE, &fh);
    assert(err == 0);

    // empty file tests
    assert(dos_file_exists("TOOLS.TXT"));
    assert(dos_file_is_eof(fh));
    assert(dos_file_size(fh, &size) == 0);
    assert(size == 0);
    printf("Empty size, exists and eof working.\n");

    // write to file
    assert(dos_write_file(fh, strlen(test_data), test_data, &bytes) == 0);
    assert(bytes == strlen(test_data));
    assert(dos_file_size(fh, &size) == 0);
    assert(dos_file_exists("TOOLS.TXT"));
    assert(dos_file_is_eof(fh));
    assert(size == bytes);
    assert(dos_close_file(fh) == 0);
    assert(dos_file_exists("TOOLS.TXT"));
    printf("Write size, exists and eof working.\n");

    // reopen file
    assert(dos_open_file("TOOLS.TXT", ACCESS_READ_WRITE, &fh) == 0);
    assert(dos_file_size(fh, &size) == 0);
    assert(dos_file_exists("TOOLS.TXT"));
    assert(!dos_file_is_eof(fh));
    assert(size == bytes);
    bytes = 0;
    assert(dos_read_file(fh, 1, buffer, &bytes) == 0);
    assert(bytes == 1);
    assert(!dos_file_is_eof(fh));
    bytes = size;
    assert(dos_read_file(fh, size, buffer, &bytes) == 0);
    assert(bytes = size - 1);
    assert(dos_file_is_eof(fh));
    printf("Reopen size, exists and eof working.\n");

    // Write at EOF (should extend file)
    const char* append = " - APPENDED";
    err = dos_write_file(fh, strlen(append), append, &bytes);
    assert(err == 0);
    assert(dos_file_is_eof(fh));
    printf("Extended working.\n");

    // Seek back
    dos_file_position_t pos;
    err = dos_move_file_pointer(fh, -12, FSEEK_END, &pos);
    assert(err == 0);
    assert(!dos_file_is_eof(fh));
    printf("Seek back working.\n");

    // Verify file grew
    assert(dos_file_size(fh, &size) == 0);
    assert(size == strlen(test_data) + bytes);
    assert(dos_close_file(fh) == 0);
    printf("File grow working.\n");

    // edge cases
    assert(dos_file_size(fh, &size) != 0);
    assert(dos_file_size(0xFFFF, &size) != 0);
    printf("Edge cases working.\n");

    printf("17. Testing file ext...\n");
    assert(dos_delete_file("TOOLS.TXT") == 0);
    assert(!dos_file_exists("TOOLS.TXT"));
    assert(!dos_file_exists(""));

    // file extension
    assert(strcmp(dos_file_ext("FILE.TXT"), "TXT") == 0);
    assert(strcmp(dos_file_ext("PROGRAM.EXE"), "EXE") == 0);
    // Multiple dots
    assert(strcmp(dos_file_ext("ARCHIVE.TAR.GZ"), "GZ") == 0);
    printf("Multiple dots 'ARCHIVE.TAR.GZ' -> 'GZ'\n");

    // Path with extensions
    assert(strcmp(dos_file_ext("C:\\DIR\\SUBDIR\\FILE.DAT"), "DAT") == 0);
    printf("Path with extension 'C:\\\\DIR\\\\FILE.DAT' -> 'DAT'\n");
    // No extension
    assert(dos_file_ext("NOEXT") == NULL);
    printf("No extension 'NOEXT' -> NULL\n");
    assert(dos_file_ext("C:\\PATH\\NOEXT") == NULL);
    printf("Path with no extension -> NULL\n");
    // Empty extension
    assert(dos_file_ext("FILE.") == NULL);
    printf("Empty extension 'FILE.' -> NULL\n");
    // Dot in directory but not filename
    assert(dos_file_ext("C:\\DIR.OLD\\FILE") == NULL);
    printf("Dot in directory only -> NULL\n");
    // Mixed case
    assert(strcmp(dos_file_ext("File.Txt"), "Txt") == 0);
    printf("Mixed case preserved 'File.Txt' -> 'Txt'\n");
    // Edge cases
    assert(dos_file_ext("") == NULL);
    printf("Empty string -> NULL\n");
    assert(dos_file_ext(NULL) == NULL);
    printf("NULL -> NULL\n");

    printf("ALL TESTS PASSED\n");
}

#endif
