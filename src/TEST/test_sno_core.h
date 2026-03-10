/**
 * @file test_sno.h
 * @brief Test harness for SNOBOL4-C core pattern matching primitives
 *
 * @copyright Copyright (c) 2026 Jeremy Simon Thornton
 * @license MIT License — see LICENSE file
 */
#ifndef TEST_SNO_H
#define TEST_SNO_H

#include "../SNO/sno_core.h"
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
    span(&sub, "0123456789");
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

void test_nul(void) {
    view_t sub;
    cursor_t orig;

    // Basic: succeeds without advancing cursor
    char buf1[] = "TEST";
    sub = bind(buf1);
    orig = sub.begin;
    assert(nul(&sub) && sub.begin == orig && *sub.begin == 'T');

    // Works at end of string (cursor at EOF)
    sub = bind(buf1);
    sub.begin = sub.end;  // Advance to end
    orig = sub.begin;
    assert(nul(&sub) && sub.begin == orig);

    // Works on empty string
    char buf2[] = "";
    sub = bind(buf2);
    orig = sub.begin;
    assert(nul(&sub) && sub.begin == orig);

    // NULL safety: NULL subject fails
    assert(!nul(NULL));

    // NULL safety: NULL begin fails
    sub = view(NULL, buf1);
    assert(!nul(&sub));

    // NULL safety: NULL end fails (consistent with all primitives)
    sub = view(buf1, NULL);
    assert(!nul(&sub));

    // NULL safety: both NULL fails
    sub = view(NULL, NULL);
    assert(!nul(&sub));

    // Inverted view (begin > end): succeeds (valid view state, 0 chars to match)
    char buf3[] = "X";
    sub = view(&buf3[1], &buf3[0]);
    orig = sub.begin;
    assert(nul(&sub) && sub.begin == orig);

    // Chaining: nul() doesn't interfere with subsequent matches
    char buf4[] = "ABC";
    sub = bind(buf4);
    assert(nul(&sub) && chr(&sub, 'A') && nul(&sub) && chr(&sub, 'B') && chr(&sub, 'C'));

    // Multiple nul() calls are idempotent
    sub = bind(buf4);
    orig = sub.begin;
    assert(nul(&sub) && nul(&sub) && nul(&sub) && sub.begin == orig);

    // Composition: nul() in alternation for optional match (SNOBOL: 'X' | NULL)
    char buf5[] = "Y";
    sub = bind(buf5);
    orig = sub.begin;
    // Try 'X', if fails, nul() succeeds without consuming
    assert((chr(&sub, 'X') || nul(&sub)) && sub.begin == orig);  // 'X' failed, nul() succeeded
    assert(chr(&sub, 'Y'));  // Still at 'Y', can match it

    // Composition: nul() for optional prefix
    char buf6[] = "VALUE";
    sub = bind(buf6);
    // Optional sign: ('+' | '-' | NULL)
    assert((chr(&sub, '+') || chr(&sub, '-') || nul(&sub)) && *sub.begin == 'V');

    // Composition: nul() for optional suffix
    char buf7[] = "123";
    sub = bind(buf7);
    assert(span(&sub, "0123456789") && (chr(&sub, 'L') || nul(&sub)));  // No 'L', nul() succeeds

    // Composition: nul() as termination guard
    char buf8[] = "123  ";
    sub = bind(buf8);
    assert(span(&sub, "0123456789") && skip(&sub, " \t") && nul(&sub) && sub.begin == sub.end);

    // Composition: nul() in skip() macro (span || nul)
    char buf9[] = "   TEXT";
    sub = bind(buf9);
    orig = sub.begin;
    assert(skip(&sub, " \t") && sub.begin == orig + 3 && *sub.begin == 'T');  // span() matched

    char buf10[] = "TEXT";
    sub = bind(buf10);
    orig = sub.begin;
    assert(skip(&sub, " \t") && sub.begin == orig && *sub.begin == 'T');  // span() failed, nul() succeeded

    // Real-world: optional delimiter
    char buf11[] = "KEYVALUE";
    sub = bind(buf11);
    assert(brk(&sub, "=") && (chr(&sub, '=') || nul(&sub)));  // No '=', nul() allows continuation

    // Real-world: parse optional sign + number
    char buf12[] = "456";
    sub = bind(buf12);
    assert((chr(&sub, '+') || chr(&sub, '-') || nul(&sub)) && span(&sub, "0123456789"));

    char buf13[] = "-789";
    sub = bind(buf13);
    assert((chr(&sub, '+') || chr(&sub, '-') || nul(&sub)) && span(&sub, "0123456789"));

    // Verify nul() doesn't consume anything in composition
    char buf14[] = "ABC";
    sub = bind(buf14);
    cursor_t before = sub.begin;
    assert(nul(&sub) && nul(&sub) && nul(&sub));
    assert(sub.begin == before);  // Cursor never moved
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

    // Composition: len() skips payload, chr() matches delimiters
    // Pattern: "A?B?C?" where ? is any single char
    char buf9[] = "A1B2C3";
    sub = bind(buf9);
    assert(chr(&sub, 'A') &&  // Match delimiter 'A'
           len(&sub, 1) &&    // Skip payload '1'
           chr(&sub, 'B') &&  // Match delimiter 'B'
           len(&sub, 1) &&    // Skip payload '2'
           chr(&sub, 'C') &&  // Match delimiter 'C'
           len(&sub, 1) &&    // Skip payload '3'
           sub.begin == sub.end);

    // Inverted view (begin > end): len() should fail safely
    char buf11[] = "X";
    sub = view(&buf11[1], &buf11[0]);
    orig = sub.begin;
    assert(!len(&sub, 1));
    assert(sub.begin == orig);

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
    assert(!len(&sub, 10) && sub.begin == orig);  // Fails, cursor unchanged at 'A'
    assert(chr(&sub, 'A'));                        // Match 'A' at current position - works!

    // Chaining: successful len() composes with next primitive
    sub = bind(buf13);  // Reset
    assert(len(&sub, 1) && chr(&sub, 'B') && chr(&sub, 'C') && sub.begin == sub.end);

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

}

