# Phase 8: Documentation

**Goal:** someone new can understand how the game is put together, build it, test a change and land it,
from the repository alone. Runs alongside every other phase: each phase updates the documents it
affects in the same commits.

**Exit criteria**

- [ ] The documents below exist and match the code.
- [ ] Every public interface (the public header of each subsystem, phase 6.6) has a doc comment per
      class and per function.
- [ ] Doxygen builds with no warnings for the public headers (CI job, HTML as an artefact).

## Documents

| Document | Contents | Written in phase |
|---|---|---|
| `port/README.md` | What the port is, how it relates to `Source/`, quick start (exists) | kept current |
| `port/RUNNING.md` | For players (exists) | kept current |
| `port/MULTIPLAYER.md` | Network play (exists) | kept current |
| `port/TODO.md` | What is done and next (exists) | kept current |
| `port/STYLE.md` | Naming (phase 2), formatting, ownership (5), errors (3.8), `const`, threads (7), comments, the `// 0xADDR` rule | 2, then each phase adds its rules |
| `port/CONTRIBUTING.md` | Build presets, running unit tests, replays, golden frames, sanitizers, tidy ratchet, metrics; commit rules (one kind of change, `Fix:` commits, hash-neutral); recording a new golden replay | 0, 1 |
| `port/ARCHITECTURE.md` | Layers and libraries, the frame loop, `World`, events, snapshots, handles and pools, determinism rules (what the simulation may not do), threads, where each subsystem lives | 6, 7 |
| `port/game/<subsystem>/README.md` | What the subsystem does, its main types, how its update works, its known oddities from the original | 2 (skeleton), filled in 4 to 6 |
| `port/refactor/` | This plan; ticked off as it goes | all |
| `port/refactor/decisions/NNNN-title.md` | One short record per decision (context, decision, consequences): the six in [README.md](README.md) first, then each later one | as decided |
| `port/refactor/known-ub.md`, `stale-references.md` | Behaviour of the original kept on purpose | 0, 5 |
| `port/tools/refactor/address_map.csv` | Original addresses ↔ names | 2 |

## Comments in code

- A function's comment says what it does and why, not how; the address stays as `// 0xADDR`.
- Behaviour kept from the original that looks wrong gets a comment that says so ("The original ...;
  kept because replays and network play depend on it").
- No commented-out code; no comments that repeat the code.
- `TODO` only with an issue reference.

## Tooling

- [ ] `port/Doxyfile` for the public headers; CI job `docs` (warnings as errors).
- [ ] Optional: a generated subsystem dependency graph (from `deps.py`) in `ARCHITECTURE.md`.
