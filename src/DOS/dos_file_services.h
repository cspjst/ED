#ifndef DOS_FILE_SERVICES_H
#define  DOS_FILE_SERVICES_H

#ifndef __LARGE__
    //#error "This module requires large memory model (ie far data pointers)"
#endif

#include "dos_error_types.h"
#include "dos_file_types.h"
#include <stdint.h>

// 36  Get disk free space
dos_error_code_t dos_get_disk_free_space(uint8_t drive_number, dos_file_disk_space_info_t* info);

// 37  Get/set switch character (undocumented)
// 38  Get/set country dependent information
// 39  Create subdirectory (mkdir)
// 3A  Remove subdirectory (rmdir)
// 3B  Change current subdirectory (chdir)
//
// 3C  Create file using handle
dos_error_code_t dos_create_file(const char* path_name, dos_file_attributes_t create_attributes, dos_file_handle_t* fhandle);

// 3D  Open file using handle
dos_error_code_t dos_open_file(const char* path_name, uint8_t access_attributes, dos_file_handle_t* fhandle);

// 3E  Close file using handle
dos_error_code_t dos_close_file(dos_file_handle_t fhandle);

// 3F  Read file or device using handle
dos_error_code_t dos_read_file(dos_file_handle_t fhandle, uint16_t do_bytes, char* buffer, uint16_t* done_bytes);

// 40  Write file or device using handle
dos_error_code_t dos_write_file(dos_file_handle_t fhandle, uint16_t do_bytes, const char* buffer, uint16_t* done_bytes);

// 41  Delete file
dos_error_code_t dos_delete_file(const char* path_name);

// 42  Move file pointer using handle
dos_error_code_t dos_move_file_pointer(dos_file_handle_t fhandle, dos_file_position_t foffset, uint8_t forigin, dos_file_position_t* new_pos);

// 43  Change file mode
dos_error_code_t  dos_get_file_attributes(const char* path_name, dos_file_attributes_t* attr);

dos_error_code_t dos_set_file_attributes(const char* path_name, dos_file_attributes_t attributes);

// 44  I/O control for devices (IOCTL)
// 45  Duplicate file handle
// 46  Force duplicate file handle
// 47  Get current directory

// 56  Rename file
dos_error_code_t dos_rename_file(const char* old_path, const char* new_path);

#endif
