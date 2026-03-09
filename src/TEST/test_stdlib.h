#ifndef TEST_STDLIB_H
#define TEST_STDLIB_H

#include "dos_stdlib.h"
#include "dos_stdio.h"
#include "dos_assert.h"
#include "dos_stddef.h" // For size_t
/*
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stddef.h>
*/

// Helper function to check if a block of memory is zeroed
int is_zeroed(const void* ptr, size_t size) {
    const unsigned char* p = (const unsigned char*)ptr;
    for (size_t i = 0; i < size; ++i) {
        if (p[i] != 0) {
            return 0; // Found a non-zero byte
        }
    }
    return 1; // All bytes were zero
}

void test_malloc_basic() {
    printf("Running test_malloc_basic...\n");

    // Test 1: Allocate a small block
    size_t size1 = 10;
    void* ptr1 = malloc(size1);
    assert(ptr1 != NULL); // Should succeed
    printf("  Allocated %zu bytes at %p\n", size1, ptr1);

    // Write some data to the block
    unsigned char* byte_ptr1 = (unsigned char*)ptr1;
    for (size_t i = 0; i < size1; ++i) {
        byte_ptr1[i] = (unsigned char)(i % 256);
    }

    // Verify the data
    for (size_t i = 0; i < size1; ++i) {
        assert(byte_ptr1[i] == (i % 256));
    }
    printf("  Data write/read verified for %zu bytes.\n", size1);

    // Test 2: Allocate a larger block
    size_t size2 = 1000;
    void* ptr2 = malloc(size2);
    assert(ptr2 != NULL); // Should succeed
    printf("  Allocated %zu bytes at %p\n", size2, ptr2);

    // Write and verify data in the second block
    unsigned char* byte_ptr2 = (unsigned char*)ptr2;
    for (size_t i = 0; i < size2; ++i) {
        byte_ptr2[i] = (unsigned char)(~i % 256); // Different pattern
    }
    for (size_t i = 0; i < size2; ++i) {
        assert(byte_ptr2[i] == ((~i) % 256));
    }
    printf("  Data write/read verified for %zu bytes.\n", size2);

    // Free the blocks
    free(ptr1);
    printf("  Freed ptr1 (%p)\n", ptr1);
    free(ptr2);
    printf("  Freed ptr2 (%p)\n", ptr2);

    printf("test_malloc_basic passed.\n\n");
}

void test_malloc_zero() {
    printf("Running test_malloc_zero...\n");

    // Test allocation of 0 bytes
    void* ptr = malloc(0);
    // Standard behavior is implementation-defined. DOS alloc likely returns NULL.
    // Our code will request 0 paragraphs, which DOS alloc probably fails, returning NULL.
    // This is acceptable and consistent.
    // assert(ptr == NULL); // This might be true depending on DOS alloc behavior for 0 paras.
    // A more robust malloc might return a unique pointer or NULL for malloc(0).
    // For now, just test that it doesn't crash.
    if (ptr != NULL) {
        printf("  malloc(0) returned %p (not NULL)\n", ptr);
        free(ptr); // Ensure free can handle the pointer returned by malloc(0)
        printf("  Freed malloc(0) result.\n");
    } else {
        printf("  malloc(0) returned NULL (as expected if DOS alloc fails for 0 paras)\n");
    }

    printf("test_malloc_zero passed (no crash).\n\n");
}

void test_free_null() {
    printf("Running test_free_null...\n");

    // Test freeing a NULL pointer
    free(NULL);
    printf("  free(NULL) called successfully (no crash).\n");

    printf("test_free_null passed.\n\n");
}

