#ifndef sno_TEST_H
#define sno_TEST_H

#include "sno.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>

void test_bind(void) {
    sno_view_t v = sno_bind("TEST");
    assert(v.begin && v.end && sno_size(v) == 4);

    v = sno_bind("");
    assert(v.begin && v.begin == v.end && sno_size(v) == 0);

    v = sno_bind(NULL);
    assert(!v.begin && !v.end && sno_size(v) == 0);
}

void test_view(void) {
    char buf[] = "HELLO";

    sno_view_t v = sno_view(&buf[1], &buf[4]);
    assert(v.begin == &buf[1] && v.end == &buf[4] && sno_size(v) == 3);

    v = sno_view(buf, buf);
    assert(sno_size(v) == 0);

    v = sno_view(NULL, buf);
    assert(!v.begin && v.end == buf);

    v = sno_view(buf, NULL);
    assert(v.begin == buf && !v.end);

    v = sno_view(NULL, NULL);
    assert(!v.begin && !v.end);

    v = sno_view(&buf[3], &buf[1]);
    assert(v.begin == &buf[3] && v.end == &buf[1]);
}

void test_size(void) {
    assert(sno_size(sno_bind("ABC")) == 3);
    assert(sno_size(sno_view("X", "X")) == 0);
    assert(sno_size(sno_view(NULL, "TEST")) == 0);
    assert(sno_size(sno_view("TEST", NULL)) == 0);
    assert(sno_size(sno_view(NULL, NULL)) == 0);

    char s[] = "TEST";
    int sz = sno_size(sno_view(&s[3], &s[1]));
    assert(sz >= 0);

    assert(sno_size(sno_bind(NULL)) == 0);
}

void test_lit(void) {
    char input[] = "HELLO";
    sno_view_t sub = sno_bind(input);
    assert(sno_lit(&sub, sno_bind("HEL")) && sub.begin == input + 3 && *sub.begin == 'L');

    sub = sno_bind("TEST");
    sno_cursor_t orig = sub.begin;
    assert(!sno_lit(&sub, sno_bind("FAIL")) && sub.begin == orig);

    sub = sno_bind("ANY");
    orig = sub.begin;
    assert(sno_lit(&sub, sno_bind("")) && sub.begin == orig);

    sub = sno_bind("HI");
    orig = sub.begin;
    assert(!sno_lit(&sub, sno_bind("HELLO")) && sub.begin == orig);

    sub = sno_view(NULL, NULL);
    assert(!sno_lit(&sub, sno_bind("X")) && !sub.begin && !sub.end);

    sub = sno_bind("SAFE");
    orig = sub.begin;
    assert(!sno_lit(&sub, sno_view(NULL, NULL)) && sub.begin == orig);

    sub = sno_view(&input[3], &input[1]);
    orig = sub.begin;
    assert(!sno_lit(&sub, sno_bind("X")) && sub.begin == orig);

    sub = sno_bind("");
    orig = sub.begin;
    assert(!sno_lit(&sub, sno_bind("X")) && sub.begin == orig);

    sub = sno_bind("Hello");
    orig = sub.begin;
    assert(!sno_lit(&sub, sno_bind("HELLO")) && sub.begin == orig);
}

