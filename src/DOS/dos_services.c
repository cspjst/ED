#include "dos_services.h"
#include "dos_services_constants.h"
#include "dos_error_types.h"

/**
* @brief Provides a safe method for changing interrupt vectors
* @note returns nothing
* INT 21,25 - Set Interrupt Vector
* AH = 25h
* AL = interrupt number
* DS:DX = pointer to interrupt handler
*/
void dos_set_interrupt_vector(uint8_t vec_num, void* phandler) {
    __asm {
        .8086
        pushf                                ; preserve what int 21h may not
        push    ds                           ; due to unreliable behaviour

        lds     dx, phandler                ; copy pointer to handler into DS:DX
        mov     al, vec_num                 ; interrupt vector number
        mov     ah, DOS_SET_INTERRUPT_VECTOR    ; 25h service
        int     DOS_SERVICE

        pop     ds
        popf
    }
}

/**
* @brief Standard method for retrieving interrupt vectors
* INT 21,35 - Get Interrupt Vector
* AH = 35h
* AL = interrupt vector number
* on return:
* ES:BX = pointer to interrupt handler.
*
* @param vec_num
* @return void* segment:offset pointer to interrupt handler
*/
void* dos_get_interrupt_vector(uint8_t vec_num) {
    void* phandler = 0;
    __asm {
        .8086
        pushf
        push    ds

        mov     al, vec_num                 ; interrupt vector number
        mov     ah, DOS_GET_INTERRUPT_VECTOR    ; 35h service
        int     DOS_SERVICE
        lea     di, phandler
        mov     [di], bx                    ; copy segment into address_t (little endian)
        mov     [di + 2] , es               ; copy offset

        pop     ds
        popf
    }
    return phandler;
}

/**
 * @brief INT 21,4C - Terminate Process With Return Code
 *
 * AH = 4C
 * AL = return code (for batch files)
 * returns nothing
 * - approved method of program termination
 * - restores the terminate, Ctrl-Break, and critical error exit
 * 	 addresses, flushes all buffers, frees memory and returns to
 * 	 DOS via the termination handler address
 * - does not close FCBs
 * - this function is not supported in versions of DOS before 2.x
 */
void dos_terminate_process_with_return_code(uint8_t return_code) {
    __asm {
        .8086
        mov     al, return_code
        mov     ah, DOS_TERMINATE_PROCESS_WITH_RETURN_CODE
        int     DOS_SERVICE
        // program ends here - no return
    }
}
