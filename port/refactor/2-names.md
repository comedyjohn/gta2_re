# Phase 2: Names

**Goal:** every identifier says what the thing is. No addresses, no struct sizes, no `field_XX` offsets,
no `sub_`, `byte_` or `dword_` placeholders, one naming convention enforced by clang-tidy, and the code in
one directory per subsystem.

This is the biggest single gain in readability, and most of it is mechanical once the convention is
fixed: the names already carry meaning after their prefix (`field_2C_game_exit_type`,
`ExitGame_4B8BD0`). The research part is the 118 `sub_` functions, 107 unnamed fields, about 450 raw
data globals and the 20 type names that collide once their size suffix is gone.

**Depends on:** phase 0, phase 1, decisions 1 and 2 in [README.md](README.md). **Blocks:** phases 3 to 6.

**Exit criteria**

- [ ] `readability-identifier-naming` passes with the options below.
- [ ] `grep -E '_[0-9A-F]{6}\b|\bfield_[0-9A-Fa-f]+|\bsub_[0-9A-Fa-f]{6}|\b(byte|word|dword|unk|flt|off|stru)_[0-9A-Fa-f]{6}' port/game -r`
      finds only comments.
- [ ] No type name ends in a struct size (`_BC`, `_0x40`, `_2B00`, ...).
- [ ] `port/tools/refactor/address_map.csv` has a row for every original function, global and member.
- [ ] Files live in `port/game/<subsystem>/`, named after their main type in `snake_case`.
- [ ] Every commit hash-neutral.

## 2.1 The convention (decision 1)

Proposal: the naming rules of the Google C++ Style Guide, a published, widely known standard, which
also needs the fewest manual decisions here because today's names are already snake_case after their
prefixes.

| Kind | Rule | Today | After |
|---|---|---|---|
| Types (class, struct, enum, alias) | `PascalCase`, no size | `Game_0x40`, `Car_BC`, `sound_obj`, `miss2_0x11C` | `Game`, `Car`, `SoundManager`, `ScriptThread` |
| Functions and methods | `PascalCase`, no address | `ExitGame_4B8BD0`, `get_input_bits_4CEAC0`, `sub_4C9ED0` | `ExitGame`, `GetInputBits`, (researched name) |
| Data members | `snake_case_` | `field_2C_game_exit_type`, `field_0_bShutDown`, `field_4_pPrev` | `game_exit_type_`, `shut_down_`, `prev_` |
| Public data members of plain structs (no invariants) | `snake_case` | `field_0_rng_idx` | `rng_index` |
| Locals and parameters | `snake_case` | `pCar`, `new_timer`, `x_off` | `car`, `new_timer`, `x_offset` |
| Constants (`constexpr`, `const` globals) | `kPascalCase` | `kAng45_6F677C`, `kFp255_678414` | `kAng45`, `kFp255` |
| Enumerators | `kPascalCase` | `GameExitType::PlayerQuit_2` | `GameExitType::kPlayerQuit` (explicit `= 2` stays) |
| Mutable globals (until phase 6) | `g` + `PascalCase` | `gGame_0x40_67E008`, `bDo_mike_67D5CC` | `gGame`, `gDebugDoMike` |
| Macros (few left after phase 1) | `GTA2_UPPER_CASE` | `GTA2_COUNTOF` | unchanged |
| Namespaces | `snake_case` | none | `gta2`, `gta2::sim`, ... (phase 6) |
| Files | `snake_case`, after the main type | `Car_BC.cpp`, `miss2_0x11C.cpp` | `vehicles/car.cpp`, `script/script_thread.cpp` |

Hungarian prefixes go (`b`, `p`, `pp`, `u`, `n`): the type says it. A pointer member that would read
oddly without them gets a noun (`field_C_pAny` becomes `owner_` or whatever it holds).

Alternative (if decided): `m_camelCase` members and `camelCase` locals; the tooling below works the same,
only the name proposals change.

The clang-tidy options that enforce it go into `port/.clang-tidy`:

```yaml
CheckOptions:
  readability-identifier-naming.ClassCase: CamelCase
  readability-identifier-naming.StructCase: CamelCase
  readability-identifier-naming.EnumCase: CamelCase
  readability-identifier-naming.TypeAliasCase: CamelCase
  readability-identifier-naming.FunctionCase: CamelCase
  readability-identifier-naming.MethodCase: CamelCase
  readability-identifier-naming.PrivateMemberCase: lower_case
  readability-identifier-naming.PrivateMemberSuffix: '_'
  readability-identifier-naming.ProtectedMemberCase: lower_case
  readability-identifier-naming.ProtectedMemberSuffix: '_'
  readability-identifier-naming.PublicMemberCase: lower_case
  readability-identifier-naming.LocalVariableCase: lower_case
  readability-identifier-naming.ParameterCase: lower_case
  readability-identifier-naming.ConstexprVariableCase: CamelCase
  readability-identifier-naming.ConstexprVariablePrefix: k
  readability-identifier-naming.GlobalConstantCase: CamelCase
  readability-identifier-naming.GlobalConstantPrefix: k
  readability-identifier-naming.EnumConstantCase: CamelCase
  readability-identifier-naming.EnumConstantPrefix: k
  readability-identifier-naming.GlobalVariableCase: CamelCase
  readability-identifier-naming.GlobalVariablePrefix: g
  readability-identifier-naming.NamespaceCase: lower_case
  readability-identifier-naming.MacroDefinitionCase: UPPER_CASE
```

Until phase 2 ends, the check runs only on subsystems already renamed (a `.clang-tidy` in each finished
subsystem directory turns it on).

- [ ] Write `port/STYLE.md`: the table above, plus the rules other phases add (ownership, error handling,
      `const`, comments, the `// 0xADDR` comment on every function that has an original).

## 2.2 The address map (before any rename)

Generated once from the current sources, then updated by the rename tool on every rename, so it never
goes stale. `port/tools/refactor/address_map.csv`:

| Column | Example |
|---|---|
| `kind` | `function`, `method`, `global`, `member`, `type`, `enumerator` |
| `address` | `0x4B8BD0` (functions, globals); offset `0x2C` for members |
| `owner` | `Game_0x40` (members and methods; the old name, the new one in `new_owner`) |
| `old_name` | `ExitGame_4B8BD0` |
| `new_name` | `ExitGame` |
| `new_owner` | `Game` |
| `addr_96f` | the 9.6f address from the `// 9.6f 0x...` comments |
| `decompme` | the decomp.me scratch from the `// https://decomp.me/scratch/...` comments |
| `file` | where it is defined now |

- [ ] `port/tools/refactor/address_map.py --init`: builds the table with libclang (every declaration with
      an address in its name or its `// 0xADDR` comment, every member with its offset from the `field_XX`
      name), and moves the 9.6f and decomp.me comments into it (they leave the code: phase 1, task 1.2).
- [ ] `address_map.py --lookup <old name|address|new name>`: prints the row, for porting fixes from
      `Source/` or looking something up in Ghidra.

## 2.3 The rename tool

Renames must be exact: the same member name `field_0_pHead` exists in many classes, overloads must not
change which function a call picks, and macros hide uses. So renames go by **USR** (clang's unique symbol
ID), not by text.

`port/tools/refactor/rename.py` (Python, `libclang==18.1.1`, the same LLVM as clang-tidy):

```
rename.py [--dry-run] <renames.csv>        # columns: old qualified name, new name
rename.py --propose <subsystem> <kind>     # writes a renames.csv with mechanical proposals to review
```

1. Parses every translation unit in `build_port/clang64/compile_commands.json` with libclang (8 in
   parallel).
2. Resolves each old qualified name (`Game_0x40::field_2C_game_exit_type`, `Game_0x40::ExitGame_4B8BD0`,
   `Car_BC`, `gGame_0x40_67E008`) to its USR.
3. Visits every cursor; when a declaration or reference resolves to one of the USRs, records the file,
   offset and length of its spelling. Covers declarations, definitions, member references, calls,
   constructor and destructor names, `using`, template arguments and friend declarations.
4. Refuses the whole batch when:
   - a new name already exists in the same scope (a member, method, nested type or base-class member),
   - two old names map to the same new name in one scope (an accidental overload set),
   - a reference sits inside a macro expansion (listed for a manual edit),
   - a reference sits in a file outside `port/` (none should).
5. Applies all edits from the end of each file backwards, updates `address_map.csv`, and prints a
   summary for the commit message.

After each batch: build GCC 32/64 and Clang 64, run the unit tests and the replays, and
`grep` that the old names are gone (outside `address_map.csv`).

`--propose` generates the mechanical names:

| Kind | Rule | Example |
|---|---|---|
| Function/method | strip `_[0-9A-F]{6}` (and `_inline`/`_ool` variants after phase 1.3), convert `snake_case` to `PascalCase` | `get_input_bits_4CEAC0` → `GetInputBits` |
| Member | strip `field_<hex>_`, drop Hungarian `b`/`p`/`pp`, convert camel parts to snake, add `_` | `field_75340_rec_buf_idx` → `rec_buf_idx_` |
| Type | strip `_<hex>`/`_0x<hex>` size suffixes | `Garage_48` → `Garage` |
| Constant | strip the address | `kAng45_6F677C` → `kAng45` |
| Enumerator | strip `_<digits>`, add `k`, PascalCase | `Running_1` → `kRunning` |
| Global | strip address and size | `gGame_0x40_67E008` → `gGame` |

