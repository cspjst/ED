#ifndef TEST_STRING_H
#define TEST_STRING_H


#include "dos_string.h"
#include "dos_stdio.h"
#include "dos_assert.h"

void test_strlen() {

    assert(strlen("") == 0);
    assert(strlen("a") == 1);
    assert(strlen("hello") == 5);
    assert(strlen("1234567890") == 10);

    char far_str[] = "far test string";
    assert(strlen(far_str) == 15);

    printf("strlen() asm tests passed\n\n");
}

void test_strcmp() {

    // Equal strings
    assert(strcmp("", "") == 0);
    assert(strcmp("a", "a") == 0);
    assert(strcmp("hello", "hello") == 0);
    assert(strcmp("same string", "same string") == 0);

    printf("= ok\n");

    // s1 < s2 (lexicographically, ASCII order)
    assert(strcmp("a", "b") < 0);
    assert(strcmp("abc", "abd") < 0);
    assert(strcmp("hello", "world") < 0);
    assert(strcmp("test", "testing") < 0);      // prefix: shorter < longer
    assert(strcmp("ABC", "XYZ") < 0);            // uppercase ordering
    assert(strcmp("A", "a") < 0);                // ASCII: 'A' (65) < 'a' (97)

    printf("< ok\n");

    // s1 > s2
    assert(strcmp("b", "a") > 0);
    assert(strcmp("abd", "abc") > 0);
    assert(strcmp("world", "hello") > 0);
    assert(strcmp("testing", "test") > 0);       // prefix: longer > shorter
    assert(strcmp("XYZ", "ABC") > 0);
    assert(strcmp("a", "A") > 0);

    printf("> ok\n");

    // Empty vs non-empty
    assert(strcmp("", "x") < 0);
    assert(strcmp("x", "") > 0);

    printf("empty ok\n");

    // Single character comparisons
    assert(strcmp("x", "y") < 0);
    assert(strcmp("z", "a") > 0);

    printf("char ok\n");

    // Far-pointer explicit test (large model)
    char far1[] = "far string one";
    char far2[] = "far string two";
    char far3[] = "far string one";  // identical content, possibly different segment

    assert(strcmp(far1, far2) < 0);
    assert(strcmp(far2, far1) > 0);
    assert(strcmp(far1, far3) == 0);
    assert(strcmp(far1, far1) == 0);    // self-comparison

    printf("far ok\n");

    // Null terminator boundary: ensure comparison stops at first null// Null terminator boundary: ensure comparison stops at first null
    char buffer1[10] = "test";
    char buffer2[10] = "test";

    // Garbage AFTER null — should be ignored by strcmp
    buffer1[5] = 'X';
    buffer2[5] = 'Y';

    assert(strcmp(buffer1, buffer2) == 0);  // ✓ Only "test" compared

    printf("null ok\n");

    printf("strcmp() tests passed\n\n");
}

void test_strchr(void) {

    // Character found at start
    assert(strchr("hello", 'h') == "hello");

    // Character found in middle
    assert(strchr("hello", 'e') == "hello" + 1);
    assert(strchr("hello", 'l') == "hello" + 2);  // First 'l'

    // Character found at end (before null)
    assert(strchr("hello", 'o') == "hello" + 4);

    // Character not found
    assert(strchr("hello", 'x') == NULL);
    assert(strchr("hello", 'H') == NULL);  // Case sensitive

    // Empty string
    assert(strchr("", 'a') == NULL);
    assert(strchr("", '\0') == "");  // Null terminator is "found" at start

    // Search for null terminator explicitly
    assert(strchr("test", '\0') == "test" + 4);

    // Single character string
    assert(strchr("a", 'a') == "a");
    assert(strchr("a", 'b') == NULL);
    assert(strchr("a", '\0') == "a" + 1);

    // Special characters
    assert(strchr("a\rb\nc", '\r') == "a\rb\nc" + 1);
    assert(strchr("a\rb\nc", '\n') == "a\rb\nc" + 3);
    assert(strchr("tab\there", '\t') == "tab\there" + 3);

    // Byte values >127 (unsigned char comparison)
    assert(strchr("\xFF\x00", 0xFF) == "\xFF\x00");
    assert(strchr("\x80\x81\x82", 0x81) == "\x80\x81\x82" + 1);

    // Far pointer explicit test (large model)
    char far_str[] = "far pointer test";
    assert(strchr(far_str, 'p') == far_str + 4);
    assert(strchr(far_str, 'z') == NULL);

    // Return value is within original string bounds
    const char* txt = "boundary test";
    char* result = strchr(txt, 't');
    assert(result >= txt && result < txt + 13);  // Within bounds
    assert(*result == 't');

    printf("strchr() tests passed\n\n");
}

