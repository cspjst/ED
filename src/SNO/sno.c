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
    if (!subject || !subject->begin || !subject->end || !charset.begin || !charset.end) return false;
    if (subject->begin >= subject->end) return false;  // 1+ requires non-empty subject
    if (charset.begin >= charset.end) return false;     // empty set always fails

    if (!char_in_set(*subject->begin, charset)) return false;
    subject->begin++;
    return true;
}

bool sno_notany(sno_view_t* subject, sno_view_t charset) {
    if (!subject || !subject->begin || !subject->end || !charset.begin || !charset.end) return false;
    if (subject->begin >= subject->end) return false;  // 1+ requires non-empty subject

    if (charset.begin >= charset.end) {
        subject->begin++;
        return true;  // empty charset matches any char
    }

    if (char_in_set(*subject->begin, charset)) return false;
    subject->begin++;
    return true;
}

bool sno_span(sno_view_t* subject, sno_view_t charset) {
    if (!subject || !subject->begin || !subject->end || !charset.begin || !charset.end) return false;
    if (subject->begin >= subject->end) return false;  // 1+ requires non-empty subject
    if (charset.begin >= charset.end) return false;     // empty set always fails

    sno_cursor_t start = subject->begin;
    while (subject->begin < subject->end && char_in_set(*subject->begin, charset))
        subject->begin++;
    return subject->begin != start;
}

bool sno_break(sno_view_t* subject, sno_view_t charset) {
    if (!subject || !subject->begin || !subject->end || !charset.begin || !charset.end) return false;
    // 0+ semantics: empty subject is VALID (skip 0 chars)
    while (subject->begin < subject->end && !char_in_set(*subject->begin, charset))
        subject->begin++;
    return true;  // always succeeds for valid inputs
}

bool sno_skip(sno_view_t* subject, sno_view_t charset) {
    if (!subject || !subject->begin || !subject->end || !charset.begin || !charset.end) return false;
    // 0+ semantics: empty subject is VALID (skip 0 chars)
    while (subject->begin < subject->end && char_in_set(*subject->begin, charset))
        subject->begin++;
    return true;  // always succeeds for valid inputs
}

bool sno_var(sno_view_t* subject, char* buf, size_t buflen) {
    if (!subject || !subject->begin || !subject->end || !buf || buflen == 0) return false;
    int len = sno_size(*subject);
    if (len < 0 || (size_t)len >= buflen) return false;  // need space for null terminator

    sno_cursor_t p = subject->begin;
    while (p < subject->end) *buf++ = *p++;
    *buf = '\0';
    subject->begin = subject->end;  // consume entire view
    return true;
}

bool sno_int(sno_view_t* subject, int* n) {
    if (!subject || !subject->begin || !subject->end || !n) return false;
    if (subject->begin >= subject->end) return false;  // reject empty subject
    if (!char_in_set(*subject->begin, sno_bind("+-0123456789"))) return false;

    sno_view_t temp = *subject;
    sno_any(&temp, sno_bind("+-"));  // optional sign (atomic)

    sno_cursor_t p = temp.begin;
    if (!sno_span(&temp, sno_bind("0123456789"))) return false;  // require digits

    int num = 0;
    while (p < temp.begin) {
        num = num * 10 + (*p - '0');
        p++;
    }
    if (*subject->begin == '-') num = -num;

    *n = num;
    *subject = temp;
    return true;
}
