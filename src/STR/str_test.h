#ifndef STR_TEST_H
#define STR_TEST_H

#include "str.h"
#include <assert.h>
#include <stdio.h>

// Test str_make_view()
static void test_make_view(void) {

    str_view_t v;

    // Empty string
    v = str_make_view("");
    assert(v.begin != NULL);
    assert(v.end == v.begin);
    assert(str_view_size(v) == 0);

    // Single char
    v = str_make_view("A");
    assert(str_view_size(v) == 1);
    assert(*v.begin == 'A');
    assert(*v.end == '\0');

    // Normal string
    v = str_make_view("HELLO");
    assert(str_view_size(v) == 5);
    assert(v.begin[0] == 'H');
    assert(v.end[-1] == 'O');
    assert(*v.end == '\0');
}

// Test str_view_size()
static void test_view_size(void) {
    str_view_t v = str_make_view("12345");
    assert(str_view_size(v) == 5);

    v.end = v.begin;  // Empty view
    assert(str_view_size(v) == 0);

    v.end = v.begin + 1;  // Single char
    assert(str_view_size(v) == 1);
}

// Test str_var()
static void test_var(void) {
    str_view_t v = str_make_view("TEST");
    char buf[10];

    // Exact fit (needs space for null terminator)
    assert(str_var(v, buf, 5) == 4);  // 4 chars + null
    assert(strcmp(buf, "TEST") == 0);

    // Too small buffer
    assert(str_var(v, buf, 4) == 0);  // Needs 5 bytes total

    // Oversized buffer
    assert(str_var(v, buf, 80) == 4);
    assert(buf[4] == '\0');

    // Empty view
    str_view_t empty = str_make_view("");
    assert(str_var(empty, buf, 10) == 0);
    assert(buf[0] == '\0');
}

static void test_token(void) {
    char digits[] = "0123456789";  // Proper C string

    // Create view of "123" (indices 1,2,3)
    str_view_t tok = str_token(&digits[1], &digits[4]);
    // str_token(begin, end) expects HALF-OPEN range [begin, end):
        //   • begin = pointer to FIRST character INCLUDED in view
        //   • end   = pointer to FIRST character EXCLUDED from view (one past last)
        //   • Size = end - begin (pointer arithmetic)
        //   • Valid indices: [0, size) → begin[0] .. begin[size-1]
        //   • Last character: *(end - 1) or end[-1]
    assert(str_view_size(tok) == 3);     // 4 - 1 = 3 ✓
    assert(tok.begin[0] == '1');          // First char ✓
    assert(tok.begin[1] == '2');          // Second char ✓
    assert(tok.begin[2] == '3');          // Third char ✓
    assert(tok.end[-1] == '3');           // Last char in view ✓
    assert(tok.end == &digits[4]);        // end points to '4' (one past '3') ✓
}

// Test str_first_char()
static void test_first_char(void) {
    str_view_t charset = str_make_view("ABC");

    // Found
    assert(str_first_char(charset, 'A') == charset.begin + 0);
    assert(str_first_char(charset, 'B') == charset.begin + 1);
    assert(str_first_char(charset, 'C') == charset.begin + 2);

    // Not found
    assert(str_first_char(charset, 'D') == NULL);
    assert(str_first_char(charset, '\0') == NULL);

    // Empty charset
    str_view_t empty = str_make_view("");
    assert(str_first_char(empty, 'A') == NULL);
}

static void test_match(void) {
    str_view_t subject;
    str_view_t pat;
    const char* next;

    // Prefix match
    subject = str_make_view("HELLO");
    pat = str_make_view("HEL");
    next = str_match(subject, pat);
    assert(next != NULL);
    assert(next == subject.begin + 3);

    // Full match (entire string)
    subject = str_make_view("HELLO");
    pat = str_make_view("HELLO");
    next = str_match(subject, pat);
    assert(next != NULL);
    assert(next == subject.end);

    // Mismatch at start
    subject = str_make_view("HELLO");
    pat = str_make_view("WORLD");
    next = str_match(subject, pat);
    assert(next == NULL);

    // Empty pattern matches immediately (SNOBOL semantics)
    subject = str_make_view("HELLO");
    pat = str_make_view("");
    next = str_match(subject, pat);
    assert(next != NULL);
    assert(next == subject.begin);

    // Match at end AFTER cursor advancement (simulated)
    subject = str_make_view("HELLO WORLD");
    subject.begin += 6;  // Manually advance past "HELLO " to 'W'
    pat = str_make_view("WORLD");
    next = str_match(subject, pat);
    assert(next != NULL);
    assert(next == subject.end);  // Points to null terminator
}

