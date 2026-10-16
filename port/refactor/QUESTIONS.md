## Q1: Valgrind finds 3 uninitialised reads on all 4 replays (task R0.3, 2026-10-16)
What I tried: `port/tools/refactor/valgrind.sh build_port/linux64` (replays finish in about 1 min each, not hours). Logs: `build_port/linux64/valgrind/*.vg`.
What I saw (the replay hashes are unaffected today, the reads are of heap/static memory that happens to be stable):
1. `PathFinder_2FD4::RemovePed_554620` (`PathFinder_2FD4.cpp:254`): `field_0_ped_id` is compared before anything sets it; `Init_5545E0` doesn't touch it.
2. `Door_38::CanOpen_49C6D0` (`Door_38.cpp:103`, 11-attract-downtown only): `field_10_car_bc`/the door type is read uninitialised.
3. `SetTexture` (`platform/renderer.cpp:484`): `pPixelData` from `load_tiles_5AADF0` (`gtx_0x106C.cpp:669`, `Aligned_malloc`) has bytes never written. Drawing only, no effect on the hash.
Options: A) `Fix:` commits that give the fields the value the original's allocator gave (zero) and rewrite the logs; B) record all three in `known-ub.md` as the original's.
My recommendation: A for 3 (zeroing the buffer, the original zeroed heap pages) and A for 1 and 2 if the user agrees that zero is right. I can't tell whether "zero" is what the original had (§0 rule 2), so I have not changed anything.
Status: answered: fix them (A). Done in the three Fix: commits; valgrind is clean on linux64.
