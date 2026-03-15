#ifndef DOS_MEMORY_TYPES_H
#define DOS_MEMORY_TYPES_H

#ifdef USE_DOSLIBC
    #include "../STD/dos_stdint.h"
#else
    #include <stdint.h>
#endif

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

#endif