void test_span(void) {
    view_t sub;
    cursor_t orig;

    // Basic: match digits greedily
    char buf1[] = "123abc";
    sub = bind(buf1);
    orig = sub.begin;
    assert(span(&sub, "0123456789") && sub.begin == orig + 3 && *sub.begin == 'a');

    // Failure: first char not in charset (anchored: no scanning)
    char buf2[] = "abc123";
    sub = bind(buf2);
    orig = sub.begin;
    assert(!span(&sub, "0123456789") && sub.begin == orig);

    // Empty charset always fails
    char buf3[] = "123";
    sub = bind(buf3);
    orig = sub.begin;
    assert(!span(&sub, "") && sub.begin == orig);

    // Composition: span + chr for token parsing
    char buf4[] = "123,456";
    sub = bind(buf4);
    assert(span(&sub, "0123456789") && chr(&sub, ',') && span(&sub, "0123456789"));

    // Composition: span + len for fixed+variable field
    char buf5[] = "ID:12345";
    sub = bind(buf5);
    assert(str(&sub, "ID:") && span(&sub, "0123456789") && sub.begin == sub.end);

    // 1. Run of blanks: SPAN(' ')
    char sbuf1[] = "   TEXT";
    view_t s = bind(sbuf1);
    assert(span(&s, " ") && s.begin == &sbuf1[3] && *s.begin == 'T');

    // 2. String of digits: SPAN('0123456789')
    char sbuf2[] = "123abc";
    s = bind(sbuf2);
    assert(span(&s, "0123456789") && s.begin == &sbuf2[3] && *s.begin == 'a');

    // 3. Word (uppercase letters): SPAN('ABCDEFGHIJKLMNOPQRSTUVWXYZ')
    char sbuf3[] = "HELLO WORLD";
    s = bind(sbuf3);
    assert(span(&s, "ABCDEFGHIJKLMNOPQRSTUVWXYZ") &&
           s.begin == &sbuf3[5] && *s.begin == ' ');
}

