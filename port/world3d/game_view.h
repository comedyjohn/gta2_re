// The parts of the running game the 3D renderer reads. Read only: nothing here changes the game.
#pragma once

#include "gta2_map.h"
#include "world_mesh.h"

#include <cstdint>
#include <string>

namespace world3d
{
// The map the game has loaded, as upstream's gta2::Map. False when no map is loaded.
bool ReadMap(gta2::Map& out, std::string* error);
// The slope table (gGmpSlopes_6F5BA8) in upstream's form, 64 entries
void ReadSlopes(gta2::SlopeInfo out[gta2::kSlopeTypeCount]);
// The style file the game loaded ("data\\wil.sty"), or "" when none
std::string StylePath();
// A pointer that changes when the game loads another map (gMap_0x370_6F6268->field_0_pDmap)
const void* MapIdentity();
// The slope type (0..63) of the block at a cell and level, -1 when there is none (or no map)
int BlockSlopeType(int x, int y, int level);
// The grey levels the game shades faces with this frame (PLAN.md 2.5), indexed by ShadeKind; `lit`: the game
// asks the renderer for lights (gLightingDrawFlag_7068F4 & 0x8000)
struct Shades
{
    uint8_t level[10] = {};
    bool lit = false;
};
Shades ReadShades();
// The OpenGL texture of a map tile as the game draws it now (through its tile remap), 0: nothing is drawn
unsigned TileTexture(int tile);
} // namespace world3d
