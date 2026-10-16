# Refactoring plan: `port/game/` to a modern, maintainable C++23 code base

This plan takes the forked game code (`port/game/`, about 160,000 lines) from "decompiled code that
builds cleanly" to code that would pass review in a well-run modern C++ project. `TODO.md` tracks what is
done; this file says what to do and in which order. Tick phases off here, and keep `TODO.md`'s "Code health"
section pointing at the current phase.

## The bar

"Highest standard" has to be measurable, or it never ends. Each line below is checked by a tool in CI.

| Area | Target | Checked by |
|---|---|---|
| Behaviour | The golden replays give the same per-frame state hash on every build | replay runner (phase 0) |
| Warnings | 0 with `-Wall -Wextra -Wpedantic -Wshadow -Wconversion` (GCC, Clang) and `/W4 /permissive-` (MSVC), as errors | CI builds |
| Static analysis | 0 findings from the checked-in `.clang-tidy` (bugprone, cert, cppcoreguidelines, modernize, performance, readability) | CI clang-tidy |
| Formatting | `.clang-format` applied everywhere | CI `clang-format --dry-run -Werror` |
| Memory safety | ASan, UBSan and TSan clean on the replay corpus; file loaders fuzzed | sanitizer and fuzz jobs |
| Ownership | No naked `new`/`delete`/`malloc` outside the allocators | `cppcoreguidelines-owning-memory` |
| Naming | One convention, enforced; no address or size in any identifier | `readability-identifier-naming` |
| Size | No function over 150 lines or cognitive complexity 40; files under 2,000 lines | `readability-function-size`, `-cognitive-complexity` |
| Layering | core, data, simulation, presentation and platform are separate libraries; the simulation builds and runs headless | CMake target dependencies |
| Portability | No Win32 types or headers in `game/`; builds on Linux, Windows and macOS, x86-64 and ARM64 | CI matrix |
| Tests | Unit tests for core and data; replay tests for the simulation | `ctest` |

## Where it stands (2026-10-16)

| Measure | Now |
|---|---|
| Lines in `game/` | 137,000 `.cpp` + 23,000 `.hpp`, 265 files |
| Identifiers that carry an address (`Foo_4A71C0`) | 4,115 in headers |
| `field_XX_` member names / unnamed `field_XX` | 2,807 / 107 |
| `sub_XXXXXX` functions | 118 |
| Globals (`g..._ADDR`) / raw data (`byte_`, `dword_` ...) | 607 / 456 (868 definitions) |
| Classes and structs | 682, named after their size (`Car_BC`, `Game_0x40`) |
| `new` / `delete` / `malloc`-`free` | 171 / 65 / 47 |
| `memcpy`/`memset` | 218 |
| `sprintf`/`strcpy`/`strcat` family | 683 |
| `goto` | 224 |
| Files that include `windows.h` / Win32 references | 33 / 154 |
| Struct-over-file reads (`Global_Read_4A71C0`) | 77 |
| Largest files | `Ped.cpp` 12,100 lines, `Car_BC.cpp` 8,000, `miss2_0x11C.cpp` 7,900, `sound_obj.cpp` 7,700 |
| Largest functions | `CarAI_78::UpdateStateMachine_44E560` 1,100 lines, `Char_B4::UpdateAnimState_546360` 960, `miss2_0x11C::PreExecOpCode_5108D0` 730 |
| `TODO`/`FIXME` | 252 |
| Tests | `--selftest` (renderer), `--audiotest`, `--simcheck` (maths tables), `tools/nettest.sh` (lockstep without game data). Nothing exercises gameplay. |

Done already: `-Wall -Wextra -Wsign-compare` clean, clang-tidy's mechanical modernize/bugprone/performance
fixes, C++ casts, `nullptr`, `using`, the first `enum class`es.

## Ground rules

1. **Behaviour does not change during refactoring.** The game is deterministic (lockstep multiplayer,
   replays), so "the same" can be checked exactly: same inputs, same state hash every frame. A refactor
   commit that changes a hash is wrong, even if the game looks fine.
2. **Bug fixes are separate commits**, marked as such, with the golden hashes re-recorded in the same
   commit and the reason given.
3. **One kind of change per commit.** Do mechanical changes with tools (clang-tidy `-fix`, clangd
   rename, scripts kept in `port/tools/refactor/`), never by hand across many files. Review the diff.
