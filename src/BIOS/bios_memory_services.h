/**
 * @file bios_memory_services.h
 * @brief BIOS memory detection functions for DOS systems
 *
 * @url https://www.stanislavs.org/helppc/int_12.html
 *
 * @details Provides interfaces to query system memory configuration using
 * various BIOS interrupts. Functions automatically adapt to the available
 * BIOS features on the target system.
 *
 * ## BIOS Memory Functions Timeline
 *
 * | Function | First Appeared | Systems | Notes |
 * |----------|---------------|---------|-------|
 * | INT 12h | 1981 (IBM PC) | All | Returns conventional memory (0-640K) in AX |
 * | INT 15h, AH=88h | 1984 (PC/AT) | 286+ | Extended memory size (above 1MB) |
 * | INT 15h, AH=87h | 1984 (PC/AT) | 286+ | Block move to/from extended memory |
 * | INT 15h, AH=8Fh | 1987 (PS/2) | PS/2 only | First memory map interface |
 * | INT 15h, AH=E801h | ~1990 | 386+ | Better extended memory detection (1MB-16MB and >16MB) |
 * | INT 15h, AH=E820h | ~1994 | 486+ | Modern memory map with region types |
 *
 * @note INT 12h is the only function available on all systems including the original IBM PC.
 *       Later functions require runtime CPU/BIOS detection.
 */
#ifndef BIOS_MEMORY_SERVICES_H
#define BIOS_MEMORY_SERVICES_H


#define BIOS_MEMORY_SERVICES    12h

/**
 * return contiguous 1k memory blocks found at startup
 */
unsigned short bios_get_startup_memory_kb();

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