void test_strrchr(void) {

    // Character found at start (only occurrence)
    assert(strrchr("hello", 'h') == "hello");

    // Character found in middle (single occurrence)
    assert(strrchr("hello", 'e') == "hello" + 1);

    // Character found multiple times - should return LAST occurrence
    assert(strrchr("hello", 'l') == "hello" + 3);  // Second 'l', not first
    assert(strrchr("banana", 'a') == "banana" + 5);  // Last 'a'

    // Character found at end
    assert(strrchr("hello", 'o') == "hello" + 4);

    // Character not found
    assert(strrchr("hello", 'x') == NULL);
    assert(strrchr("hello", 'H') == NULL);  // Case sensitive

    // Empty string
    assert(strrchr("", 'a') == NULL);
    assert(strrchr("", '\0') == "");  // Null terminator found at start

    // Search for null terminator explicitly
    assert(strrchr("test", '\0') == "test" + 4);

    // Single character string
    assert(strrchr("a", 'a') == "a");
    assert(strrchr("a", 'b') == NULL);
    assert(strrchr("a", '\0') == "a" + 1);

    // Special characters
    assert(strrchr("a\rb\nc", '\r') == "a\rb\nc" + 1);
    assert(strrchr("a\rb\nc", '\n') == "a\rb\nc" + 3);
    assert(strrchr("tab\there", '\t') == "tab\there" + 3);

    // Multiple special chars - returns last
    assert(strrchr("a\rb\nc\r", '\r') == "a\rb\nc\r" + 5);  // Second '\r'

    // Byte values >127 (extended ASCII / OEM codepage)
    assert(strrchr("\xFF\x00", 0xFF) == "\xFF\x00");
    assert(strrchr("\x80\x81\x82\x81", 0x81) == "\x80\x81\x82\x81" + 3);  // Last occurrence

    // Far pointer explicit test (large model)
    char* far_str = "far pointer test";
    assert(strrchr(far_str, 't') == far_str + 12);  // Last 't' in "test"
    assert(strrchr(far_str, 'z') == NULL);

    // Return value is within original string bounds
    const char* txt = "boundary test";
    char* result = strrchr(txt, 't');
    assert(result >= txt && result <= txt + 13);  // Within bounds (may point to null)
    assert(*result == 't');

    // Consistency check: strrchr vs strchr for single occurrence
    const char* single = "abcdef";
    assert(strchr(single, 'c') == strrchr(single, 'c'));  // Only one 'c'

    // Difference check: strrchr vs strchr for multiple occurrences
    const char* multi = "ababa";
    assert(strchr(multi, 'a') != strrchr(multi, 'a'));  // First vs last 'a'
    assert(strchr(multi, 'a') == multi);                // First at start
    assert(strrchr(multi, 'a') == multi + 4);           // Last at end

    printf("strrchr() tests passed\n\n");
}

