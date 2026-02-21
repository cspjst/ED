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
set(CMAKE_LINKER wlink)         # Use Watcom's linker
set(CMAKE_EXECUTABLE_SUFFIX ".exe")

# watcom compiler options
# https://users.pja.edu.pl/~jms/qnx/help/watcom/compiler-tools/cpopts.html
if(WATCOM)
    add_compile_options(
        -0                  # Generate 8086/8088 instructions ONLY (no 286+)
        -d0                 # no debug info
        #-e0                 # no fp
        #-os                 # optimize for space
        -za99               # enable partial C99 compatibility
        -ml                 # memory model options - large model
        -bt=dos             # Target DOS
        -w1                 # Mild warnings (avoid /w3 due to 8086 quirks)
        -zld                # Link only referenced functions
        #-zu                 # Smart library elimination
        #-zs                 # Strip debug info from linked libraries
        -zq                 # Quiet mode (cleaner output)
        #-dNDEBUG
    )
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

# Optional: Install target
# rename me...
# install(TARGETS example DESTINATION bin)
