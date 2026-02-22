#include "dos_stdio.h"
#include "dos_errno.h"
#include "dos_string.h"
#include "dos_bool.h"
#include "dos_limits.h"
#include "dos_arg.h"
#include "../DOS/dos_services_constants.h"
#include "../DOS/dos_file_constants.h"

static void print_hex(unsigned long val, bool uppercase) {
    if (val > 15) print_hex(val >> 4, uppercase);
    int digit = val & 0xF;
    putchar(digit < 10 ? '0' + digit : (uppercase ? 'A' : 'a') + digit - 10);
}

static void print_uint(unsigned long val, int base) {
    if (val >= base) print_uint(val / base, base);
    int digit = val % base;
    putchar(digit < 10 ? '0' + digit : 'A' + digit - 10);
}

static void print_int(long val, int base) {
    if (val < 0) {
        putchar('-');
        if (val == LONG_MIN) { // avoid overflow
            if (base == 10) {
                print_uint((unsigned long)(-(val + 1)) + 1, 10);
            }
            return;
        }
        val = -val;
    }
    print_uint(val, base);
}

#ifdef DOS_STDIO_PRINTF_FLOAT
static void print_float(double val) {
    if (val < 0) {
        putchar('-');
        val = -val;
    }
    long int_part = (long)val;
    print_int(int_part, 10);
    putchar('.');
    double frac = val - int_part;
    if (frac < 0) frac = -frac;
    long frac_part = (long)(frac * 1000);
    print_int(frac_part, 10);
}
#endif

#ifdef DOS_STDIO_PRINTF_SCIENTIFIC
static void print_scientific(double val, bool uppercase) {
    if (val < 0) {
        putchar('-');
        val = -val;
    }
    int exp = 0;
    while (val >= 10.0) {
        val /= 10.0;
        exp++;
    }
    while (val < 1.0 && val != 0.0) {
        val *= 10.0;
        exp--;
    }
    print_float(val);
    putchar(uppercase ? 'E' : 'e');
    print_int(exp, 10);
}
#endif

// character output
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

// string output
int fputs(const char* str, FILE* stream) {
    while (*str) {
        if (fputc(*str++, stream) == EOF) return EOF;
    }
    return 0;
}

// formatted output
int printf(const char* format, ...) {
    va_list args;
    va_start(args, format);

    const char* p = format;
    while (*p) {
        if (*p == '%') {
            p++;

            bool is_long = false;
            if (*p == 'l') {
                is_long = true;
                p++;
            }

            switch (*p) {
                case 'c': {
                    int c = va_arg(args, int);
                    putchar(c);
                    break;
                }
                case 's': {
                    char* str = va_arg(args, char*);
                    if (!str) str = "(null)";
                    while (*str) putchar(*str++);
                    break;
                }
                case 'd':
                case 'i':
                    if (is_long)
                        print_int(va_arg(args, long), 10);
                    else
                        print_int(va_arg(args, int), 10);
                    break;
                case 'u':
                    if (is_long)
                        print_uint(va_arg(args, unsigned long), 10);
                    else
                        print_uint(va_arg(args, unsigned int), 10);
                    break;
                case 'x':
                    if (is_long)
                        print_hex(va_arg(args, unsigned long), false);
                    else
                        print_hex(va_arg(args, unsigned int), false);
                    break;
                case 'p':
                case 'X':
                    if (is_long)
                        print_hex(va_arg(args, unsigned long), true);
                    else
                        print_hex(va_arg(args, unsigned int), true);
                    break;
                case 'o':
                    if (is_long)
                        print_uint(va_arg(args, unsigned long), 8);
                    else
                        print_uint(va_arg(args, unsigned int), 8);
                    break;

#ifdef DOS_STDIO_PRINTF_FLOAT
                case 'f':
                    print_float(va_arg(args, double));
                    break;
#endif

#ifdef DOS_STDIO_PRINTF_SCIENTIFIC
                case 'e':
                    print_scientific(va_arg(args, double), false);
                    break;
                case 'E':
                    print_scientific(va_arg(args, double), true);
                    break;
#endif
                case '%':
                    putchar('%');
                    break;
                default:
                    putchar('%');
                    putchar(*p);
                    break;
            }
        } else {
            putchar(*p);
        }
        p++;
    }

    va_end(args);
    return 0;
}

// character input
int fgetc(FILE* stream) {
    char buffer;
    uint16_t bytes_read = 0;
    dos_error_code_t err;

    err = dos_read_file((dos_file_handle_t)(unsigned int)stream, 1, &buffer, &bytes_read);

    if (err != DOS_SUCCESS || bytes_read == 0) return EOF;
    return (unsigned char)buffer;   // unsigned char cast to int (per C standard)
}

// string input
char* fgets(char* s, int size, FILE* stream) {
    if (!s || !stream || size <= 0) return NULL;

    dos_file_handle_t handle = (dos_file_handle_t)(unsigned int)stream;
    char* p = s;
    uint16_t bytes_read = 0;
    dos_error_code_t err;
    size--;

    if (handle == DOS_STDIN_HANDLE) { // character by character input
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
        while(*p++ != '\n' && p != s + bytes_read);
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
        //errno = dos_to_errno(err);
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
