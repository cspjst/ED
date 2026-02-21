#ifndef TINY_ERRNO_H
#define TINY_ERRNO_H

#include "../DOS/dos_error_types.h"

extern int errno;

// errno values for Standard C errors
#define EPERM   1       // Operation not permitted
#define ENOENT  2       // No such file or directory
#define EIO     5       // Input/output error
#define EBADF   9       // Bad file descriptor
#define ENOMEM  12      // Out of memory
#define EACCES  13      // Permission denied
#define EEXIST  17      // File exists
#define EFAULT  14      // Bad address
#define ENOSYS  38      // Function not implemented
#define ENODEV  19      // No such device
#define EXDEV   18      // Cross-device link
#define EMFILE  24      // Too many open files
#define EINVAL  22      // Invalid argument

// mapping function
int dos_to_errno(dos_error_code_t dos_err);

#endif
