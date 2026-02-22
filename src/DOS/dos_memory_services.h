#ifndef DOS_MEMORY_SERVICES_H
#define DOS_MEMORY_SERVICES_H

#ifndef __LARGE__
    #error "This module requires large memory model (ie far data pointers)"
#endif

#include "dos_error_types.h"
#include "../STD/dos_stdint.h"

#define DOS_PARAGRAPH_SIZE  16UL

#pragma pack(1)
typedef struct {
    uint16_t offset;
    uint16_t segment;       // reverse order as Intel is little-endian
} dos_segoff_t;
#pragma pack()

typedef union {
    void* ptr;
    uint32_t memloc;
    dos_segoff_t segoff;
    uint16_t words[2];
    uint8_t bytes[4];
} dos_address_t;

dos_error_code_t dos_allocate_memory_blocks(uint16_t paragraphs, uint16_t* segment);

dos_error_code_t dos_free_allocated_memory_blocks(uint16_t segment);

dos_error_code_t dos_get_free_memory_paragraphs(uint16_t* free);

#endif
