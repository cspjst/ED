#ifdef USE_DOSLIBC
    #include "STD/dos_stdio.h"
#else
    #include <stdio.h>
#endif

//#include "TEST/test_bios.h"
//#include "TEST/test_dos_memory.h"
//#include "TEST/test_dos_services.h"
//#include "TEST/test_dos_files.h"
#include "TEST/test_stdio.h"
#include "TEST/test_string.h"
#include "TEST/test_files.h"
#include "TEST/test_stdlib.h"
//#include "TEST/test_sno_core.h"
//#include "TEST/test_sno_extra.h"

int main() {

#ifdef USE_DOSLIBC
    printf("Using DOSLIBC Test Harness\n");
#else
    printf("Native LIBC Test Harness\n");
#endif

    //SNO
    //test_sno_core();
    //test_sno_extra();

    // BIOS
    //test_bios_memory();
    //test_bios_keys();

    // DOS
    //test_dos_memory();
    //test_dos_services();
    //test_dos_files();

    // STD
    test_stdio();
    test_string();
    test_stdlib();
    #ifdef USE_DOSLIBC_FILE_IO
        test_files();
    #endif

}
