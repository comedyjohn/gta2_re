#!/bin/sh
# Builds the native port: port/build.sh 32 | 64 | all   (default: 32)
# Sets up what's missing on Debian/Ubuntu first (the 32-bit SDL2 headers in ~/sdl32), then
# configures and builds with the matching preset (port/CMakePresets.json). Output:
# build_port/linux32/gta2 and build_port/linux64/gta2.
set -eu
port="$(cd "$(dirname "$0")" && pwd)"
repo="$(dirname "$port")"

if [ ! -f "$repo/3rdParty/GTA2Hax/d3ddll/d3ddll.hpp" ] || [ ! -f "$repo/3rdParty/GTA2Hax/3rdParty/imgui/imgui.h" ]; then
    git -C "$repo" submodule update --init 3rdParty/GTA2Hax
    git -C "$repo/3rdParty/GTA2Hax" submodule update --init 3rdParty/imgui
fi

# The presets build with the system's gcc/g++ (CMakePresets.json). Check up front that it can
# produce the asked-for width, which otherwise fails deep inside CMake's compiler check.
check_compiler() {
    command -v g++ >/dev/null 2>&1 || { echo "build.sh: g++ is missing (Debian/Ubuntu: sudo apt install g++)" >&2; exit 1; }
    flag=""
    [ "$1" = 32 ] && [ "$(uname -m)" = x86_64 ] && flag=-m32
    if ! echo 'int main(){return 0;}' | g++ $flag -x c++ - -o /dev/null 2>/dev/null; then
        if [ -n "$flag" ]; then
            echo "build.sh: g++ can't build 32-bit programs (Debian/Ubuntu: sudo apt install g++-multilib)" >&2
        else
            echo "build.sh: g++ can't build a test program" >&2
        fi
        exit 1
    fi
}

build() {
    check_compiler "$1"
    if [ "$1" = 32 ] && [ ! -f "$HOME/sdl32/usr/include/SDL2/SDL.h" ]; then
        "$port/tools/setup_sdl32.sh" "$HOME/sdl32"
    fi
    (cd "$port" && cmake --preset "linux$1" && cmake --build --preset "linux$1")
    echo "Built $repo/build_port/linux$1/gta2"
}

case "${1:-32}" in
    32 | 64) build "${1:-32}" ;;
    all) build 32 && build 64 ;;
    *) echo "usage: $0 [32|64|all]" >&2 && exit 2 ;;
esac
