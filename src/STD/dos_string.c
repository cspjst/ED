#include "dos_string.h"
#include "dos_errno.h"

// String examination

size_t strlen(const char* str) {
    if (!str) return 0;

    size_t len = 0;
    while (*str++) len++;

    return len;
}

int strcmp(const char* s1, const char* s2) {
    while (*s1 != '\0' && *s2 != '\0' && *s1 == *s2) {
        s1++;
        s2++;
    }
    return (unsigned char)*s1 - (unsigned char)*s2;
}

int strncmp(const char* lhs, const char* rhs, size_t count) {
    if (!lhs || !rhs) return (lhs > rhs) - (lhs < rhs);

    for (; count && *lhs && (*lhs == *rhs); --count, ++lhs, ++rhs);

    return count ? (unsigned char)*lhs - (unsigned char)*rhs : 0;
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

// Character array manipulation

int memcmp(const void* lhs, const void* rhs, size_t count) {
    if (!lhs || !rhs) return (lhs > rhs) - (lhs < rhs);

    const unsigned char* l = (const unsigned char*)lhs;
    const unsigned char* r = (const unsigned char*)rhs;

    while (count--) {
        if (*l != *r) return *l - *r;
        l++;
        r++;
    }

    return 0;
}

void* memset(void* dest, int ch, size_t count) {
    if (!dest) return NULL;

    unsigned char* d = (unsigned char*)dest;
    unsigned char c = (unsigned char)ch;

    while (count--) *d++ = c;

    return dest;
}

void* memcpy(void* dest, const void* src, size_t count) {
    if (!dest || !src) return NULL;

    unsigned char* d = (unsigned char*)dest;
    const unsigned char* s = (const unsigned char*)src;

    while (count--) *d++ = *s++;

    return dest;
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
