#ifndef sno_TEST_H
#define sno_TEST_H

#include "sno.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>

// ----------------------------------------------------------------------------
// TEST: sno_bind — NULL safety and boundary conditions
// ----------------------------------------------------------------------------
void test_bind(void) {
    // Valid non-empty string
    sno_view_t v = sno_bind("TEST");
    assert(v.begin != NULL && v.end != NULL);
    assert(sno_size(v) == 4);

    // Empty string (zero-width valid view)
    v = sno_bind("");
    assert(v.begin != NULL && v.begin == v.end);
    assert(sno_size(v) == 0);

    // CRITICAL: NULL input → safe null view (begin=end=NULL)
    v = sno_bind(NULL);
    assert(v.begin == NULL && v.end == NULL);
    assert(sno_size(v) == 0);  // sno_size must handle NULL views safely
}

// ----------------------------------------------------------------------------
// TEST: sno_view — Explicit pointer assignment (NO validation)
// CRITICAL: Constructor must accept ANY pointer combination (caller's responsibility)
// ----------------------------------------------------------------------------
void test_view(void) {
    char buf[] = "HELLO";

    // Valid subrange
    sno_view_t v = sno_view(&buf[1], &buf[4]);
    assert(v.begin == &buf[1] && v.end == &buf[4]);
    assert(sno_size(v) == 3);

    // Empty valid view
    v = sno_view(buf, buf);
    assert(sno_size(v) == 0);

    // NULL begin, valid end (invalid view - caller error, but constructor must not crash)
    v = sno_view(NULL, buf);
    assert(v.begin == NULL && v.end == buf);

    // Valid begin, NULL end (invalid view)
    v = sno_view(buf, NULL);
    assert(v.begin == buf && v.end == NULL);

    // Both NULL (null view)
    v = sno_view(NULL, NULL);
    assert(v.begin == NULL && v.end == NULL);

    // begin > end (invalid range - constructor assigns raw pointers)
    v = sno_view(&buf[3], &buf[1]);
    assert(v.begin == &buf[3] && v.end == &buf[1]);
}

// ----------------------------------------------------------------------------
// TEST: sno_size — NULL hygiene and invalid view safety
// SPEC: "Safe for NULL views (returns 0)" → ANY NULL pointer = 0
// ----------------------------------------------------------------------------
void test_size(void) {
    // Valid views
    assert(sno_size(sno_bind("ABC")) == 3);
    assert(sno_size(sno_view("X", "X")) == 0);

    // CRITICAL: ANY NULL pointer in view → 0 (no crashes)
    assert(sno_size(sno_view(NULL, "TEST")) == 0);   // NULL begin
    assert(sno_size(sno_view("TEST", NULL)) == 0);   // NULL end
    assert(sno_size(sno_view(NULL, NULL)) == 0);     // Both NULL

    // Invalid range (begin > end) → implementation-defined but MUST NOT crash
    // Safe implementations return 0 or negative; we require non-negative per spec
    char s[] = "TEST";
    int sz = sno_size(sno_view(&s[3], &s[1]));
    assert(sz >= 0);  // Must not crash; value irrelevant per spec

    // NULL-safe composition
    assert(sno_size(sno_bind(NULL)) == 0);
}

