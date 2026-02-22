/**
 * @file dos_limits.h
 * @brief C99 limits for basic integer types for 8086 large-model freestanding DOS
 * @note Defines limits for char, short, int, long (not exact-width types)
 */
#ifndef DOS_LIMITS_H
#define DOS_LIMITS_H

/* ============================================================================
 * BASIC TYPE LIMITS (8086 data model: char=8, int=16, long=32)
 * ============================================================================
 */

#define CHAR_BIT   8
#define CHAR_MAX   127
#define CHAR_MIN   (-128)

#define SCHAR_MAX  127
#define SCHAR_MIN  (-128)
#define UCHAR_MAX  255

#define SHRT_MAX   32767
#define SHRT_MIN   (-32768)
#define USHRT_MAX  65535U

#define INT_MAX    32767
#define INT_MIN    (-32768)
#define UINT_MAX   65535U

#define LONG_MAX   2147483647L
#define LONG_MIN   (-2147483647L - 1)
#define ULONG_MAX  4294967295UL

#endif /* DOS_LIMITS_H */
