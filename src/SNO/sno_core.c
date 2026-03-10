#include "sno_core.h"

// Helper functions
static bool char_in_set(char c, view_t charset) {
    cursor_t p = charset.begin;
    while (p < charset.end && *p != c) p++;
    return p != charset.end;
}

static bool char_in_cstr(char c, const char* charset) {
    while (*charset && *charset != c) charset++;
    return *charset != '\0';
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
// 2.3
bool str(view_t* subject, const char* match) {
    if (!subject || !subject->begin || !match) return false;
    if (*match == '\0') return true; // empty match string always succeeds (SNOBOL null string semantics)

    cursor_t s = subject->begin;
    const char* m = match;
    // compare character by character while both have data
    while (*m != '\0' && s < subject->end && *s == *m) {
        s++;
        m++;
    }

    if (*m != '\0') return false;   // partial match = failure (subject ended early)
    subject->begin = s;             // advance cursor past matched literal
    return true;
}

bool chr(view_t* subject, char c) {
    if (!subject || !subject->begin || subject->begin >= subject->end) return false;
    if (*subject->begin != c) return false; // check ONLY current character
    subject->begin++;                       // advance cursor past matched literal
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
    any(&temp, "+-");  // optional sign (atomic)

    cursor_t p = temp.begin;
    if (!span(&temp, "0123456789")) return false;  // require digits

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

//2.6
bool nul(view_t* subject) {
    if (!subject || !subject->begin || !subject->end) return false;
    // Always succeeds without consuming characters
    return true;
}

// 2.7
unsigned int at(view_t* subject, cursor_t p) {
    if (!subject || !subject->begin || !p) return 0;
    if (p < subject->begin || p > subject->end) return 0;  /* out of bounds */
    return (unsigned int)(p - subject->begin) + 1u;        /* 1-based index */
}

// 2.8
bool len(view_t* subject, unsigned int length) {
    if (!subject || !subject->begin || size(*subject) < length) return false;
    subject->begin += length;
    return true;
}

// 2.9
bool span(view_t* subject, const char* charset) {
    if (!subject || !subject->begin || !subject->end || !charset) return false;
    if (subject->begin >= subject->end) return false;   // 1+ requires non-empty subject
    if (*charset == '\0') return false;                 // empty charset always fails

    cursor_t start = subject->begin;
    while (subject->begin < subject->end && char_in_cstr(*subject->begin, charset))
        subject->begin++;
    return subject->begin != start;  // true iff ≥1 char matched
}

bool brk(view_t* subject, const char* charset) {
    if (!subject || !subject->begin || !subject->end || !charset) return false;
    // 0+ semantics: empty subject is VALID (skip 0 chars)
    // Empty charset is also valid — will consume entire subject
    while (subject->begin < subject->end && !char_in_cstr(*subject->begin, charset))
        subject->begin++;
    return true;  // always succeeds for valid inputs
}

bool any(view_t* subject, const char* charset) {
    if (!subject || !subject->begin || !subject->end || !charset) return false;
    if (subject->begin >= subject->end) return false;  // 1+ requires non-empty subject
    if (*charset == '\0') return false;                 // empty charset always fails

    if (!char_in_cstr(*subject->begin, charset)) return false;
    subject->begin++;
    return true;
}

bool notany(view_t* subject, const char* charset) {
    if (!subject || !subject->begin || !subject->end || !charset) return false;
    if (subject->begin >= subject->end) return false;  // 1+ requires non-empty subject

    // Empty charset: nothing excluded, so any char matches
    if (*charset == '\0') {
        subject->begin++;
        return true;
    }

    if (char_in_cstr(*subject->begin, charset)) return false;
    subject->begin++;
    return true;
}
