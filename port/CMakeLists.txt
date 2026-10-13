# Native Linux build of the decompiled game. A separate project from the top-level CMakeLists.txt,
# which builds the matching VC6 executable:
#
#   cmake -S port -B build_port && cmake --build build_port -j
#
# The game sources are compiled unchanged, against port/include/ (a Win32/DirectX-shaped API
# declared for the game) and linked with port/platform/ (its implementation).
cmake_minimum_required(VERSION 3.16)

# Structures hold 32-bit pointers and their sizes are asserted, so the port is 32-bit for now. Set
# before project() so CMake detects a 32-bit target and looks for i386 libraries (SDL2).
set(CMAKE_C_FLAGS_INIT "-m32")
set(CMAKE_CXX_FLAGS_INIT "-m32")
set(CMAKE_EXE_LINKER_FLAGS_INIT "-m32")
project(gta2_native C CXX)

set(REPO_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/..)

set(CMAKE_CXX_STANDARD 14) # C++17 removed the throw(std::bad_alloc) specifications crt_stubs.cpp uses
set(CMAKE_CXX_EXTENSIONS ON)
add_compile_options(-fshort-wchar -fno-strict-aliasing -fwrapv)

# SDL2, 32-bit (Debian/Ubuntu: libsdl2-dev:i386). SDL_config.h forwards to an architecture
# directory, so both include directories are needed.
find_path(SDL2_INCLUDE_DIR SDL.h PATH_SUFFIXES SDL2 REQUIRED)
find_path(SDL2_ARCH_INCLUDE_DIR SDL2/_real_SDL_config.h)
find_library(SDL2_LIBRARY NAMES SDL2 SDL2-2.0 REQUIRED)
message(STATUS "SDL2: ${SDL2_LIBRARY}")

# The game's source list lives in the top-level CMakeLists.txt (gta2_lib_src); read it from there
# so the two builds can't drift apart.
file(READ ${REPO_ROOT}/CMakeLists.txt top_cmake)
string(REGEX MATCH "set\\(gta2_lib_src([^)]*)\\)" _ "${top_cmake}")
string(REGEX MATCHALL "Source/[^ \t\r\n]+\\.cpp" game_sources "${CMAKE_MATCH_1}")
list(TRANSFORM game_sources PREPEND ${REPO_ROOT}/)
list(LENGTH game_sources game_source_count)
if (game_source_count LESS 100)
    message(FATAL_ERROR "Read only ${game_source_count} sources from gta2_lib_src in ../CMakeLists.txt")
endif()

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
        ${CMAKE_CURRENT_SOURCE_DIR}/include ${FORWARDERS} ${REPO_ROOT}/Source ${REPO_ROOT})
    target_compile_definitions(${target} PRIVATE _CRT_SECURE_NO_WARNINGS _CRT_NON_CONFORMING_SWPRINTFS)
endfunction()

add_library(dear_imgui STATIC
    ${REPO_ROOT}/3rdParty/GTA2Hax/3rdParty/imgui/imgui.cpp
    ${REPO_ROOT}/3rdParty/GTA2Hax/3rdParty/imgui/imgui_draw.cpp)
target_compile_options(dear_imgui PRIVATE -fshort-wchar -w)

add_library(gta2_game STATIC
    ${game_sources}
    ${REPO_ROOT}/test.cpp
    ${REPO_ROOT}/StdAfx.cpp
    ${REPO_ROOT}/Source/3rdParty/mss.cpp   # Miles Sound System, stubbed (the same stubs the VC6 build ships as mss32.dll)
    ${REPO_ROOT}/Source/3rdParty/bink.cpp) # Bink video, stubbed likewise
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

add_executable(gta2
    platform/main.cpp
    platform/crt.cpp
    platform/wchar.cpp
    platform/win32.cpp
    platform/directx.cpp
    platform/dinput.cpp
    platform/renderer.cpp
    platform/selftest.cpp)
gta2_game_target(gta2)
target_compile_options(gta2 PRIVATE -Wall -Wno-unused-function)
target_link_libraries(gta2 PRIVATE gta2_game dear_imgui gta2_sdl)