void test_any(void) {
    sno_view_t sub;
    sno_cursor_t orig;

    char buf1[] = "XYZ";
    sub = sno_bind(buf1);
    orig = sub.begin;
    assert(sno_any(&sub, sno_bind("XYZ")) && sub.begin == orig + 1 && *sub.begin == 'Y');

    char buf2[] = "!5";
    sub = sno_bind(buf2);
    orig = sub.begin;
    assert(sno_any(&sub, sno_bind("!@#$%")) && sub.begin == orig + 1 && *sub.begin == '5');

    char buf3[] = "X";
    sub = sno_bind(buf3);
    orig = sub.begin;
    assert(!sno_any(&sub, sno_bind("ABC")) && sub.begin == orig);

    sub = sno_bind("");
    orig = sub.begin;
    assert(!sno_any(&sub, sno_bind("A")) && sub.begin == orig);

    char buf4[] = "A";
    sub = sno_bind(buf4);
    orig = sub.begin;
    assert(!sno_any(&sub, sno_bind("")) && sub.begin == orig);

    assert(!sno_any(NULL, sno_bind("A")));

    sub = sno_view(NULL, NULL);
    assert(!sno_any(&sub, sno_bind("A")) && !sub.begin && !sub.end);

    char buf5[] = "A";
    sub = sno_view(buf5, NULL);
    orig = sub.begin;
    assert(!sno_any(&sub, sno_bind("A")) && sub.begin == orig && sub.end == NULL);

    char buf6[] = "A";
    sub = sno_view(NULL, buf6);
    assert(!sno_any(&sub, sno_bind("A")) && !sub.begin && sub.end == buf6);

    char buf7[] = "a";
    sub = sno_bind(buf7);
    orig = sub.begin;
    assert(!sno_any(&sub, sno_bind("A")) && sub.begin == orig);

    char buf8[] = "AAA";
    sub = sno_bind(buf8);
    orig = sub.begin;
    assert(sno_any(&sub, sno_bind("A")) && sub.begin == orig + 1 && *sub.begin == 'A');

    char buf9[] = "123abc";
    sub = sno_bind(buf9);
    int count = 0;
    while (sno_any(&sub, sno_bind("0123456789"))) count++;
    assert(count == 3 && strncmp(sub.begin, "abc", 3) == 0);

    char buf10[] = "Z";
    sub = sno_bind(buf10);
    assert(sno_any(&sub, sno_bind("Z")) && sub.begin == sub.end);

    char buf11[] = "!!!END";
    sub = sno_bind(buf11);
    orig = sub.begin;
    assert(sno_any(&sub, sno_bind("!")) && sub.begin == orig + 1 && *sub.begin == '!');

    char buf12[] = "!!!END";
    sub = sno_view(&buf12[2], &buf12[0]);
    orig = sub.begin;
    assert(!sno_any(&sub, sno_bind("!")) && sub.begin == orig);
}

void test_notany(void) {
    sno_view_t sub;
    sno_cursor_t orig;

    char buf1[] = "X9Z";
    sub = sno_bind(buf1);
    orig = sub.begin;
    assert(sno_notany(&sub, sno_bind("012345678")) && sub.begin == orig + 1 && *sub.begin == '9');

    char buf2[] = "A";
    sub = sno_bind(buf2);
    orig = sub.begin;
    assert(sno_notany(&sub, sno_bind("")) && sub.begin == orig + 1 && sub.begin == sub.end);

    char buf3[] = "5";
    sub = sno_bind(buf3);
    orig = sub.begin;
    assert(!sno_notany(&sub, sno_bind("0123456789")) && sub.begin == orig);

    char buf4[] = "";
    sub = sno_bind(buf4);
    orig = sub.begin;
    assert(!sno_notany(&sub, sno_bind("A")) && sub.begin == orig);

    char buf5[] = "Z";
    sub = sno_bind(buf5);
    assert(sno_notany(&sub, sno_bind("ABC")) && sub.begin == sub.end);
    orig = sub.begin;
    assert(!sno_notany(&sub, sno_bind("A")) && sub.begin == orig);

    assert(!sno_notany(NULL, sno_bind("A")));

    sub = sno_view(NULL, NULL);
    assert(!sno_notany(&sub, sno_bind("A")) && !sub.begin && !sub.end);

    char buf6[] = "A";
    sub = sno_view(buf6, NULL);
    orig = sub.begin;
    assert(!sno_notany(&sub, sno_bind("A")) && sub.begin == orig && sub.end == NULL);

    char buf7[] = "A";
    sub = sno_view(NULL, buf7);
    assert(!sno_notany(&sub, sno_bind("A")) && !sub.begin && sub.end == buf7);

    char buf8[] = "a";
    sub = sno_bind(buf8);
    orig = sub.begin;
    assert(!sno_notany(&sub, sno_bind("a")) && sub.begin == orig);

    char buf9[] = "FIELD,REST";
    sub = sno_bind(buf9);
    while (sno_notany(&sub, sno_bind(","))) ;
    assert(*sub.begin == ',');

    char buf10[] = "FIELD,REST";
    sub = sno_view(&buf10[2], &buf10[0]);
    orig = sub.begin;
    assert(!sno_notany(&sub, sno_bind(",")) && sub.begin == orig);
}