4. **Keep the link to the original.** Every function keeps its `// 0xADDR` comment, and
   `port/tools/refactor/address_map.csv` (generated from those comments) maps old names to new ones, so
   `Source/`, Ghidra and the 9.6f build stay usable for research.
5. **Phase order matters.** Safety net, then tooling, names, types, control flow, ownership and
   architecture. Each phase makes the next one cheaper and safer. Don't start a phase before the one it
   depends on has met its exit criteria.

## Phase 0: Safety net (blocks everything else)

Without this, every later phase is guesswork on 160,000 lines.

1. **State hash.** `StateHash` walks the simulation state each frame: pools (peds, cars, objects,
   sprites), players, the mission script VM, the map's dynamic state, the RNG and the frame counter. It
   hashes values, not addresses, so 32-bit and 64-bit builds agree. Add it to `--simcheck`'s
   `Hash`.
2. **Headless replay runner.** `gta2 --replay <file.rep> --frames N --hash-log out.txt` plays a
   recording (the `InputRecorder_67F8B0` playback path) with no window and no audio device (null
   renderer and mixer), and writes `frame hash` lines.
3. **Golden corpus.** About 20 recordings of 5 to 10 minutes each: every district, on-foot and driving,
   shooting, police levels 1 to 6, gang missions, the train and tank, dying and wasted, a 4-player
   network match (`nettest`), and a save/load round trip. The `.rep` files and expected hash logs live
   outside the public repository, since they need the game data.
4. **Where it runs.** Locally with `port/tools/replaytest.sh`, and on a self-hosted CI runner that has
   the game data. Public CI runs only what needs no game data.
5. **Unit tests** (doctest or Catch2 via `ctest`), runnable without game data: `Fix16`/`Ang16` maths
   against the tables, the pools, the script VM's opcodes on hand-written bytecode, the file readers on
   tiny generated files, `rand()`'s sequence.
6. **Sanitizer jobs.** ASan+UBSan for the unit tests in public CI; the replays under ASan+UBSan on the
   private runner.

**Exit:** the corpus replays to identical hashes on GCC 32/64, Clang 64 and ARM64; a deliberate
one-character behaviour change (`<` to `<=` in `CarAI_78`) turns at least one replay red.

## Phase 1: Tooling baseline

1. Check in `.clang-format` (based on the current style: Allman braces, 4 spaces, 140 columns). Format
   `game/` and `platform/` in one commit and list it in `.git-blame-ignore-revs`.
2. Check in `.clang-tidy` with the target check set. Record the current findings as a baseline file;
   CI fails only on new findings in changed lines, so the baseline can only shrink.
3. Warnings as errors in CI. Add `-Wshadow` and `-Wpedantic` now. `-Wconversion` comes in phase 3.
4. An MSVC build of the port (`/W4 /permissive- /std:c++latest`) in CI, which the Windows target needs
   anyway.
5. `include-what-you-use` once, to cut the include web. That speeds up builds, and every later phase
   rebuilds a lot.

**Exit:** CI enforces format, warnings and "no new tidy findings" on every push.

## Phase 2: Names

This is the biggest single change for readability, and it is mostly mechanical once the convention is
fixed.

1. **Write `port/STYLE.md`:** the naming convention (proposal: `PascalCase` types and functions,
   `camelCase` locals and parameters, `m_camelCase` members, `kPascalCase` constants, `snake_case` files),
   plus the rules for this repository (address comments, ownership, error handling).
2. **Classes and files.** Drop size suffixes: `Car_BC` to `Car`, `Game_0x40` to `Game`, `Map_0x370` to
   `Map`, `sound_obj` to `SoundManager`, `miss2_0x11C` to `MissionScript`. Rename the files to match.
   About 680 types, one subsystem per commit.
3. **Functions.** Strip the address suffix from 4,115 identifiers (`ExitGame_4B8BD0` to `ExitGame`,
   keeping `// 0x4B8BD0`). Name the 118 `sub_` functions from what they do (Ghidra, 9.6f symbols,
   call sites). Use clangd's rename driven by a script, so overloads and macros are handled.
4. **Members.** `field_2C_game_exit_type` to `m_exitType`. Name the 107 unnamed `field_XX`. A member
   that can't be understood yet gets a descriptive provisional name plus a comment, never a bare offset.
