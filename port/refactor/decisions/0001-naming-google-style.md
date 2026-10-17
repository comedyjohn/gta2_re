# 0001: Naming follows the Google C++ style

Date: 2026-10-16. Status: decided by the user.

## Context
The decompiled code names things after the original binary: `ExitGame_4B8BD0`, `Car_BC`,
`field_2C_game_exit_type`. Names mix Hungarian prefixes, sizes and addresses with the meaning. A
convention was needed that a tool (clang-tidy `readability-identifier-naming`) can enforce and that
needs few manual decisions, since today's names are already `snake_case` after their prefixes.

## Decision
`PascalCase` types and functions, `snake_case` locals and parameters, `snake_case_` private and
protected members, `snake_case` public members of plain structs, `kPascalCase` constants and
enumerators, `g` + `PascalCase` mutable globals, `snake_case` files. The table is in
[STYLE.md](../../STYLE.md) and [2-names.md](../2-names.md) 2.1.

## Consequences
- Hungarian prefixes (`b`, `p`, `pp`, `u`, `n`) go; the type says it.
- The rename tool proposes names by rule; what no rule can decide is filled in by hand.
- The alternative (`m_camelCase`) was considered and not taken.
