# 0004: The original game's files stay loadable

Date: 2026-10-16. Status: decided by the user.

## Context
Players and the replay corpus depend on the original saves, replays, player data and mission scripts.

## Decision
They stay loadable forever. The port may write a new format of its own for its own saves, with a version
field in it.

## Consequences
- Phase 3's typed readers must reproduce what the struct-over-file reads (`Global_Read_4A71C0`) do today,
  including the odd cases.
- A format change needs a version number and a reader for the old one.
