#ifndef SNO_EXTRA_H
#define SNO_EXTRA_H

#ifdef POLICY_USE_DOS_STD
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




#endif