void test_brk(void) {
    view_t sub;
    cursor_t orig;
    char buf[64];

    // Green Book Example 1: BREAK(' ') — everything up to next blank
    char buf1[] = "HELLO WORLD";
    sub = bind(buf1);
    orig = sub.begin;
    assert(brk(&sub, " ") && sub.begin == orig + 5 && *sub.begin == ' ');

    // Verify we captured "HELLO"
    sub = bind(buf1);
    cursor_t start = sub.begin;
    assert(brk(&sub, " "));
    size_t len = sub.begin - start;
    assert(len == 5 && strncmp(start, "HELLO", len) == 0);

    // Green Book Example 2: BREAK(',.;:!?') — everything up to punctuation
    char buf2[] = "Hello, world!";
    sub = bind(buf2);
    orig = sub.begin;
    assert(brk(&sub, ",.;:!?") && sub.begin == orig + 5 && *sub.begin == ',');

    // No punctuation found — consumes to end
    char buf3[] = "No punctuation here";
    sub = bind(buf3);
    assert(brk(&sub, ",.;:!?") && sub.begin == sub.end);

    // Green Book Example 3: BREAK('+-0123456789') — everything up to number
    char buf4[] = "Value: 123";
    sub = bind(buf4);
    orig = sub.begin;
    assert(brk(&sub, "+-0123456789") && sub.begin == orig + 7 && *sub.begin == '1');

    // Verify we captured "Value: "
    sub = bind(buf4);
    start = sub.begin;
    assert(brk(&sub, "+-0123456789"));
    len = sub.begin - start;
    assert(len == 7 && strncmp(start, "Value: ", len) == 0);

    // Zero-match: first char IS in charset (still succeeds)
    char buf5[] = ",rest";
    sub = bind(buf5);
    orig = sub.begin;
    assert(brk(&sub, ",.;:!?") && sub.begin == orig && *sub.begin == ',');

    // Empty subject: succeeds, cursor unchanged
    char buf6[] = "";
    sub = bind(buf6);
    orig = sub.begin;
    assert(brk(&sub, " ") && sub.begin == orig);

    // Empty charset: consumes entire subject (nothing to stop on)
    char buf7[] = "EVERYTHING";
    sub = bind(buf7);
    assert(brk(&sub, "") && sub.begin == sub.end);

    // Composition: brk() + chr() to consume delimiter
    char buf8[] = "FIELD,REST";
    sub = bind(buf8);
    assert(brk(&sub, ",") && chr(&sub, ',') && brk(&sub, ",") && sub.begin == sub.end);

    // Composition: brk() + span() for token parsing
    char buf9[] = "  123  456  ";
    sub = bind(buf9);
    assert(brk(&sub, "0123456789") &&  // Skip leading spaces
           span(&sub, "0123456789") &&  // Match digits
           brk(&sub, "0123456789") &&  // Skip trailing spaces
           span(&sub, "0123456789") &&  // Match more digits
           brk(&sub, "0123456789"));    // Skip final spaces

    // Composition: brk() + var() to extract token
    char buf10[] = "KEY=value";
    sub = bind(buf10);
    start = sub.begin;
    assert(brk(&sub, "="));  // Stop at '='
    len = sub.begin - start;
    assert(len == 3 && strncmp(start, "KEY", len) == 0);
    assert(chr(&sub, '=') && var(&sub, buf, sizeof(buf)) && strcmp(buf, "value") == 0);

    // NULL safety
    assert(!brk(NULL, " "));
    sub = view(NULL, NULL);
    assert(!brk(&sub, " "));
    sub = bind("TEST");
    assert(!brk(&sub, NULL));

    // Inverted view: succeeds (0 chars to skip), cursor unchanged
    char buf11[] = "X";
    sub = view(&buf11[1], &buf11[0]);
    cursor_t orig_begin = sub.begin;
    cursor_t orig_end = sub.end;
    assert(brk(&sub, " ") && sub.begin == orig_begin && sub.end == orig_end);

    // Multiple brk() calls are idempotent at charset boundary
    char buf12[] = "TEXT,MORE";
    sub = bind(buf12);
    assert(brk(&sub, ",") && sub.begin == &buf12[4]);  // At comma
    orig = sub.begin;
    assert(brk(&sub, ",") && sub.begin == orig);  // Still at comma (0 chars skipped)

    // Real-world: parse CSV field
    char csv[] = "field1,field2,field3";
    sub = bind(csv);
    assert(brk(&sub, ",") && chr(&sub, ',') &&
           brk(&sub, ",") && chr(&sub, ',') &&
           brk(&sub, ",") && sub.begin == sub.end);

    // Real-world: extract filename from path
    char path[] = "/usr/local/bin/program";
    sub = bind(path);
    assert(brk(&sub, "/") && chr(&sub, '/') &&  // Skip "", match '/'
            brk(&sub, "/") && chr(&sub, '/') &&  // Skip "usr", match '/'
            brk(&sub, "/") && chr(&sub, '/') &&  // Skip "local", match '/'
            brk(&sub, "/") && chr(&sub, '/') &&  // Skip "bin", match '/'
            var(&sub, buf, sizeof(buf)) && strcmp(buf, "program") == 0);
}

