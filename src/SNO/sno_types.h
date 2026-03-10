/**
 * @file sno_types.h
 * @brief SNOBOL4 Pattern Matching Library for C — Type Definitions
 *
 * @author Jeremy Simon Thornton
 * @copyright Copyright (c) 2026 Jeremy Simon Thornton
 * @license MIT License — see LICENSE file or https://opensource.org/licenses/MIT
 *
 * @version 0.9.1
 * @date 2026
 */
#ifndef SNO_TYPES_H
#define SNO_TYPES_H

/**
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

#endif