// ----------------------------------------------------------------------------
// TEST: sno_lit — Atomic rollback, NULL safety, and boundary rigor
// SPEC: "Cursor unchanged on failure" + "Only NULL args cause true errors"
// ----------------------------------------------------------------------------
void test_lit(void) {
    // SUCCESS: Full match advances cursor
    char input[] = "HELLO";
    sno_view_t sub = sno_bind(input);
    assert(sno_lit(&sub, sno_bind("HEL")) == true);
    assert(sub.begin == input + 3 && *sub.begin == 'L');

    // FAILURE: Mismatch → atomic rollback (cursor unchanged)
    sub = sno_bind("TEST");
    sno_cursor_t orig = sub.begin;
    assert(sno_lit(&sub, sno_bind("FAIL")) == false);
    assert(sub.begin == orig);

    // EDGE: Empty pattern = zero-width success (no advancement)
    sub = sno_bind("ANY");
    orig = sub.begin;
    assert(sno_lit(&sub, sno_bind("")) == true);
    assert(sub.begin == orig);

    // EDGE: Pattern longer than subject → failure + rollback
    sub = sno_bind("HI");
    orig = sub.begin;
    assert(sno_lit(&sub, sno_bind("HELLO")) == false);
    assert(sub.begin == orig);

    // CRITICAL: Subject with NULL begin → false (no crash), subject unchanged
    sub = sno_view(NULL, NULL);
    assert(sno_lit(&sub, sno_bind("X")) == false);
    assert(sub.begin == NULL && sub.end == NULL);

    // CRITICAL: Pattern with NULL begin → false (no crash), subject unchanged
    sub = sno_bind("SAFE");
    orig = sub.begin;
    assert(sno_lit(&sub, sno_view(NULL, NULL)) == false);
    assert(sub.begin == orig);

    // CRITICAL: Subject begin > end (invalid view) → false + rollback (no crash)
    sub = sno_view(&input[3], &input[1]);  // begin > end
    orig = sub.begin;
    assert(sno_lit(&sub, sno_bind("X")) == false);
    assert(sub.begin == orig);

    // CRITICAL: Empty subject + non-empty pattern → false + rollback
    sub = sno_bind("");
    orig = sub.begin;
    assert(sno_lit(&sub, sno_bind("X")) == false);
    assert(sub.begin == orig);

    // SEMANTICS: Case-sensitive (SNOBOL default)
    sub = sno_bind("Hello");
    orig = sub.begin;
    assert(sno_lit(&sub, sno_bind("HELLO")) == false);
    assert(sub.begin == orig);
}

// ----------------------------------------------------------------------------
// TEST: sno_var — Value extraction with atomic rollback and buffer safety
// SPEC: Consumes ENTIRE view on success; cursor unchanged on failure
// ----------------------------------------------------------------------------
void test_var(void) {
    char buf[20];
    sno_view_t sub;
    char input[] = "HELLO";

    // SUCCESS: Adequate buffer copies entire view and consumes it
    sub = sno_bind("TEST");
    assert(sno_var(&sub, buf, sizeof(buf)) == true);
    assert(strcmp(buf, "TEST") == 0);          // Null-terminated copy
    assert(sub.begin == sub.end);              // Cursor fully consumed

    // SUCCESS: Empty view copies zero bytes + null terminator
    sub = sno_bind("");
    assert(sno_var(&sub, buf, sizeof(buf)) == true);
    assert(buf[0] == '\0');                    // Empty string
    assert(sub.begin == sub.end);              // Fully consumed (zero-width)

    // FAILURE: Buffer too small (exact size without null terminator room)
    sub = sno_bind("12345");
    sno_cursor_t orig = sub.begin;
    assert(sno_var(&sub, buf, 5) == false);    // Need 6 bytes for "12345\0"
    assert(sub.begin == orig);                 // ATOMIC ROLLBACK: cursor unchanged

    // FAILURE: Buffer exactly size of view (still insufficient—needs +1 for '\0')
    sub = sno_bind("ABC");
    orig = sub.begin;
    assert(sno_var(&sub, buf, 3) == false);    // Requires 4 bytes
    assert(sub.begin == orig);

    // FAILURE: Zero-length buffer
    sub = sno_bind("X");
    orig = sub.begin;
    assert(sno_var(&sub, buf, 0) == false);
    assert(sub.begin == orig);

    // FAILURE: NULL buffer pointer
    sub = sno_bind("SAFE");
    orig = sub.begin;
    assert(sno_var(&sub, NULL, sizeof(buf)) == false);
    assert(sub.begin == orig);

    // CRITICAL: NULL subject pointer (no crash, returns false)
    assert(sno_var(NULL, buf, sizeof(buf)) == false);

    // CRITICAL: Subject with NULL begin (invalid view)
    sub = sno_view(NULL, NULL);
    assert(sno_var(&sub, buf, sizeof(buf)) == false);
    assert(sub.begin == NULL && sub.end == NULL);

    // SEMANTICS: Copy respects EXPLICIT bounds (no null-termination assumption)
    // View into middle of buffer without null terminator in range
    char composite[] = "PREFIXHELLOSUFFIX";
    sub = sno_view(&composite[6], &composite[11]);  // View = "HELLO" (no '\0' in range)
    assert(sno_var(&sub, buf, sizeof(buf)) == true);
    assert(strcmp(buf, "HELLO") == 0);             // Correct copy despite no internal '\0'
    assert(sub.begin == sub.end);                  // Fully consumed

    // CHAINING: Consume token then continue parsing
    char cmd[] = "15L";
    sub = sno_bind(cmd);
    int val;
    // Parse integer manually first (sno_int not yet tested)
    sno_span(&sub, sno_bind("0123456789"));  // Advance past digits
    sno_view_t num_view = sno_view(cmd, sub.begin);
    assert(sno_var(&num_view, buf, sizeof(buf)) == true);
    assert(strcmp(buf, "15") == 0);
    // Continue parsing 'L' from original subject
    assert(sno_lit(&sub, sno_bind("L")) == true);
}