void test_strncmp() {

    // Equal strings within count
    assert(strncmp("hello", "hello", 5) == 0);
    assert(strncmp("same", "same", 4) == 0);
    assert(strncmp("", "", 0) == 0);
    assert(strncmp("", "", 10) == 0);
    printf("= ok\n");

    // Equal within limit, different after
    assert(strncmp("hello", "help", 3) == 0);
    assert(strncmp("hello", "help", 4) != 0);
    assert(strncmp("testing", "tested", 4) == 0);
    assert(strncmp("testing", "tested", 5) != 0);
    printf("boundary ok\n");

    // s1 < s2
    assert(strncmp("a", "b", 1) < 0);
    assert(strncmp("abc", "abd", 3) < 0);
    assert(strncmp("hello", "world", 5) < 0);
    assert(strncmp("test", "testing", 4) == 0);
    assert(strncmp("test", "testing", 8) < 0);
    assert(strncmp("ABC", "XYZ", 3) < 0);
    assert(strncmp("A", "a", 1) < 0);
    printf("< ok\n");

    // s1 > s2
    assert(strncmp("b", "a", 1) > 0);
    assert(strncmp("abd", "abc", 3) > 0);
    assert(strncmp("world", "hello", 5) > 0);
    assert(strncmp("testing", "test", 8) > 0);
    assert(strncmp("XYZ", "ABC", 3) > 0);
    assert(strncmp("a", "A", 1) > 0);
    printf("> ok\n");

    // count = 0 always equal
    assert(strncmp("abc", "xyz", 0) == 0);
    assert(strncmp("", "anything", 0) == 0);
    assert(strncmp(NULL, NULL, 0) == 0);
    printf("count=0 ok\n");

    // One string shorter than count
    assert(strncmp("hi", "hello", 5) != 0);
    assert(strncmp("hi", "hi!", 3) != 0);
    assert(strncmp("a", "abc", 10) != 0);
    printf("shorter ok\n");

    // Empty vs non-empty
    assert(strncmp("", "x", 1) < 0);
    assert(strncmp("x", "", 1) > 0);
    assert(strncmp("", "", 1) == 0);
    printf("empty ok\n");

    // NULL handling
    assert(strncmp(NULL, NULL, 10) == 0);
    assert(strncmp(NULL, "test", 4) < 0);
    assert(strncmp("test", NULL, 4) > 0);
    printf("null ok\n");

    // High-bit chars (unsigned compare)
    assert(strncmp("\xFF", "\x01", 1) > 0);
    assert(strncmp("\x01", "\xFF", 1) < 0);
    assert(strncmp("\x80\x81", "\x80\x82", 2) < 0);
    assert(strncmp("\xFF\x00", "\xFF\x01", 2) < 0);
    printf("high-bit ok\n");

    // Exact length boundary
    assert(strncmp("abc", "abc", 3) == 0);
    assert(strncmp("abc", "abd", 3) < 0);
    assert(strncmp("xyz", "xya", 3) > 0);
    printf("exact-len ok\n");

    // Large count exceeds strings
    assert(strncmp("a", "a", 100) == 0);
    assert(strncmp("a", "b", 100) < 0);
    assert(strncmp("short", "shorter", 1000) < 0);
    printf("large-count ok\n");

    // Null terminator stops comparison
    char buf1[10] = "test";
    char buf2[10] = "test";
    buf1[5] = 'X';
    buf2[5] = 'Y';
    assert(strncmp(buf1, buf2, 10) == 0);
    printf("null-boundary ok\n");

    // Far pointers (large model)
    char far1[] = "far strncmp test one";
    char far2[] = "far strncmp test two";
    char far3[] = "far strncmp test one";
    assert(strncmp(far1, far2, 20) < 0);
    assert(strncmp(far2, far1, 20) > 0);
    assert(strncmp(far1, far3, 20) == 0);
    assert(strncmp(far1, far1, 20) == 0);
    printf("far ok\n");

    // Special characters
    assert(strncmp("a\rb\nc", "a\rb\nd", 5) < 0);
    assert(strncmp("tab\there", "tab\tthere", 4) == 0);
    assert(strncmp("x\0y", "x\0z", 3) == 0);
    printf("special-chars ok\n");

    // Return value magnitude
    assert(strncmp("abc", "adc", 3) == ('b' - 'd'));
    assert(strncmp("xyz", "xya", 3) == ('z' - 'a'));
    printf("magnitude ok\n");

    printf("strncmp() tests passed\n\n");
}

