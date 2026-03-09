#ifndef SNO_TEST_H
#define SNO_TEST_H

#include "../SNO/sno.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>

void test_bind(void) {
    view_t v = bind("TEST");
    assert(v.begin && v.end && size(v) == 4);

    v = bind("");
    assert(v.begin && v.begin == v.end && size(v) == 0);

    v = bind(NULL);
    assert(!v.begin && !v.end && size(v) == 0);
}

void test_view(void) {
    char buf[] = "HELLO";

    view_t v = view(&buf[1], &buf[4]);
    assert(v.begin == &buf[1] && v.end == &buf[4] && size(v) == 3);

    v = view(buf, buf);
    assert(size(v) == 0);

    v = view(NULL, buf);
    assert(!v.begin && v.end == buf);

    v = view(buf, NULL);
    assert(v.begin == buf && !v.end);

    v = view(NULL, NULL);
    assert(!v.begin && !v.end);

    v = view(&buf[3], &buf[1]);
    assert(v.begin == &buf[3] && v.end == &buf[1]);
}

void test_size(void) {
    assert(size(bind("ABC")) == 3);
    assert(size(view("X", "X")) == 0);
    assert(size(view(NULL, "TEST")) == 0);
    assert(size(view("TEST", NULL)) == 0);
    assert(size(view(NULL, NULL)) == 0);

    char s[] = "TEST";
    int sz = size(view(&s[3], &s[1]));
    assert(sz >= 0);

    assert(size(bind(NULL)) == 0);
}

void test_str(void) {
    char input[] = "HELLO";
    view_t sub = bind(input);
    assert(str(&sub, bind("HEL")) && sub.begin == input + 3 && *sub.begin == 'L');

    sub = bind("TEST");
    cursor_t orig = sub.begin;
    assert(!str(&sub, bind("FAIL")) && sub.begin == orig);

    sub = bind("ANY");
    orig = sub.begin;
    assert(str(&sub, bind("")) && sub.begin == orig);

    sub = bind("HI");
    orig = sub.begin;
    assert(!str(&sub, bind("HELLO")) && sub.begin == orig);

    sub = view(NULL, NULL);
    assert(!str(&sub, bind("X")) && !sub.begin && !sub.end);

    sub = bind("SAFE");
    orig = sub.begin;
    assert(!str(&sub, view(NULL, NULL)) && sub.begin == orig);

    sub = view(&input[3], &input[1]);
    orig = sub.begin;
    assert(!str(&sub, bind("X")) && sub.begin == orig);

    sub = bind("");
    orig = sub.begin;
    assert(!str(&sub, bind("X")) && sub.begin == orig);

    sub = bind("Hello");
    orig = sub.begin;
    assert(!str(&sub, bind("HELLO")) && sub.begin == orig);
}

