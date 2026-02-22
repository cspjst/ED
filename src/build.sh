#!/bin/bash
# Build script: wcl for BOTH compile AND link (cross-compiler compatible)

CFLAGS="-0 -bt=dos -ml -os -d0 -e0 -za99 -w1 -zq"
DEFS="-D__DOS__ -D__8086__"
OUT="edlin.exe"

echo "=== Compiling ==="
for src in main.c \
           BIOS/bios_keyboard_services.c BIOS/bios_memory_services.c \
           DOS/dos_file_services.c DOS/dos_file_tools.c \
           DOS/dos_memory_services.c DOS/dos_services.c \
           SNO/sno.c \
           STD/dos_errno.c STD/dos_stdio.c STD/dos_stdlib.c STD/dos_string.c; do
    obj="${src%.c}.obj"
    echo "  $src -> $obj"
    wcl -c $CFLAGS $DEFS "$src" -fo="$obj" || exit 1
done

echo "=== Linking (wcl driver) ==="

# Link with wcl - CRITICAL: -fe=FILENAME (no space, with =)
wcl -bt=dos -ml -fe="$OUT" \
    main.obj \
    BIOS/bios_keyboard_services.obj \
    BIOS/bios_memory_services.obj \
    DOS/dos_file_services.obj \
    DOS/dos_file_tools.obj \
    DOS/dos_memory_services.obj \
    DOS/dos_services.obj \
    SNO/sno.obj \
    STD/dos_errno.obj \
    STD/dos_stdio.obj \
    STD/dos_stdlib.obj \
    STD/dos_string.obj

echo "=== Done ==="
ls -l "$OUT" 2>&1 || echo "ERROR: $OUT not created"
