#include "bios_memory_services.h"
#include "../STD/dos_stdint.h"

uint16_t bios_get_startup_memory_kb() {
    uint16_t mem_blocks;
    __asm {
        int BIOS_MEMORY_SERVICES        ; Returns KB of base memory in AX
        mov mem_blocks, ax
    }
    return mem_blocks;
}
