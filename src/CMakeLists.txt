cmake_minimum_required(VERSION 3.10)

# Warning: This skips critical compiler checks. Only use this if Watcom fails CMake's detection
# Necessary to suppress compiler checks for cross compilation using OW2 and C under ARM environments
set(CMAKE_C_COMPILER_WORKS 1)

project(
    ED
    VERSION 0.1.0
    LANGUAGES C
)

# Toolchain setup
set(CMAKE_SYSTEM_NAME DOS)      # Target DOS
set(CMAKE_C_COMPILER wcl)
set(CMAKE_CXX_COMPILER wcl)
# set(CMAKE_LINKER wlink)        # wcl handles linking with -flag syntax
set(CMAKE_EXECUTABLE_SUFFIX ".exe")

# watcom compiler options
# https://users.pja.edu.pl/~jms/qnx/help/watcom/compiler-tools/cpopts.html
if(WATCOM)
    # === COMPILER-ONLY FLAGS (passed to wcl -c) ===
    # These affect code generation, warnings, and compilation behavior
    add_compile_options(
        -0                  # Generate 8086/8088 instructions ONLY (no 286+)
        -d0                 # no debug info in object files
        #-e0                 # no fp emulation (uncomment to disable float support)
        -os                 # optimize for space (critical for 64KB segments)
        -za99               # enable partial C99 compatibility
        -ml                 # memory model: large (far data pointers) [must match link]
        -bt=dos             # Target DOS runtime
        -w1                 # Mild warnings (avoid /w3 due to 8086 quirks)
        -zq                 # Quiet mode (cleaner output) [also valid for linker]
    )

    # === LINKER-ONLY FLAGS (passed to wcl during link step) ===
    # CRITICAL: Single quoted string with SPACE separation
    # wcl will translate these to wlink directives internally
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -ml -bt=dos -zq -zld -zu -zs -fo=none")

    add_compile_definitions(
        __DOS__
        __8086__  # Explicit 8086 target
        # custom policies here
    )
endif()

# Add definitions
#

# WARNING: Using GLOB for convenience. If adding new files, rerun:
#   ./cmk.sh
file(GLOB SOURCES
    CONFIGURE_DEPENDS
    *.c
    SNO/*.c
    DOS/*.c
    BIOS/*.c
    STD/*.c
)

# message(Source list="${SOURCES}")

add_executable(ED ${SOURCES})

# === FREESTANDING BUILD: Exclude standard C runtime ===
# For minimal libc: do not link clibl.lib, emu87.lib, etc.
# The -fo=none flag above handles this; this property is redundant but explicit
set_target_properties(ED PROPERTIES
    LINK_SEARCH_START_STATIC ON
    LINK_SEARCH_END_STATIC ON
)

# Optional: Install target
# rename me...
# install(TARGETS example DESTINATION bin)
