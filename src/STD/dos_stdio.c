#include "dos_stdio.h"
#include "dos_errno.h"
#include "dos_string.h"
#include "dos_bool.h"
#include "dos_limits.h"
#include "dos_arg.h"
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
#endif

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
#endif

// Core I/O primitives

int fputc(int c, FILE* stream) {
    dos_file_handle_t handle = (dos_file_handle_t)(unsigned int)stream;
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

    err = dos_read_file((dos_file_handle_t)(unsigned int)stream, 1, &buffer, &bytes_read);

    if (err != DOS_SUCCESS || bytes_read == 0) return EOF;
    return (unsigned char)buffer;
}

char* fgets(char* s, int size, FILE* stream) {
    if (!s || !stream || size <= 0) return NULL;

    dos_file_handle_t handle = (dos_file_handle_t)(unsigned int)stream;
    char* p = s;
    uint16_t bytes_read = 0;
    dos_error_code_t err;
    size--;

    if (handle == DOS_STDIN_HANDLE) {
        char c;
        while (size) {
            err = dos_read_file(handle, 1, &c, &bytes_read);
            if (err != DOS_SUCCESS && p == s) return NULL;
            if (err != DOS_SUCCESS) break;
            *p++ = c;
            if (c == '\n') break;
            size--;
        }
    } else {
        dos_error_code_t err = dos_read_file(handle, size, s, &bytes_read);
        if (bytes_read == 0) return NULL;
        for (int i = 0; i < bytes_read; i++) {
            if (s[i] == '\n') {
                s[i+1] = '\0';
                return s;
            }
        }
        s[bytes_read] = '\0';
    }
    *p = '\0';
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
    if (!mode || !mode[0]) {
        errno = EINVAL;
        return NULL;
    }
    errno = 0;
    dos_file_handle_t handle = 0;
    dos_error_code_t err = 0;
    dos_file_position_t p;
    dos_file_access_attributes_t attr = (mode[1] == '+') ? ACCESS_READ_WRITE : ACCESS_READ_ONLY;

    switch (mode[0]) {
        case 'r':
            err = dos_open_file(filename, attr, &handle);
            break;
        case 'w':
            err = dos_create_file(filename, CREATE_READ_WRITE, &handle);
            break;
        case 'a':
            err = dos_open_file(filename, attr, &handle);
            if (err) err = dos_create_file(filename, CREATE_READ_WRITE, &handle);
            if (!err) dos_move_file_pointer(handle, 0, SEEK_END, &p);
            break;
        default:
            errno = EINVAL;
            return NULL;
    }
    if (err) {
        return NULL;
    }
    return (FILE*)(unsigned int)handle;
}

int fclose(FILE* stream) {
    if (!stream) {
        errno = EBADF;
        return EOF;
    }
    errno = 0;
    dos_error_code_t err = dos_close_file((dos_file_handle_t)(unsigned int)stream);
    if (err) {
        errno = dos_to_errno(err);
        return EOF;
    }
    return 0;
}

#endif