void test_any(void) {
    view_t sub;
    cursor_t orig;

    char buf1[] = "XYZ";
    sub = bind(buf1);
    orig = sub.begin;
    assert(any(&sub, bind("XYZ")) && sub.begin == orig + 1 && *sub.begin == 'Y');

    char buf2[] = "!5";
    sub = bind(buf2);
    orig = sub.begin;
    assert(any(&sub, bind("!@#$%")) && sub.begin == orig + 1 && *sub.begin == '5');

    char buf3[] = "X";
    sub = bind(buf3);
    orig = sub.begin;
    assert(!any(&sub, bind("ABC")) && sub.begin == orig);

    sub = bind("");
    orig = sub.begin;
    assert(!any(&sub, bind("A")) && sub.begin == orig);

    char buf4[] = "A";
    sub = bind(buf4);
    orig = sub.begin;
    assert(!any(&sub, bind("")) && sub.begin == orig);

    assert(!any(NULL, bind("A")));

    sub = view(NULL, NULL);
    assert(!any(&sub, bind("A")) && !sub.begin && !sub.end);

    char buf5[] = "A";
    sub = view(buf5, NULL);
    orig = sub.begin;
    assert(!any(&sub, bind("A")) && sub.begin == orig && sub.end == NULL);

    char buf6[] = "A";
    sub = view(NULL, buf6);
    assert(!any(&sub, bind("A")) && !sub.begin && sub.end == buf6);

    char buf7[] = "a";
    sub = bind(buf7);
    orig = sub.begin;
    assert(!any(&sub, bind("A")) && sub.begin == orig);

    char buf8[] = "AAA";
    sub = bind(buf8);
    orig = sub.begin;
    assert(any(&sub, bind("A")) && sub.begin == orig + 1 && *sub.begin == 'A');

    char buf9[] = "123abc";
    sub = bind(buf9);
    int count = 0;
    while (any(&sub, bind("0123456789"))) count++;
    assert(count == 3 && strncmp(sub.begin, "abc", 3) == 0);

    char buf10[] = "Z";
    sub = bind(buf10);
    assert(any(&sub, bind("Z")) && sub.begin == sub.end);

    char buf11[] = "!!!END";
    sub = bind(buf11);
    orig = sub.begin;
    assert(any(&sub, bind("!")) && sub.begin == orig + 1 && *sub.begin == '!');

    char buf12[] = "!!!END";
    sub = view(&buf12[2], &buf12[0]);
    orig = sub.begin;
    assert(!any(&sub, bind("!")) && sub.begin == orig);
}

void test_notany(void) {
    view_t sub;
    cursor_t orig;

    char buf1[] = "X9Z";
    sub = bind(buf1);
    orig = sub.begin;
    assert(notany(&sub, bind("012345678")) && sub.begin == orig + 1 && *sub.begin == '9');

    char buf2[] = "A";
    sub = bind(buf2);
    orig = sub.begin;
    assert(notany(&sub, bind("")) && sub.begin == orig + 1 && sub.begin == sub.end);

    char buf3[] = "5";
    sub = bind(buf3);
    orig = sub.begin;
    assert(!notany(&sub, bind("0123456789")) && sub.begin == orig);

    char buf4[] = "";
    sub = bind(buf4);
    orig = sub.begin;
    assert(!notany(&sub, bind("A")) && sub.begin == orig);

    char buf5[] = "Z";
    sub = bind(buf5);
    assert(notany(&sub, bind("ABC")) && sub.begin == sub.end);
    orig = sub.begin;
    assert(!notany(&sub, bind("A")) && sub.begin == orig);

    assert(!notany(NULL, bind("A")));

    sub = view(NULL, NULL);
    assert(!notany(&sub, bind("A")) && !sub.begin && !sub.end);

    char buf6[] = "A";
    sub = view(buf6, NULL);
    orig = sub.begin;
    assert(!notany(&sub, bind("A")) && sub.begin == orig && sub.end == NULL);

    char buf7[] = "A";
    sub = view(NULL, buf7);
    assert(!notany(&sub, bind("A")) && !sub.begin && sub.end == buf7);

    char buf8[] = "a";
    sub = bind(buf8);
    orig = sub.begin;
    assert(!notany(&sub, bind("a")) && sub.begin == orig);

    char buf9[] = "FIELD,REST";
    sub = bind(buf9);
    while (notany(&sub, bind(","))) ;
    assert(*sub.begin == ',');

    char buf10[] = "FIELD,REST";
    sub = view(&buf10[2], &buf10[0]);
    orig = sub.begin;
    assert(!notany(&sub, bind(",")) && sub.begin == orig);
}

