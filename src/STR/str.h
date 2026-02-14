#ifndef STR_H
#define STR_H

#include <stddef.h>
#include <string.h>

typedef struct {
    const char* begin;
    const char* end;
} str_view_t;

str_view_t str_make_view(const char* cstring);

int str_view_size(const str_view_t view);

str_view_t str_token(const char* begin, const char* end);

int str_int(str_view_t view);

size_t str_var(const str_view_t view, char* cstring, size_t csize);

const char* str_first_char(str_view_t charset, char c);

const char* str_any(str_view_t subject, str_view_t charset);

const char* str_match(str_view_t subject, str_view_t pattern);

const char* str_span(str_view_t subject, str_view_t charset);

int str_eat(str_view_t* view, const char* cursor);

#define str_skip(view, charset) do { while (str_eat(&(view), str_any((view), (charset)))); } while(0)

#endif
