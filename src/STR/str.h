#ifndef STR_H
#define STR_H

#include <stddef.h>
#include <string.h>
#include <stdbool.h>

typedef const char* str_cursor_t;

typedef struct {
    str_cursor_t begin;  // Current parsing position (cursor)
    str_cursor_t end;    // End of valid input (exclusive bound)
} str_view_t;

// ----------------------------------------------------------------------------
// VIEW CONSTRUCTION
// ----------------------------------------------------------------------------

// Bind null-terminated C string to parsing context
// Returns view spanning [cstring, first null terminator)
str_view_t str_bind(str_cursor_t cstring);

// Construct explicit view from raw pointers (half-open range [begin, end))
// Caller must ensure begin <= end and both point within same buffer
str_view_t str_view(str_cursor_t begin, str_cursor_t end);

// ----------------------------------------------------------------------------
// METRICS
// ----------------------------------------------------------------------------

// Return byte length of view (end - begin)
// Safe for NULL views (returns 0)
// Named str_size to avoid collision with SNOBOL's LEN(n) primitive
int str_size(str_view_t view);

// ----------------------------------------------------------------------------
// PATTERN MATCHING (cursor-mutating primitives)
//
// SNOBOL TRANSLATION PRINCIPLE:
//   - All operations mutate subject->begin on SUCCESS
//   - Cursor remains UNCHANGED on FAILURE (atomic rollback)
//   - Return true/false enables natural &&/|| chaining
//   - Only NULL arguments cause true errors (not "no match")
// ----------------------------------------------------------------------------

// Match exact literal sequence (case-sensitive)
// SNOBOL: subject pattern
// SUCCESS: cursor += str_size(pattern)  (full match)
// FAILURE: cursor unchanged             (mismatch or bounds exceeded)
// RETURNS: true on full match, false otherwise
bool str_match(str_view_t* subject, str_view_t pattern);

// Match exactly ONE character from charset
// SNOBOL: ANY('...')
// SUCCESS: cursor++                     (char in charset)
// FAILURE: cursor unchanged             (char not in charset or EOF)
// RETURNS: true if matched, false otherwise
bool str_any(str_view_t* subject, str_view_t charset);

// Match 1+ consecutive characters from charset (greedy)
// SNOBOL: SPAN('...')
// SUCCESS: cursor advanced past longest prefix of charset chars (≥1 matched)
// FAILURE: cursor unchanged             (first char not in charset or EOF)
// RETURNS: true if ≥1 char matched, false otherwise
bool str_span(str_view_t* subject, str_view_t charset);

// Skip 0+ characters from charset (idempotent whitespace skipping)
// SNOBOL: (SPAN('...') | NULL)
// SUCCESS: cursor advanced past all consecutive charset chars (may be 0)
// FAILURE: never fails for valid inputs (skipping zero chars is valid)
// RETURNS: true for all valid inputs, false ONLY for NULL arguments
bool str_skip(str_view_t* subject, str_view_t charset);

// Parse signed integer from current cursor position
// Format: [sign] digits (sign optional, digits required)
// SUCCESS: cursor advanced past entire integer, *out = parsed value
// FAILURE: cursor unchanged (invalid format: sign without digits, non-digit)
// RETURNS: true on valid integer, false on parse error or NULL args
bool str_int(str_view_t* subject, int* out);

// Copy current view contents to buffer (null-terminated)
// SUCCESS: entire view consumed (cursor = subject->end), buf contains copy
// FAILURE: cursor unchanged (buffer too small or NULL args)
// RETURNS: true if copy succeeded (str_size(view) < buflen), false otherwise
bool str_var(str_view_t* subject, char* buf, size_t buflen);

#endif
