/**
 * @file test_extra.h
 * @brief Tests for SNOBOL4-C optional utilities
 *
 * @copyright Copyright (c) 2026 Jeremy Simon Thornton
 * @license MIT License — see LICENSE file
 */
#ifndef TEST_EXTRA_H
#define TEST_EXTRA_H

#include "../SNO/sno_extra.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>

void test_strdupl(void) {
    char dst[128];

    /* n = 0 → empty */
    assert(strdupl(dst, "ABC", 0) && dst[0] == '\0');

    /* n = 1 → copy */
    assert(strdupl(dst, "ABC", 1) && strcmp(dst, "ABC") == 0);

    /* n = 3 → repeat */
    assert(strdupl(dst, "AB", 3) && strcmp(dst, "ABABAB") == 0);

    /* Empty source → empty regardless of n */
    assert(strdupl(dst, "", 10) && dst[0] == '\0');

    /* NULL safety */
    assert(!strdupl(NULL, "X", 1));
    assert(!strdupl(dst, NULL, 1));
}

void test_strtrim(void) {
    char dst[64];

    /* No whitespace */
    assert(strtrim(dst, "HELLO") && strcmp(dst, "HELLO") == 0);

    /* Leading blanks */
    assert(strtrim(dst, "   HELLO") && strcmp(dst, "HELLO") == 0);

    /* Trailing blanks */
    assert(strtrim(dst, "HELLO   ") && strcmp(dst, "HELLO") == 0);

    /* Both ends */
    assert(strtrim(dst, "  HELLO  ") && strcmp(dst, "HELLO") == 0);

    /* Tabs */
    assert(strtrim(dst, "\t\tHELLO\t\t") && strcmp(dst, "HELLO") == 0);

    /* Mixed whitespace */
    assert(strtrim(dst, " \t HELLO \t ") && strcmp(dst, "HELLO") == 0);

    /* All whitespace → empty */
    assert(strtrim(dst, "   \t\t") && dst[0] == '\0');

    /* Empty input */
    assert(strtrim(dst, "") && dst[0] == '\0');

    /* In-place */
    char buf[] = "  TEST  ";
    assert(strtrim(buf, buf) && strcmp(buf, "TEST") == 0);

    /* NULL safety */
    assert(!strtrim(NULL, "X"));
    assert(!strtrim(dst, NULL));
}

void test_strreplace(void) {
    char dst[64];

    /* Green Book: Binary ones complement */
    assert(strreplace(dst, "111001", "01", "10") && strcmp(dst, "000110") == 0);

    /* Green Book: Punctuation to blanks */
    assert(strreplace(dst, "Hello, world!", ".,;:?!", "      ") &&
           strcmp(dst, "Hello  world ") == 0);

    /* Green Book: Rightmost mapping wins */
    assert(strreplace(dst, "FEET", "EE", "AO") && strcmp(dst, "FOOT") == 0);

    /* In-place operation */
    char s[] = "FEET";
    assert(strreplace(s, s, "EE", "AO") && strcmp(s, "FOOT") == 0);

    /* Identity: empty mapping fails */
    assert(!strreplace(dst, "TEST", "", ""));

    /* Length mismatch fails */
    assert(!strreplace(dst, "TEST", "ABC", "XY"));

    /* NULL safety */
    assert(!strreplace(dst, "X", NULL, "Y"));
    assert(!strreplace(dst, "X", "X", NULL));
    assert(!strreplace(NULL, "X", "X", "Y"));

    /* Characters not in mapping unchanged */
    assert(strreplace(dst, "HELLO", "EO", "0O") && strcmp(dst, "H0LLO") == 0);

    /* Multiple different mappings */
    assert(strreplace(dst, "ABCABC", "ABC", "XYZ") && strcmp(dst, "XYZXYZ") == 0);

    /* Case sensitivity: only uppercase mapped, lowercase unchanged */
    assert(strreplace(dst, "AaBb", "AB", "XY") && strcmp(dst, "XaYb") == 0);

    /* To map both cases, specify both */
    assert(strreplace(dst, "AaBb", "ABab", "XYxy") && strcmp(dst, "XxYy") == 0);
}

void test_sno_extra(void) {
    test_strdupl();
    test_strtrim();
    test_strreplace();
    printf("All extra utility tests pass!\n");
}

#endif
