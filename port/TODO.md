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
   presets, `GTA2_BITS`). 64-bit builds and passes the self-tests; making it playable is below.

## Left to do

The order follows what a player notices first.

### Missing for players
- [ ] **Bink movies.** `Source/3rdParty/bink.cpp` is still stubs, and the `.bik` files are reported
      absent, so the intros are skipped. Plan: decode them with FFmpeg's Bink decoder.
- [ ] **3D sound.** Nothing implements `AIL_*_3D_*` yet, and `AIL_enumerate_3D_providers` offers no
      provider, so the options menu's 3D setting does nothing. Plan: OpenAL, or panning by position
      in `platform/mss.cpp`'s mixer.
- [ ] **Higher resolutions than 640x480.** The picture is scaled into the window at 4:3. The game's
      projection code would need to know the new size.
- [ ] **Multiplayer.** There is no DirectPlay replacement (the game finds no connections). Not
      scheduled.

### Polish
- [ ] **Radio hitch.** Streams are decoded whole when opened (`SDL_LoadWAV`). Decode in chunks to
      remove the hitch when the radio station changes.
- [ ] **ImGui debug overlay** (`ImGuiDebug.cpp`). It needs ImGui's OpenGL backend; until then
      `gbh_SetBeginSceneCB` ignores it.

### Code health
- [ ] **Compile without `-fpermissive`, and with warnings on for `Source/`.** Any fix in `Source/`
      must not change what VC6 generates: check it with `tools/check_matching.sh`, which must report
      "changed: 0".
- [ ] **Run under AddressSanitizer and UndefinedBehaviorSanitizer.**
- [ ] **Floating-point.** VC6's x87 code and GCC's SSE differ in precision. Check that replays
      (`InputRecorder_67F8B0`) still play back the same.
- [ ] **A playable 64-bit build.** `port/build.sh 64` builds, and its renderer and audio
      self-tests pass (CI runs both widths), but the game isn't playable at 64 bits yet:
      - **Pointers kept in 32-bit integers:** 141 casts GCC flags when compiling for 64 bits
        (`-Wpointer-to-int-cast -Wint-to-pointer-cast`): 80 in multiplayer (`NetPlay.cpp`,
        `Network_20324.cpp`, unused while DirectPlay is unavailable), 23 in `error.cpp`, and about 38
        in game code (`Car_BC.cpp` 12, `char.cpp` 6, `sound_obj.cpp` 3, then one or two each in
        `winmain`, `sprite.hpp`, `gtx_0x106C`, `Police_38`, `KanjiFont_8EC`, `Bink`, `text_0x14`,
        `registry`, `memory`, `Frontend`, `Draw`). The fix is `sptr` (`Source/types.hpp`): `s32` under
        MSVC, pointer-sized otherwise, so VC6 output is unchanged (`NetPlay`'s packet buffers are done
        this way). Check each change with `tools/check_matching.sh`.
      - **Structures with pointers that are read from or written to files** (save games, replays,
        map/style chunks) change layout at 64 bits. GCC can't flag these; they need reading.
      - **Structures whose asserted size counts 4-byte pointers** (skipped at 64 bits): `Camera_0xBC`,
        `CarInfo_808`, `Car_BC`, `Car_Door_10`, `Ped`, `Ped_List_4`, `ScriptStackFrame_C`,
        `ScriptStackFramePool_25C`, `SpritePool`, `miss2_0x11C`, `miss2_8`. Most only matter in
        memory, but any of them that reaches a file belongs to the previous point.
      - Then play-test, ideally side by side with the 32-bit build (replays help).

## Working rules (from the top-level CLAUDE.md)
- Work on a branch, never `master`. The current branch is `claude/nice-feynman-sw4r6m`.
- A port change to `Source/` must keep every `MATCH_FUNC` matching.
- Keep commits small, one milestone or one fix each.