void test_any(void) {
    view_t sub;
    cursor_t orig;
    char buf[64];

    // Green Book Example: ANY('AEIOU') matches any vowel
    char buf1[] = "HELLO";
    sub = bind(buf1);
    sub.begin++;  // Position cursor at 'E'
    assert(any(&sub, "AEIOU") && *sub.begin == 'L');  // 'E' matched, now at 'L'

    // Verify failure on non-vowel
    sub = bind(buf1);  // Reset to start
    orig = sub.begin;
    assert(!any(&sub, "AEIOU") && sub.begin == orig);  // 'H' not in set, cursor unchanged

    // NOTANY('AEIOU') matches non-vowels
    char buf2[] = "HELLO";
    sub = bind(buf2);
    assert(notany(&sub, "AEIOU") && *sub.begin == 'E');  // 'H' not a vowel, matched, now at 'E'

    // Verify failure on vowel
    sub = bind(buf2);
    sub.begin++;  // Position at 'E'
    orig = sub.begin;
    assert(!notany(&sub, "AEIOU") && sub.begin == orig);  // 'E' IS vowel, cursor unchanged

    // Green Book: Duplicate/irrelevant order in charset
    char buf3[] = "TEST";
    sub = bind(buf3);
    assert(any(&sub, "STRUCTURE") && *sub.begin == 'E');  // 'T' in set
    sub = bind(buf3);
    assert(any(&sub, "CERSTU") && *sub.begin == 'E');     // Same result, different order

    // Basic match: single character from set
    char buf4[] = "XYZ";
    sub = bind(buf4);
    orig = sub.begin;
    assert(any(&sub, "XYZ") && sub.begin == orig + 1 && *sub.begin == 'Y');

    // Match at middle of string
    sub = bind(buf4);
    sub.begin = &buf4[1];  // Start at 'Y'
    assert(any(&sub, "XYZ") && sub.begin == &buf4[2] && *sub.begin == 'Z');

    // Failure: character not in charset
    char buf5[] = "ABC";
    sub = bind(buf5);
    orig = sub.begin;
    assert(!any(&sub, "XYZ") && sub.begin == orig);

    // Failure: empty subject
    char buf6[] = "";
    sub = bind(buf6);
    orig = sub.begin;
    assert(!any(&sub, "A") && sub.begin == orig);

    // Failure: empty charset
    char buf7[] = "A";
    sub = bind(buf7);
    orig = sub.begin;
    assert(!any(&sub, "") && sub.begin == orig);

    // NULL safety
    assert(!any(NULL, "A"));
    sub = view(NULL, NULL);
    assert(!any(&sub, "A"));
    sub = bind("TEST");
    assert(!any(&sub, NULL));

    // Case sensitivity: 'a' != 'A'
    char buf8[] = "aA";
    sub = bind(buf8);
    orig = sub.begin;
    assert(any(&sub, "a") && *sub.begin == 'A');
    sub = bind(buf8);
    orig = sub.begin;
    assert(!any(&sub, "A") && sub.begin == orig);

    // Special characters: match space, punctuation, etc.
    char buf9[] = " !@#";
    sub = bind(buf9);
    assert(any(&sub, " !@") && *sub.begin == '!');      // ' ' matched, now at '!'
    assert(any(&sub, " !@#") && *sub.begin == '@');     // '!' matched, now at '@'
    assert(any(&sub, " !@#") && *sub.begin == '#');     // '@' matched, now at '#'
    assert(any(&sub, " !@#") && sub.begin == sub.end);  // '#' matched, now at EOF

    // Verify failure on char not in set
    sub = bind(buf9);
    sub.begin = &buf9[3];  // Position at '#'
    orig = sub.begin;
    assert(!any(&sub, " !@") && sub.begin == orig);  // '#' not in " !@", cursor unchanged

    // Numeric characters
    char buf10[] = "012";
    sub = bind(buf10);
    assert(any(&sub, "0123456789") && any(&sub, "0123456789") &&
           any(&sub, "0123456789") && sub.begin == sub.end);

    // Composition: any() in chain for token parsing
    char buf11[] = "A1B2";
    sub = bind(buf11);
    assert(any(&sub, "AB") && any(&sub, "0123456789") &&
           any(&sub, "AB") && any(&sub, "0123456789") &&
           sub.begin == sub.end);

    // Composition: any() + brk() for "first of set, then rest"
    char buf12[] = "KEY=value";
    sub = bind(buf12);
    assert(any(&sub, "KLP") && brk(&sub, "=") && chr(&sub, '=') && var(&sub, buf, sizeof(buf)));

    // Inverted view: fails safely (no chars to match)
    char buf13[] = "X";
    sub = view(&buf13[1], &buf13[0]);
    orig = sub.begin;
    assert(!any(&sub, "X") && sub.begin == orig);

    // Charset with duplicates: behavior unchanged
    char buf14[] = "AAA";
    sub = bind(buf14);
    assert(any(&sub, "AAA") && any(&sub, "AAA") && any(&sub, "AAA") && sub.begin == sub.end);
    sub = bind(buf14);
    assert(any(&sub, "A") && any(&sub, "A") && any(&sub, "A") && sub.begin == sub.end);

    // Real-world: parse hex digit
    char hex[] = "0x1A3F";
    sub = bind(hex);
    assert(str(&sub, "0x") &&
           any(&sub, "0123456789ABCDEF") &&
           any(&sub, "0123456789ABCDEF") &&
           any(&sub, "0123456789ABCDEF") &&
           any(&sub, "0123456789ABCDEF") &&
           sub.begin == sub.end);

    // Real-world: match operator from set
    char expr[] = "A+B-C";
    sub = bind(expr);
    assert(any(&sub, "ABCDEFGHIJKLMNOPQRSTUVWXYZ") &&  // 'A'
           any(&sub, "+-*/") &&                          // '+'
           any(&sub, "ABCDEFGHIJKLMNOPQRSTUVWXYZ") &&  // 'B'
           any(&sub, "+-*/") &&                          // '-'
           any(&sub, "ABCDEFGHIJKLMNOPQRSTUVWXYZ") &&  // 'C'
           sub.begin == sub.end);
}