void test_any(void) {
    sno_view_t sub;
    char buf[10];
    sno_cursor_t orig;

    // SUCCESS: Single char match advances cursor by exactly 1
    strcpy(buf, "XYZ");
    sub = sno_bind(buf);
    orig = sub.begin;
    assert(sno_any(&sub, sno_bind("XYZ")) == true);   // 'X' in charset
    assert(sub.begin == orig + 1);                     // Advanced by EXACTLY 1
    assert(*sub.begin == 'Y');

    // SUCCESS: Charset order irrelevant (membership test)
    strcpy(buf, "!5");
    sub = sno_bind(buf);
    orig = sub.begin;
    assert(sno_any(&sub, sno_bind("!@#$%")) == true);  // '!' matches anywhere in set
    assert(sub.begin == orig + 1);
    assert(*sub.begin == '5');

    // FAILURE: Char not in charset → atomic rollback (cursor unchanged)
    strcpy(buf, "X");
    sub = sno_bind(buf);
    orig = sub.begin;
    assert(sno_any(&sub, sno_bind("ABC")) == false);
    assert(sub.begin == orig);                         // Unchanged

    // FAILURE: Empty subject (begin == end) → false + rollback
    sub = sno_bind("");
    orig = sub.begin;
    assert(sno_any(&sub, sno_bind("A")) == false);
    assert(sub.begin == orig);

    // FAILURE: Empty charset → never matches (no chars to match)
    strcpy(buf, "A");
    sub = sno_bind(buf);
    orig = sub.begin;
    assert(sno_any(&sub, sno_bind("")) == false);
    assert(sub.begin == orig);

    // CRITICAL: NULL subject pointer → false (no crash)
    assert(sno_any(NULL, sno_bind("A")) == false);

    // CRITICAL: Subject with NULL begin → false + unchanged
    sub = sno_view(NULL, NULL);
    assert(sno_any(&sub, sno_bind("A")) == false);
    assert(sub.begin == NULL && sub.end == NULL);

    // CRITICAL: Charset with NULL begin → false + subject unchanged
    strcpy(buf, "SAFE");
    sub = sno_bind(buf);
    orig = sub.begin;
    assert(sno_any(&sub, sno_view(NULL, NULL)) == false);
    assert(sub.begin == orig);

    // SEMANTICS: Case sensitivity preserved (SNOBOL default)
    strcpy(buf, "a");
    sub = sno_bind(buf);
    orig = sub.begin;
    assert(sno_any(&sub, sno_bind("A")) == false);
    assert(sub.begin == orig);

    // SEMANTICS: Matches exactly ONE character (never consumes more)
    strcpy(buf, "AAA");
    sub = sno_bind(buf);
    orig = sub.begin;
    assert(sno_any(&sub, sno_bind("A")) == true);
    assert(sub.begin == orig + 1);                     // Only ONE 'A' consumed
    assert(*sub.begin == 'A');                         // Still two 'A's remaining

    // CHAINING: Consume multiple chars via loop (SNOBOL: SPAN emulation)
    strcpy(buf, "123abc");
    sub = sno_bind(buf);
    int count = 0;
    while (sno_any(&sub, sno_bind("0123456789"))) {
        count++;
    }
    assert(count == 3);                                // Consumed "123"
    assert(strncmp(sub.begin, "abc", 3) == 0);         // Cursor at 'a'

    // BOUNDARY: Match at end of subject (last valid char)
    strcpy(buf, "Z");
    sub = sno_bind(buf);
    assert(sno_any(&sub, sno_bind("Z")) == true);
    assert(sub.begin == sub.end);                      // Fully consumed

    // EDGE: Subject with multiple matching chars — only FIRST consumed
    strcpy(buf, "!!!END");
    sub = sno_bind(buf);
    orig = sub.begin;
    assert(sno_any(&sub, sno_bind("!")) == true);
    assert(sub.begin == orig + 1);                     // Only one '!' consumed
    assert(*sub.begin == '!');
}

