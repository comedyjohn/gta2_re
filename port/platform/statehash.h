// The simulation's state, hashed: what the replay tests (gta2 --replay, port/tools/replaytest.sh)
// compare frame by frame between builds, so a refactor that changes what the game does is caught.
// One hash per component, so a difference also says where it is. See port/refactor/0-safety-net.md.
#pragma once

#include <stdint.h>

#include <string>

struct StateHashes
{
    uint64_t total;
    uint64_t rng, sprites, peds, cars, Objects, players, script, police;
};

// Hashes the simulation as it is now. Only call it while a game runs (gGame_0x40_67E008 is set).
StateHashes port_state_hash();

// The state the hash covers, as text, one line per value, to diff two builds where their hashes
// first differ (gta2 --replay ... --break-at FRAME)
std::string port_state_dump();

// Calls f(offset) for every 4-byte slot in [base, base + len) that holds the pointer the game last
// stored there through PTR32 (port/platform/ptr32.cpp)
void Ptr32_SlotsIn(const void* base, uint32_t len, void (*f)(uint32_t offset, void* ctx), void* ctx);
