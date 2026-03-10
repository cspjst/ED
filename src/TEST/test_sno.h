#ifndef TEST_SNO_H
#define TEST_SNO_H

#include "../SNO/sno.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>

void test_bind(void) {
    view_t v = bind("TEST");
    assert(v.begin && v.end && size(v) == 4);

    v = bind("");
    assert(v.begin && v.begin == v.end && size(v) == 0);

    v = bind(NULL);
    assert(!v.begin && !v.end && size(v) == 0);
}

void test_view(void) {
    char buf[] = "HELLO";

    view_t v = view(&buf[1], &buf[4]);
    assert(v.begin == &buf[1] && v.end == &buf[4] && size(v) == 3);

    v = view(buf, buf);
    assert(size(v) == 0);

    v = view(NULL, buf);
    assert(!v.begin && v.end == buf);

    v = view(buf, NULL);
    assert(v.begin == buf && !v.end);

    v = view(NULL, NULL);
    assert(!v.begin && !v.end);

    v = view(&buf[3], &buf[1]);
    assert(v.begin == &buf[3] && v.end == &buf[1]);
}

void test_size(void) {
    assert(size(bind("ABC")) == 3);
    assert(size(view("X", "X")) == 0);
    assert(size(view(NULL, "TEST")) == 0);
    assert(size(view("TEST", NULL)) == 0);
    assert(size(view(NULL, NULL)) == 0);

    char s[] = "TEST";
    int sz = size(view(&s[3], &s[1]));
    assert(sz >= 0);

    assert(size(bind(NULL)) == 0);
}

void test_str(void) {
    char input[] = "HELLO";
    view_t sub = bind(input);
    assert(str(&sub, "HEL") && sub.begin == input + 3 && *sub.begin == 'L');

    sub = bind("TEST");
    cursor_t orig = sub.begin;
    assert(!str(&sub, "FAIL") && sub.begin == orig);

    sub = bind("ANY");
    orig = sub.begin;
    assert(str(&sub, "") && sub.begin == orig);  // Empty string always matches

    sub = bind("HI");
    orig = sub.begin;
    assert(!str(&sub, "HELLO") && sub.begin == orig);  // Pattern longer than subject

    sub = view(NULL, NULL);
    assert(!str(&sub, "X") && !sub.begin && !sub.end);  // NULL subject

    sub = bind("SAFE");
    orig = sub.begin;
    assert(!str(&sub, NULL) && sub.begin == orig);  // NULL pattern is error

    sub = view(&input[3], &input[1]);  // Inverted view (begin > end)
    orig = sub.begin;
    assert(!str(&sub, "X") && sub.begin == orig);

    sub = bind("");
    orig = sub.begin;
    assert(!str(&sub, "X") && sub.begin == orig);  // Empty subject, non-empty pattern

    sub = bind("Hello");
    orig = sub.begin;
    assert(!str(&sub, "HELLO") && sub.begin == orig);  // Case-sensitive mismatch

    // Partial match at end of subject should fail
    sub = bind("AB");
    orig = sub.begin;
    assert(!str(&sub, "ABC") && sub.begin == orig);

    // Match exactly to end of subject should succeed
    sub = bind("END");
    assert(str(&sub, "END") && sub.begin == sub.end);
}