void test_span(void) {
    view_t sub;
    cursor_t orig;

    char buf1[] = "A123";
    sub = bind(buf1);
    orig = sub.begin;
    assert(span(&sub, bind("A")) && sub.begin == orig + 1 && *sub.begin == '1');

    char buf2[] = "12345abc";
    sub = bind(buf2);
    orig = sub.begin;
    assert(span(&sub, bind("0123456789")) && sub.begin == orig + 5 && *sub.begin == 'a');

    char buf3[] = "999";
    sub = bind(buf3);
    assert(span(&sub, bind("0123456789")) && sub.begin == sub.end);

    char buf4[] = "X123";
    sub = bind(buf4);
    orig = sub.begin;
    assert(!span(&sub, bind("0123456789")) && sub.begin == orig);

    char buf5[] = "";
    sub = bind(buf5);
    orig = sub.begin;
    assert(!span(&sub, bind("A")) && sub.begin == orig);

    char buf6[] = "A";
    sub = bind(buf6);
    orig = sub.begin;
    assert(!span(&sub, bind("")) && sub.begin == orig);

    char buf7[] = "aaaabbb";
    sub = bind(buf7);
    orig = sub.begin;
    assert(span(&sub, bind("a")) && sub.begin == orig + 4 && *sub.begin == 'b');

    char buf8[] = "321cba";
    sub = bind(buf8);
    orig = sub.begin;
    assert(span(&sub, bind("abc123")) && sub.begin == orig + 6 && sub.begin == sub.end);

    char buf9[] = "aAaA";
    sub = bind(buf9);
    orig = sub.begin;
    assert(span(&sub, bind("a")) && sub.begin == orig + 1 && *sub.begin == 'A');

    char buf10[] = "123,456,789";
    sub = bind(buf10);
    assert(span(&sub, bind("0123456789")) && str(&sub, bind(",")) && span(&sub, bind("0123456789")) && *sub.begin == ',');

    assert(!span(NULL, bind("A")));

    sub = view(NULL, NULL);
    assert(!span(&sub, bind("A")) && !sub.begin && !sub.end);

    char buf11[] = "A";
    sub = view(buf11, NULL);
    orig = sub.begin;
    assert(!span(&sub, bind("A")) && sub.begin == orig && sub.end == NULL);

    char buf12[] = "A";
    sub = view(NULL, buf12);
    assert(!span(&sub, bind("A")) && !sub.begin && sub.end == buf12);

    char buf13[] = "Z";
    sub = bind(buf13);
    assert(span(&sub, bind("Z")) && sub.begin == sub.end);

    char buf14[] = "AAA";
    sub = bind(buf14);
    orig = sub.begin;
    assert(span(&sub, bind("A")) && sub.begin == orig + 3);
    sub = bind(buf14);
    orig = sub.begin;
    assert(span(&sub, bind("XYZA")) && sub.begin == orig + 3);

    char buf15[] = "AAA";
    sub = view(&buf15[2], &buf15[0]);
    orig = sub.begin;
    assert(!span(&sub, bind("A")) && sub.begin == orig);
}

void test_brk(void) {
    view_t sub;
    cursor_t orig;

    char buf1[] = "FIELD,REST";
    sub = bind(buf1);
    orig = sub.begin;
    assert(brk(&sub, bind(",")) && sub.begin == orig + 5 && *sub.begin == ',');

    char buf2[] = ",REST";
    sub = bind(buf2);
    orig = sub.begin;
    assert(brk(&sub, bind(",")) && sub.begin == orig && *sub.begin == ',');

    char buf3[] = "FIELD";
    sub = bind(buf3);
    assert(brk(&sub, bind(",")) && sub.begin == sub.end);

    char buf4[] = "";
    sub = bind(buf4);
    assert(brk(&sub, bind(",")) && sub.begin == sub.end);

    char buf5[] = "TEST";
    sub = bind(buf5);
    assert(brk(&sub, bind("")) && sub.begin == sub.end);

    char buf6[] = "aaaabbb";
    sub = bind(buf6);
    orig = sub.begin;
    assert(brk(&sub, bind("b")) && sub.begin == orig + 4 && *sub.begin == 'b');

    char buf7[] = "aAaA";
    sub = bind(buf7);
    orig = sub.begin;
    assert(brk(&sub, bind("A")) && sub.begin == orig + 1 && *sub.begin == 'A');

    char buf8[] = "123,456,789";
    sub = bind(buf8);
    assert(brk(&sub, bind(",")) && any(&sub, bind(",")) && brk(&sub, bind(",")) && any(&sub, bind(",")) && brk(&sub, bind(",")) && sub.begin == sub.end);

    assert(!brk(NULL, bind(",")));

    sub = view(NULL, NULL);
    assert(!brk(&sub, bind(",")) && !sub.begin && !sub.end);

    char buf9[] = "A";
    sub = view(buf9, NULL);
    orig = sub.begin;
    assert(!brk(&sub, bind(",")) && sub.begin == orig && sub.end == NULL);

    char buf10[] = "A";
    sub = view(NULL, buf10);
    assert(!brk(&sub, bind(",")) && !sub.begin && sub.end == buf10);

    char buf11[] = "Z";
    sub = bind(buf11);
    assert(brk(&sub, bind("A")) && sub.begin == sub.end);

    char buf12[] = "ABcDE";
    sub = bind(buf12);
    orig = sub.begin;
    assert(brk(&sub, bind("c")) && sub.begin == orig + 2 && *sub.begin == 'c');
}

