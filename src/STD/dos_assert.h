#ifndef DOS_ASSERT_H
#define DOS_ASSERT_H

#include "dos_stdlib.h"
#include "dos_stdio.h"

#ifdef NDEBUG
    #define dos_assert(expr) ((void)0)
#else
#define dos_assert(condition) \
    ((condition) \
    ? (void)0 \
    : (printf("Assertion failed: %s, file %s, line %d\n", \
             #condition, __FILE__, __LINE__), \
        exit(1)))
#endif

#ifdef POLICY_USE_DOS_STDLIB
    #define assert  dos_assert
#endif

#endif
