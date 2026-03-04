/**
 * @url https://www.stanislavs.org/helppc/
 */
#ifndef DOS_MEMORY_SERVICES_H
#define DOS_MEMORY_SERVICES_H

#ifndef __LARGE__
    //#error "This module requires large memory model (ie far data pointers)"
#endif

#include "dos_error_types.h"
#include "dos_memory_constants.h"
#include "dos_memory_types.h"
#include "../STD/dos_stdint.h"

dos_error_code_t dos_allocate_memory_blocks(uint16_t paragraphs, uint16_t* segment);

dos_error_code_t dos_free_allocated_memory_blocks(uint16_t segment);

dos_error_code_t dos_get_free_memory_paragraphs(uint16_t* free);

#endif
