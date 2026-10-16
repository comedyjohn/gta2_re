# Refactoring `port/game/`: the plan

The game code in `port/game/` was forked from the byte-matching decompilation in `Source/`. It builds
cleanly (GCC and Clang, 32 and 64 bits, `-Wall -Wextra`, 0 warnings), but it is still decompiled code:
names carry addresses and struct sizes, state lives in 1,684 globals, objects point at each other through
raw pointers into fixed pools, functions run to 1,100 lines, and the game talks to an emulated Win32/DirectX.

This directory is the plan to turn it into code that would pass review in a well-run modern C++23 project,
**without changing what the game does**. It is written so that each task can be picked up cold: what to
change, how to do it, how to check it, and what is done when.

| File | Phase | What it delivers |
|---|---|---|
| [0-safety-net.md](0-safety-net.md) | 0 | State hash, headless replay runner, golden replays, unit tests, sanitizers |
| [1-tooling.md](1-tooling.md) | 1 | Formatting, strict warnings, clang-tidy ratchet, scaffolding removed, CI |
| [2-names.md](2-names.md) | 2 | Naming convention, rename tool, every address and size gone from names, subsystem directories |
| [3-types-and-data.md](3-types-and-data.md) | 3 | Conversions, arrays/spans, strings and text, unions, strong ids, file readers, error handling |
| [4-control-flow.md](4-control-flow.md) | 4 | Giant functions split, `goto` gone, state machines, dead code removed |
| [5-ownership.md](5-ownership.md) | 5 | One pool type with handles, RAII, no raw memory functions on objects |
| [6-architecture.md](6-architecture.md) | 6 | Layered libraries, `World` instead of globals, platform interfaces instead of Win32 |
| [7-concurrency.md](7-concurrency.md) | 7 | Render, audio and loading threads around a deterministic simulation |
| [8-documentation.md](8-documentation.md) | 8 | Architecture docs, interface docs, contributor guide |
| [EXECUTOR.md](EXECUTOR.md) | all | The runbook: task order, commands, checks, when to stop and ask |

Tools that measure and drive the work live in `port/tools/refactor/`:

- `metrics.py`: the numbers below, totals, per file (`--files`) or per subsystem (`--subsystems`).
- `subsystems.py`: which subsystem every file belongs to. The plan works through the code one subsystem
  at a time, and phase 6 turns each into a library.
- `strict_warnings.py`: how many warnings a stricter flag set would add, without building.

The phase files name further tools to write (`rename.py`, `address_map.py`, `tidy_ratchet.py`,
`replaytest.sh`, `coverage.sh`, `callgraph.py`, `deps.py`); each is a task of the phase that needs it.

## Goal: the bar

"Highest standard" has to be measurable, or it never ends. When every row holds, the refactor is done.

| Area | Target | Checked by | Phase |
|---|---|---|---|
| Behaviour | The golden replays give the same per-frame state hash on every build | replay runner | 0 |
| Formatting | `.clang-format` applied everywhere | CI `clang-format --dry-run -Werror` | 1 |
| Warnings | 0, as errors, with the flag set in [1-tooling.md](1-tooling.md) (GCC, Clang; MSVC `/W4 /permissive-` once it builds) | CI builds | 1, 3 |
| Static analysis | 0 findings from the checked-in `.clang-tidy` | CI clang-tidy ratchet | 1 to 6 |
| Naming | One convention, enforced; no address or struct size in any identifier | `readability-identifier-naming` | 2 |
| Size | No function over 150 lines or cognitive complexity 40; no file over 2,000 lines | `readability-function-size`, `-cognitive-complexity`, `metrics.py` | 4 |
| Memory safety | AddressSanitizer, UBSan and ThreadSanitizer clean on the replays; every file loader fuzzed | sanitizer and fuzz jobs | 0, 3, 5, 7 |
| Ownership | No naked `new`/`delete`/`malloc`/`memcpy` on objects outside the allocators | `cppcoreguidelines-owning-memory`, `-no-malloc` | 5 |
| Layering | core, data, simulation, presentation and platform are separate libraries, dependencies point down only; the simulation runs headless | CMake target graph | 6 |
| Globals | Fewer than 20 mutable globals, each documented | `cppcoreguidelines-avoid-non-const-global-variables` | 6 |
| Portability | No Win32 types or headers in `game/`; builds and passes tests on Linux, Windows and macOS, x86-64 and ARM64 | CI matrix | 6 |
| Tests | Unit tests for core and data; replay tests for the simulation; golden frames for presentation | `ctest` | 0 onwards |
| Docs | Every public interface documented; architecture and style guides current | review | 8 |