void test_notany(void) {
    view_t sub;
    cursor_t orig;

    // Green Book: NOTANY('AEIOU') matches non-vowels
    char buf1[] = "HELLO";
    sub = bind(buf1);
    assert(notany(&sub, "AEIOU") && *sub.begin == 'E');  // 'H' not a vowel, matched, now at 'E' ✓

    sub = bind(buf1);
    sub.begin++;  // Position at 'E'
    orig = sub.begin;
    assert(!notany(&sub, "AEIOU") && sub.begin == orig);  // 'E' IS a vowel, FAIL, cursor unchanged ✓

    // Verify we can still match the vowel with any()
    assert(any(&sub, "AEIOU") && *sub.begin == 'L');  // Now consume 'E', cursor at 'L' ✓

    // Empty charset: matches any character
    char buf2[] = "XYZ";
    sub = bind(buf2);
    assert(notany(&sub, "") && notany(&sub, "") && notany(&sub, "") && sub.begin == sub.end);

    // Failure: char IS in charset
    char buf3[] = "ABC";
    sub = bind(buf3);
    orig = sub.begin;
    assert(!notany(&sub, "ABC") && sub.begin == orig);

    // Composition: notany() + span() for "non-delimiter then rest"
    char buf4[] = "FIELD,REST";
    sub = bind(buf4);
    assert(notany(&sub, ",") && notany(&sub, ",") && notany(&sub, ",") &&
           notany(&sub, ",") && notany(&sub, ",") && chr(&sub, ','));
}