void test_chr(void) {
    view_t sub;
    cursor_t orig;

    // Basic match: single character at current position
    char buf1[] = "ABC";
    sub = bind(buf1);
    orig = sub.begin;
    assert(chr(&sub, 'A') && sub.begin == orig + 1 && *sub.begin == 'B');

    // Match at middle of string
    sub = bind(buf1);
    sub.begin = &buf1[1];  // Start at 'B'
    assert(chr(&sub, 'B') && sub.begin == &buf1[2] && *sub.begin == 'C');

    // Match at end of string (cursor advances to end)
    sub = bind(buf1);
    sub.begin = &buf1[2];  // Start at 'C'
    assert(chr(&sub, 'C') && sub.begin == sub.end);

    // Failure: character mismatch at current position
    char buf2[] = "XYZ";
    sub = bind(buf2);
    orig = sub.begin;
    assert(!chr(&sub, 'A') && sub.begin == orig);  // Cursor unchanged on failure

    // Failure: empty subject (cursor at end)
    char buf3[] = "";
    sub = bind(buf3);
    orig = sub.begin;
    assert(!chr(&sub, 'X') && sub.begin == orig);

    // Failure: NULL subject
    assert(!chr(NULL, 'X'));

    // Failure: subject with NULL begin
    sub = view(NULL, buf1);
    orig = sub.begin;
    assert(!chr(&sub, 'X') && sub.begin == orig);

    // Case sensitivity: 'a' != 'A'
    char buf4[] = "aA";
    sub = bind(buf4);
    orig = sub.begin;
    assert(chr(&sub, 'a') && *sub.begin == 'A');

    sub = bind(buf4);
    orig = sub.begin;
    assert(!chr(&sub, 'A') && sub.begin == orig && *sub.begin == 'a');

    // Special characters: match space, punctuation, etc.
    char buf5[] = " !@#";
    sub = bind(buf5);
    assert(chr(&sub, ' ') && *sub.begin == '!');

    sub = bind(buf5);
    sub.begin++;  // Skip space
    assert(chr(&sub, '!') && *sub.begin == '@');

    // Numeric characters
    char buf6[] = "012";
    sub = bind(buf6);
    assert(chr(&sub, '0') && chr(&sub, '1') && chr(&sub, '2') && sub.begin == sub.end);

    // Atomic rollback: partial pattern fails, cursor unchanged
    char buf7[] = "TEST";
    sub = bind(buf7);
    orig = sub.begin;
    // Try to match 'X' at start of "TEST" - should fail, cursor unchanged
    assert(!chr(&sub, 'X') && sub.begin == orig && *sub.begin == 'T');

    // Chaining: multiple chr() calls in sequence
    char buf8[] = "CAT";
    sub = bind(buf8);
    assert(chr(&sub, 'C') && chr(&sub, 'A') && chr(&sub, 'T') && sub.begin == sub.end);

    // Chaining with failure in middle: cursor should stop at failure point
    char buf9[] = "DOG";
    sub = bind(buf9);
    assert(chr(&sub, 'D') && chr(&sub, 'O'));
    orig = sub.begin;  // Now at 'G'
    assert(!chr(&sub, 'X') && sub.begin == orig);  // Failure leaves cursor at 'G'

    // View with offset (substring view)
    char buf10[] = "PREFIXTARGET";
    sub = view(&buf10[6], &buf10[12]);  // "TARGET"
    assert(chr(&sub, 'T') && chr(&sub, 'A') && chr(&sub, 'R'));

    // Inverted view (begin > end) should fail safely
    char buf11[] = "X";
    sub = view(&buf11[1], &buf11[0]);
    orig = sub.begin;
    assert(!chr(&sub, 'X') && sub.begin == orig);

    // Match null character (if present in buffer)
    char buf12[] = "A\0B";  // Embedded null
    sub = view(buf12, buf12 + 3);  // Explicit view includes null
    assert(chr(&sub, 'A') && chr(&sub, '\0') && *sub.begin == 'B');
}

void test_at(void) {
    view_t original, cursor;
    unsigned int pos;

    // Position at start = 1 (SNOBOL 1-based)
    original = bind("TEST");
    cursor = original;
    pos = at(&original, cursor.begin);
    assert(pos == 1);

    // Position after advancing
    str(&cursor, "TE");
    pos = at(&original, cursor.begin);
    assert(pos == 3);

    // Position at end = length + 1
    cursor = bind("ABC");
    view_t b = bind("ABC");
    cursor.begin = cursor.end;  // Advance to end
    pos = at(&b, cursor.begin);
    assert(pos == 4);

    // Empty string: start and end both at position 1
    original = bind("");
    pos = at(&original, original.begin);
    assert(pos == 1);
    pos = at(&original, original.end);
    assert(pos == 1);

    // NULL safety
    assert(at(NULL, original.begin) == 0);
    assert(at(&original, NULL) == 0);

    // Out of bounds safety
    char buf[] = "X";
    original = bind(buf);
    assert(at(&original, buf - 1) == 0);  // Before buffer
    assert(at(&original, buf + 2) == 0);  // After buffer

    // View with offset (substring view)
    char text[] = "PREFIXHELLOSUFFIX";
    view_t full = bind(text);
    view_t sub = view(&text[6], &text[11]);  // "HELLO"
    pos = at(&full, sub.begin);
    assert(pos == 7);  // 'H' is at position 7 in original
    pos = at(&full, sub.end);
    assert(pos == 12); // After 'O' is position 12
}