void test_span(void) {
    sno_view_t sub;
    sno_cursor_t orig;

    char buf1[] = "A123";
    sub = sno_bind(buf1);
    orig = sub.begin;
    assert(sno_span(&sub, sno_bind("A")) && sub.begin == orig + 1 && *sub.begin == '1');

    char buf2[] = "12345abc";
    sub = sno_bind(buf2);
    orig = sub.begin;
    assert(sno_span(&sub, sno_bind("0123456789")) && sub.begin == orig + 5 && *sub.begin == 'a');

    char buf3[] = "999";
    sub = sno_bind(buf3);
    assert(sno_span(&sub, sno_bind("0123456789")) && sub.begin == sub.end);

    char buf4[] = "X123";
    sub = sno_bind(buf4);
    orig = sub.begin;
    assert(!sno_span(&sub, sno_bind("0123456789")) && sub.begin == orig);

    char buf5[] = "";
    sub = sno_bind(buf5);
    orig = sub.begin;
    assert(!sno_span(&sub, sno_bind("A")) && sub.begin == orig);

    char buf6[] = "A";
    sub = sno_bind(buf6);
    orig = sub.begin;
    assert(!sno_span(&sub, sno_bind("")) && sub.begin == orig);

    char buf7[] = "aaaabbb";
    sub = sno_bind(buf7);
    orig = sub.begin;
    assert(sno_span(&sub, sno_bind("a")) && sub.begin == orig + 4 && *sub.begin == 'b');

    char buf8[] = "321cba";
    sub = sno_bind(buf8);
    orig = sub.begin;
    assert(sno_span(&sub, sno_bind("abc123")) && sub.begin == orig + 6 && sub.begin == sub.end);

    char buf9[] = "aAaA";
    sub = sno_bind(buf9);
    orig = sub.begin;
    assert(sno_span(&sub, sno_bind("a")) && sub.begin == orig + 1 && *sub.begin == 'A');

    char buf10[] = "123,456,789";
    sub = sno_bind(buf10);
    assert(sno_span(&sub, sno_bind("0123456789")) && sno_lit(&sub, sno_bind(",")) && sno_span(&sub, sno_bind("0123456789")) && *sub.begin == ',');

    assert(!sno_span(NULL, sno_bind("A")));

    sub = sno_view(NULL, NULL);
    assert(!sno_span(&sub, sno_bind("A")) && !sub.begin && !sub.end);

    char buf11[] = "A";
    sub = sno_view(buf11, NULL);
    orig = sub.begin;
    assert(!sno_span(&sub, sno_bind("A")) && sub.begin == orig && sub.end == NULL);

    char buf12[] = "A";
    sub = sno_view(NULL, buf12);
    assert(!sno_span(&sub, sno_bind("A")) && !sub.begin && sub.end == buf12);

    char buf13[] = "Z";
    sub = sno_bind(buf13);
    assert(sno_span(&sub, sno_bind("Z")) && sub.begin == sub.end);

    char buf14[] = "AAA";
    sub = sno_bind(buf14);
    orig = sub.begin;
    assert(sno_span(&sub, sno_bind("A")) && sub.begin == orig + 3);
    sub = sno_bind(buf14);
    orig = sub.begin;
    assert(sno_span(&sub, sno_bind("XYZA")) && sub.begin == orig + 3);

    char buf15[] = "AAA";
    sub = sno_view(&buf15[2], &buf15[0]);
    orig = sub.begin;
    assert(!sno_span(&sub, sno_bind("A")) && sub.begin == orig);
}

void test_break(void) {
    sno_view_t sub;
    sno_cursor_t orig;

    char buf1[] = "FIELD,REST";
    sub = sno_bind(buf1);
    orig = sub.begin;
    assert(sno_break(&sub, sno_bind(",")) && sub.begin == orig + 5 && *sub.begin == ',');

    char buf2[] = ",REST";
    sub = sno_bind(buf2);
    orig = sub.begin;
    assert(sno_break(&sub, sno_bind(",")) && sub.begin == orig && *sub.begin == ',');

    char buf3[] = "FIELD";
    sub = sno_bind(buf3);
    assert(sno_break(&sub, sno_bind(",")) && sub.begin == sub.end);

    char buf4[] = "";
    sub = sno_bind(buf4);
    assert(sno_break(&sub, sno_bind(",")) && sub.begin == sub.end);

    char buf5[] = "TEST";
    sub = sno_bind(buf5);
    assert(sno_break(&sub, sno_bind("")) && sub.begin == sub.end);

    char buf6[] = "aaaabbb";
    sub = sno_bind(buf6);
    orig = sub.begin;
    assert(sno_break(&sub, sno_bind("b")) && sub.begin == orig + 4 && *sub.begin == 'b');

    char buf7[] = "aAaA";
    sub = sno_bind(buf7);
    orig = sub.begin;
    assert(sno_break(&sub, sno_bind("A")) && sub.begin == orig + 1 && *sub.begin == 'A');

    char buf8[] = "123,456,789";
    sub = sno_bind(buf8);
    assert(sno_break(&sub, sno_bind(",")) && sno_any(&sub, sno_bind(",")) && sno_break(&sub, sno_bind(",")) && sno_any(&sub, sno_bind(",")) && sno_break(&sub, sno_bind(",")) && sub.begin == sub.end);

    assert(!sno_break(NULL, sno_bind(",")));

    sub = sno_view(NULL, NULL);
    assert(!sno_break(&sub, sno_bind(",")) && !sub.begin && !sub.end);

    char buf9[] = "A";
    sub = sno_view(buf9, NULL);
    orig = sub.begin;
    assert(!sno_break(&sub, sno_bind(",")) && sub.begin == orig && sub.end == NULL);

    char buf10[] = "A";
    sub = sno_view(NULL, buf10);
    assert(!sno_break(&sub, sno_bind(",")) && !sub.begin && sub.end == buf10);

    char buf11[] = "Z";
    sub = sno_bind(buf11);
    assert(sno_break(&sub, sno_bind("A")) && sub.begin == sub.end);

    char buf12[] = "ABcDE";
    sub = sno_bind(buf12);
    orig = sub.begin;
    assert(sno_break(&sub, sno_bind("c")) && sub.begin == orig + 2 && *sub.begin == 'c');
}

