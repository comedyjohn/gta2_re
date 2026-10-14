# Cross toolchain for 64-bit ARM Linux (handhelds, Raspberry Pi 4/5, ARM laptops), with Debian/Ubuntu's
# g++-aarch64-linux-gnu. Configure with GTA2_BITS=64:
#   cmake -S port -B build_port/arm64 -G Ninja -DGTA2_BITS=64 \
#         -DCMAKE_TOOLCHAIN_FILE=port/cmake/aarch64-linux-gnu.cmake \
#         -DSDL2_INCLUDE_DIR=<arm64 SDL2 headers>/SDL2 -DSDL2_LIBRARY=<arm64 libSDL2-2.0.so.0>
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)
set(CMAKE_C_COMPILER aarch64-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER aarch64-linux-gnu-g++)
# SDL's own dependencies, which the linker checks, live in the multiarch directory
set(CMAKE_EXE_LINKER_FLAGS_INIT "-Wl,-rpath-link,/usr/lib/aarch64-linux-gnu")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