void test_memset() {

    // Basic fill
    char buf1[10];
    memset(buf1, 'A', 10);
    assert(buf1[0] == 'A');
    assert(buf1[9] == 'A');
    printf("basic ok\n");

    // Empty fill (count = 0)
    char buf2[10] = "original";
    memset(buf2, 'X', 0);
    assert(strcmp(buf2, "original") == 0);
    printf("count=0 ok\n");

    // Fill with zero (common use case)
    char buf3[10] = "xxxxxxxxx";
    memset(buf3, 0, 10);
    assert(buf3[0] == 0);
    assert(buf3[9] == 0);
    assert(strlen(buf3) == 0);
    printf("zero-fill ok\n");

    // Fill with non-ASCII value
    unsigned char buf4[5];
    memset(buf4, 0xFF, 5);
    assert(buf4[0] == 0xFF);
    assert(buf4[4] == 0xFF);
    printf("high-bit ok\n");

    // NULL handling
    assert(memset(NULL, 'A', 10) == NULL);
    printf("null ok\n");

    // Return value is dest pointer
    char ret_buf[10];
    void* ret = memset(ret_buf, 'Z', 10);
    assert(ret == ret_buf);
    printf("return ok\n");

    // Single byte fill
    char single;
    memset(&single, 'X', 1);
    assert(single == 'X');
    printf("single-byte ok\n");

    // Partial buffer fill
    char partial[10] = "0000000000";
    memset(partial, 'Y', 5);
    assert(partial[0] == 'Y');
    assert(partial[4] == 'Y');
    assert(partial[5] == '0');  // Unchanged
    assert(partial[9] == '0');  // Unchanged
    printf("partial ok\n");

    // Fill with space character
    char spaces[20];
    memset(spaces, ' ', 20);
    for (int i = 0; i < 20; i++) assert(spaces[i] == ' ');
    printf("space ok\n");

    // Large fill (within buffer bounds)
    char large[100];
    memset(large, 0x42, 100);
    for (int i = 0; i < 100; i++) assert(large[i] == 0x42);
    printf("large-fill ok\n");

    // Far pointers (large model)
    char far_buf[50];
    memset(far_buf, 'F', 50);
    assert(far_buf[0] == 'F');
    assert(far_buf[49] == 'F');
    printf("far ok\n");

    // Struct initialization (common idiom)
    struct { int x; int y; char c; } s;
    memset(&s, 0, sizeof(s));
    assert(s.x == 0);
    assert(s.y == 0);
    assert(s.c == 0);
    printf("struct ok\n");

    // Array initialization
    int arr[10];
    memset(arr, 0, 10 * sizeof(int));
    for (int i = 0; i < 10; i++) assert(arr[i] == 0);
    printf("array ok\n");

    // Negative int ch (truncated to unsigned char)
    char neg_buf[5];
    memset(neg_buf, -1, 5);  // -1 = 0xFF as unsigned char
    assert(neg_buf[0] == (char)0xFF);
    assert(neg_buf[4] == (char)0xFF);
    printf("negative-ch ok\n");

    // Self-fill (dest overlaps with itself, always safe)
    char self[10] = "selftest";
    memset(self, 'S', 8);
    assert(self[0] == 'S');
    assert(self[7] == 'S');
    printf("self-fill ok\n");

    // Fill with newline/tab (special chars)
    char special[10];
    memset(special, '\n', 10);
    for (int i = 0; i < 10; i++) assert(special[i] == '\n');
    printf("special-chars ok\n");

    printf("memset() tests passed\n\n");
}