void test_skip(void) {
    view_t sub;
    cursor_t orig;

    char buf1[] = "   TEXT";
    sub = bind(buf1);
    orig = sub.begin;
    assert(skip(&sub, bind(" \t")) && sub.begin == orig + 3 && *sub.begin == 'T');

    char buf2[] = "TEXT";
    sub = bind(buf2);
    orig = sub.begin;
    assert(skip(&sub, bind(" \t")) && sub.begin == orig && *sub.begin == 'T');

    char buf3[] = "";
    sub = bind(buf3);
    orig = sub.begin;
    assert(skip(&sub, bind(" ")) && sub.begin == orig);

    char buf4[] = "TEXT";
    sub = bind(buf4);
    orig = sub.begin;
    assert(skip(&sub, bind("")) && sub.begin == orig);

    char buf5[] = "   TEXT";
    sub = bind(buf5);
    assert(skip(&sub, bind(" ")));
    orig = sub.begin;
    assert(skip(&sub, bind(" ")) && sub.begin == orig);

    char buf6[] = "15L";
    sub = bind(buf6);
    skip(&sub, bind(" \t"));
    assert(span(&sub, bind("0123456789")) && str(&sub, bind("L")));

    char buf7[] = "  15L";
    sub = bind(buf7);
    skip(&sub, bind(" \t"));
    assert(span(&sub, bind("0123456789")) && str(&sub, bind("L")));

    char buf8[] = "aaaabbb";
    sub = bind(buf8);
    orig = sub.begin;
    assert(skip(&sub, bind("a")) && sub.begin == orig + 4 && *sub.begin == 'b');

    assert(!skip(NULL, bind(" ")));

    sub = view(NULL, NULL);
    assert(!skip(&sub, bind(" ")) && !sub.begin && !sub.end);

    char buf9[] = "A";
    sub = view(buf9, NULL);
    orig = sub.begin;
    assert(!skip(&sub, bind(" ")) && sub.begin == orig && sub.end == NULL);

    char buf10[] = "A";
    sub = view(NULL, buf10);
    assert(!skip(&sub, bind(" ")) && !sub.begin && sub.end == buf10);

    char buf11[] = "   ";
    sub = bind(buf11);
    assert(skip(&sub, bind(" ")) && sub.begin == sub.end);

    char buf12[] = "\t  \tTEXT";
    sub = bind(buf12);
    orig = sub.begin;
    assert(skip(&sub, bind(" \t")) && sub.begin == orig + 4 && *sub.begin == 'T');
}

