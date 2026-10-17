# 0002: Original addresses leave the identifiers

Date: 2026-10-16. Status: decided by the user.

## Context
Names carry the address of the original function (`ExitGame_4B8BD0`). That links the code to Ghidra and
to `Source/` (the matching decompilation), but it makes every name long and ugly.

## Decision
The address is removed from the identifier. Each function keeps a `// 0xADDR` comment, and every rename
is recorded in `port/tools/refactor/address_map.csv` (old name, new name, address, 9.6f address,
decomp.me scratch, file), so a name can be looked up in both directions.

## Consequences
- `address_map.py --lookup` finds `ExitGame_4B8BD0` from `0x4B8BD0` and from `ExitGame`.
- The `// 9.6f 0x...` and decomp.me comments move into the CSV.
- Porting a fix from `Source/` goes through the CSV.
