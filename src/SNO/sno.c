#include "sno.h"

sno_view_t sno_bind(const char* cstr) {
    sno_view_t v;
    v.begin = v.end = cstr;
    if (cstr) while (*v.end) v.end++;
    return v;
}

sno_view_t sno_view(sno_cursor_t begin, sno_cursor_t end) {
    sno_view_t v;
    v.begin = begin;
    v.end = end;
    return v;
}

int sno_size(sno_view_t view) {
    return (view.begin && view.end && view.begin < view.end) ? (int)(view.end - view.begin) : 0;
}

bool sno_lit(sno_view_t* subject, sno_view_t pattern) {
    if (!subject || !subject->begin || !pattern.begin) return false;
    if (sno_size(pattern) == 0) return true;    // empty pattern always matches

    sno_cursor_t s = subject->begin;
    sno_cursor_t p = pattern.begin;
    // compare character by character while both have data
    while (p < pattern.end && s < subject->end && *s == *p) {
        s++;
        p++;
    }

    if (p < pattern.end) return false;  // partial match = failure
    subject->begin = s;                 // advance cursor past the matched pattern
    return true;
}

bool sno_any(sno_view_t* subject, sno_view_t charset) {
    if (!subject || !subject->begin || !charset.begin ||
        sno_size(charset) == 0) {   // empty set always fails
        return false;
    }

    sno_cursor_t s = subject->begin;
    sno_cursor_t p = charset.begin;
    while(p < charset.end && *p != *s) p++; // scan the set

    if(p == charset.end) return false;  // no match = failure
    subject->begin = s + 1;             // advance cursor
    return true;
}

bool sno_notany(sno_view_t* subject, sno_view_t charset) {
    if (!subject || !subject->begin || !charset.begin) return false;

    // empty charset matches any character (since nothing is forbidden)
    if (sno_size(charset) == 0) {
        subject->begin = subject->begin + 1;
        return true;
    }

    sno_cursor_t s = subject->begin;
    sno_cursor_t p = charset.begin;
    while(p < charset.end && *p != *s) p++; // scan the set

    if (p != charset.end) return false; // any match = failure
    subject->begin = s + 1;
    return true;
}

//bool sno_span(sno_view_t* subject, sno_view_t charset);

//bool sno_skip(sno_view_t* subject, sno_view_t charset);

//bool sno_var(sno_view_t* subject, char* buf, size_t buflen);

//bool sno_int(sno_view_t* subject, int* out);


/*
bool str_span(str_view_t* subject, str_view_t charset) {
    if (!subject || !subject->begin || !charset.begin) return false;
    if (subject->begin >= subject->end) return false;

    str_cursor_t s = subject->begin;
    str_cursor_t next = str_first_char(s, charset);
    if (next == s) return false;  // First char not in set → failure

    s = next;
    while (s < subject->end) {
        next = str_first_char(s, charset);
        if (next == s) break;
        s = next;
    }

    subject->begin = s;
    return true;
}

bool str_skip(str_view_t* subject, str_view_t charset) {
    if (!subject || !subject->begin || !charset.begin) return false;

    bool matched = false;
    while (subject->begin < subject->end) {
        str_cursor_t prev = subject->begin;
        if (!str_any(subject, charset)) break;
        matched = true;
    }
    return matched;  // Returns true if ANY chars were skipped
}

bool str_var(str_view_t* subject, char* buf, size_t buflen) {
    if (!subject || !buf || buflen == 0) return false;

    int len = str_view_size(*subject);
    if (len < 0 || (size_t)len >= buflen) return false;  // Need space for null terminator

    memcpy(buf, subject->begin, (size_t)len);
    buf[len] = '\0';
    subject->begin = subject->end;  // Consume entire view
    return true;
}
 */
