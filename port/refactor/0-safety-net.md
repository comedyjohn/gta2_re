# Phase 0: Safety net

**Goal:** any change to what the simulation computes is detected automatically, on every commit, before
the refactoring itself starts. Today nothing exercises gameplay: `--selftest` checks the renderer,
`--audiotest` the mixer, `--simcheck` the maths tables and `nettest.sh` the network protocol. A refactor
of `Ped.cpp` could change how pedestrians behave and every check would stay green.

**Depends on:** nothing. **Blocks:** every other phase.

**Exit criteria**

- [ ] The golden corpus (task 0.5) replays to identical hash logs on GCC 32, GCC 64, Clang 64 and ARM64
      (qemu), run twice in a row on each. *So far: the three replays there are give identical logs on
      GCC 32 (twice), GCC 64, Clang 64, UBSan and ASan. Missing: the recorded sessions, ARM64 (no
      cross toolchain or qemu on the development machine yet).*
- [ ] A deliberate one-character behaviour change turns at least one replay red, and the report names
      the first differing frame and component: try `<` to `<=` in `CarAI_78::DetectCarAhead_44D1D0`, a
      changed constant in `Ped::Threat_Reaction_AI_465270`, and a skipped `rand` call in
      `Particle_8`. *So far: `<= 20` to `< 20` in DetectCarAhead (red at update 709, sprites and
      cars) and a skipped `rand` in `Particle_8::EmitBloodBurst_53E450` (update 166, rng first) are
      caught; `FindBestTargetPed_Mode5_466BD0(3)` to `(4)` in `Threat_Reaction_AI` is not: no replay
      has armed peds threatening the player yet (05 in the corpus table).*
- [x] Unit tests run in public CI (no game data needed), under ASan and UBSan (job
      `Linux_Native_Sanitizers` in `.github/workflows/ci.yml`).
- [ ] `port/tools/replaytest.sh` documented (in [`port/TESTING.md`](../TESTING.md), not RUNNING.md, which is
      for players) and run by the self-hosted runner (or locally, per decision 3 in
      [README.md](README.md)). *Documented; it runs locally until decision 3.*

## Status (2026-10-16)

Done: the state hash (tier 1 plus the police crews and pursuits), `gta2 --replay` and `gta2 --record`,
`replaytest.sh`, a corpus of three replays (`~/Games/gta2/corpus`), the unit tests and the sanitizer
presets and CI job. Replays reproduce: a session recorded live (real time, drawing) plays back headless
with exactly the live log on every build, drawing doesn't change the simulation, and every build logs
the same. The whole corpus runs in under 10 seconds per build.

What the safety net found before any refactoring, each fixed in a `Fix:` commit:

- `kFpEight_6FED48` was 0 in GCC builds (8 in Clang ones and the original): a static initialiser read
  another file's global before that file's initialiser had run. It changed how police crews pursue.
  `gDummyH_678584` (0 instead of 0.2) had the same cause, and the registry could be used before it was
  constructed. `port/tools/refactor/init_order.py` (in CI) keeps globals from being initialised from
  another file's.
- `PoliceCrew_38::field_35` was read without ever being set (in the original too); a roadblock's
  rectangle was freed with `delete[]`; tan(90) and tan(270) were converted to int out of range
  (undefined; ARM would have differed from x86). What UBSan reports and stays is in
  [known-ub.md](known-ub.md).

The attract replays were recorded by the 9.6 build, and drift from their recorded random numbers at
updates 191 and 581: their logs are the port's own, not a comparison with the original.

Left: recording the corpus sessions (0.5, needs a player), the network desync check (0.7), the text,
map and script VM unit tests (0.8), Valgrind (0.9, not installed), golden frames (0.10), the audio
event log (0.11), a null renderer so replays need no X server (0.3), tier 2 of the hash per subsystem.

## How the game runs a frame (what the safety net builds on)

- `RunGameFrame_4DA850` (`winmain.cpp`) runs the simulation (`ExecuteGame_4DA780` →
  `Game_0x40::ExecuteGame_4B9640` → `UpdateGame_4B9410`) on a wall-clock schedule:
  `GetFrameDurationMs_4B8BB0` is 33 ms (11 ms fast-forward; the network frame time in network games),
  and up to 8 updates run between draws. With the debug setting `skip_draw` (`bSkip_draw_67D4EA`) or
  `bForceGameUpdate_6F5880`, every loop iteration updates without waiting and never draws. That is the
  headless mode.
- `UpdateGame_4B9410` services, in a fixed order: players, collision queues, lights, cranes, crushers,
  generators, peds, public transport, garages, cars, (Mike), traffic lights, objects, the script,
  emergency crews, ambulances, police, particles, tile animations, the HUD, `rng::AdvanceCycle_48B900`,
  doors, fire engines, exploding scores.