void test_var(void) {
    char buf[20];
    view_t sub;

    sub = bind("TEST");
    assert(var(&sub, buf, sizeof(buf)) && strcmp(buf, "TEST") == 0 && sub.begin == sub.end);

    sub = bind("");
    assert(var(&sub, buf, sizeof(buf)) && buf[0] == '\0' && sub.begin == sub.end);

    sub = bind("12345");
    cursor_t orig = sub.begin;
    assert(!var(&sub, buf, 5) && sub.begin == orig);

    sub = bind("ABC");
    orig = sub.begin;
    assert(!var(&sub, buf, 3) && sub.begin == orig);

    sub = bind("X");
    orig = sub.begin;
    assert(!var(&sub, buf, 0) && sub.begin == orig);

    sub = bind("SAFE");
    orig = sub.begin;
    assert(!var(&sub, NULL, sizeof(buf)) && sub.begin == orig);

    assert(!var(NULL, buf, sizeof(buf)));

    sub = view(NULL, NULL);
    assert(!var(&sub, buf, sizeof(buf)) && !sub.begin && !sub.end);

    char composite[] = "PREFIXHELLOSUFFIX";
    sub = view(&composite[6], &composite[11]);
    assert(var(&sub, buf, sizeof(buf)) && strcmp(buf, "HELLO") == 0 && sub.begin == sub.end);

    char cmd[] = "15L";
    sub = bind(cmd);
    span(&sub, bind("0123456789"));
    view_t num_view = view(cmd, sub.begin);
    assert(var(&num_view, buf, sizeof(buf)) && strcmp(buf, "15") == 0);
    assert(str(&sub, bind("L")));
}

void test_num(void) {
    view_t sub;
    int n;

    // Valid positives (within edlin's actual usage range)
    sub = bind("123");
    assert(num(&sub, &n) && n == 123 && sub.begin == sub.end);

    sub = bind("+456");
    assert(num(&sub, &n) && n == 456 && sub.begin == sub.end);

    sub = bind("0");
    assert(num(&sub, &n) && n == 0 && sub.begin == sub.end);

    // Portable max/min guaranteed by C standard (all vintage systems support these)
    sub = bind("32767");   // Max portable positive (C standard min range)
    assert(num(&sub, &n) && n == 32767 && sub.begin == sub.end);

    sub = bind("-32767");  // Min portable negative (C standard min range)
    assert(num(&sub, &n) && n == -32767 && sub.begin == sub.end);

    // Realistic edlin line numbers (safe for 16-bit and 32-bit)
    sub = bind("9999");
    assert(num(&sub, &n) && n == 9999 && sub.begin == sub.end);

    // Stop at NOTANY boundary
    sub = bind("123x");
    assert(num(&sub, &n) && n == 123 && *sub.begin == 'x');

    sub = bind("-456y");
    assert(num(&sub, &n) && n == -456 && *sub.begin == 'y');

    // Fail on invalid first char
    sub = bind("x123");
    assert(!num(&sub, &n));

    sub = bind(" 123");
    assert(!num(&sub, &n));

    sub = bind("");
    assert(!num(&sub, &n));

    // Reject malformed signs
    sub = bind("+");
    assert(!num(&sub, &n));

    sub = bind("-");
    assert(!num(&sub, &n));

    sub = bind("++1");
    assert(!num(&sub, &n));

    sub = bind("--1");
    assert(!num(&sub, &n));

    sub = bind("+-1");
    assert(!num(&sub, &n));

    sub = bind("-+1");
    assert(!num(&sub, &n));

    sub = bind("-x");
    assert(!num(&sub, &n));

    sub = bind("+y");
    assert(!num(&sub, &n));

    sub = bind("-0");
    assert(num(&sub, &n) && n == 0 && sub.begin == sub.end);

    // Atomic rollback on failure
    char buf[] = "x123";
    sub = bind(buf);
    cursor_t orig = sub.begin;
    assert(!num(&sub, &n) && sub.begin == orig);

    // NULL safety
    assert(!num(NULL, &n));

    sub = view(NULL, NULL);
    assert(!num(&sub, &n));

    char buf1[] = "123";
    sub = view(buf1, NULL);
    orig = sub.begin;
    assert(!num(&sub, &n) && sub.begin == orig && sub.end == NULL);

    char buf2[] = "123";
    sub = view(NULL, buf2);
    assert(!num(&sub, &n) && !sub.begin && sub.end == buf2);
}

void test(void) {
    test_bind();
    test_view();
    test_size();
    test_str();
    test_any();
    test_notany();
    test_span();
    test_brk();
    test_skip();
    test_var();
    test_num();
    printf("All SNOBOL-C primitive tests pass!\n");
}

#endif
