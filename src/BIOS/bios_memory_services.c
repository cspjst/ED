#include "bios_memory_services.h"

unsigned short bios_get_startup_memory_kb() {
    unsigned short mem_blocks;
    __asm {
        .8086
		pushf               ; not all BIOS functions are well behaved
		push    ds

        int BIOS_MEMORY_SERVICES        ; Returns KB of base memory in AX
        mov mem_blocks, ax

        pop 	ds
		popf
    }
    return mem_blocks;
}
