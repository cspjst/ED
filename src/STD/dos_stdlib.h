#ifndef TINY_STDLIB_H
#define TINY_STDLIB_H

#include "dos_stddef.h"
#include "../DOS/dos_services.h"

#ifdef USE_DOSSTD
    typedef dos_malloc malloc
    typedef dos_free free
    typedef dos_calloc calloc
#endif

void* dos_malloc(size_t size);

void dos_free(void* p);

void* dos_calloc(size_t n, size_t size);

#define exit(status) dos_terminate_process_with_return_code(status)

#endif