- Replays (`InputRecorder_67F8B0`): a `ReplayHeader_10C` (map, style and script names, debug flags)
  followed by `InputRecord_C` entries, each "at RNG call count N, the input bits became X". Playback
  applies a record once `rng::get_cur_rng_41CFE0()` reaches its count, so it is independent of wall
  clock and frame timing. At most 40,000 records per file (`k_max_records`).
- Recording: debug setting `do_release_replay` writes `test\replay.rep` while playing. Playback: the
  `-r` command line switch or the debug setting `play_replay` reads `test\replay.rep`;
  `do_exit_after_replay` ends the game when the replay ends. `LoadReplayHeader_4CE380(1)` restores the
  debug flags stored in the header (`field_E4_flags`: skip police, skip audio, invulnerable, ...), so a
  replay carries its own settings.
- The registry (and so every debug setting) is the file `$XDG_CONFIG_HOME/gta2-native/registry.txt`
  (`platform/win32.cpp`).

## Tasks

### 0.1 Find out whether replays reproduce today

Nobody has checked since the port started. Do this first, because everything else rests on it.

- [x] Record a 2-minute session in the 32-bit build (`do_release_replay` = 1), on foot and in a car,
      with police. *Done with `gta2 --record` and synthetic key presses (on foot only); the attract
      replays have cars and police.*
- [x] Play it back twice with `-r`, with drawing (normal) and with `skip_draw` = 1. Watch whether the
      player ends in the same place. *The hash logs are identical.*
- [x] Play it back in the 64-bit build. *Identical, and with Clang once the static initialisation
      order bugs were fixed.*

If playback diverges, the cause is the first thing to fix (as a `Fix:` commit): the usual suspects are
reads of uninitialised memory (Valgrind finds them; `Fix16_Point` and `Fix16_Rect` locals are left
uninitialised by design), out-of-bounds reads (`get_input_bits_4CEAC0` reads
`field_8_input_masks[3 * idx + 8]`, past its 12 entries into the record buffer, as the original did),
the frame-timing-dependent order of draws and updates if a draw changes simulation state, and
`timeGetTime()` reaching the simulation (`Frontend`, `Network_20324`'s tick count, `Mike_A80`).

### 0.2 State hash

`port/platform/statehash.cpp` (port-only test code, like `simcheck.cpp`), with a small interface:

```cpp
// statehash.h
struct StateHashes
{
    uint64_t total;
    uint64_t rng, sprites, peds, cars, objects, players, script, police; // one per component
};
StateHashes port_state_hash(); // hashes the simulation as it is now
```

Rules that keep the hash stable across builds and runs:

- Hash **values**, never addresses. A pointer into a pool becomes its index in the pool's array
  (`p - pool->field_8_pool`), `nullptr` becomes -1. Pools are contiguous arrays, so this is exact.
- Hash fields one by one, never `sizeof(struct)` blocks: padding and uninitialised members differ
  between runs, and member layouts differ between 32 and 64 bits.
- Visit pools in **array order**, not through their used lists, so the hash doesn't depend on list
  order; hash the used-list order separately (as a sequence of indices), since update order is behaviour.
- The script data blocks (`ScriptManager_C1EA8::field_334C_script_data`, `field_13354_mission_script_data`)
  hold object references in `PTR32` slots (raw pointers at 32 bits, handles at 64 through
  `platform/ptr32.cpp`). They are the `PTR32` members of the command structures in `miss2_0x11C.hpp`
  (the ped, car, object, crane, light, door, crusher, generator, arrow, sprite and thread a command
  created). Walk the commands by their type (`field_2_type`) and hash those slots as pool indices,
  everything else byte by byte.
- FNV-1a 64 over little-endian bytes, as in `simcheck.cpp`'s `Hash`.

What to hash. Tier 1 is enough to start, because almost any behaviour change alters either an object's
position or the number of random numbers drawn:

| Component | Tier 1 (first commit) | Tier 2 (add as phases touch the subsystem) |
|---|---|---|
| rng | `gRng_6F6784.field_0_rng` (call count), `field_4_rnd` | |
| sprites | for every used `Sprite` in `gSpritePool_703818`: `field_14_xy`, `field_1C_zpos`, `field_0_rotation`, `field_30_sprite_type_enum`, `field_20_instance_id` | collision shape, remap |
| peds | used count of `gPedPool_6787B8`, per ped: id, health, objective, state, occupation, car index | weapons, group, threat state |
| cars | used count of `gCar_BC_Pool_67792C`, per car: model, damage, driver index, physics velocity and angular velocity | AI state, chase task, doors |
| objects | used counts of the object pools (`Object_2C`, `Object_3C`, `Object_8`), explosions, particles | per-object state |
| players | per player: ped index, score, multiplier, lives, wanted level, money | crime stats, bonuses |
| script | per thread in `miss2_0x11C_Pool_6F8064`: `field_0_cmd_this`, `field_4_next_cmd`, `field_8_cond_result`, `field_12_exec_flag`; the script data blocks (translated) | triggers, timers |
| police | crew count, pursuit targets, wanted levels | roadblocks |

### 0.3 Headless replay runner

`gta2 --replay <file.rep> [--frames N] [--hash-log out.txt] [--hash-every K] [--break-at FRAME]`:

- [x] Starts the game with a scratch registry (a temporary `XDG_CONFIG_HOME`), copies the replay to
      `test/replay.rep` in a temporary game directory that links to the real data, and sets
      `play_replay`, `do_exit_after_replay` and `skip_draw` (headless: no waiting, no drawing).
- [ ] Opens no window: a null renderer (`port_renderer_lookup` hands out no-op `gbh_*`/`Vid_*`
      functions) and `SDL_AUDIODRIVER=dummy`. Sound logic still runs, so audio code is covered.
- [x] After every `ExecuteGame_4DA780`, writes `frame rng_count total rng sprites peds cars objects
      players script police` (hex) to the hash log, every K frames (default 1).
- [x] `--break-at` stops at a frame and dumps the full tier-2 state as text, to diff two builds when
      a hash differs.
- [x] Exits with the replay's end (or after N frames) with status 0, or 2 if the game hit a fatal error.

Check: running the same replay twice gives byte-identical logs; a 10-minute replay takes under a
minute.

### 0.4 Is drawing simulation-neutral?

- [x] Run each replay headless and with drawing under Xvfb (`skip_draw` = 0), and compare the logs. They
      must match: if they don't, drawing changes simulation state (for example camera code that the
      simulation reads, or sprite visibility flags set while drawing) and the first refactor of the
      renderer would reveal it. Find and document the cause before going on; fixing it is a `Fix:`
      commit if it changes the original's behaviour.

