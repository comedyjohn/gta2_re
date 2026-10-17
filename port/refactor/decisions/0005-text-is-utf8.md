# 0005: Text is UTF-8 inside, code points only when drawing

Date: 2026-10-16. Status: decided by the user.

## Context
The original uses 8-bit strings with game-specific encodings, and `wchar_t` (2 bytes, via
`-fshort-wchar`) for some screens. `sprintf` and `swprintf` are everywhere (686 uses).

## Decision
GXT text is decoded to UTF-8 at load. `std::string` and `std::format` are used everywhere. Code points
are decoded only when drawing.

## Consequences
- The printf family and fixed buffers go in phase 3.
- Font lookup (`KanjiFont`) takes code points.
- Original GXT files are still read as they are.
