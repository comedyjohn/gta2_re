// How a vertex is shaded (PLAN.md 2.5): an index into the table of ReadShades()
enum ShadeKind : uint8_t
{
    kShadeLid0, // 255
    kShadeT1, // lid level 1, west side
    kShadeT2, // lid level 2, south side
    kShadeT3, // lid level 3, north side
    kShadeEast,
    kShadeDiagUpLeft,
    kShadeDiagUpRight,
    kShadeDiagDownLeft,
    kShadeDiagDownRight,
    kShadeRamp, // a ramp's lid without a level (field_14_dcolour)
    kShadeCount,
    kShadeClassic = 255, // the block is one the classic renderer keeps drawing (slope types 49..61)
};

// The world as triangles, built from the running game's map (PLAN.md phase 2)
#pragma once

#include "game_view.h"
#include "gta2_map.h"
#include "gta2_style.h"
#include "world_mesh.h"

#include <cstdint>
#include <string>
#include <vector>

namespace world3d
{
// Upstream's mesh frame (X east, Y up, Z north = 256 - y) to the game's (x east, y south, z up), PLAN.md 2.1
void MeshToGame(gta2::Vertex& v);

// The world as triangles, in game coordinates (2.1), rebuilt when the game's map changes
struct Scene
{
    gta2::Map map;
    gta2::Style style;
    gta2::WorldMesh mesh; // vertices converted to game coordinates
    gta2::SlopeInfo slopes[gta2::kSlopeTypeCount];
    std::vector<uint8_t> kind; // per vertex: a ShadeKind
    const void* identity = nullptr;
    std::string stylePath;
    bool valid = false;
    float buildMs = 0; // the last rebuild
};

// Brings `scene` up to date: reloads the style when its path changed, rebuilds the mesh when the map
// changed (dirty) or another map was loaded. Returns true when the mesh was rebuilt.
bool UpdateScene(Scene& scene, bool dirty);
} // namespace world3d