### 0.5 Golden corpus

Recorded once from the build at the start of phase 1, stored outside the public repository (decision 3):
`$GTA2_CORPUS/replays/NN-name.rep` plus `$GTA2_CORPUS/expected/<build>/NN-name.log`.

| # | Session (5 to 10 minutes each) | Covers |
|---|---|---|
| 01 | Downtown, on foot, walk and run through crowds, punch peds | peds, collisions, `char.cpp` animations |
| 02 | Downtown, steal and drive cars, crash into traffic and walls | vehicles, physics, damage, traffic AI |
| 03 | Gang missions in each district (one replay per district: 03a, 03b, 03c) | script VM, phones, mission objects |
| 04 | Wanted level 1 to 6: police, roadblocks, SWAT, FBI, army | services, police AI, roadblocks |
| 05 | Weapons: every weapon, explosions, fire, rockets, car bombs | objects, weapons, explosions, particles |
| 06 | Ambulance, fire engine and police car jobs; kill frenzies | emergency services, script |
| 07 | Train, bus, tank, taxi; garages (respray, bomb, crane, crusher) | public transport, garages, cranes, crushers |
| 08 | Dying and being busted; respawn; lives running out | player state |
| 09 | Save at a save point, quit, load, continue playing | save games (`PlyDat_2BC0`, `GameSession_578`) |
| 10 | Bonus stage | bonus tracker, timers |
| 11 | The three attract replays (`data/attract/attr1-3.rep`) | ships with the game, free to use |
| 12 | 4-player network deathmatch (four replays recorded together, `nettest`-style, one machine) | lockstep, `NetPlay`, `TagGame_28` |

- [ ] Record them with real input (`gta2 --record`, see [TESTING.md](../TESTING.md)). The attract
      replays (11) are in, plus `00-synthetic-walk`.
- [ ] Check each with task 0.1's procedure (twice, 32 and 64, Clang).
- [x] Generate the expected logs with the runner (`replaytest.sh --update`). One expected log per
      replay, shared by every build: they all log the same.
- [x] Keep a `corpus.txt` manifest (name, length in frames, what it covers, SHA-256 of the `.rep`).

### 0.6 `port/tools/replaytest.sh`

- [x] `replaytest.sh <build-dir> [corpus-dir]`: runs every replay of the corpus in parallel, compares
      each log with the expected one, prints `PASS`/`FAIL name: first difference at frame F, component
      C`, exits 1 on any failure.
- [x] `replaytest.sh --update <build-dir>`: rewrites the expected logs, for `Fix:` commits only.
- [ ] Self-hosted runner (decision 3): a GitHub Actions job `port-replays`, `runs-on: [self-hosted,
      gta2-data]`, that builds `linux32` and `linux64` and runs the script.