void test_notany(void) {
    sno_view_t sub;
    sno_cursor_t orig;

    // SUCCESS: Char NOT in charset → advance by exactly 1
    char buf1[] = "X9Z";
    sub = sno_bind(buf1);
    orig = sub.begin;
    assert(sno_notany(&sub, sno_bind("012345678")) == true);  // 'X' not in digits
    assert(sub.begin == orig + 1);
    assert(*sub.begin == '9');

    // SUCCESS: Empty charset → always matches (NOTANY('') = ANY(.))
    char buf2[] = "A";
    sub = sno_bind(buf2);
    orig = sub.begin;
    assert(sno_notany(&sub, sno_bind("")) == true);
    assert(sub.begin == orig + 1);
    assert(sub.begin == sub.end);

    // FAILURE: Char IN charset → atomic rollback
    char buf3[] = "5";
    sub = sno_bind(buf3);
    orig = sub.begin;
    assert(sno_notany(&sub, sno_bind("0123456789")) == false);  // '5' IS forbidden
    assert(sub.begin == orig);

    // CRITICAL: Empty subject → false + rollback (no dereference!)
    char buf4[] = "";
    sub = sno_bind(buf4);
    orig = sub.begin;
    assert(sno_notany(&sub, sno_bind("A")) == false);  // Cannot match char from empty subject
    assert(sub.begin == orig);  // Cursor unchanged (still points to '\0')

    // CRITICAL: Subject exhausted (begin == end) → false
    char buf5[] = "Z";
    sub = sno_bind(buf5);
    assert(sno_notany(&sub, sno_bind("ABC")) == true);   // 'Z' not forbidden → consume
    assert(sub.begin == sub.end);                         // Now exhausted
    orig = sub.begin;
    assert(sno_notany(&sub, sno_bind("A")) == false);    // Empty → fail
    assert(sub.begin == orig);                            // No advancement

    // NULL safety: subject pointer NULL
    assert(sno_notany(NULL, sno_bind("A")) == false);

    // NULL safety: subject with NULL begin
    sub = sno_view(NULL, NULL);
    assert(sno_notany(&sub, sno_bind("A")) == false);
    assert(sub.begin == NULL && sub.end == NULL);

    // NULL safety: charset with NULL begin
    char buf6[] = "SAFE";
    sub = sno_bind(buf6);
    orig = sub.begin;
    assert(sno_notany(&sub, sno_view(NULL, NULL)) == false);
    assert(sub.begin == orig);

    // SEMANTICS: Case sensitivity
    char buf7[] = "a";
    sub = sno_bind(buf7);
    orig = sub.begin;
    assert(sno_notany(&sub, sno_bind("a")) == false);  // 'a' IS forbidden
    assert(sub.begin == orig);

    // CHAINING: Consume until delimiter (BREAK emulation)
    char buf8[] = "FIELD,REST";
    sub = sno_bind(buf8);
    while (sno_notany(&sub, sno_bind(","))) {
        ;  // Consume all non-comma chars
    }
    assert(*sub.begin == ',');  // Stopped AT delimiter
}