// Test str_span()
static void test_span(void) {
    const str_view_t DIGITS = str_make_view("0123456789");

    str_view_t subject;
    const char* next;

    // Normal span (digits)
    subject = str_make_view("123abc");
    next = str_span(subject, DIGITS);
    assert(next != NULL);
    assert(next - subject.begin == 3);
    assert(*next == 'a');

    // Span to end of view
    subject = str_make_view("999");
    next = str_span(subject, DIGITS);
    assert(next != NULL);
    assert(next == subject.end);

    // Failure on first char (no match)
    subject = str_make_view("abc");
    next = str_span(subject, DIGITS);
    assert(next == NULL);

    // Empty subject
    subject = str_make_view("");
    next = str_span(subject, DIGITS);
    assert(next == NULL);

    // Single char match
    subject = str_make_view("5");
    next = str_span(subject, DIGITS);
    assert(next != NULL);
    assert(next == subject.end);

    // Charset with single char
    str_view_t single_char = str_make_view("X");
    subject = str_make_view("XXXy");
    next = str_span(subject, single_char);
    assert(next != NULL);
    assert(next - subject.begin == 3);
    assert(*next == 'y');
}

static void test_any(void) {
    const str_view_t DIGITS = str_make_view("0123456789");
    const str_view_t SIGNS = str_make_view("+-");

    str_view_t subject;
    const char* next;

    // Match digit '5' from STR_DIGITS
    subject = str_make_view("5abc");
    next = str_any(subject, DIGITS);
    assert(next != NULL);
    assert(next == subject.begin + 1);  // Points after '5'
    assert(*subject.begin == '5');

    // Match sign '-' from STR_SIGNS
    subject = str_make_view("-123");
    next = str_any(subject, SIGNS);
    assert(next != NULL);
    assert(next == subject.begin + 1);
    assert(*subject.begin == '-');

    // Fail: char not in charset ('x' not in digits)
    subject = str_make_view("x123");
    next = str_any(subject, DIGITS);
    assert(next == NULL);

    // Fail: empty subject
    subject = str_make_view("");
    next = str_any(subject, DIGITS);
    assert(next == NULL);

    // Fail: empty charset
    str_view_t empty_charset = str_make_view("");
    subject = str_make_view("5");
    next = str_any(subject, empty_charset);
    assert(next == NULL);
}

static void test_eat(void) {
    str_view_t cmd;
    int ate;

    // Test 1: Eat single space from " x"
    cmd = str_make_view(" x");
    ate = str_eat(&cmd, str_any(cmd, str_make_view(" ")));
    assert(ate == 1);
    assert(*cmd.begin == 'x');
    assert(cmd.end[-1] == 'x');

    // Test 2: Eat fails on mismatch ("x" not in space charset)
    cmd = str_make_view("x");
    ate = str_eat(&cmd, str_any(cmd, str_make_view(" ")));
    assert(ate == 0);
    assert(*cmd.begin == 'x');  // Cursor unchanged

    // Test 3: Eat with str_span (1+ digits)
    cmd = str_make_view("123abc");
    ate = str_eat(&cmd, str_span(cmd, str_make_view("0123456789")));
    assert(ate == 1);
    assert(*cmd.begin == 'a');
    assert(str_view_size(cmd) == 3);  // "abc" remains

    // Test 4: Eat fails on empty subject
    cmd = str_make_view("");
    ate = str_eat(&cmd, str_any(cmd, str_make_view(" ")));
    assert(ate == 0);

    // Test 5: str_skip idiom (0+ whitespace) using str_eat + str_any loop
    cmd = str_make_view("  \t-23L");
    while (str_eat(&cmd, str_any(cmd, str_make_view(" \t"))))
        ;
    assert(*cmd.begin == '-');  // All whitespace consumed

    // Test 6: str_skip on no-whitespace input (no-op)
    cmd = str_make_view("-23L");
    while (str_eat(&cmd, str_any(cmd, str_make_view(" \t"))))
        ;
    assert(*cmd.begin == '-');  // Cursor unchanged (no whitespace to skip)
}

static void test_skip(void) {
    str_view_t cmd;

    // Test 1: Skip multiple spaces → cursor advances past all
    cmd = str_make_view("   x");
    str_skip(cmd, str_make_view(" "));
    assert(*cmd.begin == 'x');
    assert(str_view_size(cmd) == 1);

    // Test 2: Skip mixed whitespace (space + tab) → cursor at 'x'
    cmd = str_make_view(" \t x");
    str_skip(cmd, str_make_view(" \t"));
    assert(*cmd.begin == 'x');

    // Test 3: No-op when first char not in charset ("x" not whitespace)
    cmd = str_make_view("x  ");
    str_skip(cmd, str_make_view(" "));
    assert(*cmd.begin == 'x');  // Cursor unchanged

    // Test 4: Skip digits from "123abc" → cursor at 'a'
    cmd = str_make_view("123abc");
    str_skip(cmd, str_make_view("0123456789"));
    assert(*cmd.begin == 'a');

    // Test 5: Skip nothing from empty string (safe no-op)
    cmd = str_make_view("");
    str_skip(cmd, str_make_view(" "));
    assert(cmd.begin == cmd.end);  // Still empty

    // Test 6: Skip all characters (entire view consumed)
    cmd = str_make_view("123");
    str_skip(cmd, str_make_view("0123456789"));
    assert(cmd.begin == cmd.end);  // Fully consumed
}

// Master test runner
void str_test(void) {
    test_make_view();
    test_view_size();
    test_var();
    test_token();
    test_first_char();
    test_match();
    test_span();
    test_any();
    test_eat();
    test_skip();
    printf("All string primitive tests passed\n");
}

#endif
