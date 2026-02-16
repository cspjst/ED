#ifndef sno_TEST_H
#define sno_TEST_H

#include "sno.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>

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

// Master test runner
void sno_test(void) {
    test_bind();
    test_view();
    test_size();
    test_lit();
    //test_var();
    //test_span();
    //test_any();
    //test_skip();
    printf("All string primitive tests passed\n");
}

#endif