void test_memcmp() {

    // Equal memory regions
    assert(memcmp("hello", "hello", 5) == 0);
    assert(memcmp("", "", 0) == 0);
    assert(memcmp("same", "same", 4) == 0);
    printf("= ok\n");

    // lhs < rhs (byte-wise comparison)
    assert(memcmp("abc", "abd", 3) < 0);
    assert(memcmp("ABC", "XYZ", 3) < 0);
    assert(memcmp("a", "b", 1) < 0);
    printf("< ok\n");

    // lhs > rhs (byte-wise comparison)
    assert(memcmp("abd", "abc", 3) > 0);
    assert(memcmp("XYZ", "ABC", 3) > 0);
    assert(memcmp("b", "a", 1) > 0);
    printf("> ok\n");

    // count = 0 always equal
    assert(memcmp("abc", "xyz", 0) == 0);
    assert(memcmp("", "anything", 0) == 0);
    assert(memcmp(NULL, NULL, 0) == 0);
    printf("count=0 ok\n");

    // Binary data comparison (including null bytes)
    unsigned char bin1[5] = {0x00, 0x01, 0x02, 0x03, 0x04};
    unsigned char bin2[5] = {0x00, 0x01, 0x02, 0x03, 0x04};
    unsigned char bin3[5] = {0x00, 0x01, 0x02, 0x03, 0x05};
    assert(memcmp(bin1, bin2, 5) == 0);
    assert(memcmp(bin1, bin3, 5) < 0);
    assert(memcmp(bin3, bin1, 5) > 0);
    printf("binary ok\n");

    // High-bit characters (unsigned comparison)
    assert(memcmp("\xFF", "\x01", 1) > 0);  // 255 - 1 = +254
    assert(memcmp("\x01", "\xFF", 1) < 0);  // 1 - 255 = -254
    assert(memcmp("\x80\x81", "\x80\x82", 2) < 0);
    printf("high-bit ok\n");

    // NULL handling
    assert(memcmp(NULL, NULL, 10) == 0);
    assert(memcmp(NULL, "test", 4) < 0);
    assert(memcmp("test", NULL, 4) > 0);
    printf("null ok\n");

    // Partial comparison (first N bytes only)
    assert(memcmp("hello world", "hello there", 5) == 0);  // "hello" matches
    assert(memcmp("hello world", "hello there", 7) != 0);  // 'w' vs 't' differs
    printf("partial ok\n");

    // Single byte comparison
    assert(memcmp("a", "a", 1) == 0);
    assert(memcmp("a", "b", 1) < 0);
    assert(memcmp("b", "a", 1) > 0);
    printf("single-byte ok\n");

    // Large memory region comparison
    char large1[100];
    char large2[100];
    memset(large1, 0x42, 100);
    memset(large2, 0x42, 100);
    assert(memcmp(large1, large2, 100) == 0);
    large2[50] = 0x43;  // Change one byte
    assert(memcmp(large1, large2, 100) != 0);
    printf("large ok\n");

    // Far pointers (large model)
    char far1[] = "far memcmp test data one";
    char far2[] = "far memcmp test data two";
    char far3[] = "far memcmp test data one";
    assert(memcmp(far1, far2, 23) < 0);
    assert(memcmp(far2, far1, 23) > 0);
    assert(memcmp(far1, far3, 23) == 0);
    printf("far ok\n");

    // Struct comparison (binary equality)
    struct { int x; int y; char c; } s1 = {42, 99, 'Z'};
    struct { int x; int y; char c; } s2 = {42, 99, 'Z'};
    struct { int x; int y; char c; } s3 = {42, 99, 'A'};
    assert(memcmp(&s1, &s2, sizeof(s1)) == 0);
    assert(memcmp(&s1, &s3, sizeof(s1)) != 0);
    printf("struct ok\n");

    // Array comparison
    int arr1[5] = {1, 2, 3, 4, 5};
    int arr2[5] = {1, 2, 3, 4, 5};
    int arr3[5] = {1, 2, 3, 4, 6};
    assert(memcmp(arr1, arr2, 5 * sizeof(int)) == 0);
    assert(memcmp(arr1, arr3, 5 * sizeof(int)) < 0);
    printf("array ok\n");

    // Difference occurs at last byte
    char last1[10] = "aaaaaaaaa";
    char last2[10] = "aaaaaaaab";
    assert(memcmp(last1, last2, 8) == 0);
    assert(memcmp(last1, last2, 9) < 0);
    printf("last-byte ok\n");

    // Difference occurs at first byte
    char first1[10] = "aaaaaaaaa";
    char first2[10] = "baaaaaaaa";
    assert(memcmp(first1, first2, 10) < 0);
    assert(memcmp(first1, first2, 1) < 0);
    printf("first-byte ok\n");

    // Self-comparison (always equal)
    char self[10] = "selftest";
    assert(memcmp(self, self, 10) == 0);
    printf("self ok\n");

    // Return value magnitude (actual byte difference)
    assert(memcmp("abc", "adc", 3) == ('b' - 'd'));
    assert(memcmp("xyz", "xya", 3) == ('z' - 'a'));
    printf("magnitude ok\n");

    printf("memcmp() tests passed\n\n");
}

void test_string() {
    test_strlen();
    test_strcmp();
    test_strchr();
    test_strncmp();
    test_memset();
    test_memcmp();
}

#endif
