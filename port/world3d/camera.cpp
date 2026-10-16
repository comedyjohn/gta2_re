#include "camera.h"

namespace world3d
{
void ClipMatrix(const GameCamera& cam, float W, float H, float out[16])
{
    const float A = (kFar + kNear) / (kFar - kNear);
    const float B = -2.0f * kFar * kNear / (kFar - kNear);
    const float E = cam.camZ + kEyeHeight;
    for (int i = 0; i < 16; i++)
    {
        out[i] = 0;
    }
    // m[col * 4 + row]
    out[0 * 4 + 0] = 2.0f * cam.f / W;
    out[3 * 4 + 0] = -2.0f * cam.f / W * cam.camX;
    out[1 * 4 + 1] = -2.0f * cam.f / H;
    out[3 * 4 + 1] = 2.0f * cam.f / H * cam.camY;
    out[2 * 4 + 2] = -A;
    out[3 * 4 + 2] = A * E + B;
    out[2 * 4 + 3] = -1.0f;
    out[3 * 4 + 3] = E;
}

void GameProject(const GameCamera& cam, float x, float y, float z, float& sx, float& sy, float& invD)
{
    const float D = cam.camZ + kEyeHeight - z;
    sx = cam.cx + cam.f * (x - cam.camX) / D;
    sy = cam.cy + cam.f * (y - cam.camY) / D;
    invD = 1.0f / D;
}
} // namespace world3d
