# Native Linux port

A native Linux build of the decompiled game: the same `Source/` that the VC6 build compiles into a
matching `decomp_main.exe`, compiled by GCC into a Linux executable, with no Wine and no Windows DLLs.

```bash
sudo apt-get install g++-multilib cmake ninja-build libsdl2-dev   # Debian/Ubuntu
port/build.sh 32      # build_port/linux32/gta2: the playable one
port/build.sh 64      # build_port/linux64/gta2: builds and passes its self-tests, not playable yet
port/build.sh all     # both
```

`build.sh` fetches the two submodules it needs (GTA2Hax and the Dear ImGui inside it) and, for 32-bit
on a 64-bit system, the 32-bit SDL2 headers (`tools/setup_sdl32.sh`, into `~/sdl32`). It then uses the
CMake presets in `CMakePresets.json`, which work directly too:

```bash
cmake --preset linux32 && cmake --build --preset linux32
cmake --preset linux64 && cmake --build --preset linux64
```

The pointer size is the one CMake option, `GTA2_BITS` (32 or 64); configure fails if the compiler
doesn't produce that size. A cross toolchain (`CMAKE_TOOLCHAIN_FILE`, e.g. for ARM handhelds) sets
the target itself, and `GTA2_BITS` must match it.

Run `build_port/linux32/gta2` from a directory holding the game's `data/` folder, or use
`port/gta2-launch` as described in [RUNNING.md](RUNNING.md). `gta2 --selftest` (renderer) and
`gta2 --audiotest` (sound) check a build without game data.

## Status

**Milestone 3: sound and music.** Players: see [RUNNING.md](RUNNING.md).

| Subsystem | Windows | Here |
|---|---|---|
| Window, message loop | user32 (`winmain.cpp`) | an SDL window; SDL events arrive as the window messages the game's window procedure handles (`platform/win32.cpp`, `platform/sdl/`) |
| Rendering | `d3ddll.dll` (`gbh_*`) and `dmavideo.dll` (`Vid_*`), loaded by name | `platform/renderer.cpp`: the same 58 functions over OpenGL 2.1, following GTA2Hax's Direct3D 9 version. The game's 640x480 picture is scaled into the window, 4:3 |
| Input | DirectInput 5 | `platform/dinput.cpp`: keyboard and SDL game controllers behind DirectInput's interfaces, laid out by the data formats the game sets, polled or buffered |
| Audio | Miles Sound System (`AIL_*`) | `platform/mss.cpp`: the 2D samples, streams and digital driver the game uses, mixed into an SDL audio device. No 3D providers, so the game keeps to 2D samples |
| Video | Bink | `Source/3rdParty/bink.cpp` stubs; `.bik` files are reported absent, so the game skips its movies |
| Multiplayer | DirectPlay | unavailable: the game finds no connections |
| Settings | registry | a persistent key/value store (`platform/win32.cpp`) |
| Files | Win32 and CRT paths | `platform/crt.cpp`: `\` becomes `/`, components matched ignoring case; a drive (`C:\GTAUDIO\`, where the game looks for the CD's music) is the game folder |

`gta2 --selftest` draws through the renderer's entry points (a palette, paletted tiles with
transparency and rotation, additive blending, a 16-bit TGA) and checks the pixels; CI runs it on a
virtual display. `gta2 --audiotest` plays samples and streams through the `AIL_*` functions
(volume, pan, loop counts and loop blocks, playback rates, 8-bit data, PCM and IMA ADPCM WAVs, the
`GTAUDIO` path) and checks the mix; it needs no display or sound card. `gta2-launch` is what players run: it finds the game folder, or unpacks an Inno
Setup installer with `innoextract`, and starts the game in it.

## How it fits together

- **The game sources compile unchanged.** `include/` holds stand-in headers named after the Windows
  ones the game includes (`windows.h`, `dinput.h`, `mmsystem.h`, `VC98/Include/DPLAY.H`, ...),
  declaring only what the game uses. Structures follow the SDK layouts, since several sit inside
  game structures whose sizes are asserted.
- **`include/port_prelude.h` is force-included** into the game and into `platform/`. It defines
  `GTA2_PORT`, MSVC's keywords (`__stdcall` and friends) and `_WIN32`: third-party headers the game
  includes (GTA2Hax's `ddraw_vc6.h`, Miles' `mss.h`) pick their Win32 branch from it. It refuses to
  build anything but 32-bit with a 2-byte `wchar_t`.
- **`platform/` implements that API.** The SDL code is in `platform/sdl/`, a library of its own that
  doesn't see the prelude, since SDL takes `_WIN32` to mean "building for Windows". The rest of
  `platform/` reaches it through `platform/platform.h`, which passes only plain types. OpenGL is
  loaded at runtime through SDL (`platform/gl.h`), so no GL headers or libGL are needed to build.
- **32-bit is the playable build.** Game structures were laid out for 4-byte pointers, and in places
  the game keeps a pointer in an `s32`. `GTA2_BITS=64` builds: the Win32 types keep their Windows
  widths (`DWORD` and `LONG` stay 32-bit, as on 64-bit Windows), the structure size checks that count
  4-byte pointers are skipped, and `sptr` (`Source/types.hpp`) is the type for an integer that holds a
  pointer (`s32` under MSVC, so the VC6 build is unchanged). What's left is in `TODO.md`.
- **`wchar_t` is 2 bytes** (`-fshort-wchar`), as on Windows: the game's text is UTF-16 and several
  structures hold `wchar_t` arrays. glibc's `wcs*` functions assume 4 bytes, so the ones the game uses
  are redirected to `platform/wchar.cpp`, including MSVC's `swprintf` (no size argument; `%s` takes
  a wide string).
- **MSVC 6 semantics:** `-fwrapv` and `-fno-strict-aliasing`, because decompiled code relies on
  wrapping arithmetic and type punning that MSVC never exploited. `-fpermissive` still covers a few
  MSVC-only constructs; removing it is one of the follow-ups below.
- **The source list is read from the top-level `CMakeLists.txt`** (`gta2_lib_src`), so a new file
  added for the VC6 build is picked up here too.
- An undefined `_MSC_VER` is 0 in `#if`, so GCC takes the `_MSC_VER <= 1200` (VC6) branches in
  `error.hpp`, `input.cpp` and friends. `include/fstream.h` provides the pre-standard `ofstream` the
  VC6 branch of `ErrorLog` holds, at its original 0x3C bytes.