void test_len(void) {
    view_t sub;
    cursor_t orig;
    char buf[64];

    // Basic: advance cursor by exact length
    char buf1[] = "ABCDEFGHIJ";
    sub = bind(buf1);
    orig = sub.begin;
    assert(len(&sub, 5) && sub.begin == orig + 5 && *sub.begin == 'F');

    // Composition: len() to skip prefix, var() captures remainder
    // Note: var() captures [current_cursor, end), not the len() match itself
    char buf2[] = "1234567890EXTRA";
    sub = bind(buf2);
    assert(len(&sub, 10) && var(&sub, buf, sizeof(buf)) &&
           strcmp(buf, "EXTRA") == 0 && sub.begin == sub.end);

    // Composition: str() to match known prefix, var() captures payload
    char buf3[] = "HEADER:payload data";
    sub = bind(buf3);
    assert(str(&sub, "HEADER:") && var(&sub, buf, sizeof(buf)) &&
           strcmp(buf, "payload data") == 0);

    // Capturing a fixed-width match requires manual extraction or temp view
    // Pattern: save start, advance with len(), copy the span manually
    char buf4[] = "1234567890EXTRA";
    sub = bind(buf4);
    cursor_t start = sub.begin;
    assert(len(&sub, 10));  // Advance cursor by 10
    size_t matched = sub.begin - start;
    assert(matched < sizeof(buf));
    memcpy(buf, start, matched);
    buf[matched] = '\0';
    assert(strcmp(buf, "1234567890") == 0);
    assert(str(&sub, "EXTRA"));  // Continue parsing remainder

    // Failure: insufficient characters, cursor unchanged
    char buf5[] = "SHORT";
    sub = bind(buf5);
    orig = sub.begin;
    assert(!len(&sub, 10) && sub.begin == orig);

    // Edge: length=0 always succeeds (matches empty, cursor unchanged)
    char buf6[] = "TEST";
    sub = bind(buf6);
    orig = sub.begin;
    assert(len(&sub, 0) && sub.begin == orig && *sub.begin == 'T');

    // Edge: exact remaining length succeeds and reaches EOF
    char buf7[] = "EXACT";
    sub = bind(buf7);
    assert(len(&sub, 5) && sub.begin == sub.end);

    // Edge: length=0 on empty subject succeeds
    char buf8[] = "";
    sub = bind(buf8);
    orig = sub.begin;
    assert(len(&sub, 0) && sub.begin == orig);

    // Failure: length=0 on NULL subject
    assert(!len(NULL, 0));

    // Failure: NULL subject with non-zero length
    assert(!len(NULL, 5));

    // Failure: subject with NULL begin
    sub = view(NULL, buf1);
    orig = sub.begin;
    assert(!len(&sub, 1) && sub.begin == orig);

    // Inverted view (begin > end): len() should fail safely
    char buf11[] = "X";
    sub = view(&buf11[1], &buf11[0]);
    orig = sub.begin;
    assert(!len(&sub, 1) && sub.begin == orig);

    // View with offset: len() operates relative to current begin
    char buf12[] = "PREFIX12345SUFFIX";
    sub = view(&buf12[6], &buf12[11]);  // "12345"
    assert(len(&sub, 3) && sub.begin == &buf12[9]);
    assert(len(&sub, 2) && sub.begin == sub.end);

    // Large length value (overflow safety): should fail, not wrap
    sub = bind("SHORT");
    orig = sub.begin;
    assert(!len(&sub, UINT_MAX) && sub.begin == orig);

    // Chaining: len() failures don't affect subsequent attempts
    char buf13[] = "ABC";
    sub = bind(buf13);
    orig = sub.begin;
    assert(!len(&sub, 10));  // Fails, cursor unchanged
    assert(sub.begin == orig);
    assert(len(&sub, 1) && chr(&sub, 'A'));  // Subsequent match still works

    // Real-world: parse fixed-width record "ID:12345NAME:JOHN  "
    char record[] = "ID:12345NAME:JOHN  ";
    sub = bind(record);
    assert(str(&sub, "ID:"));
    start = sub.begin;
    assert(len(&sub, 5));
    memcpy(buf, start, 5); buf[5] = '\0';
    assert(strcmp(buf, "12345") == 0);

    assert(str(&sub, "NAME:"));
    start = sub.begin;
    assert(len(&sub, 6));
    memcpy(buf, start, 6); buf[6] = '\0';
    assert(strncmp(buf, "JOHN  ", 6) == 0);

    printf("len() tests pass!\n");
}



