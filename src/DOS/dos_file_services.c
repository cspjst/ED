#include "dos_file_services.h"
#include "dos_error_types.h"
#include "dos_services_constants.h"
#include "dos_file_types.h"

/**
* INT 21,36 - Get Disk Free Space
* AH = 36h
* DL = drive number (0=default, 1=A:)
*
* on return:
* AX = sectors per cluster
*    = FFFF if drive is invalid
* BX = number of available clusters
* CX = number of bytes per sector
* DX = number of clusters per drive
*
* - used to determine available space on specified disk
* - see	INT 21,1B   INT 21,1C
*/
dos_error_code_t dos_get_disk_free_space(uint8_t drive_number, dos_file_disk_space_info_t* info) {
    dos_error_code_t errno = 0;
    __asm {
        .8086
        pushf
        push    ds

        mov     dl, drive_number
        mov     ah, DOS_GET_DISK_FREE_SPACE
        int     DOS_SERVICE
        cmp     ax, DOS_ERROR               ; FFFFh = invalid drive
        jne     OK
        mov     errno, 15                   ; DOS_INVALID_DRIVE_SPECIFIED
        les     di, info
        cld
        stosw                               ; AX already = DOS_ERROR
        xor     ax, ax                      ; zero out the rest of the struct
        stosw
        stosw
        stosw
        jmp     END
OK:     lds     di, info                    ; fill out the struct
        mov     [di], ax                    ; sectors per cluster
        mov     [di + 2], bx                ; available clusters
        mov     [di + 4], cx                ; bytes per sector
        mov     [di + 6], dx                ; clusters per drive

END:    pop     ds
        popf
    }
    return errno;
}

/**
* INT 21, 3C - Create File Using Handle
* AH = 3C
* CX = file attribute(see FILE ATTRIBUTES)
* DS:DX = pointer to ASCIIZ path name
*
* on return :
* CF = 0 if successful
*    = 1 if error
* AX = files handle if successful
*    = error code if failure(see DOS ERROR CODES)
*
* @note - if file already exists, it is truncated to zero bytes on opening
*/
dos_error_code_t dos_create_file(const char* path_name, dos_file_attributes_t create_attributes, dos_file_handle_t* fhandle) {
    dos_error_code_t errno = 0;
	__asm {
		.8086
		pushf
        push    ds

		lds		dx, path_name
		mov		cx, create_attributes
		mov		ah, DOS_CREATE_FILE_USING_HANDLE
		int		DOS_SERVICE
		jnc		OK
		mov		errno, ax
		mov     ax, 0FFFFh
OK:		lds     di, fhandle
        mov     [di], ax

END:    pop     ds
        popf
	}
	return errno;
}

/**
* INT 21,3D Open File Using Handle
* AH = 3D
* AL = open access mode
*      00  read only
*      01  write only
*      02  read/write
* DS:DX = pointer to an ASCIIZ file name
*
* on return:
* AX = file handle if CF not set
*    = error code if CF set  (see DOS ERROR CODES)
*/
dos_error_code_t dos_open_file(const char* path_name, uint8_t access_attributes, dos_file_handle_t* fhandle) {
    dos_error_code_t errno = 0;
	__asm {
		.8086
		pushf
        push    ds

		lds		dx, path_name
		mov		al, access_attributes
		mov		ah, DOS_OPEN_FILE_USING_HANDLE
		int		DOS_SERVICE
		jnc		OK
		mov		errno, ax
		mov     ax, 0FFFFh
OK:		lds     di, fhandle
        mov     [di], ax

END:    pop     ds
        popf
	}
	return errno;
}

/**
* INT 21,3E - Close File Using Handle
* AH = 3E
* BX = file handle to close
*
* on return:
* AX = error code if CF set  (see DOS ERROR CODES)
*
* - if file is opened for update, file time and date stamp as well as file size are updated in the directory
* - handle is freed
*/
dos_error_code_t dos_close_file(dos_file_handle_t fhandle) {
    dos_error_code_t errno = 0;
	__asm {
		.8086
		pushf
        push    ds

		mov		bx, fhandle
		mov		ah, DOS_CLOSE_FILE_USING_HANDLE
		int		DOS_SERVICE
		jnc		END
		mov		errno, ax

END:    pop     ds
        popf
	}
	return errno;
}