## Where it stands (2026-10-16)

From `metrics.py`, `clang-tidy` 18 and a `-fsyntax-only` pass with the strict warning set.

| Measure | Now |
|---|---|
| Lines in `game/` | 160,700 in 265 files (137,400 `.cpp`, 23,300 headers) |
| Functions | 3,574; 439 over 60 lines, 222 over 100, 120 over 150, 72 over 200 |
| Largest functions | `ImGuiDebugDraw` 2,274 lines, `FatalError_4A07C0` 1,532, `CarAI_78::UpdateStateMachine_44E560` 1,114, `Char_B4::UpdateAnimState_546360` 964, `miss2_0x11C::PreExecOpCode_5108D0` 733 |
| Largest files | `Ped.cpp` 12,128, `Car_BC.cpp` 7,973, `miss2_0x11C.cpp` 7,869, `sound_obj.cpp` 7,703, `char.cpp` 6,885 |
| Distinct identifiers carrying an address (`ExitGame_4B8BD0`) | about 4,100; 118 of them `sub_XXXXXX` |
| Distinct `field_XX_...` member names | about 2,800 (107 with no name at all) |
| Types named after their size (`Car_BC`, `Game_0x40`) | 223 of 449 classes and structs; 20 names collide once the size is dropped |
| Global definitions | 1,684: 490 constants (`k...`), 450 raw data (`byte_`/`dword_`...), 474 objects and pointers (`g...`), 91 debug flags (`b...`), 179 other |
| Pools | 24 instantiations of `Pool`/`PoolBasic`, intrusive free lists, raw pointers between objects |
| `new` / `delete` / `malloc`-family | 179 / 73 / 50 |
| `memcpy`/`memset`/`memmove` | 218 |
| `sprintf`/`strcpy`/`strcat`/`swprintf`... | 686 (`swprintf` alone 149) |
| `wchar_t` (2 bytes via `-fshort-wchar`) | 266 uses |
| `goto` | 224 (147 in `Ped.cpp`) |
| `union` | 28 |
| Win32 references / files including `windows.h` | 415 / 34 (`fix16.hpp` among them) |
| Struct-over-file reads (`Global_Read_4A71C0`) | 78 |
| Duplicate inline/out-of-line copies kept for VC6 inlining (`*_inline_*`, `*_ool`) | 32 |
| `TODO`/`FIXME` | 253 |
| clang-tidy findings (bugprone, cert, cppcoreguidelines, misc, modernize, performance, readability) | 37,189 unique; the breakdown and the phase that clears each check are in [1-tooling.md](1-tooling.md) |
| Strict-warning findings (`-Wconversion -Wshadow -Wold-style-cast` ...) | 3,839; breakdown in [1-tooling.md](1-tooling.md) |
| Files not matching `.clang-format` | 182 of 265 (about 10,100 lines would change) |
| Tests | `--selftest` (renderer), `--audiotest`, `--simcheck` (maths tables), `tools/nettest.sh` (lockstep, no game data). **Nothing runs gameplay.** *Since phase 0 started: replay tests (`tools/replaytest.sh`, 3 replays so far) and unit tests (17 cases); see [0-safety-net.md](0-safety-net.md)* |

## Ground rules

1. **Behaviour does not change during refactoring.** The game is deterministic: single player steps a
   fixed 33 ms (11 ms fast-forward) per update, replays sync inputs by RNG call count, and network
   games run in lockstep. So "the same" can be checked exactly, as the same state hash every frame for
   the same inputs. A refactor commit that changes a golden hash is wrong, even if the game looks fine.