void test_any(void) {
    view_t sub;
    cursor_t orig;

    char buf1[] = "XYZ";
    sub = bind(buf1);
    orig = sub.begin;
    assert(any(&sub, bind("XYZ")) && sub.begin == orig + 1 && *sub.begin == 'Y');

    char buf2[] = "!5";
    sub = bind(buf2);
    orig = sub.begin;
    assert(any(&sub, bind("!@#$%")) && sub.begin == orig + 1 && *sub.begin == '5');

    char buf3[] = "X";
    sub = bind(buf3);
    orig = sub.begin;
    assert(!any(&sub, bind("ABC")) && sub.begin == orig);

    sub = bind("");
    orig = sub.begin;
    assert(!any(&sub, bind("A")) && sub.begin == orig);

    char buf4[] = "A";
    sub = bind(buf4);
    orig = sub.begin;
    assert(!any(&sub, bind("")) && sub.begin == orig);

    assert(!any(NULL, bind("A")));

    sub = view(NULL, NULL);
    assert(!any(&sub, bind("A")) && !sub.begin && !sub.end);

    char buf5[] = "A";
    sub = view(buf5, NULL);
    orig = sub.begin;
    assert(!any(&sub, bind("A")) && sub.begin == orig && sub.end == NULL);

    char buf6[] = "A";
    sub = view(NULL, buf6);
    assert(!any(&sub, bind("A")) && !sub.begin && sub.end == buf6);

    char buf7[] = "a";
    sub = bind(buf7);
    orig = sub.begin;
    assert(!any(&sub, bind("A")) && sub.begin == orig);

    char buf8[] = "AAA";
    sub = bind(buf8);
    orig = sub.begin;
    assert(any(&sub, bind("A")) && sub.begin == orig + 1 && *sub.begin == 'A');

    char buf9[] = "123abc";
    sub = bind(buf9);
    int count = 0;
    while (any(&sub, bind("0123456789"))) count++;
    assert(count == 3 && strncmp(sub.begin, "abc", 3) == 0);

    char buf10[] = "Z";
    sub = bind(buf10);
    assert(any(&sub, bind("Z")) && sub.begin == sub.end);

    char buf11[] = "!!!END";
    sub = bind(buf11);
    orig = sub.begin;
    assert(any(&sub, bind("!")) && sub.begin == orig + 1 && *sub.begin == '!');

    char buf12[] = "!!!END";
    sub = view(&buf12[2], &buf12[0]);
    orig = sub.begin;
    assert(!any(&sub, bind("!")) && sub.begin == orig);
}

void test_notany(void) {
    view_t sub;
    cursor_t orig;

    char buf1[] = "X9Z";
    sub = bind(buf1);
    orig = sub.begin;
    assert(notany(&sub, bind("012345678")) && sub.begin == orig + 1 && *sub.begin == '9');

    char buf2[] = "A";
    sub = bind(buf2);
    orig = sub.begin;
    assert(notany(&sub, bind("")) && sub.begin == orig + 1 && sub.begin == sub.end);

    char buf3[] = "5";
    sub = bind(buf3);
    orig = sub.begin;
    assert(!notany(&sub, bind("0123456789")) && sub.begin == orig);

    char buf4[] = "";
    sub = bind(buf4);
    orig = sub.begin;
    assert(!notany(&sub, bind("A")) && sub.begin == orig);

    char buf5[] = "Z";
    sub = bind(buf5);
    assert(notany(&sub, bind("ABC")) && sub.begin == sub.end);
    orig = sub.begin;
    assert(!notany(&sub, bind("A")) && sub.begin == orig);

    assert(!notany(NULL, bind("A")));

    sub = view(NULL, NULL);
    assert(!notany(&sub, bind("A")) && !sub.begin && !sub.end);

    char buf6[] = "A";
    sub = view(buf6, NULL);
    orig = sub.begin;
    assert(!notany(&sub, bind("A")) && sub.begin == orig && sub.end == NULL);

    char buf7[] = "A";
    sub = view(NULL, buf7);
    assert(!notany(&sub, bind("A")) && !sub.begin && sub.end == buf7);

    char buf8[] = "a";
    sub = bind(buf8);
    orig = sub.begin;
    assert(!notany(&sub, bind("a")) && sub.begin == orig);

    char buf9[] = "FIELD,REST";
    sub = bind(buf9);
    while (notany(&sub, bind(","))) ;
    assert(*sub.begin == ',');

    char buf10[] = "FIELD,REST";
    sub = view(&buf10[2], &buf10[0]);
    orig = sub.begin;
    assert(!notany(&sub, bind(",")) && sub.begin == orig);
}

