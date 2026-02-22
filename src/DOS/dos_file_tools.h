#ifndef DOS_FILE_TOOLS_H
#define DOS_FILE_TOOLS_H

#include "dos_file_types.h"
#include "dos_error_types.h"
#include "../STD/dos_stdint.h"

uint8_t dos_file_exists(const char* path_name);

uint8_t dos_file_is_eof(dos_file_handle_t fhandle);

dos_error_code_t dos_file_size(dos_file_handle_t fhandle,  dos_file_size_t* size);

const char* dos_file_ext(const char* path_name);

#endif