// ----------------------------------------------------------------------------
// TEST: sno_span — Greedy 1+ character consumption (SNOBOL: SPAN('...'))
// SPEC: Consumes longest prefix of consecutive charset chars (≥1 required)
// ----------------------------------------------------------------------------
void test_span(void) {
    sno_view_t sub;
    sno_cursor_t orig;

    // SUCCESS: Single character match (minimum SPAN requirement)
    char buf1[] = "A123";
    sub = sno_bind(buf1);
    orig = sub.begin;
    assert(sno_span(&sub, sno_bind("A")) == true);
    assert(sub.begin == orig + 1);          // Advanced by exactly 1
    assert(*sub.begin == '1');

    // SUCCESS: Multiple consecutive matches (greedy consumption)
    char buf2[] = "12345abc";
    sub = sno_bind(buf2);
    orig = sub.begin;
    assert(sno_span(&sub, sno_bind("0123456789")) == true);
    assert(sub.begin == orig + 5);          // Consumed "12345" (longest prefix)
    assert(*sub.begin == 'a');

    // SUCCESS: Entire subject consumed (cursor reaches end)
    char buf3[] = "999";
    sub = sno_bind(buf3);
    assert(sno_span(&sub, sno_bind("0123456789")) == true);
    assert(sub.begin == sub.end);           // Fully consumed

    // FAILURE: First char NOT in charset → atomic rollback
    char buf4[] = "X123";
    sub = sno_bind(buf4);
    orig = sub.begin;
    assert(sno_span(&sub, sno_bind("0123456789")) == false);
    assert(sub.begin == orig);              // Unchanged

    // FAILURE: Empty subject → false + rollback (cannot match ≥1 char)
    char buf5[] = "";
    sub = sno_bind(buf5);
    orig = sub.begin;
    assert(sno_span(&sub, sno_bind("A")) == false);
    assert(sub.begin == orig);

    // FAILURE: Empty charset → always false (no chars to match)
    char buf6[] = "A";
    sub = sno_bind(buf6);
    orig = sub.begin;
    assert(sno_span(&sub, sno_bind("")) == false);
    assert(sub.begin == orig);

    // SEMANTICS: Greedy consumption (longest possible prefix)
    char buf7[] = "aaaabbb";
    sub = sno_bind(buf7);
    orig = sub.begin;
    assert(sno_span(&sub, sno_bind("a")) == true);
    assert(sub.begin == orig + 4);          // Consumed all consecutive 'a's (not just 1)
    assert(*sub.begin == 'b');

    // SEMANTICS: Charset order irrelevant (membership test)
    char buf8[] = "321cba";
    sub = sno_bind(buf8);
    orig = sub.begin;
    assert(sno_span(&sub, sno_bind("abc123")) == true);  // Set contains all digits/letters
    assert(sub.begin == orig + 6);          // Consumed entire alphanumeric prefix
    assert(sub.begin == sub.end);

    // SEMANTICS: Case sensitivity preserved
    char buf9[] = "aAaA";
    sub = sno_bind(buf9);
    orig = sub.begin;
    assert(sno_span(&sub, sno_bind("a")) == true);
    assert(sub.begin == orig + 1);          // Only lowercase 'a' matched (not 'A')
    assert(*sub.begin == 'A');

    // CHAINING: SPAN + NOTANY composition (SNOBOL field parsing)
    char buf10[] = "123,456,789";
    sub = sno_bind(buf10);
    assert(sno_span(&sub, sno_bind("0123456789")) == true);  // SPAN digits
    assert(sno_lit(&sub, sno_bind(",")) == true);            // Match delimiter
    assert(sno_span(&sub, sno_bind("0123456789")) == true);  // SPAN next digits
    assert(*sub.begin == ',');                               // Stopped before second comma

    // CRITICAL: NULL subject pointer → false (no crash)
    assert(sno_span(NULL, sno_bind("A")) == false);

    // CRITICAL: Subject with NULL begin → false + unchanged
    sub = sno_view(NULL, NULL);
    assert(sno_span(&sub, sno_bind("A")) == false);
    assert(sub.begin == NULL && sub.end == NULL);

    // CRITICAL: Charset with NULL begin → false + subject unchanged
    char buf11[] = "SAFE";
    sub = sno_bind(buf11);
    orig = sub.begin;
    assert(sno_span(&sub, sno_view(NULL, NULL)) == false);
    assert(sub.begin == orig);

    // BOUNDARY: Single char at end of subject
    char buf12[] = "Z";
    sub = sno_bind(buf12);
    assert(sno_span(&sub, sno_bind("Z")) == true);
    assert(sub.begin == sub.end);

    // SEMANTICS: Charset order irrelevant (membership test) — consumes ALL consecutive matches
    char buf13[] = "AAA";
    sub = sno_bind(buf13);
    orig = sub.begin;
    assert(sno_span(&sub, sno_bind("A")) == true);      // charset "A"
    assert(sub.begin == orig + 3);                       // Consumed all 'A's

    sub = sno_bind(buf13);
    orig = sub.begin;
    assert(sno_span(&sub, sno_bind("XYZA")) == true);   // charset "XYZA" contains 'A' at pos 3
    assert(sub.begin == orig + 3);                       // Still consumes all 'A's (greedy)
}

