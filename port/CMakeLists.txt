# Native Linux build of the decompiled game. A separate project from the top-level CMakeLists.txt,
# which builds the matching VC6 executable. Easiest through the presets (port/CMakePresets.json) or
# port/build.sh:
#
#   cmake --preset linux32 && cmake --build --preset linux32     (or: port/build.sh 32)
#   cmake --preset linux64 && cmake --build --preset linux64     (or: port/build.sh 64)
#
# The game sources are compiled unchanged, against port/include/ (a Win32/DirectX-shaped API
# declared for the game) and linked with port/platform/ (its implementation).
cmake_minimum_required(VERSION 3.16)

# GTA2_BITS picks the pointer size. 32 is what plays today: the game's structures were laid out for
# 4-byte pointers. 64 builds and handles those layouts, but hasn't been play-tested yet
# (port/TODO.md). On an x86-64 host, 32 means -m32, set before project() so CMake detects a
# 32-bit target and looks for i386 libraries. A cross toolchain (CMAKE_TOOLCHAIN_FILE, e.g. armhf or
# aarch64) brings its own target, and only the check below applies.
set(GTA2_BITS 32 CACHE STRING "Pointer size of the native build: 32 or 64")
set_property(CACHE GTA2_BITS PROPERTY STRINGS 32 64)
if (NOT GTA2_BITS STREQUAL "32" AND NOT GTA2_BITS STREQUAL "64")
    message(FATAL_ERROR "GTA2_BITS must be 32 or 64, not ${GTA2_BITS}")
endif()
cmake_host_system_information(RESULT host_arch QUERY OS_PLATFORM) # project() hasn't detected it yet
if (GTA2_BITS STREQUAL "32" AND NOT CMAKE_TOOLCHAIN_FILE AND host_arch MATCHES "x86_64|AMD64")
    set(CMAKE_C_FLAGS_INIT "-m32")
    set(CMAKE_CXX_FLAGS_INIT "-m32")
    set(CMAKE_EXE_LINKER_FLAGS_INIT "-m32")
endif()
project(gta2_native C CXX)

math(EXPR detected_bits "${CMAKE_SIZEOF_VOID_P} * 8")
if (NOT detected_bits EQUAL GTA2_BITS)
    message(FATAL_ERROR "GTA2_BITS=${GTA2_BITS}, but the compiler produces ${detected_bits}-bit code"
                        " (for 32-bit on x86-64, install g++-multilib)")
endif()
message(STATUS "GTA2 native build: ${GTA2_BITS}-bit")

set(REPO_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/..)

set(CMAKE_CXX_STANDARD 14) # C++17 removed the throw(std::bad_alloc) specifications crt_stubs.cpp uses
set(CMAKE_CXX_EXTENSIONS ON)
# MSVC semantics the decompiled code relies on: 2-byte wchar_t, signed char (unsigned by default on
# ARM), wrapping signed arithmetic and type punning
add_compile_options(-fshort-wchar -fsigned-char -fno-strict-aliasing -fwrapv)
# The same floating-point arithmetic on every machine, so a network game (lockstep: every machine runs
# the whole simulation) can't drift apart between a 32-bit PC, a 64-bit one and an ARM handheld. The
# game's maths is fixed-point; floating point only fills its trigonometry tables and takes square
# roots, both far enough from any rounding edge to come out alike today (gta2 --simcheck compares
# them). These keep it that way: 32-bit x86 uses SSE2 like the others, rather than x87 registers
# whose 80-bit intermediates depend on where the compiler spills them, and no CPU fuses a multiply
# and an add (GCC does by default on ARM).
add_compile_options(-ffp-contract=off)
if (CMAKE_SIZEOF_VOID_P EQUAL 4 AND CMAKE_SYSTEM_PROCESSOR MATCHES "x86|i.86|AMD64")
    add_compile_options(-msse2 -mfpmath=sse)
endif()

# SDL2 for the target architecture (Debian/Ubuntu: libsdl2-dev for 64-bit; for 32-bit see
# tools/setup_sdl32.sh). SDL_config.h forwards to an architecture directory, so both include
# directories are needed.
find_path(SDL2_INCLUDE_DIR SDL.h PATH_SUFFIXES SDL2 REQUIRED)
find_path(SDL2_ARCH_INCLUDE_DIR SDL2/_real_SDL_config.h)
find_library(SDL2_LIBRARY NAMES SDL2 SDL2-2.0 REQUIRED)
message(STATUS "SDL2: ${SDL2_LIBRARY}")

