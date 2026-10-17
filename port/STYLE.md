# Style of port/

The rules for new and renamed code in `port/game`, `port/platform` and `port/tests`. They follow the
Google C++ Style Guide for naming (decision 1, [port/refactor/decisions/](refactor/decisions/)); the
formatting is `.clang-format` (run `clang-format -i`; CI checks it). `port/world3d/` keeps upstream's
style. Code that has not been renamed yet (phase 2 of [refactor/](refactor/README.md)) still has the
old names: `ExitGame_4B8BD0`, `Car_BC`, `field_2C_game_exit_type`. Follow these rules when you touch a
name, not before.

## Names

| Kind | Rule | Example |
|---|---|---|
| Types (class, struct, enum, alias) | `PascalCase`, no size or address | `Game`, `Car`, `ScriptThread` |
| Functions and methods | `PascalCase`, no address | `ExitGame`, `GetInputBits` |
| Data members | `snake_case_` (private and protected) | `game_exit_type_`, `prev_` |
| Public data members of plain structs (no invariants) | `snake_case` | `rng_index` |
| Locals and parameters | `snake_case`, no Hungarian prefix | `car`, `new_timer`, `x_offset` |
| Constants (`constexpr`, `const` globals) | `kPascalCase` | `kAng45`, `kFp255` |
| Enumerators | `kPascalCase` (an explicit `= 2` stays) | `GameExitType::kPlayerQuit` |
| Mutable globals (until phase 6) | `g` + `PascalCase` | `gGame`, `gDebugDoMike` |
| Macros | `GTA2_UPPER_CASE` | `GTA2_COUNTOF` |
| Namespaces | `snake_case` | `gta2`, `gta2::sim` |
| Files | `snake_case`, after the main type; one directory per subsystem | `vehicles/car.cpp` |

The type says what a variable is, so `b`, `p`, `pp`, `u` and `n` prefixes go. A pointer member that
would read oddly without one gets a noun (`owner_`).

## The address comment

A function that has an original in the 1998 binary keeps its address in a comment on the line above:

```cpp
// 0x4B8BD0
void Game::ExitGame();
```

The address is not part of the name. `port/tools/refactor/address_map.csv` maps old names, new names and
addresses both ways (decision 2); porting a fix from `Source/` or looking something up in Ghidra goes
through it (`address_map.py --lookup`).

## Other rules

- **Behaviour first.** A refactor never changes what the game computes: the replay corpus hashes the
  whole simulation after every update (`port/tools/refactor/check.sh`).
- **Types**: `s32`, `u8`, `Fix16` and friends from `types.hpp` until phase 3 replaces them.
- **Casts**: C++ casts only (`static_cast`, `const_cast`, `reinterpret_cast`); `-Wold-style-cast` is on.
  `reinterpret_cast` between related classes skips the pointer adjustment a C cast does: use
  `static_cast`.
- **Errors**: `std::expected` for operations that can fail, `FatalError` for broken invariants, no
  exceptions in the simulation (decision 6).
- **Text**: UTF-8 `std::string` and `std::format`; code points only when drawing (decision 5).
- **Original formats** (saves, replays, player data, scripts) stay loadable (decision 4).
- **File-local** functions and variables go in an anonymous namespace, not `static`.
- **No `goto`** except for a shared cleanup block that several checks jump to; say why in a comment.
- **Warnings**: the game builds with `-Wall -Wextra -Wshadow -Wold-style-cast ...` and `-Werror`
  (`GTA2_WERROR`); fix a warning, don't silence it.
- **clang-tidy**: `port/.clang-tidy`; `tidy_ratchet.py --check` fails when a file gets more findings of
  a check than the baseline has.
- **Comments** say why, not what. The original's quirks that the game depends on are marked as such.