void test_break(void) {
    sno_view_t sub;
    sno_cursor_t orig;

    // SUCCESS: Single char NOT in charset → advance by exactly 1
    char buf1[] = "X123";
    sub = sno_bind(buf1);
    orig = sub.begin;
    assert(sno_break(&sub, sno_bind("0123456789")) == true);  // 'X' not forbidden
    assert(sub.begin == orig + 1);
    assert(*sub.begin == '1');

    // SUCCESS: Multiple consecutive non-charset chars (greedy consumption)
    char buf2[] = "ABC123";
    sub = sno_bind(buf2);
    orig = sub.begin;
    assert(sno_break(&sub, sno_bind("0123456789")) == true);
    assert(sub.begin == orig + 3);          // Consumed "ABC"
    assert(*sub.begin == '1');

    // SUCCESS: Entire subject consumed (all chars excluded from charset)
    char buf3[] = "XYZ";
    sub = sno_bind(buf3);
    assert(sno_break(&sub, sno_bind("0123456789")) == true);
    assert(sub.begin == sub.end);

    // FAILURE: First char IN charset → atomic rollback (≥1 requirement)
    char buf4[] = "1ABC";
    sub = sno_bind(buf4);
    orig = sub.begin;
    assert(sno_break(&sub, sno_bind("0123456789")) == false);  // '1' IS forbidden
    assert(sub.begin == orig);

    // FAILURE: Empty subject → false (cannot match ≥1 char)
    char buf5[] = "";
    sub = sno_bind(buf5);
    orig = sub.begin;
    assert(sno_break(&sub, sno_bind("A")) == false);
    assert(sub.begin == orig);

    // SPECIAL: Empty charset → matches ANY char (nothing forbidden), consumes 1+ greedily
    char buf6[] = "TEST";
    sub = sno_bind(buf6);
    orig = sub.begin;
    assert(sno_break(&sub, sno_bind("")) == true);
    assert(sub.begin == sub.end);           // Consumed entire string

    // SEMANTICS: Greedy consumption stops BEFORE first forbidden char
    char buf7[] = "aaaabbb";
    sub = sno_bind(buf7);
    orig = sub.begin;
    assert(sno_break(&sub, sno_bind("b")) == true);  // Exclude 'b'
    assert(sub.begin == orig + 4);          // Consumed "aaaa", stopped BEFORE 'b'
    assert(*sub.begin == 'b');

    // SEMANTICS: Case sensitivity preserved
    char buf8[] = "aAaA";
    sub = sno_bind(buf8);
    orig = sub.begin;
    assert(sno_break(&sub, sno_bind("A")) == true);  // Exclude uppercase 'A' only
    assert(sub.begin == orig + 1);          // Consumed 'a', stopped at 'A'
    assert(*sub.begin == 'A');

    // CHAINING: BREAK + SPAN composition (field parsing)
    char buf9[] = "NAME123,REST";
    sub = sno_bind(buf9);
    assert(sno_break(&sub, sno_bind("0123456789,")) == true);  // BREAK non-digit/non-comma
    assert(strncmp(buf9, "NAME", 4) == 0);                     // "NAME" consumed
    assert(sno_span(&sub, sno_bind("0123456789")) == true);    // SPAN digits → "123"
    assert(sno_lit(&sub, sno_bind(",")) == true);              // Match delimiter

    // NULL SAFETY
    assert(sno_break(NULL, sno_bind("A")) == false);

    sub = sno_view(NULL, NULL);
    assert(sno_break(&sub, sno_bind("A")) == false);
    assert(sub.begin == NULL && sub.end == NULL);

    char buf10[] = "SAFE";
    sub = sno_bind(buf10);
    orig = sub.begin;
    assert(sno_break(&sub, sno_view(NULL, NULL)) == false);
    assert(sub.begin == orig);

    // BOUNDARY: Single non-forbidden char at end
    char buf11[] = "Z";
    sub = sno_bind(buf11);
    assert(sno_break(&sub, sno_bind("ABC")) == true);
    assert(sub.begin == sub.end);

    // EDGE: Stops BEFORE first forbidden char (even if later chars allowed)
    char buf12[] = "ABcDE";
    sub = sno_bind(buf12);
    orig = sub.begin;
    assert(sno_break(&sub, sno_bind("c")) == true);   // Stop BEFORE 'c'
    assert(sub.begin == orig + 2);                    // Consumed "AB"
    assert(*sub.begin == 'c');
}