/**
* INT 21,3F - Read From File or Device Using Handle
* AH = 3F
* BX = file handle
* CX = number of bytes to read
* DS:DX = pointer to read buffer
*
* on return:
* AX = number of bytes read is CF not set
*    = error code if CF set  (see DOS ERROR CODES)
*
* - read specified number of bytes from file into buffer DS:DX
* - when AX is not equal to CX then a partial read occurred due to end of file
* - if AX is zero, no data was read, and EOF occurred before read
*/
dos_error_code_t dos_read_file(dos_file_handle_t fhandle, uint16_t do_bytes, char* buffer, uint16_t* done_bytes) {
    dos_error_code_t errno = 0;
    __asm {
        .8086
        pushf
        push    ds

        lds     dx, buffer
        mov     cx, do_bytes
        mov     bx, fhandle
        mov     ah, DOS_READ_FILE_OR_DEVICE_USING_HANDLE
        int     DOS_SERVICE
        jnc     OK
        mov     errno, ax
        mov     ax, 0xFFFF                          ; EOF as uint16_t (-1)

OK:     lds     di, done_bytes
        mov     cx, ds
        add     cx, si
        jcxz    END                                 ; skip NULL return address
        mov     [di], ax

END:    pop     ds
        popf
    }
    return errno;
}

/**
* INT 21,40 - Write To File or Device Using Handle
* AH = 40h
* BX = file handle
* CX = number of bytes to write, a zero value truncates/extends the file to the current file position
* DS:DX = pointer to write buffer
*
* on return:
* AX = number of bytes written if CF not set
*    = error code if CF set  (see DOS ERROR CODES)
*
* - if AX is not equal to CX on return, a partial write occurred
* - this function can be used to truncate a file to the current file position by writing zero bytes
*/
dos_error_code_t  dos_write_file(dos_file_handle_t fhandle, uint16_t do_bytes, const char* buffer, uint16_t* done_bytes) {
    dos_error_code_t errno = 0;
    __asm {
        .8086
        pushf
        push    ds

        lds     dx, buffer
        mov     cx, do_bytes
        mov     bx, fhandle
        mov     ah, DOS_WRITE_FILE_OR_DEVICE_USING_HANDLE
        int     DOS_SERVICE
        jnc     OK
        mov     errno, ax
        mov     ax, 0xFFFF                          ; EOF as uint16_t (-1)

OK:     lds     di, done_bytes
        mov     cx, ds
        add     cx, si
        jcxz    END                                 ; skip NULL return address
        mov     [di], ax

END:    pop     ds
        popf
    }
    return errno;
}

/**
* INT 21,41 - Delete File
* AH = 41h
* DS:DX = pointer to an ASCIIZ filename
*
* on return:
* AX = error code if CF set  (see DOS ERROR CODES)
*
* - marks first byte of file directory entry with E5 to indicate the file has been deleted.
* - The rest of the directory entry stays intact until reused.
* - FAT pointers are returned to DOS
* @note - documented as not accepting wildcards in filename but actually does in several DOS versions
*/
dos_error_code_t dos_delete_file(const char* path_name) {
    dos_error_code_t errno = 0;
	__asm {
		.8086
		pushf
        push    ds

		lds		dx, path_name
		mov		ah, DOS_DELETE_FILE
		int		DOS_SERVICE
		jnc		END
		mov		errno, ax

END:    pop     ds
        popf
	}
	return errno;
}

