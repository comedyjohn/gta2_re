#!/bin/sh
# Gets 32-bit SDL2 for building the port on a 64-bit Debian/Ubuntu: the i386 runtime from apt, and the
# headers from the libsdl2-dev:i386 package unpacked into $1 (installing that package normally tends to
# conflict with the amd64 dev packages). Then configure with -DCMAKE_PREFIX_PATH=$1/usr.
set -eu
out=$(cd "$(dirname "${1:?usage: setup_sdl32.sh <dir>}")" && pwd)/$(basename "$1")

sudo dpkg --add-architecture i386
sudo apt-get update -qq
sudo apt-get install -y -qq libsdl2-2.0-0:i386

mkdir -p "$out"
cd "$out"
apt-get download libsdl2-dev:i386
dpkg -x libsdl2-dev_*_i386.deb .
# Link against the runtime's shared library, not the package's static one or its dangling symlink
rm -f usr/lib/i386-linux-gnu/libSDL2*.a usr/lib/i386-linux-gnu/libSDL2.so usr/lib/i386-linux-gnu/libSDL2-2.0.so
sudo ln -sf /usr/lib/i386-linux-gnu/libSDL2-2.0.so.0 /usr/lib/i386-linux-gnu/libSDL2.so
echo "SDL2 headers in $out/usr; configure with -DCMAKE_PREFIX_PATH=$out/usr"