## Changing game sources for the port

The VC6 build must keep matching: a change in `Source/` for the port may not change what VC6
generates. In practice the changes needed so far were:

- include names spelled with the file's real case;
- reference parameters taking temporaries made `const` (standard C++ doesn't bind a temporary to a
  non-const reference; MSVC does);
- inline asm given a C++ equivalent under `#if defined(GTA2_PORT)`.

Check every such change against a baseline with `tools/check_matching.sh`. It rebuilds the VC6
build under Wine (set up as described in the top-level CLAUDE.md) and runs
`Scripts/bin_comp/compare_builds.py`:

```bash
# once, on a known-good commit
python3 build.py --ignore_no_match && (cd Scripts/bin_comp && python3 msvc_dump_new_data.py \
  && python3 compare_builds.py --save /some/dir/baseline)
# after the change
port/tools/check_matching.sh /some/dir/baseline   # must report "changed: 0"
```

## Follow-ups

- 3D sound: a provider behind `AIL_*_3D_*` (OpenAL, or panning by position in the mixer), so the
  options menu's 3D sound setting does something. Until then `AIL_enumerate_3D_providers` offers none.
- Streams are decoded whole when opened (`SDL_LoadWAV`); a music track is a few MB, decoded in tens of
  milliseconds. Decoding in chunks would remove that hitch when the radio changes.
- Bink movies, with FFmpeg's Bink decoder.
- The ImGui debug overlay (`ImGuiDebug.cpp`): needs ImGui's OpenGL backend; `gbh_SetBeginSceneCB` ignores it for now.
- Higher resolutions than 640x480 (the game's own projection would need to know).
- Compile without `-fpermissive`, and with warnings on for `Source/`, which will turn up the
  constructs MSVC accepted silently.
- Run under AddressSanitizer and UndefinedBehaviorSanitizer.
- Floating-point: the x87 code VC6 generated and GCC's SSE differ in precision. Most game maths is
  fixed-point (`Fix16`), but replays (`InputRecorder_67F8B0`) need checking.
