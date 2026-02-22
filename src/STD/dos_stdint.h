/**
 * @file dos_stdint.h
 * @brief C99 exact-width integer types for 8086 large-model freestanding DOS
 * @note Only exact-width types and their limits; basic type limits are in limits.h
 */
#ifndef DOS_STDINT_H
#define DOS_STDINT_H

/* ============================================================================
 * EXACT-WIDTH INTEGER TYPES (8086: char=8, short=int=16, long=32)
 * ============================================================================
 */

/* 8-bit types */
typedef signed char int8_t;
typedef unsigned char uint8_t;

/* 16-bit types */
typedef signed short int16_t;
typedef unsigned short uint16_t;

/* 32-bit types */
typedef signed long int32_t;
typedef unsigned long uint32_t;

/* ============================================================================
 * POINTER-WIDTH INTEGER TYPES (large model: far pointers = 32-bit)
 * ============================================================================
 */
typedef int32_t intptr_t;
typedef uint32_t uintptr_t;

/* ============================================================================
 * GREATEST-WIDTH INTEGER TYPES (8086 max = 32-bit)
 * ============================================================================
 */
typedef int32_t intmax_t;
typedef uint32_t uintmax_t;

/* ============================================================================
 * LIMITS FOR EXACT-WIDTH TYPES
 * ============================================================================
 */

/* 8-bit limits */
#define INT8_MIN   (-128)
#define INT8_MAX   127
#define UINT8_MAX  255

/* 16-bit limits */
#define INT16_MIN  (-32768)
#define INT16_MAX  32767
#define UINT16_MAX 65535U

/* 32-bit limits */
#define INT32_MIN  (-2147483647L - 1)
#define INT32_MAX  2147483647L
#define UINT32_MAX 4294967295UL

/* Pointer-width limits (large model: 32-bit) */
#define INTPTR_MIN  INT32_MIN
#define INTPTR_MAX  INT32_MAX
#define UINTPTR_MAX UINT32_MAX

/* Greatest-width limits */
#define INTMAX_MIN  INT32_MIN
#define INTMAX_MAX  INT32_MAX
#define UINTMAX_MAX UINT32_MAX

#endif /* DOS_STDINT_H */
