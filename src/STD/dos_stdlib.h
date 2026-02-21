#ifndef TINY_STDLIB_H
#define TINY_STDLIB_H

#include "../DOS/dos_services.h"
#include <stddef.h>

void* malloc(size_t size);

void free(void* p);

void* calloc(size_t n, size_t size);

#define exit(status) dos_terminate_process_with_return_code(status)

#endif
