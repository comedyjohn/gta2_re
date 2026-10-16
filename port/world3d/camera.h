// The game's camera as a pinhole looking straight down (PLAN.md 2.2), and the OpenGL transform that
// matches it. Not upstream's camera.h.
#pragma once

namespace world3d
{
struct GameCamera
{
    float camX = 0, camY = 0, camZ = 0, f = 0, cx = 320, cy = 240; // 2.2
    bool valid = false;
};

constexpr float kNear = 0.05f;
constexpr float kFar = 64.0f;
constexpr float kEyeHeight = 8.0f; // the eye is this far above the camera position's z

// From gViewCamera_676978 (game_view.cpp); valid = false when there is none
GameCamera ReadCamera();
// The column-major clip transform of 2.2 for a W x H screen
void ClipMatrix(const GameCamera& cam, float W, float H, float out[16]);
// The game's own projection of 2.2: screen x, y and 1/D
void GameProject(const GameCamera& cam, float x, float y, float z, float& sx, float& sy, float& invD);
} // namespace world3d
