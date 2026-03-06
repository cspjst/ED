#ifdef POLICY_USE_DOS_STD
    #include "STD/dos_stdio.h"
    #include "STD/dos_assert.h"
#else
    #include <xstdio.h>
    #include <assert.h>
#endif

#include "BIOS/test_bios.h"
#include "DOS/test_dos_memory.h"
#include "DOS/test_dos_services.h"
#include "DOS/test_dos_files.h"
//#include "STD/test_stdio.h"
//#include "STD/test_string.h"
//#include "STD/test_files.h"
//#include "STD/test_stdlib.h"
//#include "STD/dos_stdio.h"

int main() {

    printf("Test Harness\n");

    // BIOS
    //test_bios_memory();
    //test_bios_keys();

    // DOS
    //test_dos_memory();
    //test_dos_services();
    test_dos_files();

    // STD
    //test_stdio();
    //test_string();
    //test_files();
    //test_stdlib();
}
