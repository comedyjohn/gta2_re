# Phase 3: Types and data

**Goal:** the type system carries the meaning: no silent conversions, no C arrays or pointer-and-count
pairs in interfaces, strings that can't overflow, unions that can't be read as the wrong member, ids that
can't be mixed up, and file formats read by explicit, bounds-checked, fuzzed readers instead of laying
structs over bytes.

**Depends on:** phase 2 (the names make the intent visible). **Blocks:** phase 5.

**Exit criteria**

- [ ] `-Wconversion -Wsign-conversion -Wdouble-promotion` on and clean; `-Wno-narrowing`,
      `-Wno-invalid-offsetof`, `-Wno-char-subscripts` gone.
- [ ] 0 findings for: `readability-implicit-bool-conversion`, `bugprone-narrowing-conversions`,
      `cppcoreguidelines-pro-bounds-array-to-pointer-decay`, `-pro-bounds-constant-array-index`,
      `-pro-type-union-access`, `-pro-type-vararg`, `-init-variables`, `-pro-type-member-init`,
      `modernize-avoid-c-arrays` (outside `data/` layouts), `misc-const-correctness`,
      `readability-make-member-function-const`, `cert-err33-c`, `modernize-use-nodiscard`.
- [ ] No `sprintf`/`strcpy`/`strcat`/`swprintf`/`wcscpy` family in `game/`.
- [ ] `-fshort-wchar` and `port/include/port_wchar.h` gone (decision 5).
- [ ] Every file format is read through `gta2_data`'s readers; every reader has unit tests and has been
      fuzzed for an hour without a finding; `GTA2_ASSERT_FILE_SIZEOF` and `#pragma pack` gone.
- [ ] Every commit hash-neutral, except `Fix:` commits.

## 3.1 Conversions

3,839 strict-warning findings (see [1-tooling.md](1-tooling.md)); the conversion ones are the bulk:
`-Wsign-conversion` 1,726, `-Wimplicit-int-conversion` 748, `-Wshorten-64-to-32` 58,
`-Wimplicit-int-float-conversion` 50, `-Wdouble-promotion` 44, plus clang-tidy's
`bugprone-narrowing-conversions` 1,528 and `bugprone-implicit-widening-of-multiplication-result` 17.

Per subsystem, one warning kind per commit. For each finding, decide which of three cases it is:

| Case | Fix | Example |
|---|---|---|
| The narrow type is a decompilation artefact (VC6 kept a value in a byte register) | Widen the variable or parameter | a `u8` loop counter, a `s16` index into a 500-entry pool |
| The truncation or sign change is the original's behaviour | `static_cast<>` that states it, with a comment if surprising | storing a `s32` score into a `u16` HUD digit, wrapping an angle |
| A member that is laid over file data | Leave it until 3.7 replaces the layout | `gmp_block_info` fields |

