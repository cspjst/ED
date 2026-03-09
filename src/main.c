#ifdef POLICY_USE_DOS_STD
    #include "STD/dos_stdio.h"
    #include "STD/dos_assert.h"
#else
    #include <stdio.h>
    #include <assert.h>
#endif

#include "TEST/test_bios.h"
//#include "TEST/test_dos_memory.h"
//#include "TEST/test_dos_services.h"
//#include "TEST/test_dos_files.h"
//#include "TEST/test_stdio.h"
//#include "TEST/test_string.h"
//#include "TEST/test_files.h"
//#include "TEST/test_stdlib.h"
//#include "TEST/dos_stdio.h"

int main() {

#ifdef POLICY_USE_DOS_STD
    printf("POLICY_USE_DOS_STD Test Harness\n");
#else
    printf("Test Harness\n");
#endif
    // BIOS
    test_bios_memory();
    test_bios_keys();

    // DOS
    //test_dos_memory();
    //test_dos_services();
    //test_dos_files();

    // STD
    //test_stdio();
    //test_string();
    //test_files();
    //test_stdlib();
}
