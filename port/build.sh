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

build() {
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
