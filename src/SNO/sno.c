#include "sno.h"

// Internal helper function - check if a single character is in the given charset
// SUCCESS: returns true (character found in charset)
// FAILURE: returns false (character not in charset or empty set)
static bool char_in_set(char c, view_t charset) {
    cursor_t p = charset.begin;
    while (p < charset.end && *p != c) p++;
    return p != charset.end;
}

// Construction and sizing
view_t bind(const char* cstr) {
    view_t v;
    v.begin = v.end = cstr;
    if (cstr) while (*v.end) v.end++;
    return v;
}

view_t view(cursor_t begin, cursor_t end) {
    view_t v;
    v.begin = begin;
    v.end = end;
    return v;
}

unsigned int size(view_t view) {
    return (view.begin && view.end && view.begin < view.end) ? (int)(view.end - view.begin) : 0;
}

// 2.3
bool str(view_t* subject, view_t pattern) {
    if (!subject || !subject->begin || !pattern.begin) return false;
    if (size(pattern) == 0) return true;    // empty pattern always matches

    cursor_t s = subject->begin;
    cursor_t p = pattern.begin;
    // compare character by character while both have data
    while (p < pattern.end && s < subject->end && *s == *p) {
        s++;
        p++;
    }

    if (p < pattern.end) return false;  // partial match = failure
    subject->begin = s;                 // advance cursor past the matched pattern
    return true;
}

bool chr(view_t* subject, char c) {
    if (!subject || !subject->begin) return false;

    cursor_t s = subject->begin;
    while (s < subject->end && *s != c) s++;

    if(s == subject->end) return false; // no match = failure
    subject->begin = s;                 // advance cursor past the matched pattern
    return true;
}
// 2.5
bool var(view_t* subject, char* buf, size_t buflen) {
    if (!subject || !subject->begin || !subject->end || !buf || buflen == 0) return false;
    int len = size(*subject);
    if (len < 0 || (size_t)len >= buflen) return false;  // need space for null terminator

    cursor_t p = subject->begin;
    while (p < subject->end) *buf++ = *p++;
    *buf = '\0';
    subject->begin = subject->end;  // consume entire view
    return true;
}

bool num(view_t* subject, int* n) {
    if (!subject || !subject->begin || !subject->end || !n) return false;
    if (subject->begin >= subject->end) return false;  // reject empty subject
    if (!char_in_set(*subject->begin, bind("+-0123456789"))) return false;

    view_t temp = *subject;
    any(&temp, bind("+-"));  // optional sign (atomic)

    cursor_t p = temp.begin;
    if (!span(&temp, bind("0123456789"))) return false;  // require digits

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


bool any(view_t* subject, view_t charset) {
    if (!subject || !subject->begin || !subject->end || !charset.begin || !charset.end) return false;
    if (subject->begin >= subject->end) return false;  // 1+ requires non-empty subject
    if (charset.begin >= charset.end) return false;     // empty set always fails

    if (!char_in_set(*subject->begin, charset)) return false;
    subject->begin++;
    return true;
}

bool notany(view_t* subject, view_t charset) {
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

bool span(view_t* subject, view_t charset) {
    if (!subject || !subject->begin || !subject->end || !charset.begin || !charset.end) return false;
    if (subject->begin >= subject->end) return false;  // 1+ requires non-empty subject
    if (charset.begin >= charset.end) return false;     // empty set always fails

    cursor_t start = subject->begin;
    while (subject->begin < subject->end && char_in_set(*subject->begin, charset))
        subject->begin++;
    return subject->begin != start;
}

bool brk(view_t* subject, view_t charset) {
    if (!subject || !subject->begin || !subject->end || !charset.begin || !charset.end) return false;
    // 0+ semantics: empty subject is VALID (skip 0 chars)
    while (subject->begin < subject->end && !char_in_set(*subject->begin, charset))
        subject->begin++;
    return true;  // always succeeds for valid inputs
}

bool skip(view_t* subject, view_t charset) {
    if (!subject || !subject->begin || !subject->end || !charset.begin || !charset.end) return false;
    // 0+ semantics: empty subject is VALID (skip 0 chars)
    while (subject->begin < subject->end && char_in_set(*subject->begin, charset))
        subject->begin++;
    return true;  // always succeeds for valid inputs
}