void test_skip(void) {
    sno_view_t sub;
    sno_cursor_t orig;

    char buf1[] = "   TEXT";
    sub = sno_bind(buf1);
    orig = sub.begin;
    assert(sno_skip(&sub, sno_bind(" \t")) && sub.begin == orig + 3 && *sub.begin == 'T');

    char buf2[] = "TEXT";
    sub = sno_bind(buf2);
    orig = sub.begin;
    assert(sno_skip(&sub, sno_bind(" \t")) && sub.begin == orig && *sub.begin == 'T');

    char buf3[] = "";
    sub = sno_bind(buf3);
    orig = sub.begin;
    assert(sno_skip(&sub, sno_bind(" ")) && sub.begin == orig);

    char buf4[] = "TEXT";
    sub = sno_bind(buf4);
    orig = sub.begin;
    assert(sno_skip(&sub, sno_bind("")) && sub.begin == orig);

    char buf5[] = "   TEXT";
    sub = sno_bind(buf5);
    assert(sno_skip(&sub, sno_bind(" ")));
    orig = sub.begin;
    assert(sno_skip(&sub, sno_bind(" ")) && sub.begin == orig);

    char buf6[] = "15L";
    sub = sno_bind(buf6);
    sno_skip(&sub, sno_bind(" \t"));
    assert(sno_span(&sub, sno_bind("0123456789")) && sno_lit(&sub, sno_bind("L")));

    char buf7[] = "  15L";
    sub = sno_bind(buf7);
    sno_skip(&sub, sno_bind(" \t"));
    assert(sno_span(&sub, sno_bind("0123456789")) && sno_lit(&sub, sno_bind("L")));

    char buf8[] = "aaaabbb";
    sub = sno_bind(buf8);
    orig = sub.begin;
    assert(sno_skip(&sub, sno_bind("a")) && sub.begin == orig + 4 && *sub.begin == 'b');

    assert(!sno_skip(NULL, sno_bind(" ")));

    sub = sno_view(NULL, NULL);
    assert(!sno_skip(&sub, sno_bind(" ")) && !sub.begin && !sub.end);

    char buf9[] = "A";
    sub = sno_view(buf9, NULL);
    orig = sub.begin;
    assert(!sno_skip(&sub, sno_bind(" ")) && sub.begin == orig && sub.end == NULL);

    char buf10[] = "A";
    sub = sno_view(NULL, buf10);
    assert(!sno_skip(&sub, sno_bind(" ")) && !sub.begin && sub.end == buf10);

    char buf11[] = "   ";
    sub = sno_bind(buf11);
    assert(sno_skip(&sub, sno_bind(" ")) && sub.begin == sub.end);

    char buf12[] = "\t  \tTEXT";
    sub = sno_bind(buf12);
    orig = sub.begin;
    assert(sno_skip(&sub, sno_bind(" \t")) && sub.begin == orig + 4 && *sub.begin == 'T');
}