void test_skip(void) {
    view_t sub;
    cursor_t orig;

    char buf1[] = "   TEXT";
    sub = bind(buf1);
    orig = sub.begin;
    assert(skip(&sub, " \t") && sub.begin == orig + 3 && *sub.begin == 'T');

    char buf2[] = "TEXT";
    sub = bind(buf2);
    orig = sub.begin;
    assert(skip(&sub, " \t") && sub.begin == orig && *sub.begin == 'T');

    char buf3[] = "";
    sub = bind(buf3);
    orig = sub.begin;
    assert(skip(&sub, " ") && sub.begin == orig);

    char buf4[] = "TEXT";
    sub = bind(buf4);
    orig = sub.begin;
    assert(skip(&sub, "") && sub.begin == orig);

    char buf5[] = "   TEXT";
    sub = bind(buf5);
    assert(skip(&sub, " "));
    orig = sub.begin;
    assert(skip(&sub, " ") && sub.begin == orig);

    char buf6[] = "15L";
    sub = bind(buf6);
    skip(&sub, " \t");
    assert(span(&sub, "0123456789") && str(&sub, "L"));

    char buf7[] = "  15L";
    sub = bind(buf7);
    skip(&sub, " \t");
    assert(span(&sub, "0123456789") && str(&sub, "L"));

    char buf8[] = "aaaabbb";
    sub = bind(buf8);
    orig = sub.begin;
    assert(skip(&sub, "a") && sub.begin == orig + 4 && *sub.begin == 'b');

    assert(!skip(NULL, " "));

    sub = view(NULL, NULL);
    assert(!skip(&sub, " ") && !sub.begin && !sub.end);

    char buf9[] = "A";
    sub = view(buf9, NULL);
    orig = sub.begin;
    assert(!skip(&sub, " ") && sub.begin == orig && sub.end == NULL);

    char buf10[] = "A";
    sub = view(NULL, buf10);
    assert(!skip(&sub, " ") && !sub.begin && sub.end == buf10);

    char buf11[] = "   ";
    sub = bind(buf11);
    assert(skip(&sub, " ") && sub.begin == sub.end);

    char buf12[] = "\t  \tTEXT";
    sub = bind(buf12);
    orig = sub.begin;
    assert(skip(&sub, " \t") && sub.begin == orig + 4 && *sub.begin == 'T');
}

void test_sno_core(void) {
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
    test_nul();
    // 2.7
    test_at();
    // 2.8
    test_len();
    // 2.9
    test_span();
    test_brk();
    // 2.10
    test_any();
    test_notany();
    // composition
    test_skip();

    printf("All core SNOBOL-C primitive tests pass!\n");
}

#endif
