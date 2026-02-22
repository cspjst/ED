#include "dos_stdio.h"
#include "dos_errno.h"
#include "dos_string.h"
#include "dos_bool.h"
#include "dos_limits.h"
#include "dos_stdarg.h"
#include "../DOS/dos_services_constants.h"
#include "../DOS/dos_file_constants.h"

// helper functions
static int print_hex(unsigned long val, bool uppercase, FILE* stream) {
    int count = 0;
    if (val > 15) {
        int n = print_hex(val >> 4, uppercase, stream);
        if (n == EOF) return EOF;
        count += n;
    }
    int digit = val & 0xF;
    char c = digit < 10 ? '0' + digit : (uppercase ? 'A' : 'a') + digit - 10;
    if (fputc(c, stream) == EOF) return EOF;
    return count + 1;
}

static int print_uint(unsigned long val, int base, FILE* stream) {
    int count = 0;
    if (val >= base) {
        int n = print_uint(val / base, base, stream);
        if (n == EOF) return EOF;
        count += n;
    }
    int digit = val % base;
    char c = digit < 10 ? '0' + digit : 'A' + digit - 10;
    if (fputc(c, stream) == EOF) return EOF;
    return count + 1;
}

static int print_int(long val, int base, FILE* stream) {
    int count = 0;
    if (val < 0) {
        if (fputc('-', stream) == EOF) return EOF;
        count++;
        if (val == LONG_MIN) {
            if (base == 10) {
                int n = print_uint((unsigned long)(-(val + 1)), 10, stream);
                if (n == EOF) return EOF;
                return count + n;
            }
            return count;
        }
        val = -val;
    }
    int n = print_uint(val, base, stream);
    if (n == EOF) return EOF;
    return count + n;
}

#ifdef DOS_STDIO_PRINTF_FLOAT
static int print_float(double val, FILE* stream) {
    int count = 0;

    if (val < 0) {
        if (fputc('-', stream) == EOF) return EOF;
        val = -val;
        count++;
    }

    long int_part = (long)val;
    int n = print_int(int_part, 10, stream);
    if (n == EOF) return EOF;
    count += n;

    if (fputc('.', stream) == EOF) return EOF;
    count++;

    double frac = val - int_part;
    if (frac < 0) frac = -frac;
    long frac_part = (long)(frac * 1000);

    if (frac_part == 0) {
        if (fputs("000", stream) == EOF) return EOF;
        count += 3;
    } else {
        char buf[4];
        char *p = buf + 3;
        *p = '\0';
        while (frac_part > 0 && p > buf) {
            *--p = '0' + (frac_part % 10);
            frac_part /= 10;
        }
        while (p > buf) *--p = '0';
        if (fputs(buf, stream) == EOF) return EOF;
        count += 3;
    }
    return count;
}
#endif // DOS_STDIO_PRINTF_FLOAT

#ifdef DOS_STDIO_PRINTF_SCIENTIFIC
static int print_scientific(double val, bool uppercase, FILE* stream) {
    int count = 0;

    if (val < 0) {
        if (fputc('-', stream) == EOF) return EOF;
        val = -val;
        count++;
    }

    if (val == 0.0) {
        if (fputs("0.000e0", stream) == EOF) return EOF;
        return 7;
    }

    int exp = 0;
    while (val >= 10.0) { val /= 10.0; exp++; }
    while (val < 1.0)   { val *= 10.0; exp--; }

    int n = print_float(val, stream);
    if (n == EOF) return EOF;
    count += n;

    if (fputc(uppercase ? 'E' : 'e', stream) == EOF) return EOF;
    count++;

    n = print_int(exp, 10, stream);
    if (n == EOF) return EOF;
    count += n;

    return count;
}
#endif // DOS_STDIO_PRINTF_SCIENTIFIC

// Core I/O primitives

int fputc(int c, FILE* stream) {
    dos_file_handle_t handle = (dos_file_handle_t)(uintptr_t)stream;
    dos_error_code_t err = 0;
    char buffer[2];
    uint16_t nbytes = 1;

    if (c == '\n' && (stream == stdout || stream == stderr)) {
        buffer[0] = '\r';
        buffer[1] = '\n';
        nbytes = 2;
    } else {
        buffer[0] = c;
    }

    __asm {
        .8086
        pushf
        push    ds
        push    si

        mov     bx, handle
        mov     cx, nbytes
        lea     dx, buffer
        mov     ah, DOS_WRITE_FILE_OR_DEVICE_USING_HANDLE
        int     DOS_SERVICE
        jnc     END
        mov     err, ax

END:    pop     si
        pop     ds
        popf
    }

    if (err != 0) {
        errno = dos_to_errno(err);
        return EOF;
    }

    return (unsigned char)c;
}