void test_var(void) {
    char buf[20];
    sno_view_t sub;

    sub = sno_bind("TEST");
    assert(sno_var(&sub, buf, sizeof(buf)) && strcmp(buf, "TEST") == 0 && sub.begin == sub.end);

    sub = sno_bind("");
    assert(sno_var(&sub, buf, sizeof(buf)) && buf[0] == '\0' && sub.begin == sub.end);

    sub = sno_bind("12345");
    sno_cursor_t orig = sub.begin;
    assert(!sno_var(&sub, buf, 5) && sub.begin == orig);

    sub = sno_bind("ABC");
    orig = sub.begin;
    assert(!sno_var(&sub, buf, 3) && sub.begin == orig);

    sub = sno_bind("X");
    orig = sub.begin;
    assert(!sno_var(&sub, buf, 0) && sub.begin == orig);

    sub = sno_bind("SAFE");
    orig = sub.begin;
    assert(!sno_var(&sub, NULL, sizeof(buf)) && sub.begin == orig);

    assert(!sno_var(NULL, buf, sizeof(buf)));

    sub = sno_view(NULL, NULL);
    assert(!sno_var(&sub, buf, sizeof(buf)) && !sub.begin && !sub.end);

    char composite[] = "PREFIXHELLOSUFFIX";
    sub = sno_view(&composite[6], &composite[11]);
    assert(sno_var(&sub, buf, sizeof(buf)) && strcmp(buf, "HELLO") == 0 && sub.begin == sub.end);

    char cmd[] = "15L";
    sub = sno_bind(cmd);
    sno_span(&sub, sno_bind("0123456789"));
    sno_view_t num_view = sno_view(cmd, sub.begin);
    assert(sno_var(&num_view, buf, sizeof(buf)) && strcmp(buf, "15") == 0);
    assert(sno_lit(&sub, sno_bind("L")));
}

void test_int(void) {
    sno_view_t sub;
    int n;

    // Valid positives (within edlin's actual usage range)
    sub = sno_bind("123");
    assert(sno_int(&sub, &n) && n == 123 && sub.begin == sub.end);

    sub = sno_bind("+456");
    assert(sno_int(&sub, &n) && n == 456 && sub.begin == sub.end);

    sub = sno_bind("0");
    assert(sno_int(&sub, &n) && n == 0 && sub.begin == sub.end);

    // Portable max/min guaranteed by C standard (all vintage systems support these)
    sub = sno_bind("32767");   // Max portable positive (C standard min range)
    assert(sno_int(&sub, &n) && n == 32767 && sub.begin == sub.end);

    sub = sno_bind("-32767");  // Min portable negative (C standard min range)
    assert(sno_int(&sub, &n) && n == -32767 && sub.begin == sub.end);

    // Realistic edlin line numbers (safe for 16-bit and 32-bit)
    sub = sno_bind("9999");
    assert(sno_int(&sub, &n) && n == 9999 && sub.begin == sub.end);

    // Stop at NOTANY boundary
    sub = sno_bind("123x");
    assert(sno_int(&sub, &n) && n == 123 && *sub.begin == 'x');

    sub = sno_bind("-456y");
    assert(sno_int(&sub, &n) && n == -456 && *sub.begin == 'y');

    // Fail on invalid first char
    sub = sno_bind("x123");
    assert(!sno_int(&sub, &n));

    sub = sno_bind(" 123");
    assert(!sno_int(&sub, &n));

    sub = sno_bind("");
    assert(!sno_int(&sub, &n));

    // Reject malformed signs
    sub = sno_bind("+");
    assert(!sno_int(&sub, &n));

    sub = sno_bind("-");
    assert(!sno_int(&sub, &n));

    sub = sno_bind("++1");
    assert(!sno_int(&sub, &n));

    sub = sno_bind("--1");
    assert(!sno_int(&sub, &n));

    sub = sno_bind("+-1");
    assert(!sno_int(&sub, &n));

    sub = sno_bind("-+1");
    assert(!sno_int(&sub, &n));

    sub = sno_bind("-x");
    assert(!sno_int(&sub, &n));

    sub = sno_bind("+y");
    assert(!sno_int(&sub, &n));

    sub = sno_bind("-0");
    assert(sno_int(&sub, &n) && n == 0 && sub.begin == sub.end);

    // Atomic rollback on failure
    char buf[] = "x123";
    sub = sno_bind(buf);
    sno_cursor_t orig = sub.begin;
    assert(!sno_int(&sub, &n) && sub.begin == orig);

    // NULL safety
    assert(!sno_int(NULL, &n));

    sub = sno_view(NULL, NULL);
    assert(!sno_int(&sub, &n));

    char buf1[] = "123";
    sub = sno_view(buf1, NULL);
    orig = sub.begin;
    assert(!sno_int(&sub, &n) && sub.begin == orig && sub.end == NULL);

    char buf2[] = "123";
    sub = sno_view(NULL, buf2);
    assert(!sno_int(&sub, &n) && !sub.begin && sub.end == buf2);
}

void sno_test(void) {
    test_bind();
    test_view();
    test_size();
    test_lit();
    test_any();
    test_notany();
    test_span();
    test_break();
    test_skip();
    test_var();
    test_int();
    printf("All SNOBOL-C primitive tests pass!\n");
}

#endif