void test_span(void) {
    view_t sub;
    cursor_t orig;

    char buf1[] = "A123";
    sub = bind(buf1);
    orig = sub.begin;
    assert(span(&sub, bind("A")) && sub.begin == orig + 1 && *sub.begin == '1');

    char buf2[] = "12345abc";
    sub = bind(buf2);
    orig = sub.begin;
    assert(span(&sub, bind("0123456789")) && sub.begin == orig + 5 && *sub.begin == 'a');

    char buf3[] = "999";
    sub = bind(buf3);
    assert(span(&sub, bind("0123456789")) && sub.begin == sub.end);

    char buf4[] = "X123";
    sub = bind(buf4);
    orig = sub.begin;
    assert(!span(&sub, bind("0123456789")) && sub.begin == orig);

    char buf5[] = "";
    sub = bind(buf5);
    orig = sub.begin;
    assert(!span(&sub, bind("A")) && sub.begin == orig);

    char buf6[] = "A";
    sub = bind(buf6);
    orig = sub.begin;
    assert(!span(&sub, bind("")) && sub.begin == orig);

    char buf7[] = "aaaabbb";
    sub = bind(buf7);
    orig = sub.begin;
    assert(span(&sub, bind("a")) && sub.begin == orig + 4 && *sub.begin == 'b');

    char buf8[] = "321cba";
    sub = bind(buf8);
    orig = sub.begin;
    assert(span(&sub, bind("abc123")) && sub.begin == orig + 6 && sub.begin == sub.end);

    char buf9[] = "aAaA";
    sub = bind(buf9);
    orig = sub.begin;
    assert(span(&sub, bind("a")) && sub.begin == orig + 1 && *sub.begin == 'A');

    char buf10[] = "123,456,789";
    sub = bind(buf10);
    assert(span(&sub, bind("0123456789")) && str(&sub, ",") && span(&sub, bind("0123456789")) && *sub.begin == ',');

    assert(!span(NULL, bind("A")));

    sub = view(NULL, NULL);
    assert(!span(&sub, bind("A")) && !sub.begin && !sub.end);

    char buf11[] = "A";
    sub = view(buf11, NULL);
    orig = sub.begin;
    assert(!span(&sub, bind("A")) && sub.begin == orig && sub.end == NULL);

    char buf12[] = "A";
    sub = view(NULL, buf12);
    assert(!span(&sub, bind("A")) && !sub.begin && sub.end == buf12);

    char buf13[] = "Z";
    sub = bind(buf13);
    assert(span(&sub, bind("Z")) && sub.begin == sub.end);

    char buf14[] = "AAA";
    sub = bind(buf14);
    orig = sub.begin;
    assert(span(&sub, bind("A")) && sub.begin == orig + 3);
    sub = bind(buf14);
    orig = sub.begin;
    assert(span(&sub, bind("XYZA")) && sub.begin == orig + 3);

    char buf15[] = "AAA";
    sub = view(&buf15[2], &buf15[0]);
    orig = sub.begin;
    assert(!span(&sub, bind("A")) && sub.begin == orig);
}

