# 0003: Golden replays run locally first, on a self-hosted runner later

Date: 2026-10-16. Status: decided by the user.

## Context
The refactor is judged by the replay corpus: each replay logs a hash of the whole simulation after every
update, and a refactor must leave every log byte for byte unchanged. The replays need the game data,
which can't be put on a public CI runner.

## Decision
The executor runs `port/tools/replaytest.sh` locally. When a self-hosted runner exists, it runs them as
the gate (`port-replays` in `.github/workflows/port.yml`, label `gta2-data`).

## Consequences
- Every commit of the refactor is checked by `port/tools/refactor/check.sh`, which includes the replays.
- Nothing from `~/Games` is committed.
