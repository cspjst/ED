#include "str.h"

str_view_t str_make_view(const char* cstring) {
    str_view_t v;
    v.begin = v.end = cstring;

    if (cstring) while (*v.end) v.end++;

    return v;
}

int str_view_size(const str_view_t view) {
    return (int)(view.end - view.begin);
}

str_view_t str_token(const char* begin, const char* end) {
    str_view_t v;
    v.begin = begin;
    v.end = end;
    return v;
}

size_t str_var(const str_view_t view, char* cstring, size_t csize) {
    size_t vsize = str_view_size(view);
    if(!cstring || vsize >= csize) return 0;

    memcpy(cstring, view.begin, vsize);
    cstring[vsize] = '\0';

    return vsize;
}

const char* str_first_char(str_view_t charset, char c) {
    for (const char* p = charset.begin; p < charset.end; p++)
        if (*p == c) return p;
    return NULL;
}

const char* str_any(str_view_t subject, str_view_t charset) {
    if (subject.begin >= subject.end) return NULL;
    return str_first_char(charset, *subject.begin) ? subject.begin + 1 : NULL;
}

const char* str_match(str_view_t subject, str_view_t pattern) {
    const char* s = subject.begin;
    const char* p = pattern.begin;
    while (p < pattern.end && s < subject.end && *s == *p) { s++; p++; }
    return (p == pattern.end) ? s : NULL;
}

const char* str_span(str_view_t subject, str_view_t charset) {
    const char* s = subject.begin;
    if (s == subject.end || !str_first_char(charset, *s)) return NULL;
    while (++s < subject.end && str_first_char(charset, *s))
        ;
    return s;
}

int str_eat(str_view_t* view, const char* cursor) {
    if (!cursor) return 0;
    view->begin = cursor;
    return 1;
}