Widening changes behaviour when the value can overflow the old type (a counter that wrapped at 255
now doesn't). The replays catch it; when they do, it is a decision: keep the wrap explicitly
(`static_cast<u8>(x + 1)`), or `Fix:` it.

`-Wshorten-64-to-32` findings are 64-bit bugs or proofs that a value fits: review each, never cast them
away blindly.

Floating point: the simulation uses `float` in places (87 uses) and must compute the same everywhere
(network lockstep). `port/CMakeLists.txt` already pins `-ffp-contract=off` and SSE2 on 32-bit x86.
Make `float`-to-`double` promotions explicit (`-Wdouble-promotion`), keep the original precision, and
never introduce `-ffast-math` or `std::fma`. `--simcheck` and the replays check it.

## 3.2 Booleans and `const`

- [ ] `readability-implicit-bool-conversion` (7,243): `clang-tidy -fix` per subsystem. Pointer tests
      become `!= nullptr`, integer tests `!= 0`; `char_type`/`s32` members that are really flags become
      `bool` (layout: memory-only structs only, after 3.7 for file structs).
- [ ] `misc-const-correctness` (1,469), `readability-make-member-function-const` (733),
      `readability-non-const-parameter` (27): `-fix` per subsystem; then `const` references and
      `std::span<const T>` for parameters that only read.
- [ ] `modernize-use-nodiscard` (50) and `[[nodiscard]]` on every query function (`Is*`, `Get*`, `Find*`,
      `Can*`) and on everything that returns an error.
- [ ] `cppcoreguidelines-prefer-member-initializer` (832), `-pro-type-member-init` (230),
      `-init-variables` (918): initialise members and locals. Hash-neutral where the value was always
      written before being read; where it wasn't, the original read garbage, and the change is a `Fix:`
      (Valgrind from phase 0 lists the cases). The `[[maybe_unused]] Fix16_Point` locals that only
      existed for the original's stack layout are deleted.

## 3.3 Arrays, spans, indices

- [ ] Members: `T x[N]` → `std::array<T, N>` (601 findings), except file layouts (3.7).
- [ ] Parameters: `T* p, s32 count` → `std::span<T>`; `T*` that points into an array →
      `std::span` or an index.
- [ ] Loops over whole arrays → range-for or `std::ranges` algorithms (`std::ranges::find_if`,
      `std::ranges::any_of`, ...).
- [ ] Debug and sanitizer builds define `_GLIBCXX_ASSERTIONS` (libstdc++) / `_LIBCPP_HARDENING_MODE`
      (libc++), so `std::array`/`std::span` indexing is bounds-checked there; release builds don't pay
      for it.
- [ ] Out-of-bounds accesses the original relies on are made explicit. Known case:
      `InputRecorder::GetInputBits` reads `field_8_input_masks[3 * idx + 8]`, which runs past the
      12-entry mask array into the record buffer: it reads the previous record's inputs. Write it as
      that (`records_[idx - 1].inputs`), proven hash-neutral by the replays.

## 3.4 Strings and text

Today: 686 calls of the C string family (`swprintf` 149, `wcsncpy` 85, `sprintf`, `strcpy`, `strcat`
the rest), 266 `wchar_t` uses, built with `-fshort-wchar` (2-byte `wchar_t` like Windows), and
`port/include/port_wchar.h`/`port_crt.h` rename C functions with macros (`#define wcslen port_wcslen`,
`#define _stricmp strcasecmp`, ...).

Kinds of strings, and what each becomes:

| Kind | Where | Becomes |
|---|---|---|
| Log and error messages | `error.cpp`, `ErrorLog`, debug output | `std::format` into a logger (3.8) |
| File and directory names | `"data\\nyc.gci"`, `"data\\%c.gxt"`, `"test\\replay.rep"` | `std::filesystem::path` built with `/` (the platform layer resolves case and separators, phase 6) |
| Fixed-size names in structures | player names, map/style/script names in headers and saves | `std::string` in memory; fixed-size char arrays only in the file readers/writers |
| Game text (GXT, UTF-16 in the file) | HUD, pager, frontend, credits (`swprintf` into `wchar_t` buffers) | decision 5 |

Decision 5, proposal: decode GXT to UTF-8 when loading; all game text is `std::string` (UTF-8) and
`std::format`; the font renderer decodes code points when drawing (it already maps characters to
glyphs; Kanji goes through `KanjiFont_8EC`). Alternative: `std::u16string`, closer to the data, but
`std::format` has no `char16_t` support, so it needs a small formatter of its own.

- [ ] Logging/debug strings first (no behaviour), then paths, then names, then game text.
- [ ] Remove `-fshort-wchar` when no `wchar_t` is left, then `port_wchar.h` and the C-name macros of
      `port_crt.h`.
- [ ] Text width and wrapping must stay identical (the HUD lays text out by character count): golden
      frames from phase 0 check it.

## 3.5 Unions

28 unions. Three kinds:

| Kind | Examples | Becomes |
|---|---|---|
| Tagged by a member of the same object | `Sprite`'s owner (`field_8_car_bc_ptr` / `char_b4_ptr` / `object_2C_ptr`, tag `field_30_sprite_type_enum`); `SoundObject_10::SoundObjectType`; `Object_5C`'s `TAny` | Private union plus checked accessors (`AsCar()` asserts the tag and returns `Car*`; the decompilation already has `AsCar_40FEB0`-style accessors). No layout change. `std::variant` where the object isn't in a hot pool |
| Script command operands, tagged by the command type (`miss2_0x11C.hpp`, 9 unions; `Miss2_25C`) | the script image is loaded from the `.scr` file | Typed command views over the script bytes, created by the VM per command type (3.7, script reader) |
| Packets | `network_data.hpp`'s `ReceivedPacket` | Parse into a `std::variant` of message structs on receipt |
| Type punning (bits of a float, bytes of an int) | | `std::bit_cast` |

- [ ] Name the anonymous structs inside unions (`-Wnested-anon-types`, `-Wgnu-anonymous-struct`: 23).

## 3.6 Strong types for ids, indices and units

`Fix16` and `Ang16` already show how much a distinct type helps. Today ids and indices are bare `s32`,
`s16`, `u8`, `char_type`, so a gang index can be passed where a player index is expected.

- [ ] `core/strong_id.hpp`: `template <class Tag, class Rep> class StrongId` (explicit construction,
      comparison, hashing, `value()`; no arithmetic).
- [ ] Ids: `PedId`, `CarId`, `ObjectId`, `PlayerIndex`, `GangIndex`, `ScriptThreadId`, `SpriteId`
      (`field_20_instance_id`), `CarModel` (an `enum class`), `WeaponType`, `TextKey`.
- [ ] Map coordinates: `TileCoord` (the `u8 x, y, z` map block indices) next to `Fix16` world
      positions, with explicit conversions (`ToTile(Fix16)`, `TileCentre(TileCoord)`).
- [ ] Time: `FrameCount` and `Milliseconds` where frame counters and millisecond timers are mixed
      (`PlayerScoreTracker`'s timers add `GetFrameDurationMs` per frame).
- [ ] One point type: `Fix16_Point` and `Fix16_Point_POD` become one trivially copyable `Vec2Fx`
      (and `Vec3Fx`), `Fix16_Rect` a `RectFx`.

`bugprone-easily-swappable-parameters` stays off until this is done, then goes on.

## 3.7 File formats: explicit readers

Today, 78 `File::Global_Read_4A71C0` calls read bytes straight into structures (11 asserted with
`GTA2_ASSERT_FILE_SIZEOF`, several with `#pragma pack`), with 64-bit branches where a structure holds a
pointer (`PTR32`, the style's sprite index and deltas, the text's TKEY entries, the map's column words).
That only works on little-endian machines, trusts every size in the file, and ties memory layout to
the file layout.

New library `gta2_data` (`port/game/data/`), no game types, no globals:

- `BinaryReader`: wraps `std::span<const std::byte>`; `u8()`, `u16le()`, `s32le()`, `fix16()`,
  `bytes(n)`, `string_fixed(n)`, `chunk()` (four-character id plus size); never reads out of bounds;
  sets an error and returns zeros after the first failure, so parsers check once at the end.
- `BinaryWriter`: the inverse, for saves, player data and replays.
- One parser per format, `ParseX(std::span<const std::byte>) -> std::expected<XFile, LoadError>`, into
  plain value structs (`std::vector`, `std::array`, `std::string`; no pointers). The game builds its
  runtime structures from them.

| Format | Files | Loaded by today | Notes |
|---|---|---|---|
| Map `.gmp` | `data/*.gmp` | `map_0x370.cpp` (10 reads), `RouteFinder.cpp` (6) | chunked (`chunk.hpp`); compressed column/block data; zones, lights, animations, junctions |
| Style `.sty` | `data/*.sty` | `gtx_0x106C.cpp` (17) | chunked: palettes, tiles, sprites, sprite index, deltas, car infos, object infos, fonts |
| Text `.gxt` | `data/{e,f,g,i,j,s}.gxt` | `text_0x14.cpp` (4) | TKEY (key, offset) and TDAT (UTF-16) chunks |
| Mission script `.scr` | `data/*.scr` | `ScriptManager_C1EA8.cpp` (18) | the script image the VM runs in place; `PTR32` slots (handled in phase 5) |
| Car info `.gci` | `data/nyc.gci` | `CarInfo_808.cpp` (`ReadToken_430B10`) | a text format: a tokenizer, not a binary reader |
| Kanji font | `data/kanji.dat` | `KanjiFont_8EC.cpp` (3) | |
| Audio banks `.sdt`/`.raw` | `data/audio/*` | `cSampleManager.cpp` (`fread`) | sample table plus raw PCM |
| Replays `.rep` | `test/replay.rep`, `data/attract/*.rep` | `InputRecorder_67F8B0.cpp` | `ReplayHeader_10C` plus `InputRecord_C` records; writer too |
| Player data and saves | `player/*.dat`, saves | `PlyDat_2BC0.cpp` (14), `ScriptManager_C1EA8.cpp` (writes 6), `Frontend.cpp` (2) | writer too; must stay read-compatible (decision 4) |

Per format, in this order (simplest first): replay, text, kanji, car info, player data, saves, script,
style, map, audio.

- [ ] Parser and value structs in `data/`, with unit tests on files built by a test-side writer (CI,
      no game data).
- [ ] Locally: parse every real file of that format; for formats with a writer, parse → write → compare
      byte-identical.
- [ ] Switch the game's loader to the parser; replays hash-neutral.
- [ ] Fuzz target `port/tests/fuzz/fuzz_<format>.cpp` (libFuzzer, Clang `-fsanitize=fuzzer,address,undefined`),
      seeded with synthetic files; one hour without findings, then in the nightly CI for 10 minutes each.
- [ ] Remove the old structures, `GTA2_ASSERT_FILE_SIZEOF`, `#pragma pack` and the 64-bit loader
      branches of that format.

## 3.8 Errors and logging

- [ ] Policy in `STYLE.md` (decision 6): no exceptions in the simulation; `std::expected<T, Error>` for
      operations that can fail (loading, network, saves); `FatalError` only for broken invariants;
      `assert`-style `GTA2_ASSERT` (on in debug, sanitizer and CI builds) for internal checks.
- [ ] `error.cpp`'s `FatalError_4A07C0` (1,532 lines, a `switch` over `Gta2Error` codes building
      messages) becomes a table `{Gta2Error, format string}` and one function; `Gta2Error` an `enum class`.
- [ ] `cert-err33-c` (438): every `fread`/`fwrite`/`fclose`/... result is checked or explicitly
      ignored (most go away with 3.7 and phase 6's file system interface).
- [ ] A small logger (`core/log.hpp`: levels, categories, `std::format`, to `stderr` and the error log
      file) replaces `ErrorLog` and the debug `printf`s.

## 3.9 Macros and constants

- [ ] Remaining macros (`cppcoreguidelines-macro-usage`, 34) become `constexpr` constants or inline
      functions; `GTA2_COUNTOF` becomes `std::size`.
- [ ] The 490 `k...` constants defined with `DEFINE_GLOBAL_INIT` become `constexpr` (or `inline
      constexpr` in headers), so they live in the type system and in the headers that use them.
- [ ] `performance-enum-size` (132): give enums explicit underlying types, after 3.7 so no file layout
      depends on an enum's size.

## Risks

| Risk | Mitigation |
|---|---|
| A widened or re-signed type changes arithmetic (wrap, sign extension) | Replays; keep intended wraps as explicit casts |
| Text layout changes after the text type changes | Golden frames from phase 0 |
| A reader differs from the struct it replaces in a corner (padding, sign, a 64-bit branch) | Parse all real files and compare against the old loader's structures before switching; replays |
| Save compatibility breaks | Round-trip tests on real saves (locally); a version field for any new format |
| Accessors on hot unions cost time | They compile to the same code in release; assert only in debug |
