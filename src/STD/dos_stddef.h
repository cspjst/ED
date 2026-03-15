#ifndef DOS_STDDEF_H
#define DOS_STDDEF_H

/**
 * size_t- Unsigned integer type for object sizes (result of sizeof)
 * 8086 large model DOS context objects live within 64KB segments, ergo 16-bit sufficient
 */
typedef unsigned short size_t;

/**
 * prtdiff_t - Signed integer type for pointer differences
 * Must be signed counterpart to size_t for pointer arithmetic
 */
typedef signed short ptrdiff_t;

/**
 * NULL - Null pointer constant
 * C standard: integer constant expression with value 0, cast to void*
 */
#define NULL ((void*)0)

/**
 * offsetof(type, member) - Byte offset of member within struct
 */
#define offsetof(type, member) ((size_t)&(((type*)0)->member))

#endif
