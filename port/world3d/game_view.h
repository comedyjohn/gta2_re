// The parts of the running game the 3D renderer reads. Read only: nothing here changes the game.
#pragma once

#include "gta2_map.h"
#include "world_mesh.h"

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
} // namespace world3d
