#ifndef TEST_STRING_H
#define TEST_STRING_H

/*
#include "dos_string.h"
#include "dos_stdio.h"
#include "dos_assert.h"
*/
#include <string.h>
#include <stdio.h>
#include <assert.h>

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
    printf("=== Testing strrchr() C99 (large model) ===\n");

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
    char far* far_str = "far pointer test";
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

void test_string() {
    test_strlen();
    test_strcmp();
    test_strchr();
}

#endif
