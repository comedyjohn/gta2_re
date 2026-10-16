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

12. Keyboard and pad input (`platform/padinput.cpp` the pad layer, `platform/controls.cpp` its link to
    the game, `include/port_input.h` the game's hooks, `platform/overlay.cpp` the menus; tests in
    `tests/test_padinput.cpp`, `tests/test_overlay.cpp` and the corpus replay `00-synthetic-pad`):
    - Every control has a key (the game's own binding) and a pad input (a layout for walking and one
      for driving, in `pad.txt`); both work at once. The game's own one-device-per-control pad support
      is no longer offered to it. Keys bound to pad buttons by older builds go back to their default.
    - The pad is read once per update; the stick's heading and push, the gas trigger, the turret stick
      and a spawned car are recorded in replays as port records and applied by game code, so pad
      sessions replay exactly (on every build).
    - Walking: the stick sets the heading (the character turns to it), speed follows the push
      (`port_input_apply`, `port_walk_speed` in `char.cpp`). Driving: triggers bound to Forward/Backward
      are analog pedals (`port_gas_scale` in `CarPhysics_B0::ComputeEngineTorque_561970`), right stick
      turns turrets.
    - Esc / Start: pause menu (Settings, Quit game) in place of the game's quit question; `esc_menu = 0`
      in `pad.txt` (or E / Back in the settings) gives the original Esc back. F1 legend, F2 / Guide
      settings, F5 / Back car spawn menu. In single player any of these menus freezes the game and its
      sound (`port_game_frozen` in `RunGameFrame_4DA850`); the pad drives the front end's menus. A button held when the layout changes or a menu closes waits to be let go.
    - `GTA2_TEST_PAD=file` replaces the pad with a script (`tests/fixtures/pad-session.txt`).
    - The registry is saved through a temporary file, so it can't be left half written.

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
- [ ] **3D renderer and RTX Remix** (from gebdag/gta2-rtx-remix): planned in `world3d/PLAN.md`. Also does
      the item above (any window size, 16:9).
- [ ] **Multiplayer with real game data.** Built and tested without game data (`MULTIPLAYER.md`);
      next is a real match: two copies on one PC, two machines, then mixed systems. Then, if wanted, a
      graphical lobby (the original dialog's controls drawn with Dear ImGui) and NAT hole punching.

### Polish
- [ ] **Controller overlay follow-ups** (see item 12): play-test on the 8BitDo Pro 2 (walking directions,
      analog speed, driving, turret, the new input layer); F1/F2 shadow the game's radio keys (move the overlay to F7/F8, or
      put the radio on the pad); the turret turn sound (`field_B8_turret_rotated` is reset by
      `HandleUserInput`) and an analog turret speed; spawned cars' facing and the other models; names of
      controls 9..11 against the game's options menu; a tank page of its own; the spawn position uses the C library's sin/cos (a pad replay with a spawn could
      differ between x86 and ARM).
- [ ] **ASan: texture upload reads past the tile buffer, sometimes.** `SetTexture` (`renderer.cpp:428`,
      from `TextureCache_15D8::ReadTextures_5B92E0`) reads one byte after the 4,128,768-byte tile buffer
      `gtx_0x106C::load_tiles_5AADF0` allocates. Seen 2026-10-16 in about one corpus replay in four under
      `asan64`, a different one each run (also before the input rework); the plain builds don't notice.
- [ ] **Radio hitch.** Streams are decoded whole when opened (`SDL_LoadWAV`). Decode in chunks to
      remove the hitch when the radio station changes.
- [ ] **ImGui debug overlay** (`ImGuiDebug.cpp`). It needs ImGui's OpenGL backend; until then
      `gbh_SetBeginSceneCB` ignores it.

### Code health
- [x] **Clang builds the port** (Clang 18, 64-bit; needed for macOS).
- [x] **C++23 clean-up, first pass:** `nullptr`, `true`/`false`, `using` aliases, `<cstdio>`-style
      headers, `static_assert`, no VC6 branches or inline asm, `// 0xADDR` comments instead of the
      `MATCH_FUNC` markers, no `EXPORT`/`__stdcall`, ErrorLog on `FILE*`.
- [x] **No `-fpermissive`:** GCC 32/64-bit and Clang 64-bit build without it.
- [x] **Warnings on for `game/`.** `game/` builds with `-Wall -Wextra` (including `-Wsign-compare`) and
      0 warnings in GCC 32/64-bit and Clang 64-bit. Off on purpose: unused parameters, char
      subscripts, format truncation, unknown pragmas (`port/CMakeLists.txt` says why). Warning
      options for a game target go through `gta2_game_target(<target> <options>)`, which puts them
      ahead of its `-Wno-*` options: Clang's `-Wall` turns those back on when it comes later.
- [ ] **Refactoring to a maintainable code base:** the plan, its order, tasks and exit criteria are in
      `refactor/README.md` (one file per phase). Phase 0 (safety net) is mostly done: state hash,
      `gta2 --replay`/`--record`, `tools/replaytest.sh`, unit tests ([TESTING.md](TESTING.md)); what is left
      is in `refactor/0-safety-net.md`, first of all recording the corpus sessions.
      The six decisions are made (2026-10-16); `refactor/EXECUTOR.md` is the step-by-step runbook for
      finishing phases 0 to 4.
- [ ] **More modernising** (done: C++ casts, range-for, `= default`, constexpr limits, `ReplayState`, `CrusherStates`,
      `GameExitType`, and clang-tidy's `bugprone-*`/`performance-*` fixes that were mechanical: loop counters as wide
      as their bounds, `GTA2_COUNTOF` as a constexpr template, explicit `strcmp` compares, default-case in every
      switch, no reserved identifiers, trivial destructors, explicit casts for `T**` to `void*`). Left, none of them
      worth the risk of changing decompiled control flow: more `enum class` (`GameState`, `Gta2Error`, the Frontend
      and `gmp_*` enums), `std::array`, `bugprone-branch-clone` (70: merge case labels / `||` conditions),
      `bugprone-assignment-in-if-condition` (87), `modernize-use-std-print` (22), `modernize-use-std-numbers`
      (`fix16.cpp`'s pi literal: changes the constant), `bugprone-signed-char-misuse` (24: VC6's signed `char` is
      intended). Keep `typedef struct` blocks, `resource.h` and Miles' `3rdParty/mss.h` out of clang-tidy's
      `modernize-use-using` and `modernize-macro-to-enum`: they drop calling conventions and break the resource compiler.
- [x] **Run under AddressSanitizer and UndefinedBehaviorSanitizer.** Presets `asan64` and `ubsan64`; the
      unit tests run under both in CI, the replay corpus passes under both locally. What stays is listed
      in `refactor/known-ub.md`.
- [ ] **Floating-point.** VC6's x87 code and GCC's SSE differ in precision. Check that replays
      (`InputRecorder_67F8B0`) still play back the same. *The port's builds agree with each other
      (GCC/Clang, 32/64 bits: identical replay logs). Against the original, the attract replays (recorded
      by the 9.6 build) drift at updates 191 and 581; a replay recorded by 10.5.exe would tell whether
      the port and 10.5 compute the same.*
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
