/**
 * @file dos_stddef.h
 * @brief C99 stddef types and macros for 8086 large-model freestanding DOS
 * @note Defines size_t, ptrdiff_t, NULL, and offsetof(); no dependencies
 */
#ifndef DOS_STDDEF_H
#define DOS_STDDEF_H

/* ============================================================================
 * SIZE_T - Unsigned integer type for object sizes (result of sizeof)
 * ============================================================================
 * 8086 large model: objects live within 64KB segments → 16-bit sufficient
 * If you later support >64KB objects, change to uint32_t
 * ============================================================================
 */
typedef unsigned short size_t;

/* ============================================================================
 * PTRDIFF_T - Signed integer type for pointer differences
 * ============================================================================
 * Must be signed counterpart to size_t for pointer arithmetic
 * ============================================================================
 */
typedef signed short ptrdiff_t;

/* ============================================================================
 * NULL - Null pointer constant
 * ============================================================================
 * C standard: integer constant expression with value 0, cast to void*
 * ============================================================================
 */
#define NULL ((void*)0)

/* ============================================================================
 * OFFSETOF(type, member) - Byte offset of member within struct
 * ============================================================================
 * Standard implementation: treat address 0 as base, take member address
 * Note: This is a common trick; works on all mainstream compilers
 * ============================================================================
 */
#define offsetof(type, member) ((size_t)&(((type*)0)->member))

#endif
