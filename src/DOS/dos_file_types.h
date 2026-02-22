#ifndef DOS_SERVICES_FILES_TYPES_H
#define DOS_SERVICES_FILES_TYPES_H

#include "../STD/dos_stdint.h"

typedef uint16_t dos_file_handle_t;

typedef uint16_t dos_file_attributes_t;

typedef int32_t dos_file_position_t;            // N.B. signed offset

typedef uint32_t dos_file_size_t;

/**
* DOS int 21h, 36h      Get Disk Free Space
*/
typedef struct {
    int16_t sectors_per_cluster;    // 0FFFFH if the drive number is invalid
    int16_t available_clusters;
    int16_t bytes_per_sector;
    int16_t clusters_per_drive;
} dos_file_disk_space_info_t;

/**
 *  |5|4|3|2|1|0|  CX  valid file attributes
	 | | | | | `---- 1 = read only
	 | | | | `----- 1 = hidden
	 | | | `------ 1 = system
	 | `--------- not used for this call
	 `---------- 1 = archive
 */
typedef enum {
    ATTR_READ_ONLY = 1,
    ATTR_HIDDEN,
    ATTR_SYSTEM = 4,
    ATTR_ARCHIVE = 32
} dos_valid_file_attributes_t;

typedef enum {
    GET_DEVICE_INFO = 0,   //IOCTL,0   Get Device Information
	SET_DEVICE_INFO        // IOCTL,1   Set Device Information
	//IOCTL,2   Read From Character Device
	//IOCTL,3   Write to Character Device
	//IOCTL,4   Read From Block Device
	//IOCTL,5   Write to Block Device
	//IOCTL,6   Get Input Status
	//IOCTL,7   Get Output Status
	//IOCTL,8   Device Removable Query
	//IOCTL,9   Device Local or Remote Query
	//IOCTL,A   Handle Local or Remote Query
	//IOCTL,B   Set Sharing Retry Count
	//IOCTL,C   Generic I/O for Handles
	//IOCTL,D   Generic I/O for Block Devices (3.2+)
	//IOCTL,E   Get Logical Drive (3.2+)
	//IOCTL,F   Set Logical Drive (3.2+)
} dos_ioctl_t;

// DOS Device Data Word — bit masks (16-bit, bits 0 to 15)
// Only bits 0–8 are defined; bits 9–15 are reserved (must be 0)
typedef enum {
    DOS_DEV_STDIN       = 0x0001U,  // bit 0: standard input device
    DOS_DEV_STDOUT      = 0x0002U,  // bit 1: standard output device
    DOS_DEV_NUL         = 0x0004U,  // bit 2: NUL device
    DOS_DEV_CLOCK       = 0x0008U,  // bit 3: clock device
    // bit 4: reserved
    // bit 5: reserved
    DOS_DEV_EOF_PENDING = 0x0040U,  // bit 6: EOF on input (read-only; set when Ctrl+Z is next char)
    DOS_DEV_BINARY      = 0x0080U,  // bit 7: 1 = binary mode, 0 = text (translated) mode
    DOS_DEV_CHAR        = 0x0100U   // bit 8: 1 = character device (always set for CON, AUX, etc.)
    // bits 9–15: reserved, must be 0
} dos_device_data_word_t;

typedef enum {
    DOS_STREAM_MODE_TEXT = 0,
    DOS_STREAM_MODE_BINARY = 0x80
} dos_stream_mode_t;

typedef enum {
    FSEEK_SET,
    FSEEK_CUR,
    FSEEK_END
} dos_file_seek_attributes_t;

/**
* Bitfields for create file attributes:
*
* 0     read-only
* 1     hidden
* 2     system
* 3     volume label (ignored)
* 4     reserved, must be zero (directory)
* 5     archive bit
* 7     if set, file is shareable under Novell NetWare
*
* |5|4|3|2|1|0|  CX  valid file attributes
*  | | | | | `---- 1 = read only                        (DOSBOX ignored)
*  | | | | `----- 1 = hidden
*  | | | `------ 1 = system
*  | `--------- not used for this call
*  `--
*/
typedef enum {
    CREATE_READ_WRITE = 0,
    CREATE_READ_ONLY,
    CREATE_HIDDEN,
    CREATE_SYSTEM = 4,
    CREATE_VOLUME = 8,
    CREATE_ARCHIVE = 32,
    CREATE_SHAREABLE = 128
} dos_file_create_attributes_t;

/**
* Access modes in AL:
* |7|6|5|4|3|2|1|0|  AL
* | | | | | `-------- read/write/update access mode
* | | | | `--------- reserved, always 0
* | `-------------- sharing mode (see below) (DOS 3.1+)
* `--------------- 1 = private, 0 = inheritable (DOS 3.1+)
*
* Sharing mode bits (DOS 3.1+):        Access mode bits:
* 654                                                                    210
* 000  compatibility mode (exclusive)    000  read access
* 001  deny others read/write access     001  write access
* 010  deny others write access          010  read/write access
* 011  deny others read access
* 100  full access permitted to all
*
* - will open normal, hidden and system files
* @note file pointer is placed at beginning of file
*/
typedef enum {
    ACCESS_READ_ONLY = 0,
    ACCESS_WRITE_ONLY,
    ACCESS_READ_WRITE,
    SHARE_EXCLUSIVE = 0,
    DENY_READ_WRITE = 16,
    DENY_WRITE = 32,
    DENY_READ = 48,
    SHARE_FULL = 64,
    PRIVATE = 128
} dos_file_access_attributes_t;


#endif
