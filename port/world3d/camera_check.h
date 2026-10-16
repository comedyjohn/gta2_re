// GTA2_WORLD3D_CHECK=1: compares the clip matrix with the game's own screen positions of every tile corner
// (PLAN.md 3.3) and prints the largest error once a second. Does nothing without the variable.
#pragma once

struct Vert;

namespace world3d
{
// `verts`: the 8 vertices of a tile draw (screen positions, then world positions)
// `slopeType`: of the block being drawn (-1: unknown). Blocks with a slope (any type but 0) are counted apart:
// the classic renderer draws them from other vertices and the mesh does the same by geometry.
void CheckCamera(const Vert* verts, int slopeType);
// The largest error of the last second, in pixels (for the panel)
float LastCameraError();
} // namespace world3d
