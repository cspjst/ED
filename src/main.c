#ifdef USE_STDDOS

#include "../STD/dos_stdio.h"
#include "../STD/dos_assert.h"

#else

#include <stdio.h>
#include <assert.h>

#endif

#include "BIOS/test_bios.h"
//#include "STD/test_stdio.h"
//#include "DOS/test_dos_memory.h"
//#include "STD/test_string.h"
//#include "STD/test_files.h"
//#include "STD/test_stdlib.h"
//#include "STD/dos_stdio.h"

int main() {

    printf("Test Harness\n");

    // BIOS
    test_bios_memory();
    //test_bios_keys();

    // DOS
    //test_dos_memory();

    // STD
    //test_stdio();
    //test_string();
    //test_files();
    //test_stdlib();
}
