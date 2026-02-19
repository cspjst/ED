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

// Internal helper function - check if a single character is in the given charset
// SUCCESS: returns true (character found in charset)
// FAILURE: returns false (character not in charset or empty set)
// RETURNS: true if char in set, false otherwise
static bool char_in_set(char c, sno_view_t charset) {
    sno_cursor_t p = charset.begin;
    while (p < charset.end && *p != c) p++;
    return p != charset.end;
}

bool sno_any(sno_view_t* subject, sno_view_t charset) {
    if (!subject || !subject->begin || !charset.begin
        || sno_size(charset) == 0  // empty set always fails
    ) return false;

    if (!char_in_set(*subject->begin, charset)) return false;
    subject->begin++;
    return true;
}

bool sno_notany(sno_view_t* subject, sno_view_t charset) {
    if (!subject || !subject->begin || !charset.begin
        || subject->begin >= subject->end   // empty subject is a fail
    ) return false;

    if (sno_size(charset) == 0) {
        subject->begin++;
        return true;    // empty charset matches any character (since nothing is forbidden)
    }

    if (char_in_set(*subject->begin, charset)) return false;
    subject->begin++;
    return true;
}

bool sno_span(sno_view_t* subject, sno_view_t charset) {
    if (!subject || !subject->begin || !charset.begin) return false;

    sno_cursor_t start = subject->begin;
    while (subject->begin < subject->end && char_in_set(*subject->begin, charset))
        subject->begin++;

    return subject->begin != start;
}

bool sno_break(sno_view_t* subject, sno_view_t charset) {
    if (!subject || !subject->begin || !charset.begin) return false;
    
    while (subject->begin < subject->end && !char_in_set(*subject->begin, charset))
        subject->begin++;

    return true;  // even if we skipped 0 chars, it's still valid
}

bool sno_skip(sno_view_t* subject, sno_view_t charset) {
    if (!subject || !subject->begin || !charset.begin) return false;

    while (subject->begin < subject->end && char_in_set(*subject->begin, charset)) {
        subject->begin++;
    }

    return true;  // even if skipped 0 chars, it's still valid
}

bool sno_var(sno_view_t* subject, char* buf, size_t buflen) {
    if (!subject || !subject->begin || !buf || buflen == 0
        || sno_size(*subject) >= buflen // need at least len + 1 bytes for null terminator
    ) return false;

    while (subject->begin < subject->end) *buf++ = *subject->begin++;   // copy string
    *buf = '\0';  // null-terminate

    return true;
}

bool sno_int(sno_view_t* subject, int* n) {
    if (!subject || !subject->begin || !n 
        || subject->begin == subject->end
    ) return false;
    
    // first char must be valid number start
    if (!char_in_set(*subject->begin, sno_bind("+-0123456789"))) return false;
    
    sno_view_t temp = *subject;    // copy view

    sno_any(&temp, sno_bind("+-"));    // optional sign

    sno_cursor_t p = temp.begin;    // copy start 
    if (!sno_span(&temp, sno_bind("0123456789"))) return false;  // no digits after sign
    
    // parse digits
    int num = 0;
    while (p < temp.begin) {
        num = num * 10 + (*p - '0');
        p++;
    }
    if (*subject->begin == '-')  num = -num;    
    
    *n = num;
    *subject = temp;  // advance original cursor past the number
    return true;
}