### 0.7 Network desync check

The state hash doubles as a desync detector for real network games:

- [ ] Every 30 frames, each machine sends its `total` hash with its inputs (`NetPlay`'s per-frame
      message); a mismatch is logged with the frame and the component hashes. That turns a "the game
      went strange" report into a frame number.
- [ ] `nettest.sh` checks that all players report the same hashes.

### 0.8 Unit tests

No game data needed, so they run in public CI.

- [x] Add doctest (single header, `3rdParty/doctest/doctest.h`, pinned version) and `port/tests/` with a
      `gta2_tests` executable built against `gta2_game` (same prelude and options) and registered with
      `enable_testing()`/`add_test()`; `ctest --preset linux64` runs it.
- [ ] First tests, chosen because later phases change exactly this code. *Done: `test_fix16`,
      `test_tables` (the ang16 row, plus `--simcheck`'s hashes), `test_rng`, `test_pool`, `test_bitset`.
      Left: text, map reader, script VM.*

| Test file | Covers | Golden values from |
|---|---|---|
| `test_fix16.cpp` | `Fix16` `+ - * /`, rounding, `FromInt`/`ToInt`, overflow wrap, `sqrt` | the current implementation (characterisation) |
| `test_ang16.cpp` | sin/cos/tan tables, `PolarToCartesian_41FC20`, angle wrap | `--simcheck` |
| `test_rng.cpp` | `srand_4F7A40`, the first 10,000 values of `rand_4F7C00`, `get_int_4F7AE0`, `get_uint8_4F7B70` | the current implementation |
| `test_pool.cpp` | `Pool`/`PoolBasic`: allocate, deallocate, used-list order, `AllocateAppend` | current behaviour |
| `test_bitset.cpp` | `BitSet32` | |
| `test_text.cpp` | `text_0x14` on a GXT file built in the test (tiny writer in `port/tests/fixtures/`) | |
| `test_map_reader.cpp` | `Map_0x370` chunk loading on a 1-block synthetic map | |
| `test_script_vm.cpp` | a few opcodes (`SET`, `ADD`, `IF`, `GOTO`, `WAIT`) on hand-assembled bytecode | the original's behaviour |

### 0.9 Sanitizers and Valgrind

- [x] CMake presets `asan64`, `ubsan64` (`-fsanitize=undefined`; it recovers, so one run lists every
      report, and `UBSAN_OPTIONS=halt_on_error=1` stops at the first) next to
      the existing `build_port/asan64`.
- [x] Public CI: unit tests under ASan+UBSan.
- [ ] Self-hosted: the corpus under ASan+UBSan (`ASAN_OPTIONS=detect_leaks=0`; *done locally: the
      corpus passes under both, see [known-ub.md](known-ub.md)*), and weekly under
      Valgrind memcheck (uninitialised reads, which the hash would otherwise turn into flaky failures).
- [x] Every finding is a `Fix:` commit or an entry in `port/refactor/known-ub.md` with its reason (some
      undefined behaviour is the original's and is reproduced on purpose, like the input mask read above).

### 0.10 Golden frames (presentation)

Replays don't see drawing. Phase 2 to 6 touch the renderer front end, HUD and menus too.

- [ ] `--replay ... --screenshot-at F1,F2,... --screenshot-dir D` renders through the real OpenGL
      renderer under Xvfb (Mesa llvmpipe, deterministic) and writes PNGs.
- [ ] Ten reference frames per corpus replay plus the front-end menus (driven by recorded menu input),
      compared pixel-exact on the self-hosted runner (same Mesa version), with a diff image on failure.

### 0.11 Audio event log

- [ ] `--audio-log out.txt`: every sample start/stop/volume/pitch/position call the sound logic makes
      to `cSampleManager`, per frame. Hash it into the log as an extra component, so `sound_obj.cpp`
      (7,700 lines) is covered when it is refactored.

## Risks

| Risk | Mitigation |
|---|---|
| Replays don't reproduce at all today | Task 0.1 first; fix the non-determinism before anything else |
| The hash reads uninitialised or padded memory and fails at random | Field-by-field hashing, Valgrind run, run each replay twice |
| The hash misses a change | Tier 1 plus RNG call count catches nearly everything; the mutation check in the exit criteria proves it on purpose; extend to tier 2 per subsystem before refactoring it |
| Game data can't be in public CI | Unit tests and nettest stay public; replays on a self-hosted runner or locally |
| 32-bit and 64-bit logs differ legitimately (pointer-sized fields) | Hash indices instead of pointers; if they still differ, keep one expected log per build and investigate the difference (it is a 64-bit bug) |
