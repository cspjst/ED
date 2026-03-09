/**
 * C function implementation of the SNOBOL4 pattern matching semantics
 * As defined in 'The SNOBOL4 Programming Language' 2nd edition Griswold, Poage & Polonksy 1971
 * aka the Green Book
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
 */
#ifndef SNO_H
#define SNO_H

#include <stddef.h>
#include <string.h>
#include <stdbool.h>

/**
 * 2.4.2 Anchored Mode
 * The cursor acts in SNOBOL 'anchored' mode
 */
typedef const char* cursor_t;

/**
 * The string view is [cursor, eof)
 */
typedef struct {
    cursor_t begin;  // Current parsing position (cursor)
    cursor_t end;    // End of valid input (exclusive bound)
} view_t;

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
 * 2.3 Scanning
 * SNOBOL: subject pattern
 * Pattern matching needs explicit literal matching functions for string and character literals:
 * @brief Match exact literal sequence (case-sensitive)
 * SUCCESS: cursor += size(pattern)  (full match)
 * FAILURE: cursor unchanged         (mismatch or bounds exceeded)
 * @return true on full match, false otherwise
 */
bool str(view_t* subject, view_t pattern);

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
 * @brief always succeeds, even if subject itself has the null string as value.
 * SUCCESS: entire view consumed (cursor = subject->end)
 * @return true
 */
bool nul(view_t* subject);

/**
 * 2.7 Cursor Position SNOBOL @
 * @brief convert cursor pointer to a 1 based index into the subject
 * @return cursor as an index into the view
 */
unsigned int at(view_t* subject, cursor_t p);

/**
 * 2.8 SNOBOL LEN(length)
 * @brief match a string of specified length
 * SUCCESS: cursor advanced past the length
 * FAILURE: cursor unchanged (insufficient characters)
 * @return true on length characters, false otherwise
 */
bool len(unsigned int length);

/**
 * 2.9 SNOBOL SPAN and BREAK
 * Match 1+ consecutive characters from charset (greedy)
 * SUCCESS: cursor advanced past longest prefix of charset chars (≥1 matched)
 * FAILURE: cursor unchanged             (first char not in charset or EOF)
 * @return: true if ≥1 char matched, false otherwise
 */
bool span(view_t* subject, view_t charset);

/**
 * Skip 0+ characters NOT in charset (SNOBOL: BREAK('set'))
 * ALWAYS succeeds for valid inputs — stops BEFORE first char in charset (or at end)
 * RETURNS: true for all valid inputs, false ONLY for NULL arguments
 */
bool brk(view_t* subject, view_t charset);



// Match exactly ONE character from charset
// SNOBOL: ANY('...')
// SUCCESS: cursor++                     (char in charset)
// FAILURE: cursor unchanged             (char not in charset or EOF)
// RETURNS: true if matched, false otherwise
bool any(view_t* subject, view_t charset);

// Match exactly ONE character NOT in charset
// SNOBOL: NOTANY('...')
// SUCCESS: cursor++                     (char not in charset)
// FAILURE: cursor unchanged             (char in charset or EOF)
// RETURNS: true if matched, false otherwise
bool notany(view_t* subject, view_t charset);

// Skip 0+ characters from charset (idempotent whitespace skipping)
// SNOBOL: (SPAN('...') | NULL)
// SUCCESS: cursor advanced past all consecutive charset chars (may be 0)
// FAILURE: never fails for valid inputs (skipping zero chars is valid)
// RETURNS: true for all valid inputs, false ONLY for NULL arguments
bool skip(view_t* subject, view_t charset);





#endif