2. **Bug fixes are separate commits** with `Fix:` in the subject, the golden hashes re-recorded in the
   same commit, and the reason in the message. Never mix a fix into a refactor.
3. **One kind of change per commit**, and one subsystem at a time. Mechanical changes are made by a tool
   (clang-tidy `-fix`, the rename tool, a script kept in `port/tools/refactor/`), never by hand across
   many files. The commit message names the tool and its arguments, so the change can be replayed.
4. **Every commit builds and passes**: GCC 32 and 64, Clang 64, the unit tests, `--selftest`,
   `--audiotest`, `nettest.sh`, and (where the game data is) the replay corpus.
5. **Keep the link to the original.** Every function keeps its `// 0xADDR` comment, and the generated
   `port/tools/refactor/address_map.csv` maps each original address to its old and new name, so
   `Source/`, Ghidra, the 9.6f build and decomp.me stay usable for research.
6. **Phase order matters.** Don't start a phase before the phases it depends on have met their exit
   criteria. Within a phase, work through the subsystems in the order below.
7. **Update the plan.** Tick tasks off in the phase file in the same commit that finishes them, and
   keep "Where it stands" current by re-running `metrics.py`.

## Order

```
0 safety net ──► 1 tooling ──► 2 names ──► 3 types and data ──► 5 ownership ──┐
                                   │                                          ├──► 6 architecture ──► 7 concurrency
                                   └────────► 4 control flow ─────────────────┘
8 documentation: alongside every phase
```

| Phase | Depends on | Size | Commits (estimate) | Main risk |
|---|---|---|---|---|
| 0 Safety net | none | M | 25 | A hash over pointers or uninitialised memory gives false failures |
| 1 Tooling | 0 | M | 40 | The format commit conflicts with open branches |
| 2 Names | 0, 1 | XL, mostly mechanical | 300 | A rename through a macro or an overload set changes the wrong symbol |
| 3 Types and data | 2 | L | 150 | `-Wconversion` fixes that change values; a reader that differs from the struct layout it replaces |
| 4 Control flow | 0, 2 | XL, manual | 250 | Changed evaluation order or early exits |
| 5 Ownership | 3 | L | 100 | Pool iteration order or reuse of stale state changes |
| 6 Architecture | 4, 5 | XL | 250 | Initialisation order of former globals; the platform rewrite |
| 7 Concurrency | 6 | L | 60 | Data races and non-determinism |
| 8 Documentation | alongside | M | 40 | Falling out of date |

S is a few sessions, M about a week of sessions, L several weeks, XL the bulk of the work. These are
rough guesses from the measured volume; the real pace depends mostly on phase 2's name research and
phase 4's coverage gaps, which can't be measured before phase 0 exists.

**How much of each phase is mechanical** (done by a tool and checked by the replay hashes) and how much
needs judgement:

| Phase | Mechanical share | What a tool does | What needs judgement |
|---|---|---|---|
| 0 Safety net | ~0% | nothing | All of it, but it's small. The reference replays must be recorded by playing |
| 1 Tooling | ~80% | Formatting (one command, 10,100 lines); clang-tidy `-fix` for include cleanup (2,886), literal suffixes (2,598), redundant `inline` (545), braces; most of the 850 C casts | Scaffolding removal, merging 32 duplicate inline copies, CI and ratchet setup, 46 `-Wshadow` renames |
| 2 Names | ~90% of edits, ~50% of time | `rename.py` renames about 4,100 functions, 2,800 members, 223 types, constants and enumerators by rule | Researching names: 20 colliding type names, 118 `sub_` functions, 107 unnamed fields, about 450 raw data globals, 91 debug flags |
| 3 Types and data | ~50% of edits, ~25% of time | `-fix` for implicit bool conversions (7,243), `const` (1,469 + 733), member initialisers (832) | About 2,600 conversion warnings (widen, cast or bug), 686 C string calls, readers for 10 file formats, unions, id types |
| 4 Control flow | ~5% | `else-after-return`, boolean simplification (~430) | Splitting 120 functions over 150 lines, state machines, the script VM, 224 `goto`s, dead code, file splits; each function needs replay coverage first. The riskiest phase |
| 5 Ownership | ~20% | Moving the 24 pools to one template, once written | About 160 pointer members to handles with every use site checked; 218 `memcpy`/`memset` calls; stale-reference decisions |
| 6 Architecture | ~25% | Step 1 of the globals (`gPedManager_6787BC->` to `gWorld->peds().`) | Explicit dependencies, 107 upward include edges, 11 platform interfaces replacing Win32/DirectX (each play-tested), encapsulation |
| 7 Concurrency | ~0% | nothing | Snapshot, render, audio and loader threads; ThreadSanitizer |
| 8 Documentation | ~0% | Doxygen setup | Writing, alongside every phase |