5. **Globals.** `gGame_0x40_67E008` to `gGame` for now (phase 6 removes most of them). Name the 456 raw
   `byte_`/`dword_` globals, or turn them into locals when they are only used inside one function
   (common in decompiled code).
6. **Magic numbers** to named constants and `enum class` (the remaining plain enums: `GameState`,
   `Gta2Error`, the Frontend enums, `gmp_*`, `SCRCMD`).

**Exit:** `readability-identifier-naming` passes; no identifier contains an address or a size;
`address_map.csv` is generated and complete.

## Phase 3: Types and data

1. **`std::array`, `std::span`, `std::string_view`** instead of C arrays, pointer+count pairs and
   `const char*`. Replace the 683 `sprintf`/`strcpy`/`strcat` calls with `std::format`/`std::format_to_n`
   and `std::string` or fixed-capacity strings. Game text (GXT) becomes `std::u16string`.
2. **Strong types for ids and units.** `PedId`, `CarId`, `GangId`, `TileCoord` and so on, instead of
   `s32`/`u8`/`u16`, so a mixed-up argument stops compiling. `Fix16` and `Ang16` already work like this.
3. **`-Wconversion` clean.** Make every narrowing explicit, in one commit per subsystem. This is the
   biggest source of silent 32/64-bit and signedness bugs.
4. **Unions** (25), such as `field_C_pAny`, become `std::variant` or a tagged type with checked
   accessors.
5. **Explicit file readers.** A `BinaryReader` (bounds-checked, little-endian, error-returning)
   replaces the 77 struct-over-file reads and the 11 file-layout structs (map, style, text, script,
   saves, replays). Fuzz each loader with libFuzzer. Explicit readers also make big-endian hosts work
   and remove the last 64-bit pointer workarounds (`PTR32`, the 64-bit loader branches).
6. **`const`, `[[nodiscard]]`, `noexcept`** on interfaces. Use `std::expected<T, Error>` for operations
   that can fail (loading, network); `FatalError` stays only for real invariant violations.

**Exit:** `-Wconversion` is on and clean; no C arrays in interfaces; every loader is fuzzed for an hour
without a finding.

## Phase 4: Control flow and function size

Every change here can alter behaviour in subtle ways (evaluation order, early returns), so it relies on
phase 0's replays.

