#include "dos_stdlib.h"
#include "dos_assert.h"
#include "../DOS/dos_memory_services.h"

void* malloc(size_t size) {
    dos_address_t addr = {0};
    size = (uint16_t)((size + 15) >> 4); // convert to paragraphs (roundup)
    return (dos_allocate_memory_blocks(size, &addr.segoff.segment) == 0) ? addr.ptr : NULL;
}

void free(void* p) {
    if (p == NULL) return;
    dos_address_t addr = {0};
    addr.ptr = p;
    assert(dos_free_allocated_memory_blocks(addr.segoff.segment) == 0);
}

void* calloc(size_t n, size_t size) {         // overflow
    dos_address_t addr = {0};
    size = (uint16_t)(((size * n) + 15) >> 4);   // size x n then convert to paragraphs (roundup)
    if(dos_allocate_memory_blocks(size, &addr.segoff.segment) == 0) {
        __asm {
            .8086
            pushf

            mov     ax, size
            shl     ax, 1
            shl     ax, 1
            shl     ax, 1
            mov     cx, ax          ; CX = paragraphs * 8 = number of WORDS needed
            xor     ax, ax
            cld
            les     di, addr
            rep     stosw           ; clear allocted memory

            popf
        }
        return addr.ptr;
    }
    return NULL;
}