int fputs(const char* str, FILE* stream) {
    while (*str) {
        if (fputc(*str++, stream) == EOF) return EOF;
    }
    return 0;
}

// Core formatted output

int fprintf(FILE* stream, const char* format, ...) {
    va_list args;
    va_start(args, format);

    int count = 0;
    const char* p = format;

    while (*p) {
        if (*p != '%') {
            if (fputc(*p, stream) == EOF) { va_end(args); return EOF; }
            count++;
            p++;
            continue;
        }

        p++;  /* skip '%' */

        bool is_long = false;
        if (*p == 'l') {
            is_long = true;
            p++;
        }

        int n = 0;
        switch (*p++) {
            case 'c':
                n = (fputc(va_arg(args, int), stream) == EOF) ? EOF : 1;
                break;

            case 's': {
                char* str = va_arg(args, char*);
                if (!str) str = "(null)";
                while (*str) {
                    if (fputc(*str++, stream) == EOF) { n = EOF; break; }
                    count++;
                }
                if (n != EOF) n = 0;
                break;
            }

            case 'd':
            case 'i':
                n = is_long ? print_int(va_arg(args, long), 10, stream)
                            : print_int(va_arg(args, int), 10, stream);
                break;

            case 'u':
                n = is_long ? print_uint(va_arg(args, unsigned long), 10, stream)
                            : print_uint(va_arg(args, unsigned int), 10, stream);
                break;

            case 'x':
                n = is_long ? print_hex(va_arg(args, unsigned long), false, stream)
                            : print_hex(va_arg(args, unsigned int), false, stream);
                break;

            case 'p':
            case 'X':
                n = is_long ? print_hex(va_arg(args, unsigned long), true, stream)
                            : print_hex(va_arg(args, unsigned int), true, stream);
                break;

            case 'o':
                n = is_long ? print_uint(va_arg(args, unsigned long), 8, stream)
                            : print_uint(va_arg(args, unsigned int), 8, stream);
                break;

#ifdef DOS_STDIO_PRINTF_FLOAT
            case 'f':
                n = print_float(va_arg(args, double), stream);
                break;
#endif

#ifdef DOS_STDIO_PRINTF_SCIENTIFIC
            case 'e':
                n = print_scientific(va_arg(args, double), false, stream);
                break;
            case 'E':
                n = print_scientific(va_arg(args, double), true, stream);
                break;
#endif

            case '%':
                n = (fputc('%', stream) == EOF) ? EOF : 1;
                break;

            default:
                if (fputc('%', stream) == EOF || fputc(p[-1], stream) == EOF)
                    n = EOF;
                else
                    n = 2;
                break;
        }

        if (n == EOF) { va_end(args); return EOF; }
        count += n;
    }

    va_end(args);
    return count;
}

// Input functions

int fgetc(FILE* stream) {
    char buffer;
    uint16_t bytes_read = 0;
    dos_error_code_t err;

    err = dos_read_file((dos_file_handle_t)(uintptr_t)stream, 1, &buffer, &bytes_read);

    if (err != DOS_SUCCESS || bytes_read == 0) return EOF;
    return (unsigned char)buffer;
}

char* fgets(char* s, int size, FILE* stream) {
    if (!s || !stream || size <= 0) return NULL;

    char* p = s;
    int remaining = size - 1;  // Reserve space for null terminator

    while (remaining > 0) {
        int c = fgetc(stream);  // Use core primitive; handles all streams

        if (c == EOF) {
            // EOF/error: return NULL if nothing read, else partial line
            if (p == s) return NULL;
            break;
        }

        *p++ = (char)c;
        remaining--;

        if (c == '\n') break;  // Line complete
    }

    *p = '\0';  // Always null-terminate
    return s;
}

void perror(const char *s) {
    if (s && *s) {
        fputs(s, stderr);
        fputs(": ", stderr);
    }
    fputs(strerror(errno), stderr);
    fputs("\r\n", stderr);
}

// File handling

#ifdef DOS_STDIO_FILE_HANDLING