void test_brk(void) {
    view_t sub;
    cursor_t orig;

    char buf1[] = "FIELD,REST";
    sub = bind(buf1);
    orig = sub.begin;
    assert(brk(&sub, bind(",")) && sub.begin == orig + 5 && *sub.begin == ',');

    char buf2[] = ",REST";
    sub = bind(buf2);
    orig = sub.begin;
    assert(brk(&sub, bind(",")) && sub.begin == orig && *sub.begin == ',');

    char buf3[] = "FIELD";
    sub = bind(buf3);
    assert(brk(&sub, bind(",")) && sub.begin == sub.end);

    char buf4[] = "";
    sub = bind(buf4);
    assert(brk(&sub, bind(",")) && sub.begin == sub.end);

    char buf5[] = "TEST";
    sub = bind(buf5);
    assert(brk(&sub, bind("")) && sub.begin == sub.end);

    char buf6[] = "aaaabbb";
    sub = bind(buf6);
    orig = sub.begin;
    assert(brk(&sub, bind("b")) && sub.begin == orig + 4 && *sub.begin == 'b');

    char buf7[] = "aAaA";
    sub = bind(buf7);
    orig = sub.begin;
    assert(brk(&sub, bind("A")) && sub.begin == orig + 1 && *sub.begin == 'A');

    char buf8[] = "123,456,789";
    sub = bind(buf8);
    assert(brk(&sub, bind(",")) && any(&sub, bind(",")) && brk(&sub, bind(",")) && any(&sub, bind(",")) && brk(&sub, bind(",")) && sub.begin == sub.end);

    assert(!brk(NULL, bind(",")));

    sub = view(NULL, NULL);
    assert(!brk(&sub, bind(",")) && !sub.begin && !sub.end);

    char buf9[] = "A";
    sub = view(buf9, NULL);
    orig = sub.begin;
    assert(!brk(&sub, bind(",")) && sub.begin == orig && sub.end == NULL);

    char buf10[] = "A";
    sub = view(NULL, buf10);
    assert(!brk(&sub, bind(",")) && !sub.begin && sub.end == buf10);

    char buf11[] = "Z";
    sub = bind(buf11);
    assert(brk(&sub, bind("A")) && sub.begin == sub.end);

    char buf12[] = "ABcDE";
    sub = bind(buf12);
    orig = sub.begin;
    assert(brk(&sub, bind("c")) && sub.begin == orig + 2 && *sub.begin == 'c');
}

void test_skip(void) {
    view_t sub;
    cursor_t orig;

    char buf1[] = "   TEXT";
    sub = bind(buf1);
    orig = sub.begin;
    assert(skip(&sub, bind(" \t")) && sub.begin == orig + 3 && *sub.begin == 'T');

    char buf2[] = "TEXT";
    sub = bind(buf2);
    orig = sub.begin;
    assert(skip(&sub, bind(" \t")) && sub.begin == orig && *sub.begin == 'T');

    char buf3[] = "";
    sub = bind(buf3);
    orig = sub.begin;
    assert(skip(&sub, bind(" ")) && sub.begin == orig);

    char buf4[] = "TEXT";
    sub = bind(buf4);
    orig = sub.begin;
    assert(skip(&sub, bind("")) && sub.begin == orig);

    char buf5[] = "   TEXT";
    sub = bind(buf5);
    assert(skip(&sub, bind(" ")));
    orig = sub.begin;
    assert(skip(&sub, bind(" ")) && sub.begin == orig);

    char buf6[] = "15L";
    sub = bind(buf6);
    skip(&sub, bind(" \t"));
    assert(span(&sub, bind("0123456789")) && str(&sub, "L"));

    char buf7[] = "  15L";
    sub = bind(buf7);
    skip(&sub, bind(" \t"));
    assert(span(&sub, bind("0123456789")) && str(&sub, "L"));

    char buf8[] = "aaaabbb";
    sub = bind(buf8);
    orig = sub.begin;
    assert(skip(&sub, bind("a")) && sub.begin == orig + 4 && *sub.begin == 'b');

    assert(!skip(NULL, bind(" ")));

    sub = view(NULL, NULL);
    assert(!skip(&sub, bind(" ")) && !sub.begin && !sub.end);

    char buf9[] = "A";
    sub = view(buf9, NULL);
    orig = sub.begin;
    assert(!skip(&sub, bind(" ")) && sub.begin == orig && sub.end == NULL);

    char buf10[] = "A";
    sub = view(NULL, buf10);
    assert(!skip(&sub, bind(" ")) && !sub.begin && sub.end == buf10);

    char buf11[] = "   ";
    sub = bind(buf11);
    assert(skip(&sub, bind(" ")) && sub.begin == sub.end);

    char buf12[] = "\t  \tTEXT";
    sub = bind(buf12);
    orig = sub.begin;
    assert(skip(&sub, bind(" \t")) && sub.begin == orig + 4 && *sub.begin == 'T');
}