Anything the rules can't decide (empty result, collision, `sub_`, unnamed `field_XX`, `byte_`/`dword_`)
is written to the CSV with an empty new name and a `TODO` column, and the batch won't run until it is
filled in by hand.

## 2.4 Types that need real names

223 of the 449 types carry a size. Stripping it leaves 20 collisions, which need a name from what each
type does (read its methods and where it is created; `Game_0x40::Game_0x40` creates most of the
managers):

| Collision | Types | Likely names (to confirm) |
|---|---|---|
| `Object` | `Object_2C`, `Object_3C`, `Object_5C`, `Object_8` | `GameObject` (a map object), `ObjectPhysics`, `ObjectManager` (`gObject_5C_6F8F84` services them all), `ObjectLink` |
| `Light` | `Light_28`, `Light_1D4CC` | `Light`, `LightGrid` |
| `Door` | `Door_38`, `Door_4D4` | `Door`, `DoorManager` |
| `Weapon` | `Weapon_30`, `Weapon_8` | `Weapon`, `WeaponManager` |
| `Particle` | `Particle_4C`, `Particle_8` | `Particle`, `ParticleSystem` |
| `Ambulance` | `Ambulance_20`, `Ambulance_110` | `AmbulanceCrew`, `AmbulanceService` |
| `Char` | `Char_B4`, `Char_8` | `PedBody` (the ped's physical side), `PedBodyLink` |
| `CarInfo` | `CarInfo_2C`, `CarInfo_808` | `CarModelInfo`, `CarInfoTable` (the `.gci` data) |
| `ExplosionPool` | `ExplosionPool_3D4`, `ExplosionPool_7A8` | by what each holds |
| `Sprite` | `Sprite`, `Sprite_3C` | `Sprite`, by use |
| `Taxi` | `Taxi_4`, `Taxi_8` | `TaxiManager`, `Taxi` |
| `TileAnim` | `TileAnim_2`, `TileAnim_18` | `TileAnimator`, `TileAnimation` |
| `RouteFinder` | `RouteFinder`, `RouteFinder_10`, `RouteFinder_200` | `RouteFinder`, `Junction`, `JunctionList` |
| `miss2` / `Miss2` | `miss2_0x11C`, `miss2_8`, `Miss2_25C`, `Miss2_C` | `ScriptThread`, `GosubStack`, `ScriptObjectSlots`, ... |
| `Mike` | `Mike_8`, `Mike_78`, `Mike_80`, `Mike_A80` | a developer's debug code (`bDo_mike`): name by purpose or delete in phase 4 |
| `Network` | `Network_4` ... `Network_20324` | `NetworkLobby` (`Network_20324`) and its parts |
| `kanji`, `text` | `kanji_0x2`/`0x20`, `text_0x14`/`0xC` | `KanjiGlyph`, `KanjiFont`; `GameText`, `TextEntry` |

Some classes are named after an assumption or a placeholder (`sound_obj`, `Hud_2B00`, `CokeZero_FC`,
`naughty_sinoussi_0x800`, `test`): name them by what they do.

## 2.5 Research names

| What | How many | Procedure |
|---|---|---|
| `sub_XXXXXX` functions | 118 | Read the body and the call sites; check `og_function_data_v96f.csv` and the 9.6f symbols for a name; check Ghidra's project if it has one. Name by verb and object (`FindNearestCarDoor`). If still unclear, a descriptive provisional name plus a comment saying so, never `sub_` |
| Unnamed `field_XX` members | 107 | Find every read and write (`rename.py --dry-run` lists them); name by use; a member that is written but never read is a candidate for deletion in phase 4 (`unused_` provisional name meanwhile) |
| Raw data globals `byte_`/`word_`/`dword_`/`unk_`/`flt_` | about 450 (307 initialised) | First `port/tools/refactor/global_users.py`: lists, per global, the functions that read and write it. Used in one function only: becomes a local or a `static` local (hash-neutral only if the function never relies on the previous call's value: check for reads before writes). Initialised and never written: a `constexpr` constant. Otherwise name it by use |
| Debug flags `b...` | 91 | `gDebug` + setting name (`bDo_mike_67D5CC` → `gDebugDoMike`); phase 6 collects them into a `DebugSettings` struct |

## 2.6 Directories and files

- [ ] `port/game/<subsystem>/` for each subsystem in `subsystems.py` (`core`, `data`, `world`, `sprites`,
      `objects`, `vehicles`, `peds`, `services`, `script`, `game`, `audio`, `presentation`, `network`,
      `app`); `git mv` the files, update `subsystems.py`, use `file(GLOB_RECURSE ...)` in CMake, and
      include as `#include "vehicles/car.hpp"` (one include root, `port/game/`).
- [ ] File names follow the main type, `snake_case` (`Car_BC.cpp` → `vehicles/car.cpp`,
      `char.cpp` → `peds/ped_body.cpp`). Header-only enum files keep one enum each, named after it.
- [ ] A file move is its own commit (`git mv` only), so history follows it (`git log --follow`).

## 2.7 Order of work

Per subsystem, in the order of [README.md](README.md) (core, data, world, sprites, objects, vehicles,
peds, services, script, game, audio, presentation, network, app), as separate commits:

1. Types (collisions from 2.4 decided first).
2. Methods and free functions.
3. Data members.
4. Enumerators and constants.
5. Globals.
6. Research names (2.5) for the subsystem.
7. Directory and file names (2.6).
8. Turn on `readability-identifier-naming` for the subsystem's directory.

Cross-subsystem references are fine: a rename touches every file that uses the symbol, wherever it is.

| Subsystem | Types | Functions with an address | `field_` names | `sub_` | Raw data globals | All globals |
|---|---|---|---|---|---|---|
| core | 17 | 157 | 18 | 7 | 3 | 20 |
| data | 37 | 208 | 141 | 2 | 9 | 120 |
| world | 32 | 288 | 218 | 31 | 26 | 107 |
| sprites | 16 | 278 | 77 | 7 | 4 | 59 |
| objects | 28 | 620 | 363 | 23 | 92 | 252 |
| vehicles | 28 | 957 | 329 | 24 | 122 | 342 |
| peds | 18 | 738 | 235 | 19 | 59 | 227 |
| services | 15 | 248 | 203 | 11 | 27 | 55 |
| script | 116 | 604 | 428 | 2 | 0 | 19 |
| game | 24 | 650 | 289 | 11 | 36 | 98 |
| audio | 15 | 318 | 240 | 4 | 40 | 67 |
| presentation | 59 | 656 | 378 | 13 | 31 | 238 |
| network | 27 | 135 | 159 | 1 | 2 | 7 |
| app | 1 | 298 | 0 | 1 | 10 | 71 |

(Functions are counted where they are used, so a function appears in each subsystem that calls it.)

## Example

Before:

```cpp
class InputRecorder_67F8B0
{
  public:
    u32 get_input_bits_4CEAC0();
    void save_replay_record_4CDE20(u32 inputs);
    s32 field_4_input_bits;
    ReplayState field_38_replay_state;
    InputRecord_C field_3C_rec_buff[k_max_records];
    u32 field_7533C_used_recs_count;
    u32 field_75340_rec_buf_idx;
};
EXTERN_GLOBAL(InputRecorder_67F8B0, gInputRecorder_67F8B0);

if (gpRng_67AB34->get_cur_rng_41CFE0() >= field_3C_rec_buff[field_75340_rec_buf_idx].field_0_rng_idx)
```

After (`game/input_recorder.hpp`):

```cpp
class InputRecorder
{
  public:
    u32 GetInputBits();                   // 0x4CEAC0
    void SaveReplayRecord(u32 inputs);    // 0x4CDE20
    s32 input_bits_;
    ReplayState replay_state_;
    InputRecord records_[kMaxRecords];
    u32 used_record_count_;
    u32 record_index_;
};
extern InputRecorder gInputRecorder;      // 0x67F8B0

if (gRng->GetCallCount() >= records_[record_index_].rng_index)
```

(Members stay public until phase 6 gives each class its interface.)

## Risks

| Risk | Mitigation |
|---|---|
| A rename hits a symbol of the same name in another class | USR-based renames, never text |
| Two old functions get the same new name and overload resolution changes | `rename.py` refuses same-scope collisions; replays catch the rest |
| Macros hide uses | Listed by the tool for manual edits; few macros left after phase 1 |
| The link to `Source/` and Ghidra is lost | `address_map.csv`, `// 0xADDR` comments kept |
| Long-lived branches conflict | Work subsystem by subsystem; rebase often; the rename CSVs can be re-applied to a branch with `rename.py` |
| A research name is wrong | Comment what is known; renaming again later is cheap with the tool |
