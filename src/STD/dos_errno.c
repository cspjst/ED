#include "dos_errno.h"
#include "../DOS/dos_error_types.h"
#include <stdint.h>


int errno = 0;

#pragma pack(push, 1)
static const uint8_t dos_to_errno_table[] = {
    0,          // DOS_SUCCESS
    ENOSYS,     // DOS_INVALID_FUNCTION_NUMBER
    ENOENT,     // DOS_FILE_NOT_FOUND,
    ENOENT,     // DOS_PATH_NOT_FOUND,
    EMFILE,     // DOS_TOO_MANY_OPEN_FILES,
    EACCES,     // DOS_ACCESS_DENIED,
    EBADF,      // DOS_INVALID_HANDLE,
    EFAULT,     // DOS_MCB_DESTROYED,
    ENOMEM,     // DOS_INSUFFICIENT_MEMORY,
    EFAULT,     // DOS_INVALID_MEMORY_BLOCK_ADDRESS,
    EINVAL,     // DOS_INVALID_ENVIRONMENT,
    EINVAL,     // DOS_INVALID_FORMAT,
    EINVAL,     // DOS_INVALID_ACCESS_MODE,
    EINVAL,     // DOS_INVALID_DATA,
    EPERM,      // DOS_RESERVED,
    ENODEV,     // DOS_INVALID_DRIVE_SPECIFIED,
    EPERM,      // DOS_ATTEMPT_TO_REMOVE_CURRENT_DIRECTORY,
    EXDEV,      // DOS_NOT_SAME_DEVICE
    ENOENT      // DOS_NO_MORE_FILES
};
#pragma pack(pop)

int dos_to_errno(dos_error_code_t dos_err) {
    if (dos_err >= sizeof(dos_to_errno_table)) return EINVAL;
    return dos_to_errno_table[dos_err];
}