void test_var(void) {
    char buf[20];
    view_t sub;

    sub = bind("TEST");
    assert(var(&sub, buf, sizeof(buf)) && strcmp(buf, "TEST") == 0 && sub.begin == sub.end);

    sub = bind("");
    assert(var(&sub, buf, sizeof(buf)) && buf[0] == '\0' && sub.begin == sub.end);

    sub = bind("12345");
    cursor_t orig = sub.begin;
    assert(!var(&sub, buf, 5) && sub.begin == orig);

    sub = bind("ABC");
    orig = sub.begin;
    assert(!var(&sub, buf, 3) && sub.begin == orig);

    sub = bind("X");
    orig = sub.begin;
    assert(!var(&sub, buf, 0) && sub.begin == orig);

    sub = bind("SAFE");
    orig = sub.begin;
    assert(!var(&sub, NULL, sizeof(buf)) && sub.begin == orig);

    assert(!var(NULL, buf, sizeof(buf)));

    sub = view(NULL, NULL);
    assert(!var(&sub, buf, sizeof(buf)) && !sub.begin && !sub.end);

    char composite[] = "PREFIXHELLOSUFFIX";
    sub = view(&composite[6], &composite[11]);
    assert(var(&sub, buf, sizeof(buf)) && strcmp(buf, "HELLO") == 0 && sub.begin == sub.end);

    char cmd[] = "15L";
    sub = bind(cmd);
    span(&sub, bind("0123456789"));
    view_t num_view = view(cmd, sub.begin);
    assert(var(&num_view, buf, sizeof(buf)) && strcmp(buf, "15") == 0);
    assert(str(&sub, "L"));
}

void test_num(void) {
    view_t sub;
    int n;

    // Valid positives (within edlin's actual usage range)
    sub = bind("123");
    assert(num(&sub, &n) && n == 123 && sub.begin == sub.end);

    sub = bind("+456");
    assert(num(&sub, &n) && n == 456 && sub.begin == sub.end);

    sub = bind("0");
    assert(num(&sub, &n) && n == 0 && sub.begin == sub.end);

    // Portable max/min guaranteed by C standard (all vintage systems support these)
    sub = bind("32767");   // Max portable positive (C standard min range)
    assert(num(&sub, &n) && n == 32767 && sub.begin == sub.end);

    sub = bind("-32767");  // Min portable negative (C standard min range)
    assert(num(&sub, &n) && n == -32767 && sub.begin == sub.end);

    // Realistic edlin line numbers (safe for 16-bit and 32-bit)
    sub = bind("9999");
    assert(num(&sub, &n) && n == 9999 && sub.begin == sub.end);

    // Stop at NOTANY boundary
    sub = bind("123x");
    assert(num(&sub, &n) && n == 123 && *sub.begin == 'x');

    sub = bind("-456y");
    assert(num(&sub, &n) && n == -456 && *sub.begin == 'y');

    // Fail on invalid first char
    sub = bind("x123");
    assert(!num(&sub, &n));

    sub = bind(" 123");
    assert(!num(&sub, &n));

    sub = bind("");
    assert(!num(&sub, &n));

    // Reject malformed signs
    sub = bind("+");
    assert(!num(&sub, &n));

    sub = bind("-");
    assert(!num(&sub, &n));

    sub = bind("++1");
    assert(!num(&sub, &n));

    sub = bind("--1");
    assert(!num(&sub, &n));

    sub = bind("+-1");
    assert(!num(&sub, &n));

    sub = bind("-+1");
    assert(!num(&sub, &n));

    sub = bind("-x");
    assert(!num(&sub, &n));

    sub = bind("+y");
    assert(!num(&sub, &n));

    sub = bind("-0");
    assert(num(&sub, &n) && n == 0 && sub.begin == sub.end);

    // Atomic rollback on failure
    char buf[] = "x123";
    sub = bind(buf);
    cursor_t orig = sub.begin;
    assert(!num(&sub, &n) && sub.begin == orig);

    // NULL safety
    assert(!num(NULL, &n));

    sub = view(NULL, NULL);
    assert(!num(&sub, &n));

    char buf1[] = "123";
    sub = view(buf1, NULL);
    orig = sub.begin;
    assert(!num(&sub, &n) && sub.begin == orig && sub.end == NULL);

    char buf2[] = "123";
    sub = view(NULL, buf2);
    assert(!num(&sub, &n) && !sub.begin && sub.end == buf2);
}

void test_sno(void) {
    test_bind();
    test_view();
    test_size();
    // 2.3
    test_str();
    test_chr();
    // 2.4
    // 2.5
    test_var();
    test_num();
    // 2.6
    // 2.7
    test_at();
    // 2.8
    test_len();

    test_any();
    test_notany();
    test_span();
    test_brk();
    test_skip();

    printf("All SNOBOL-C primitive tests pass!\n");
}

#endif