# The game code: port/game/, forked from the decompilation's Source/ (which still builds the
# byte-matching VC6 executable). Every .cpp in it is part of the game.
set(GAME_DIR ${CMAKE_CURRENT_SOURCE_DIR}/game)
file(GLOB game_sources CONFIGURE_DEPENDS ${GAME_DIR}/*.cpp)

# GTA2Hax includes its own headers with Windows separators; forward them from the build directory
set(FORWARDERS ${CMAKE_BINARY_DIR}/forwarders)
execute_process(COMMAND sh ${CMAKE_CURRENT_SOURCE_DIR}/tools/gen_forwarders.sh ${FORWARDERS} COMMAND_ERROR_IS_FATAL ANY)

# Everything that sees the game's view of the world: the game itself and the platform code that
# implements Win32/DirectX for it. The prelude defines _WIN32 and friends for these only.
function(gta2_game_target target)
    target_compile_options(${target} PRIVATE
        -include ${CMAKE_CURRENT_SOURCE_DIR}/include/port_prelude.h
        $<$<COMPILE_LANGUAGE:CXX>:-fpermissive> # MSVC accepted these; see port/README.md
        -Wno-write-strings -Wno-narrowing -Wno-multichar -Wno-unknown-pragmas -Wno-invalid-offsetof)
    target_include_directories(${target} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include ${FORWARDERS} ${GAME_DIR} ${REPO_ROOT})
    target_compile_definitions(${target} PRIVATE _CRT_SECURE_NO_WARNINGS _CRT_NON_CONFORMING_SWPRINTFS)
endfunction()

add_library(dear_imgui STATIC
    ${REPO_ROOT}/3rdParty/GTA2Hax/3rdParty/imgui/imgui.cpp
    ${REPO_ROOT}/3rdParty/GTA2Hax/3rdParty/imgui/imgui_draw.cpp)
target_compile_options(dear_imgui PRIVATE -fshort-wchar -w)

add_library(gta2_game STATIC
    ${game_sources}
    ${GAME_DIR}/3rdParty/bink.cpp) # Bink video, stubbed (the same stubs the VC6 build ships as binkw32.dll)
gta2_game_target(gta2_game)
target_compile_options(gta2_game PRIVATE -w) # the decompiled code is warning-heavy by nature

# The SDL side of port/platform/platform.h. It must not see the prelude (SDL reads _WIN32 as
# "building for Windows"), so it is a library of its own.
add_library(gta2_sdl STATIC platform/sdl/sdl_platform.cpp)
target_include_directories(gta2_sdl PRIVATE ${SDL2_INCLUDE_DIR})
if (SDL2_ARCH_INCLUDE_DIR)
    target_include_directories(gta2_sdl PRIVATE ${SDL2_ARCH_INCLUDE_DIR})
endif()
target_compile_options(gta2_sdl PRIVATE -Wall)
target_link_libraries(gta2_sdl PUBLIC ${SDL2_LIBRARY})

# The network layer (port/platform/net/): plain C++ and POSIX sockets, no game or SDL types, so the
# dedicated server builds from it alone
find_package(Threads REQUIRED)
add_library(gta2_net STATIC
    platform/net/socket.cpp
    platform/net/link.cpp
    platform/net/server.cpp
    platform/net/client.cpp)
target_compile_options(gta2_net PRIVATE -Wall)
target_link_libraries(gta2_net PUBLIC Threads::Threads)

# gta2-server: a session server on its own, for a machine everybody can reach (port/MULTIPLAYER.md)
add_executable(gta2-server platform/net/server_main.cpp)
target_compile_options(gta2-server PRIVATE -Wall)
target_link_libraries(gta2-server PRIVATE gta2_net)

# Which builds can play together: a hash of the game code, regenerated when it changes
file(GLOB compat_inputs ${GAME_DIR}/*.cpp ${GAME_DIR}/*.hpp ${GAME_DIR}/*.h
     ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h ${CMAKE_CURRENT_SOURCE_DIR}/platform/crt.cpp)
add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/gta2_compat.h
    COMMAND ${CMAKE_COMMAND} -DREPO=${REPO_ROOT} -DOUT=${CMAKE_BINARY_DIR}/gta2_compat.h
            -P ${CMAKE_CURRENT_SOURCE_DIR}/tools/game_hash.cmake
    DEPENDS ${compat_inputs} ${CMAKE_CURRENT_SOURCE_DIR}/tools/game_hash.cmake
    COMMENT "Hashing the game code for network compatibility")

add_executable(gta2
    ${CMAKE_BINARY_DIR}/gta2_compat.h
    platform/main.cpp
    platform/crt.cpp
    platform/wchar.cpp
    platform/win32.cpp
    platform/directx.cpp
    platform/dplay.cpp
    platform/lobby.cpp
    platform/nettest.cpp
    platform/simcheck.cpp
    platform/dinput.cpp
    platform/renderer.cpp
    platform/mss.cpp
    platform/ptr32.cpp
    platform/selftest.cpp
    platform/audiotest.cpp)
gta2_game_target(gta2)
target_compile_options(gta2 PRIVATE -Wall -Wno-unused-function)
target_include_directories(gta2 PRIVATE ${CMAKE_BINARY_DIR})
target_link_libraries(gta2 PRIVATE gta2_game dear_imgui gta2_sdl gta2_net)