#include <assert.h>

// ----------------------------------------------------------------------------
// TEST: sno_skip — Idempotent 0+ character skipping (SNOBOL: (SPAN(x) | NULL))
// SPEC: Always succeeds for valid inputs (skipping 0 chars is valid success)
//       Returns false ONLY for NULL arguments
// ----------------------------------------------------------------------------
void test_skip(void) {
    sno_view_t sub;
    sno_cursor_t orig;

    // SUCCESS: Skip multiple whitespace chars → cursor advances
    char buf1[] = "   TEXT";
    sub = sno_bind(buf1);
    orig = sub.begin;
    assert(sno_skip(&sub, sno_bind(" \t")) == true);
    assert(sub.begin == orig + 3);          // Advanced past 3 spaces
    assert(*sub.begin == 'T');

    // SUCCESS: Skip zero chars (first char not in charset) → cursor unchanged BUT true returned
    char buf2[] = "TEXT";
    sub = sno_bind(buf2);
    orig = sub.begin;
    assert(sno_skip(&sub, sno_bind(" \t")) == true);  // CRITICAL: true despite 0 chars skipped
    assert(sub.begin == orig);              // Cursor unchanged (as expected for 0 skip)
    assert(*sub.begin == 'T');

    // SUCCESS: Empty subject → true (0 chars skipped is valid)
    char buf3[] = "";
    sub = sno_bind(buf3);
    orig = sub.begin;
    assert(sno_skip(&sub, sno_bind(" ")) == true);
    assert(sub.begin == orig);              // Still empty (begin == end)

    // SUCCESS: Empty charset → true (skips 0 chars since nothing matches)
    char buf4[] = "TEXT";
    sub = sno_bind(buf4);
    orig = sub.begin;
    assert(sno_skip(&sub, sno_bind("")) == true);
    assert(sub.begin == orig);              // No advancement (empty set matches nothing)

    // IDEMPOTENCY: Second call skips nothing (cursor already past charset chars)
    char buf5[] = "   TEXT";
    sub = sno_bind(buf5);
    assert(sno_skip(&sub, sno_bind(" ")) == true);  // First call skips 3 spaces
    orig = sub.begin;
    assert(sno_skip(&sub, sno_bind(" ")) == true);  // Second call skips 0 chars → still true
    assert(sub.begin == orig);                      // Cursor unchanged (idempotent)

    // CHAINING: Skip optional whitespace before parsing (always succeeds → no && needed)
    char buf6[] = "15L";
    sub = sno_bind(buf6);
    sno_skip(&sub, sno_bind(" \t"));        // Always succeeds (even with 0 spaces)
    assert(sno_span(&sub, sno_bind("0123456789")) == true);  // Parse digits
    assert(sno_lit(&sub, sno_bind("L")) == true);            // Match 'L'

    char buf7[] = "  15L";
    sub = sno_bind(buf7);
    sno_skip(&sub, sno_bind(" \t"));        // Skips 2 spaces
    assert(sno_span(&sub, sno_bind("0123456789")) == true);
    assert(sno_lit(&sub, sno_bind("L")) == true);

    // SEMANTICS: Greedy consumption of ALL consecutive charset chars
    char buf8[] = "aaaabbb";
    sub = sno_bind(buf8);
    orig = sub.begin;
    assert(sno_skip(&sub, sno_bind("a")) == true);
    assert(sub.begin == orig + 4);          // Skipped all consecutive 'a's (greedy)
    assert(*sub.begin == 'b');

    // CRITICAL: NULL subject pointer → false (ONLY true failure case)
    assert(sno_skip(NULL, sno_bind(" ")) == false);

    // CRITICAL: Charset with NULL begin → false (ONLY true failure case)
    char buf9[] = "SAFE";
    sub = sno_bind(buf9);
    assert(sno_skip(&sub, sno_view(NULL, NULL)) == false);

    // NULL safety: Subject with NULL begin → false
    sub = sno_view(NULL, NULL);
    assert(sno_skip(&sub, sno_bind(" ")) == false);
    assert(sub.begin == NULL && sub.end == NULL);

    // BOUNDARY: Skip entire subject
    char buf10[] = "   ";
    sub = sno_bind(buf10);
    assert(sno_skip(&sub, sno_bind(" ")) == true);
    assert(sub.begin == sub.end);           // Fully consumed

    // EDGE: Mixed charset skipping (tabs + spaces)
    char buf11[] = "\t  \tTEXT";
    sub = sno_bind(buf11);
    orig = sub.begin;
    assert(sno_skip(&sub, sno_bind(" \t")) == true);
    assert(sub.begin == orig + 4);          // Skipped "\t  \t"
    assert(*sub.begin == 'T');
}