/**
* INT 21,42 - Move File Pointer Using Handle
* AH = 42h
* AL = origin of move:
*      00 = beginning of file plus offset  (SEEK_SET)
*      01 = current location plus offset	(SEEK_CUR)
*      02 = end of file plus offset  (SEEK_END)
* BX = file handle
* CX:DX = (signed) offset from origin of new file position
*
* on return:
* CF clear if successful
*     DX:AX = new file position in bytes from start of file
*     DX = high order word of number of bytes to move
*	  AX = low order word of number of bytes to move
* CF set on error
*    AX = error code
*
* @note WARNING: for origins 01h and 02h, the pointer may be positioned before the
* start of the file; no error is returned in that case (except under Windows NT),
* but subsequent attempts at I/O will produce errors.
* If the new position is beyond the current end of file, the file will be extended by the next write!
* For FAT32 drives, the file must have been opened with AX=6C00h with the "extended size"
* flag in order to expand the file beyond 2GB
*
* @note BUG: using this method to grow a file from zero bytes to a very large size
* can corrupt the FAT in some versions of DOS; the file should first
* be grown from zero to one byte and then to the desired large size
*/
dos_error_code_t dos_move_file_pointer(dos_file_handle_t fhandle, dos_file_position_t foffset, uint8_t forigin, dos_file_position_t* new_pos) {
    dos_error_code_t errno = 0;
	__asm {
		.8086
		pushf
        push    ds

        mov     dx, WORD PTR [foffset]              ; DX low order word of fposition
        mov     cx, WORD PTR [foffset + 2]          ; CX hi order word of fposition
		mov		bx, fhandle
		mov		al, forigin							; SEEK_SET, SEEK_CUR, SEEK_END
		mov		ah, DOS_MOVE_FILE_POINTER_USING_HANDLE
		int		DOS_SERVICE
		jnc		OK
		mov		errno, ax
		jmp		END

OK:		lds     si, new_pos
        mov     cx, ds
        add     cx, si
        jcxz    END                                 ; skip NULL return address
        mov     [si], ax                            ; DX:AX = new file position (32-bit)
        mov     [si + 2], dx

END:    pop     ds
        popf
	}
    return errno;
}

/**
* INT 21,43 - Get/Set File Attributes
* AH = 43h
* AL = 00 to get attribute
*    = 01 to set attribute
* DS:DX = pointer to an ASCIIZ path name
* CX = attribute to set
*
* |5|4|3|2|1|0|  CX  valid file attributes
*  | | | | | `---- 1 = read only
*  | | | | `----- 1 = hidden
*  | | | `------ 1 = system
*  | `--------- not used for this call
*  `---------- 1 = archive
*
* on return:
* AX = error code if CF set  (see DOS ERROR CODES)
* CX = the attribute if AL was 00
*/
dos_error_code_t dos_get_file_attributes(const char* path_name, dos_file_attributes_t* attr) {
    dos_error_code_t errno = 0;
	__asm {
		.8086
		pushf
        push    ds

		lds		dx, path_name
		xor		cx, cx
		xor		al, al						; AL = 00 to get attribute
		mov		ah, DOS_FILE_ATTRIBUTES
		int		DOS_SERVICE
		jnc		OK
		mov		errno, ax
OK:		lds     di, attr
        mov     [di], cx

END:    pop     ds
        popf
	}
	return errno;
}

/**
* @note DOSBOX does not allow
* @see file::attributes_t get_file_attributes(char* path_name)
*/
dos_error_code_t dos_set_file_attributes(const char* path_name, dos_file_attributes_t attributes) {
    dos_error_code_t errno = 0;
    __asm {
		.8086
		pushf
        push    ds

		lds		dx, path_name
		mov		cx, attributes
		mov		al, 1					    ; AL = 01 to set attribute
		mov		ah, DOS_FILE_ATTRIBUTES
		int		DOS_SERVICE
		jnc		END
		mov		errno, ax

END:    pop     ds
        popf
	}
	return errno;
}

/**
 * INT 21,56 - Rename File
 *	AH = 56h
 *	DS:DX = pointer to old ASCIIZ path/filename
 *	ES:DI = pointer to new ASCIIZ path/filename
 * on return:
 *	AX = error code if CF set
 */
dos_error_code_t dos_rename_file(const char* old_path, const char* new_path) {
    dos_error_code_t errno = 0;
	__asm {
		.8086
		pushf
        push    ds
        push    es

		lds		dx, old_path
		les     di, new_path
		mov		ah, DOS_RENAME_FILE
		int		DOS_SERVICE
		jnc		END
		mov		errno, ax

END:    pop     es
        pop     ds
        popf
	}
	return errno;
}
