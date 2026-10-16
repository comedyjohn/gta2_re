// The OpenGL world pass: the Scene's mesh drawn in 3D with the game's camera, shading and lights
// (PLAN.md phase 3). Must be called with the port's GL context current.
#pragma once

namespace world3d
{
struct Scene;

// After the scene's mesh was rebuilt: uploads it
void WorldPassUpload(const Scene& scene);
// Once per frame, before the first map tile is drawn. Returns false (and draws nothing) when it can't.
// Afterwards the classic renderer's GL state is current again.
bool WorldPassDraw(const Scene& scene);
// How many triangles the last WorldPassDraw drew
int WorldPassTriangles();
} // namespace world3d