1. Split the giant functions. State machines (`CarAI_78::UpdateStateMachine`, `Char_B4::UpdateAnimState`,
   `Ped`'s state handling) become one function per state, dispatched by `switch` or a table. The script
   VM's `PreExecOpCode` becomes one handler per opcode.
   `FatalError_4A07C0`'s 1,500-line `switch` over error codes becomes a table, and the 2,300-line
   `ImGuiDebugDraw` becomes one function per debug window.
2. Remove the 224 `goto`s.
3. Fix the clang-tidy findings deferred from the mechanical pass: `bugprone-branch-clone` (70),
   `bugprone-assignment-in-if-condition` (87), `modernize-use-std-print` (22).
4. Delete dead code (functions no caller reaches, debug-only paths from the original); triage the 252
   `TODO`s into issues or fixes.
5. Split files over 2,000 lines along the new function boundaries (`Ped.cpp` into movement, combat,
   AI, animation, ...).

**Exit:** function size and complexity limits pass, apart from a short, justified exception list.

## Phase 5: Ownership and memory

1. **One pool type.** The per-class pools (`Char_Pool`, `Object_2C_Pool`, `Car` pools, the free-list
   template) become one `ObjectPool<T, Capacity>` that owns its objects and hands out **generational
   handles**. Objects that refer to each other by raw pointer (a ped's car, a car's driver, a police
   crew's target) store handles, so a stale reference is detected instead of read as a dangling pointer.
   This is the root cause of a whole class of crashes in the original.
2. **RAII everywhere:** `std::unique_ptr` and containers instead of the 171 `new`s, 65 `delete`s and 47
   `malloc`/`free`s; file, texture and sound handles in owning wrappers.
3. **No `memcpy`/`memset` on non-trivial types.** Use value-initialisation, copy constructors or
   `std::fill` (the `Ped` clone, the 218 calls).

**Exit:** `cppcoreguidelines-owning-memory` and `-no-malloc` pass; the replay corpus is ASan clean.

## Phase 6: Architecture

1. **Layers as libraries:** `gta2_core` (maths, fixed point, containers, RNG), `gta2_data` (file
   formats), `gta2_sim` (map, peds, cars, AI, physics, weapons, script, players), `gta2_present`
   (renderer front end, HUD, frontend menus, audio logic), `gta2_platform` (SDL, net, file system).
   CMake dependencies only point downwards, so a cycle is a link error. The simulation must not reach
   presentation: the HUD and sound read simulation state, never the other way round. Events (for
   example "explosion at x,y") go through a queue.
2. **Globals out.** 868 global definitions become:
   - constants and tables: `constexpr`/`const` data in their module;
   - per-game state: members of a `World` (map, pools, players, script, police, gangs) owned by a
     `GameSession`;
   - scratch values the decompiler made global: locals.
   Do it in two steps. First, move each global into `World` and reach it through one accessor (a
   mechanical, hash-neutral change). Then pass `World&` or the subsystem actually needed explicitly,
   one subsystem at a time.
3. **No Win32 in `game/`.** The 33 files that include `windows.h` and the 154 Win32 references move
   behind a platform interface (window, input, audio, file system, timing, network). After that,
   `port/include/` (the fake `windows.h`, `ddraw.h`, `dinput.h`, ...) and `platform/win32.cpp` can go.
   SDL3 behind that interface.
4. **Subsystem interfaces.** Each subsystem gets a small public header and keeps its details private
   (pimpl or internal headers). Today any file can include any class.

**Exit:** `gta2_sim` builds and runs the replay corpus with no presentation or platform library linked;
`port/include/` is gone; fewer than 20 globals remain, each one documented.

## Phase 7: Concurrency and performance

This is the end goal from the start of the fork. It is only safe once phase 6 has separated the
simulation from presentation.

1. The simulation stays single-threaded and deterministic (lockstep, replays). It publishes an
   immutable frame snapshot.
2. Rendering, audio mixing and asset streaming run on their own threads and read snapshots.
3. ThreadSanitizer job on the replay corpus.
4. Profile before and after (frame time distribution on a fixed replay), and keep that as a CI
   benchmark so regressions show.

**Exit:** TSan clean; frame time no worse than before on the benchmark replay.

## Phase 8: Documentation

Doxygen comments on every public interface, `port/ARCHITECTURE.md` (layers, data flow, frame loop,
determinism rules), `STYLE.md`, and a contributor guide that says how to run the replay tests.

## Order and size

| Phase | Depends on | Size | Main risk |
|---|---|---|---|
| 0 Safety net | none | M | Hashing pointers instead of values gives false failures |
| 1 Tooling | 0 | S | The format commit conflicts with open branches |
| 2 Names | 0, 1 | XL (mechanical) | A rename through a macro changes the wrong symbol (the build catches most of it) |
| 3 Types and data | 2 | L | `-Wconversion` fixes that change values; file readers that differ from the struct layout |
| 4 Control flow | 0, 2 | XL (manual) | Changed evaluation order or early exits (the replays catch them) |
| 5 Ownership | 3 | L | Handle conversion changes iteration order in pools |
| 6 Architecture | 4, 5 | XL | Initialisation order of former globals |
| 7 Concurrency | 6 | L | Data races (TSan) and non-determinism (replays) |
| 8 Docs | runs along with all phases | M | Falling out of date |

S is a few sessions, M about a week of sessions, L several weeks, XL the bulk of the work. Phases 2 and
4 can be done one subsystem at a time. Work through the subsystems in this order, each smallest and
best-understood first: core maths, file formats, map, sprites, objects, weapons, cars and physics, peds
and AI, police and gangs, mission script, HUD and frontend, audio.

## Decisions needed before starting

1. **Naming convention.** The proposal is in phase 2. It touches every line, so it is worth agreeing on
   once.
2. **Address suffixes.** The proposal drops them from identifiers and keeps them in comments plus
   `address_map.csv`. That makes porting a late fix from `Source/` a lookup instead of a text search.
   `Source/` still has 28 WIP or stub functions, so port those first if their fixes are wanted in
   `game/`.
3. **Golden replays and game data.** They can't go into the public repository. Options: run them only
   locally, or on a self-hosted CI runner that has the data.
4. **File compatibility.** Should saves, replays and mission scripts from the original game keep
   loading? (The proposal is yes, which phase 3's readers must preserve.) Network compatibility is
   already port-only.
5. **Multiplayer across versions.** Every behaviour fix changes `GTA2_GAME_HASH`, so players must run
   the same build. That is already the rule; with frequent fixes it will be visible to players.
