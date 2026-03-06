/**
 * @file dos_stdio.h
 * @brief Minimal C99 stdio implementation for DOS environments
 *
 * MEMORY SAVING DESIGN:
 * - Direct DOS interrupt calls avoid libc overhead
 * - Macros for simple functions (putc, putchar) eliminate call overhead
 * - No buffering - direct character-by-character I/O
 * - Minimal error checking focused on essential cases
 * - Recursive printf helpers avoid large format string buffers
 * - uint16_t mode parsing in fopen avoids string processing
 *
 * FILE TYPE PUNNING:
 * - FILE is typedef'd as dos_file_handle_t (uint16_t)
 * - fopen returns (FILE*)handle - casting integer to pointer
 * - All functions cast back: (dos_file_handle_t)(uintptr_t)stream
 * - This maintains standard C FILE* interface while using simple integers
 * - stdin/stdout/stderr are handles 0,1,2 cast to FILE*
 *
 * COMPROMISES:
 * - No file buffering (performance tradeoff)
 * - Limited format specifiers in printf
 * - No locale support
 * - Console-only \r\n conversion
 * - 16-bit file positions only
 */
#ifndef DOS_STDIO_H
#define DOS_STDIO_H

#ifndef __LARGE__
    #error "This module requires large memory model (ie far data pointers)"
#endif

#include "../DOS/dos_file_services.h"
#include "dos_stddef.h"

#define DOS_STDIO_PRINTF_FLOAT
#define DOS_STDIO_PRINTF_SCIENTIFIC
#define DOS_STDIO_FILE_HANDLING

#define EOF (-1)

#define SEEK_SET FSEEK_SET
#define SEEK_CUR FSEEK_CUR
#define SEEK_END FSEEK_END

#define DOS_STDIO_GETS_MAX  256

typedef dos_file_handle_t FILE;

// C type punning
#define stdin  ((FILE*)(uintptr_t)0)
#define stdout ((FILE*)(uintptr_t)1)
#define stderr ((FILE*)(uintptr_t)2)

// character output
int fputc(int c, FILE* stream);
#define putc(c, stream) fputc(c, stream)
#define putchar(c) fputc(c, stdout)

// string output
int fputs(const char* str, FILE* stream);
#define puts(str) (fputs(str, stdout), fputc('\n', stdout))

// formatted output - fprintf is core, printf is macro
int fprintf(FILE* stream, const char* format, ...);
#define printf(fmt, ...) fprintf(stdout, fmt, ##__VA_ARGS__)

// character input
int fgetc(FILE* stream);
#define getc(stream) fgetc(stream)
#define getchar() fgetc(stdin)

// string input
char* fgets(char* s, int size, FILE* stream);
#define gets(s) fgets((s), DOS_STDIO_GETS_MAX, stdin)

// formatted input
int fscanf(FILE* stream, const char* format, ...);
#define scanf(fmt, ...) fscanf(stdin, fmt, ##__VA_ARGS__)

// error output
void perror(const char *s);

// file operations
#ifdef DOS_STDIO_FILE_HANDLING

FILE* fopen(const char* filename, const char* mode);
size_t fread(void* ptr, size_t size, size_t count, FILE* stream);
size_t fwrite(const void* ptr, size_t size, size_t count, FILE* stream);
int fseek(FILE* stream, long offset, int origin);
long ftell(FILE* stream);
int fclose(FILE* stream);

#endif // DOS_STDIO_FILE_HANDLING

#endif // DOS_STDIO_H
