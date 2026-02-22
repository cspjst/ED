#ifndef BIOS_MEMORY_SERVICES_H
#define BIOS_MEMORY_SERVICES_H

#include "../STD/dos_stdint.h"

#define BIOS_MEMORY_SERVICES    12h

/**
 * return contiguous 1k memory blocks found at startup
 */
uint16_t bios_get_startup_memory_kb();

#endif

/*
INT 12h - Get Conventional Memory Size

Returns KB of base memory (0-640K) in AX

INT 15h, AH=88h - Get Extended Memory Size

Returns KB of extended memory (above 1MB) in AX

286+ systems only

INT 15h, AH=8Fh - Get Memory Map (PS/2 and later)

Returns system memory map in ES:DI

More detailed but complex

INT 15h, AH=C7h - Get Memory Size (some systems)

Alternate extended memory call

INT 15h, AH=E801h - Get Memory Size (Pentium+)

Returns both 1MB-16MB and above 16MB memory

More accurate on newer systems

INT 15h, AH=E820h - Get Memory Map (modern)

Most detailed memory map

Returns array of memory regions with types
 */
