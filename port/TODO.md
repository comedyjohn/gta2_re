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
- [ ] **64-bit build.** Game structures hold 32-bit pointers and their sizes are asserted, so this
      is a project of its own.

## Working rules (from the top-level CLAUDE.md)
- Work on a branch, never `master`. The current branch is `claude/nice-feynman-sw4r6m`.
- A port change to `Source/` must keep every `MATCH_FUNC` matching.
- Keep commits small, one milestone or one fix each.
