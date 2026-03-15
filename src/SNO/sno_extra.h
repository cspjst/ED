#ifndef SNO_EXTRA_H
#define SNO_EXTRA_H

#ifdef POLICY_USE_DOSLIBC
    #include "dos_stddef.h"
    #include "dos_stdbool.h"
#else
    #include <stddef.h>
    #include <stdbool.h>
#endif

#include "sno_types.h"

/**
 * @brief Move cursor to absolute position (SNOBOL TAB primitive)
 * Matches all characters from current cursor to offset n (0-indexed).
 * @param s Parsing context (must not be NULL)
 * @param n Absolute offset from start (0 = start, length = end)
 * @return true if n >= current position and n <= length; false otherwise
 * @note Fails (no cursor movement) if n < current position (cannot move left)
 */
bool tab(view_t* view, size_t n);

/**
 * @brief Move cursor to position from right end (SNOBOL RTAB primitive)
 * Matches all characters from current cursor to offset (length - n).
 * @param s Parsing context (must not be NULL)
 * @param n Number of characters to leave at end (0 = match to end)
 * @return true if (length - n) >= current position; false otherwise
 * @note RTAB(0) matches remainder of string (same as rem)
 */
bool rtab(view_t* s, size_t n);

/**
 * @brief Match remainder of string to end (SNOBOL REM primitive)
 * Equivalent to RTAB(0) — matches all characters from cursor to end.
 * @param s Parsing context (must not be NULL)
 * @return true always (even zero-length match at end)
 */
bool rem(view_t* s);

/**
 * @brief Match balanced delimiters (SNOBOL BAL primitive, but generalized)
 * Matches a nonnull string balanced with respect to delimiter pair (open, close).
 * Validates nesting deterministically through explicit recursion—no backtracking.
 * The matched span includes outer delimiters (e.g., "(A)" not "A").
 * @param s Parsing context (must not be NULL)
 * @param open Opening delimiter character (e.g., '(', '[', '{')
 * @param close Closing delimiter character (e.g., ')', ']', '}')
 * @return true if balanced expression matched (cursor advanced); false otherwise (cursor unchanged)
 * @note Fails on: missing opening delimiter, unclosed opens, mismatched nesting, or EOF before close
 * @note Generalizes SNOBOL's hardcoded BAL (parentheses-only) to arbitrary delimiter pairs
 * @note Every failure path rolls back cursor completely—preserves failure contract
 */
bool bal(view_t* s, char open, char close);

/**
 * @brief Repeat string n times (SNOBOL DUPL)
 * @param dst Output buffer (must have space for strlen(src)*n + 1)
 * @param src Source string (null-terminated)
 * @param n Number of repetitions (0 produces empty string)
 * @return dst on success, NULL on NULL args
 * @note Caller is responsible for ensuring dst has sufficient space
 */
char* strdupl(char* dst, const char* src, unsigned int n);

/**
 * @brief Trim leading and trailing whitespace (SNOBOL TRIM)
 * @param dst Output buffer (must have space for strlen(src) + 1)
 * @param src Source string (null-terminated)
 * @return dst on success, NULL on NULL args
 * @note Trims spaces (' ') and tabs ('\t') from both ends
 * @note In-place safe: dst may equal src
 */
char* strtrim(char* dst, const char* src);

/**
 * @brief Character substitution (SNOBOL REPLACE)
 * @param dst Output buffer (must have space for strlen(src) + 1)
 * @param src Source string (null-terminated)
 * @param from Characters to replace (null-terminated)
 * @param to Replacement characters (null-terminated, must equal strlen(from))
 * @return dst on success, NULL on NULL args or length mismatch
 * @note Rightmost mapping wins for duplicate chars in from
 * @note In-place safe: dst may equal src (single-char replacement doesn't change length)
 */
char* strreplace(char* dst, const char* src, const char* from, const char* to);

#endif