void test_calloc_basic() {
    printf("Running test_calloc_basic...\n");

    // Test 1: Allocate memory for 5 integers (assuming int is 2 bytes in DOS)
    size_t num_ints = 5;
    size_t int_size = sizeof(int);
    int* int_array = (int*)calloc(num_ints, int_size);
    assert(int_array != NULL); // Should succeed
    printf("  Allocated memory for %zu ints (total %zu bytes) at %p\n", num_ints, num_ints * int_size, int_array);

    // Verify the memory is zeroed
    for (size_t i = 0; i < num_ints; ++i) {
        assert(int_array[i] == 0);
    }
    printf("  calloc result is zeroed for %zu ints.\n", num_ints);

    // Test 2: Allocate memory for a structure
    typedef struct { char a; int b; } test_struct_t;
    size_t num_structs = 2;
    test_struct_t* struct_array = (test_struct_t*)calloc(num_structs, sizeof(test_struct_t));
    assert(struct_array != NULL); // Should succeed
    printf("  Allocated memory for %zu structs (total %zu bytes) at %p\n", num_structs, num_structs * sizeof(test_struct_t), struct_array);

    // Verify the structure memory is zeroed
    for (size_t i = 0; i < num_structs; ++i) {
        assert(struct_array[i].a == 0);
        assert(struct_array[i].b == 0);
    }
    printf("  calloc result is zeroed for %zu structs.\n", num_structs);

    // Test 3: Allocate a single byte (odd size to test word calculation edge case)
    unsigned char* single_byte = (unsigned char*)calloc(1, 1);
    assert(single_byte != NULL); // Should succeed
    assert(*single_byte == 0); // Should be zero
    printf("  Allocated and verified single byte calloc at %p (value: %d)\n", single_byte, *single_byte);

    // Free the allocated memory
    free(int_array);
    printf("  Freed int array (%p)\n", int_array);
    free(struct_array);
    printf("  Freed struct array (%p)\n", struct_array);
    free(single_byte);
    printf("  Freed single byte (%p)\n", single_byte);

    printf("test_calloc_basic passed.\n\n");
}

void test_calloc_zero() {
    printf("Running test_calloc_zero...\n");

    // Test calloc with 0 items
    int* ptr1 = (int*)calloc(0, sizeof(int));
    // Standard behavior is implementation-defined for n=0 or size=0.
    // Our code calculates total_size=0, paragraphs=0, DOS alloc likely returns NULL.
    // if (ptr1 != NULL) free(ptr1); // Free if allocated.
    printf("  calloc(0, sizeof(int)) returned %p\n", ptr1);
    if (ptr1 != NULL) {
        free(ptr1);
        printf("  Freed calloc(0, sizeof(int)) result.\n");
    }

    // Test calloc with 0 size
    int* ptr2 = (int*)calloc(10, 0);
    // Standard behavior is implementation-defined for n=0 or size=0.
    // Our code calculates total_size=0, paragraphs=0, DOS alloc likely returns NULL.
    // if (ptr2 != NULL) free(ptr2); // Free if allocated.
    printf("  calloc(10, 0) returned %p\n", ptr2);
    if (ptr2 != NULL) {
        free(ptr2);
        printf("  Freed calloc(10, 0) result.\n");
    }

    printf("test_calloc_zero passed (no crash, results implementation-defined).\n\n");
}

void test_mixed_allocations() {
    printf("Running test_mixed_allocations...\n");

    void* ptrs[5] = {NULL}; // Initialize array of pointers

    // Allocate a mix of sizes
    ptrs[0] = malloc(100);
    ptrs[1] = calloc(10, sizeof(double)); // Assuming double is supported or use int/float
    ptrs[2] = malloc(50);
    ptrs[3] = calloc(1, 1024);
    ptrs[4] = malloc(1); // Smallest possible

    // Check allocations succeeded
    for (int i = 0; i < 5; ++i) {
        assert(ptrs[i] != NULL);
        printf("  Allocated block %d at %p\n", i, ptrs[i]);
    }

    // Verify calloc'd blocks are zeroed
    assert(is_zeroed(ptrs[1], 10 * sizeof(double)));
    assert(is_zeroed(ptrs[3], 1024));
    printf("  calloc'd blocks verified as zeroed.\n");

    // Free them in a different order
    free(ptrs[2]); // malloc(50)
    printf("  Freed block 2 (%p)\n", ptrs[2]);
    free(ptrs[4]); // malloc(1)
    printf("  Freed block 4 (%p)\n", ptrs[4]);
    free(ptrs[1]); // calloc(10, sizeof(double))
    printf("  Freed block 1 (%p)\n", ptrs[1]);
    free(ptrs[3]); // calloc(1, 1024)
    printf("  Freed block 3 (%p)\n", ptrs[3]);
    free(ptrs[0]); // malloc(100)
    printf("  Freed block 0 (%p)\n", ptrs[0]);

    printf("test_mixed_allocations passed.\n\n");
}

void test_stdlib() {

    test_malloc_basic();
    test_malloc_zero();
    test_free_null();
    test_calloc_basic();
    test_calloc_zero();
    test_mixed_allocations();

}

#endif
