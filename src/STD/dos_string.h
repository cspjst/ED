#ifndef STRING_H
#define STRING_H

#include "dos_stddef.h"

// String manipulation
// strcpy // copies one string to another
// strcat // concatenates two strings
// strdup // allocates a copy of a string - must free

// String examination
size_t strlen(const char* str);                 // returns the length of a given string
int strcmp(const char* s1, const char* s2);     // compares two strings
// strncmp // compares a certain amount of characters of two strings
char* strchr(const char* str, int ch);          // finds the first occurrence of a character
char* strrchr(const char* str, int ch);         // finds the last occurrence of a character
// strspn // returns the length of the maximum initial segment that consists of only the characters found in another byte string
// strcspn returns the length of the maximum initial segment that consists of only the characters not found in another byte string
// strpbrk // finds the first location of any character in one string, in another string
// strstr //finds the first occurrence of a substring of characters
// strtok // finds the next token in a byte string

// Character array manipulation
// memchr   // searches an array for the first occurrence of a character
// memcmp   // compares two buffers
// memset   // fills a buffer with a character
// memcpy   // copies one buffer to another
// memmove  // moves one buffer to another
// memccpy  // copies one buffer to another, stopping after the specified delimiter

// Miscellaneous
const char* strerror(int errnum);

#endif
