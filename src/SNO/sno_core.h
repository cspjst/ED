/**
 * @file sno_core.h
 * @brief SNOBOL4 Pattern Matching Library for C - Core Functions
 *
 * C function implementation of the SNOBOL4 pattern matching semantics
 * as defined in 'The SNOBOL4 Programming Language' 2nd edition,
 * Griswold, Poage & Polonsky, 1971 (aka "The Green Book").
 *
 * @author Jeremy Simon Thornton
 * @copyright Copyright (c) 2026 Jeremy Simon Thornton
 * @license MIT License — see LICENSE file or https://opensource.org/licenses/MIT
 *
 * @note Design Decisions:
 *  + Immutable Subject - null treminated C string.
 *  + No Backtracking - only SNOBOL 'anchored' mode, simplicity and efficiency.
 *  + Context-free recognition remains possible through explicit recursion — not hidden engine magic.
 *  + String View - [begin, end) half-open span of last match — zero-copy substring.
 *  + Failure Contract - every pattern function on failure leaves the string view unchanged.
 *
 * @note SNOBOL Translation Principles:
 *  + All operations mutate subject->begin on SUCCESS
 *  + Cursor remains UNCHANGED on FAILURE (atomic rollback)
 *  + Return true/false enables natural &&/|| chaining
 *  + Only NULL arguments cause true errors (not "no match")
 *
 * @version 0.9.1
 * @date 2026
 */
#ifndef SNO_CORE_H
#define SNO_CORE_H

#ifdef POLICY_USE_DOS_STD
    #include "dos_stddef.h"
    #include "dos_stdbool.h"
#else
    #include <stddef.h>
    #include <stdbool.h>
#endif

#include "sno_types.h"

/**
 * Bind null-terminated C string to parsing context
 * Returns a view spanning [cstring, first null terminator)
 */
view_t bind(const char* cstr);

/**
 * Construct explicit view from raw pointers - a half-open range [begin, end)
 * Caller must ensure begin <= end and both point within same buffer
 */
view_t view(cursor_t begin, cursor_t end);

/**
 * Return byte length of view (end - begin)
 * Safe for NULL views (returns 0)
 */
unsigned int size(view_t view);

//SNOBOL4 Primitives by Green Book sections:


/**
 * 2.3 Scanning - Match exact literal sequence (case-sensitive)
 * SNOBOL: subject "literal"
 * SUCCESS: cursor += strlen(match)  (full match)
 * FAILURE: cursor unchanged         (mismatch or bounds exceeded)
 * @param subject  parsing context (mutated on success)
 * @param match    null-terminated C string to match
 * @return true on full match, false otherwise
 * @note Empty string ("") always matches (SNOBOL null string semantics)
 * @note NULL match argument is an error (returns false)
 */
bool str(view_t* subject, const char* match);

/**
 * @brief Match exact literal (case-sensitive)
 * SUCCESS: cursor +1  (full match)
 * FAILURE: cursor unchanged (mismatch or bounds exceeded)
 * @return true on match, false otherwise
 */
bool chr(view_t* subject, char c);

/**
 * 2.4 Modes of Scanning
 * 2.4.1 Unanchored Mode SNOBOL &ANCHOR = 0
 * Not implemented.
 * @note Context-free recognition remains possible through explicit recursion
 * 2.4.2 Anchored Mode SNOBOL &ANCHOR = 1
 * The anchored mode of scanning is generally more efficient than the unanchored mode,
 * since the scanner examines fewer possibilities.
 * Anchored scanning should be used where possible.
 */
 #define ANCHOR     1

/**
 * 2.5 Value Assignment through Pattern Matching
 * Pattern matching may be viewed as a means of decomposing a string into substrings.
 * To be useful, a substring found by the scanner often must be assigned as the value of a variable.
 * 2.5.1 Conditional Value Assignment
 * Not implemented
 * 2.5.2 Immediate Value Assignment SNOBOL $
 * @brief Copy current view contents to buffer (null-terminated)
 * SUCCESS: entire view consumed (cursor = subject->end), buf contains copy
 * FAILURE: cursor unchanged (buffer too small or NULL args)
 * @return true if copy succeeded (ssize(view) < buflen), false otherwise
 */
bool var(view_t* subject, char* buf, size_t buflen);

/**
 * 2.5 Value Assignment through Pattern Matching
 * Parse signed integer from current cursor position
 * Format: [sign] digits (sign optional, digits required)
 * SUCCESS: cursor advanced past entire integer, *out = parsed value
 * FAILURE: cursor unchanged (invalid format: sign without digits, non-digit)
 * @return true on valid integer, false on parse error or NULL args
 */
bool num(view_t* subject, int* n);

