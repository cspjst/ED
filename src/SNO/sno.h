#ifndef sno_H
#define sno_H

#include <stddef.h>
#include <string.h>
#include <stdbool.h>

typedef const char* sno_cursor_t;

typedef struct {
    sno_cursor_t begin;  // Current parsing position (cursor)
    sno_cursor_t end;    // End of valid input (exclusive bound)
} sno_view_t;

// ----------------------------------------------------------------------------
// VIEW CONSTRUCTION
// ----------------------------------------------------------------------------

// Bind null-terminated C string to parsing context
// Returns view spanning [cstring, first null terminator)
sno_view_t sno_bind(const char* cstr);

// Construct explicit view from raw pointers (half-open range [begin, end))
// Caller must ensure begin <= end and both point within same buffer
sno_view_t sno_view(sno_cursor_t begin, sno_cursor_t end);

// ----------------------------------------------------------------------------
// METRICS
// ----------------------------------------------------------------------------

// Return byte length of view (end - begin)
// Safe for NULL views (returns 0)
// Named sno_size to avoid collision with SNOBOL's LEN(n) primitive
int sno_size(sno_view_t view);

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
// SUCCESS: cursor += sno_size(pattern)  (full match)
// FAILURE: cursor unchanged             (mismatch or bounds exceeded)
// RETURNS: true on full match, false otherwise
bool sno_lit(sno_view_t* subject, sno_view_t pattern);

// Match exactly ONE character from charset
// SNOBOL: ANY('...')
// SUCCESS: cursor++                     (char in charset)
// FAILURE: cursor unchanged             (char not in charset or EOF)
// RETURNS: true if matched, false otherwise
bool sno_any(sno_view_t* subject, sno_view_t charset);

// Match exactly ONE character NOT in charset
// SNOBOL: NOTANY('...')
// SUCCESS: cursor++                     (char not in charset)
// FAILURE: cursor unchanged             (char in charset or EOF)
// RETURNS: true if matched, false otherwise
bool sno_notany(sno_view_t* subject, sno_view_t charset);

// Match 1+ consecutive characters from charset (greedy)
// SNOBOL: SPAN('...')
// SUCCESS: cursor advanced past longest prefix of charset chars (≥1 matched)
// FAILURE: cursor unchanged             (first char not in charset or EOF)
// RETURNS: true if ≥1 char matched, false otherwise
bool sno_span(sno_view_t* subject, sno_view_t charset);

// Skip 0+ characters NOT in charset (SNOBOL: BREAK('set'))
// ALWAYS succeeds for valid inputs — stops BEFORE first char in charset (or at end)
// RETURNS: true for all valid inputs, false ONLY for NULL arguments
bool sno_break(sno_view_t* subject, sno_view_t charset);

// Skip 0+ characters from charset (idempotent whitespace skipping)
// SNOBOL: (SPAN('...') | NULL)
// SUCCESS: cursor advanced past all consecutive charset chars (may be 0)
// FAILURE: never fails for valid inputs (skipping zero chars is valid)
// RETURNS: true for all valid inputs, false ONLY for NULL arguments
bool sno_skip(sno_view_t* subject, sno_view_t charset);

// Copy current view contents to buffer (null-terminated)
// SUCCESS: entire view consumed (cursor = subject->end), buf contains copy
// FAILURE: cursor unchanged (buffer too small or NULL args)
// RETURNS: true if copy succeeded (sno_size(view) < buflen), false otherwise
bool sno_var(sno_view_t* subject, char* buf, size_t buflen);

// Parse signed integer from current cursor position
// Format: [sign] digits (sign optional, digits required)
// SUCCESS: cursor advanced past entire integer, *out = parsed value
// FAILURE: cursor unchanged (invalid format: sign without digits, non-digit)
// RETURNS: true on valid integer, false on parse error or NULL args
bool sno_int(sno_view_t* subject, int* n);

#endif