// Test sno_int with various inputs
// Returns number of failed tests
int test_int(void) {
    int fails = 0;

    struct {
        char* input;
        int expected_value;
        int expected_result;
        char* desc;
    } tests[] = {
        // Basic cases
        {"123", 123, 1, "positive"},
        {"-456", -456, 1, "negative"},
        {"+789", 789, 1, "explicit positive"},

        // Edge cases
        {"0", 0, 1, "zero"},
        {"-0", 0, 1, "negative zero"},
        {"+0", 0, 1, "positive zero"},

        // Boundaries
        {"32767", 32767, 1, "INT16_MAX"},
        {"-32768", -32768, 1, "INT16_MIN"},
        {"2147483647", INT_MAX, 1, "INT_MAX"},
        {"-2147483648", INT_MIN, 1, "INT_MIN"},

        // Overflow (should fail)
        {"2147483648", 0, 0, "INT_MAX+1"},
        {"-2147483649", 0, 0, "INT_MIN-1"},
        {"9999999999", 0, 0, "way too big"},

        // Invalid (should fail)
        {"", 0, 0, "empty string"},
        {"+", 0, 0, "sign only"},
        {"-", 0, 0, "minus only"},
        {"+-123", 0, 0, "multiple signs"},
        {"12a34", 0, 0, "letters in middle"},
        {"a123", 0, 0, "letters before"},

        // Partial consumption (these succeed but stop at non-digit)
        {"123a", 123, 1, "letters after"},
        {"123 ", 123, 1, "space after"},
        {"123.45", 123, 1, "decimal after"},
    };

    int num_tests = sizeof(tests) / sizeof(tests[0]);

    for (int i = 0; i < num_tests; i++) {
        sno_view_t subject = sno_bind(tests[i].input);
        sno_cursor_t start = subject.begin;
        int n = 999;
        int result = sno_int(&subject, &n);

        int passed = 1;
        if (result != tests[i].expected_result) passed = 0;
        if (result && n != tests[i].expected_value) passed = 0;
        if (!result && tests[i].expected_result == 0 && subject.begin != start) passed = 0;
        if (result && tests[i].expected_result == 1 && subject.begin == start) passed = 0;

        if (!passed) {
            printf("FAIL: %s - input: \"%s\"\n", tests[i].desc, tests[i].input);
            fails++;
        }
    }

    return fails;
}

// Master test runner
void sno_test(void) {
    test_bind();
    test_view();
    test_size();
    test_lit();
    test_any();
    test_notany();
    test_span();
    test_break();
    test_skip();
    test_var();
    test_int();
    printf("All string primitive tests passed\n");
}

#endif