Human input is needed for recording the replays (phase 0), the decisions below (before phase 2) and
play-testing each platform interface (phase 6). Everything else is checked by the replay hashes, so once
phase 0 exists the mechanical parts can run unattended. Phase 4 needs only phases 0 and 2, so it can
overlap phases 3 and 5.

**Subsystem order** (phases 2 to 5), smallest and best understood first, each building on the ones
before it: core, data, world, sprites, objects, vehicles, peds, services, script, game, audio,
presentation, network, app. `subsystems.py` lists the files of each.

| Subsystem | Lines | Types | Functions with addresses | `field_` names | Globals |
|---|---|---|---|---|---|
| core | 5,063 | 17 | 157 | 18 | 20 |
| data | 5,659 | 37 | 208 | 141 | 120 |
| world | 9,382 | 32 | 288 | 218 | 107 |
| sprites | 4,794 | 16 | 278 | 77 | 59 |
| objects | 16,760 | 28 | 620 | 363 | 252 |
| vehicles | 22,938 | 28 | 957 | 329 | 342 |
| peds | 23,851 | 18 | 738 | 235 | 227 |
| services | 7,584 | 15 | 248 | 203 | 55 |
| script | 11,225 | 116 | 604 | 428 | 19 |
| game | 10,865 | 24 | 650 | 289 | 98 |
| audio | 10,359 | 15 | 318 | 240 | 67 |
| presentation | 21,611 | 59 | 656 | 378 | 238 |
| network | 5,530 | 27 | 135 | 159 | 7 |
| app | 4,437 | 1 | 298 | 0 | 71 |
| scaffolding (deleted in phase 1) | 631 | 3 | 79 | 0 | 0 |

(Function and field counts are per subsystem and overlap where a subsystem calls into another.)

## Decisions (made 2026-10-16)

The user adopted all six proposals; [EXECUTOR.md](EXECUTOR.md) §0 lists them as the executor applies
them.

1. **Naming convention:** Google C++ style naming (`PascalCase` types and functions, `snake_case`
   variables, `snake_case_` members, `kPascalCase` constants and enumerators, `snake_case` files). See
   [2-names.md](2-names.md).
2. **Address suffixes:** dropped from identifiers; each stays as a `// 0xADDR` comment and in
   `address_map.csv`.
3. **Golden replays:** run locally (`port/tools/replaytest.sh`) and on a self-hosted CI runner with the
   game data, the runner being the gate once it exists.
4. **File compatibility:** saves, replays, player data and mission scripts of the original game stay
   read-compatible forever; the port may write a new format for its own saves.
5. **Text encoding:** GXT text decoded to UTF-8 at load, `std::string`/`std::format` everywhere, code
   points decoded only when drawing. See [3-types-and-data.md](3-types-and-data.md).
6. **Exceptions:** none in the simulation (`std::expected` for fallible operations, `FatalError` for
   broken invariants); allowed at load time and in tools.

## Glossary

- **State hash**: a 64-bit hash of the simulation state after a frame, made from values (positions,
  counters, indices), never from addresses. See [0-safety-net.md](0-safety-net.md).
- **Golden replay**: a recorded play session (`.rep`) together with the hash log the reference build
  produced from it.
- **Hash-neutral**: a change after which every golden replay still produces identical hash logs.
- **Ratchet**: a CI check that compares a count (warnings, tidy findings) per file against a checked-in
  baseline, fails when it goes up, and asks for the baseline to be lowered when it goes down.