/**
 * 2.6 The Null String in Pattern Matching SNOBOL NULL
 * Attempts to match the null string always succeed
 * @brief matches empty string at current position (consumes 0 characters)
 * SUCCESS: cursor unchanged (matches zero-length string)
 * FAILURE: never fails for valid inputs (NULL args return false)
 * @return true for valid inputs, false for NULL arguments
 *
 * @note SNOBOL: NULL matches at any position without advancing cursor
 * @note Useful for: optional elements, pattern alternation, termination checks
 */
bool nul(view_t* subject);

/**
 * 2.7 Cursor Position SNOBOL @
 * @brief convert cursor pointer to a 1-based index into the subject
 * @return cursor as a 1-based index into the view, or 0 on invalid input
 *
 * SNOBOL: @x assigns current cursor position (1-based) to variable x
 * This function computes that position given an explicit cursor pointer.
 *
 * @note Returns 0 (invalid position) for:
 *   - NULL subject or subject->begin
 *   - NULL cursor pointer p
 *   - p outside the valid range [subject->begin, subject->end]
 *
 * @note SNOBOL uses 1-based indexing: first character is position 1
 */
unsigned int at(view_t* subject, cursor_t p);

/**
 * 2.8 SNOBOL LEN(length)
 * @brief match a string of specified length (any characters)
 * SUCCESS: cursor advanced by exactly length characters
 * FAILURE: cursor unchanged (insufficient characters remain)
 * @param subject  parsing context (mutated on success)
 * @param length   number of characters to match
 * @return true on success, false on failure or NULL arguments
 * @note Content-agnostic: matches any characters, not specific values
 * @note length=0 always succeeds (matches empty string, cursor unchanged)
 */
bool len(view_t* subject, unsigned int length);

/**
 * 2.9 SNOBOL SPAN(charset)
 * @brief match 1+ consecutive characters from charset (greedy, anchored)
 * SUCCESS: cursor advanced past longest prefix of charset chars (≥1 matched)
 * FAILURE: cursor unchanged (first char not in charset, empty subject, or empty charset)
 * @param subject  parsing context (mutated on success)
 * @param charset  null-terminated C string defining allowed characters
 * @return true if ≥1 char matched, false otherwise
 * @note Anchored: attempts match ONLY at current cursor position
 * @note Greedy: consumes all consecutive charset chars from current position
 */
bool span(view_t* subject, const char* charset);

/**
 * 2.9 SNOBOL BREAK(charset)
 * @brief skip 0+ characters NOT in charset (greedy, anchored)
 * SUCCESS: cursor advanced to first char in charset (or end of subject)
 * FAILURE: never fails for valid inputs (returns false only for NULL args)
 * @param subject  parsing context (mutated on success)
 * @param charset  null-terminated C string defining stopping characters
 * @return true for valid inputs, false for NULL arguments
 * @note Anchored: starts at current cursor, no implicit scanning
 * @note Zero-match allowed: succeeds even if first char IS in charset
 * @note Complement of span(): brk() matches chars NOT in charset
 */
bool brk(view_t* subject, const char* charset);

/**
 * 2.9 SNOBOL ANY(charset)
 * @brief match exactly ONE character from charset (anchored)
 * SUCCESS: cursor advanced by 1 (char matched)
 * FAILURE: cursor unchanged (char not in charset, empty subject, or empty charset)
 * @param subject  parsing context (mutated on success)
 * @param charset  null-terminated C string defining allowed characters
 * @return true if matched, false otherwise
 * @note Anchored: attempts match ONLY at current cursor position
 * @note Duplicate chars in charset are ignored; order is irrelevant
 * @note Faster than alternation: ANY("AEIOU") vs 'A'|'E'|'I'|'O'|'U'
 */
bool any(view_t* subject, const char* charset);

/**
 * 2.9 SNOBOL NOTANY(charset)
 * @brief match exactly ONE character NOT in charset (anchored)
 * SUCCESS: cursor advanced by 1 (char matched)
 * FAILURE: cursor unchanged (char in charset, empty subject)
 * @param subject  parsing context (mutated on success)
 * @param charset  null-terminated C string defining excluded characters
 * @return true if matched, false otherwise
 * @note Anchored: attempts match ONLY at current cursor position
 * @note Empty charset matches ANY character (nothing is excluded)
 */
bool notany(view_t* subject, const char* charset);

/**
 * SNOBOL: SPAN('set') | NULL
 * Composition example: skip() is NOT a primitive, but derived from span() + nul()
 * Demonstrates: alternation (||) with atomic rollback enables 0+ matching
 */
#define skip(subject, charset) (span((subject), (charset)) || nul((subject)))

#endif
