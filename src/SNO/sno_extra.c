/**
 * @file sno_extra.c
 * @brief SNOBOL4-C Library — Optional Utilities Implementation
 *
 * @author Jeremy Simon Thornton
 * @copyright Copyright (c) 2026 Jeremy Simon Thornton
 * @license MIT License — see LICENSE file
 */
#include "sno_extra.h"

char* strdupl(char* dst, const char* src, unsigned int n) {
    if (!dst || !src) return NULL;
    if (n == 0) return dst;

    // Copy src n times
    char* p = dst;
    for (unsigned int i = 0; i < n; i++) {
        const char* s = src;
        while (*s) *p++ = *s++;
    }
    *p = '\0';
    return dst;
}

char* strtrim(char* dst, const char* src) {
    if (!dst || !src) return NULL;

    // trim leading whitespace
    while (*src == ' ' || *src == '\t') src++; // find first non-whitespace
    if (*src == '\0') return dst;              // hit end, all whitespace

    const char* end = src;
    while (*end) end++;                        // find end of string

    // trim trailing whitespace
    while (end > src && (end[-1] == ' ' || end[-1] == '\t')) 
        end--;                                 // work in from end

    char* p = dst;
    while (src < end) *p++ = *src++;           // copy trimmed range
    *p = '\0';

    return dst;
}

char* strreplace(char* dst, const char* src, const char* from, const char* to) {
    if (!dst || !src || !from || !to ||
        *from == '\0' || *to == '\0') return NULL; // empty mapping fails per SNOBOL spec

    char* p = dst;
    while (*src) {
        char c = *src++;
        char rplc = c;  // default: unchanged

        // Search for mapping (rightmost wins: later entries overwrite) 
        const char* f = from;
        const char* t = to;
        while (*f) {
            if (c == *f) rplc = *t;
            f++; t++;
        }
        *p++ = rplc;
    }
    *p = '\0';

    return dst;
}
