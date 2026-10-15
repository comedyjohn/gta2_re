# Native port: where it stands and what is left

Read this first when picking the port up again. `README.md` explains how the port fits together and
`RUNNING.md` is for players. Update this file whenever a milestone lands or an item is done.

## Done

1. Milestone 1: `Source/` compiles and links with GCC (32-bit, `-fshort-wchar`) and the game starts.
2. Milestone 2: SDL window and message loop, OpenGL 2.1 renderer (the 58 `gbh_*`/`Vid_*` functions),
   keyboard and controllers behind DirectInput, the registry as a file, the launcher. `gta2 --selftest`
   checks the renderer.
3. Milestone 3: Miles (`AIL_*`) over SDL audio: 2D samples, WAV streams, the `GTAUDIO` path.
   `gta2 --audiotest` checks the mix.
4. 32 and 64 bits from one option: `port/build.sh 32|64|all` (or the `linux32`/`linux64` CMake
   presets, `GTA2_BITS`). 64-bit builds and passes the self-tests.
5. The 64-bit pointer fixes: pointers in 32-bit integers, the mission script's pointer slots and the
   file structures that hold pointers (see "Play-test the 64-bit build" below for what is covered).
6. Multiplayer, for every system the port builds for (`MULTIPLAYER.md`): DirectPlay over the port's
   own reliable UDP protocol, a session server in the hosting game or on its own (`gta2-server`), the
   lobby on the command line (`--host`, `--join`), the same simulation on every system (MSVC's
   `rand()`, SSE2 and no fused multiply-adds), builds of other game code turned away, NetPlay's 64-bit
   casts, and `tools/nettest.sh` (32 against 64 bits in CI; ARM64 by hand under qemu).
7. The game starts: the DirectX 6.1 check (`GetDirectXVersion_4C4EC0`) no longer probes for
   `DDRAW.DLL`, which stopped every start with "Incorrect version of DirectX". It now gets to loading
   game data; nothing past that has been run.
8. ARM64 builds (`cmake/aarch64-linux-gnu.cmake`): the profiler's `rdtsc` has a clock on other CPUs.
9. The front end shows, with real game data: `ShowWindow` and focus changes no longer post
   `WM_ACTIVATE`/`WM_SETFOCUS` to a window that is active already. The game shows its window on every
   video mode change, which it makes on `WM_SETFOCUS`, so the queue never emptied and the front end
   (run only when it is empty) never drew.
10. The game code is forked into `game/` and built as C++23. `Source/` stays the byte-matching
    decompilation; `game/` no longer has to match.
11. The 64-bit build starts a game: the light grid was allocated for 4-byte pointers and corrupted
    the heap. Found with AddressSanitizer.

## Left to do

The order follows what a player notices first.

### Missing for players
- [ ] **Single player with real game data.** Being played (32-bit, and 64-bit since the light grid
      fix). Report what breaks; the 64-bit AddressSanitizer build points at memory bugs directly.
- [ ] **Bink movies.** `Source/3rdParty/bink.cpp` is still stubs, and the `.bik` files are reported
      absent, so the intros are skipped. Plan: decode them with FFmpeg's Bink decoder.
- [ ] **3D sound.** Nothing implements `AIL_*_3D_*` yet, and `AIL_enumerate_3D_providers` offers no
      provider, so the options menu's 3D setting does nothing. Plan: OpenAL, or panning by position
      in `platform/mss.cpp`'s mixer.
- [ ] **Higher resolutions than 640x480.** The picture is scaled into the window at 4:3. The game's
      projection code would need to know the new size.
- [ ] **Multiplayer with real game data.** Built and tested without game data (`MULTIPLAYER.md`);
      next is a real match: two copies on one PC, two machines, then mixed systems. Then, if wanted, a
      graphical lobby (the original dialog's controls drawn with Dear ImGui) and NAT hole punching.

### Polish
- [ ] **Radio hitch.** Streams are decoded whole when opened (`SDL_LoadWAV`). Decode in chunks to
      remove the hitch when the radio station changes.
- [ ] **ImGui debug overlay** (`ImGuiDebug.cpp`). It needs ImGui's OpenGL backend; until then
      `gbh_SetBeginSceneCB` ignores it.

### Code health
- [ ] **Compile without `-fpermissive`, and with warnings on for `game/`,** with GCC and Clang
      (Clang is needed for macOS; about five causes, mostly in headers: extra qualification on
      members, pointers cast to 32-bit integers, addresses of temporaries).
- [ ] **Run under AddressSanitizer and UndefinedBehaviorSanitizer.**
- [ ] **Floating-point.** VC6's x87 code and GCC's SSE differ in precision. Check that replays
      (`InputRecorder_67F8B0`) still play back the same.
- [ ] **Play-test the 64-bit build.** `port/build.sh 64` builds and its renderer and audio
      self-tests pass (CI runs both widths). The known 64-bit problems are fixed, without changing
      what VC6 generates, but nobody has played it yet:
      - Done: pointers kept in 32-bit integers in game code (`sptr`: car thread triggers, a ped's
        last collision, a police crew's target, `Aligned_malloc_4FE510`, the Kanji glyph outline,
        NetPlay's buffers and exit callback); the mission script's object slots (`PTR32`, a 32-bit
        handle at 64 bits); the style's sprite index and sprite deltas, the text's TKEY entries, the
        map's column words and zone list (64-bit loader branches and size macros).
      - Checked and left as they are: the save game, replay and the other map/style/text chunks
        hold no pointers. The structures whose asserted size counts 4-byte pointers (`Camera_0xBC`,
        `CarInfo_808`, `Car_BC`, `Car_Door_10`, `Ped`, `Ped_List_4`, `ScriptStackFrame_C`,
        `ScriptStackFramePool_25C`, `SpritePool`, `miss2_0x11C`, `miss2_8`) only live in memory.
      - Casts GCC still flags at 64 bits (`-Wpointer-to-int-cast -Wint-to-pointer-cast`) and why
        they are harmless: `Network_20324.cpp`'s dialog procedures (the port's lobby doesn't use
        them; `NetPlay.cpp` has none left), `error.cpp` (integer arguments read back as `const char*`, then cast to an
        integer again), file offsets stored in pointer fields and read back
        (`gtx_0x106C`, `text_0x14`), values only used as numbers (`sound_obj.cpp`'s hash and parity
        bit, `sprite.hpp`'s collision id, `registry.cpp`/`Frontend.cpp` reading a DWORD into a
        pointer variable, `winmain`'s `ShellExecuteA` result), and pointers handed to stubs or
        never read (`Bink.cpp`, `Draw.cpp`'s screen table).
      - Next: play with real game data, ideally side by side with the 32-bit build (replays help),
        and under the sanitizers above. Anything that still breaks only at 64 bits most likely
        stores a pointer through a 4-byte type GCC doesn't flag (`*(u32*)&p`, a `memcpy` of 4
        bytes, a stride of 4 or 8 written out by hand).

## Working rules (from the top-level CLAUDE.md)
- Work on a branch, never `master`. The current branch is `claude/nice-feynman-sw4r6m`.
- A port change to `Source/` must keep every `MATCH_FUNC` matching.
- Keep commits small, one milestone or one fix each.
