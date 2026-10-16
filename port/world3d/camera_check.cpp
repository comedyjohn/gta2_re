#include "camera_check.h"

#include "camera.h"
#include "3rdParty/GTA2Hax/d3ddll/d3ddll.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>

namespace world3d
{
namespace
{
float gLastError = 0;
}

float LastCameraError()
{
    return gLastError;
}

void CheckCamera(const Vert* verts, int slopeType)
{
    static const bool enabled = getenv("GTA2_WORLD3D_CHECK") != nullptr;
    if (!enabled)
    {
        return;
    }
    static float worst = 0, worstOther = 0;
    static int tiles = 0, others = 0;
    static auto last = std::chrono::steady_clock::now();
    const GameCamera cam = ReadCamera();
    if (cam.valid)
    {
        float m[16];
        ClipMatrix(cam, 640, 480, m);
        for (int i = 0; i < 4; i++)
        {
            const Vert& w = verts[i + 4];
            const float c0 = m[0] * w.x + m[12];
            const float c1 = m[5] * w.y + m[13];
            const float cw = m[11] * w.z + m[15];
            const float px = (c0 / cw + 1) * 320, py = (1 - c1 / cw) * 240;
            float& target = slopeType != 0 ? worstOther : worst;
            target = std::max(target, std::max(std::fabs(px - verts[i].x), std::fabs(py - verts[i].y)));
        }
        (slopeType != 0 ? others : tiles)++;
    }
    const auto now = std::chrono::steady_clock::now();
    if (now - last > std::chrono::seconds(1))
    {
        fprintf(stderr, "world3d: camera error max %.3f px over %d tiles (sloped blocks: %.3f over %d)\n", worst,
                tiles, worstOther, others);
        gLastError = worst;
        worst = worstOther = 0;
        tiles = others = 0;
        last = now;
    }
}
} // namespace world3d
