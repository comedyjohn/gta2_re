# 0006: No exceptions in the simulation

Date: 2026-10-16. Status: decided by the user.

## Context
The simulation must be deterministic and its cost predictable; the original reports errors with
`FatalError` or return codes.

## Decision
`std::expected` for operations that can fail, `FatalError` for broken invariants. Exceptions are allowed
at load time and in tools.

## Consequences
- New fallible functions return `std::expected<T, Error>`.
- Existing `-1`/`0` returns are converted in phase 3 or 4, per function, hash-neutral.
- Exceptions are not a way out of the simulation loop.
