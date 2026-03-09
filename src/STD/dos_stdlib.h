#ifndef DOS_STDLIB_H
#define DOS_STDLIB_H

#include "dos_stddef.h"
#include "../DOS/dos_services.h"

void* dos_malloc(size_t size);
void dos_free(void* p);
void* dos_calloc(size_t n, size_t size);

#ifdef POLICY_USE_DOS_STD
    #define malloc  dos_malloc
    #define free    dos_free
    #define calloc  dos_calloc
    #define exit(status) dos_terminate_process_with_return_code(status)
#endif

#endif