FILE* fopen(const char* filename, const char* mode) {
    if (!filename || !mode || !mode[0]) {
        errno = EINVAL;
        return NULL;
    }

    dos_file_handle_t handle = 0;
    dos_error_code_t err = DOS_SUCCESS;
    dos_file_position_t pos;

    switch (mode[0]) {
        case 'r':
            // "r" / "r+" - must exist, fail if not found
            err = dos_open_file(
                filename,
                (mode[1]=='+') ? ACCESS_READ_WRITE : ACCESS_READ_ONLY,
                &handle
            );
            if (err) {
                errno = dos_to_errno(err);
                return NULL;
            }
            break;

        case 'w':
            // "w" / "w+" - truncate or create
            // "wx" / "w+x" - exclusive create: fail if file exists
            if (mode[1] == 'x' || mode[2] == 'x') {
                // Exclusive create: try to open first; if succeeds, file exists → fail
                err = dos_open_file(filename, ACCESS_READ_ONLY, &handle);
                if (err == DOS_SUCCESS) {   // File exists → close probe handle and fail
                    dos_close_file(handle);
                    errno = EEXIST;
                    return NULL;
                }
            } else {
                dos_delete_file(filename);  // Non-exclusive: truncate by deleting first (ignore "not found")
            }
            err = dos_create_file(filename, CREATE_READ_WRITE, &handle); // DOS only able RW
            if (err) {
                errno = dos_to_errno(err);
                return NULL;
            }
            break;

        case 'a':
            // Try open existing first
            err = dos_open_file(filename, ACCESS_READ_WRITE, &handle);
            if (err) err = dos_create_file(filename, CREATE_READ_WRITE, &handle);
            if (err) {
                errno = dos_to_errno(err);
                return NULL;
            }
            dos_move_file_pointer(handle, 0, SEEK_END, &pos); // Append semantics: always seek to end
            break;

        default:
            errno = EINVAL;
            return NULL;
    }

    errno = 0;
    return (FILE*)(uintptr_t)handle;
}

int fclose(FILE* stream) {
    if (!stream) {
        errno = EBADF;
        return EOF;
    }
    errno = 0;
    dos_error_code_t err = dos_close_file((dos_file_handle_t)(uintptr_t)stream);
    if (err) {
        errno = dos_to_errno(err);
        return EOF;
    }
    errno = 0;
    return 0;
}

size_t fread(void* ptr, size_t size, size_t count, FILE* stream) {
    if (!ptr || !stream || size == 0 || count == 0)
        return 0;

    uint16_t bytes = (uint16_t)(size * count);
    uint16_t done = 0;
    dos_error_code_t err = dos_read_file(
        (dos_file_handle_t)(uintptr_t)stream,
        bytes,
        (char*)ptr,
        &done
    );

    if (err != DOS_SUCCESS) {
        errno = dos_to_errno(err);
        return 0;
    }
    return done / size;
}

size_t fwrite(const void* ptr, size_t size, size_t count, FILE* stream) {
    if (!ptr || !stream || size == 0 || count == 0)
        return 0;

    uint16_t bytes = (uint16_t)(size * count);
    uint16_t done = 0;
    dos_error_code_t err = dos_write_file(
        (dos_file_handle_t)(uintptr_t)stream,
        bytes,
        (const char*)ptr,
        &done
    );

    if (err != DOS_SUCCESS) {
        errno = dos_to_errno(err);
        return 0;
    }
    return done / size;
}

int fseek(FILE* stream, long offset, int origin) {
    if (!stream) {
        errno = EBADF;
        return -1;
    }
    dos_file_position_t pos = 0;
    dos_error_code_t err = dos_move_file_pointer(
        (dos_file_handle_t)(uintptr_t)stream,
        (dos_file_position_t)offset,
        (uint8_t)origin,
        &pos
    );
    if (err != DOS_SUCCESS) {
        errno = dos_to_errno(err);
        return -1;
    }
    return 0;
}

long ftell(FILE* stream) {
    if (!stream) {
        errno = EBADF;
        return -1L;
    }
    dos_file_position_t pos = 0;
    dos_error_code_t err = dos_move_file_pointer(
        (dos_file_handle_t)(uintptr_t)stream,
        0,
        SEEK_CUR,
        &pos
    );
    if (err != DOS_SUCCESS) {
        errno = dos_to_errno(err);
        return -1L;
    }
    return (long)pos;
}

#endif // DOS_STDIO_FILE_HANDLING
