#include "dos_string.h"
#include "dos_errno.h"

size_t strlen(const char *s) {
    size_t len = 0;
    __asm {
        .8086
        pushf
        push    ds

        les     di, s

        xor     al, al          ; AL = 0: search for null terminator
        mov     cx, 0FFFFh      ; CX = max scan count (64KB segment limit)
        cld
        repne   scasb           ; Scan ES:DI for AL

        // On exit: DI points past null, CX = remaining count
        // Length = (0xFFFF - CX) - 1 (exclude the null byte itself)
        mov     ax, 0FFFFh
        sub     ax, cx          ; AX = bytes scanned including null
        dec     ax              ; Adjust: exclude null terminator
        mov     len, ax

        pop     ds
        popf
    }
    return len;
}

int strcmp(const char* s1, const char* s2) {
    while (*s1 != '\0' && *s2 != '\0' && *s1 == *s2) {
        s1++;
        s2++;
    }
    return (unsigned char)*s1 - (unsigned char)*s2;
}

char* strchr(const char* str, int ch) {
    while (*str != '\0' && (unsigned char)*str != (unsigned char)ch)
        str++;
    return (unsigned char)*str == (unsigned char)ch ? (char*)str : NULL;
}

char* strrchr(const char* str, int ch) {
    char* last = NULL;
    while (*str != '\0') {
        if (*str == ch) last = (char*)str;
        str++;
    }
    if (ch == '\0') return (char*)str;
    return last;
}

// POSIX.1-2008 compliant strerror()
const char* strerror(int errnum) {
    switch (errnum) {
        case 0:       return "No error";
        case EPERM:   return "Operation not permitted";
        case ENOENT:  return "No such file or directory";
        case EIO:     return "Input/output error";
        case EBADF:   return "Bad file descriptor";
        case ENOMEM:  return "Cannot allocate memory";
        case EACCES:  return "Permission denied";
        case EFAULT:  return "Bad address";
        case EEXIST:  return "File exists";
        case EXDEV:   return "Cross-device link";
        case ENODEV:  return "No such device";
        case EINVAL:  return "Invalid argument";
        case EMFILE:  return "Too many open files";
        case ENOSYS:  return "Function not implemented";
        default:      return "Unknown error";
    }
}
