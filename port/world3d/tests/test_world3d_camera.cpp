// The clip transform against the game's own projection (port/world3d/PLAN.md 2.2)
#include <doctest.h>

#include "../camera.h"

#include <cmath>

using namespace world3d;

TEST_CASE("camera: the clip matrix lands where the game's projection puts a point")
{
    GameCamera cam;
    cam.camX = 100.5f;
    cam.camY = 80.25f;
    cam.camZ = 2;
    cam.f = 640;
    cam.cx = 320;
    cam.cy = 240;
    cam.valid = true;
    const float W = 640, H = 480;
    float m[16];
    ClipMatrix(cam, W, H, m);

    const float pts[][3] = {{100.5f, 80.25f, 0}, {100.5f, 80.25f, 5}, {97.0f, 78.0f, 0}, {104.0f, 82.5f, 3},
                            {100.5f + 5, 80.25f - 3.75f, 1}, {99.0f, 83.0f, 2.5f}, {101, 79, 4}};
    for (const auto& p : pts)
    {
        float x[4] = {p[0], p[1], p[2], 1};
        float c[4];
        for (int r = 0; r < 4; r++)
        {
            c[r] = 0;
            for (int k = 0; k < 4; k++)
            {
                c[r] += m[k * 4 + r] * x[k];
            }
        }
        const float ndcX = c[0] / c[3], ndcY = c[1] / c[3], ndcZ = c[2] / c[3];
        const float px = (ndcX + 1) * W / 2, py = (1 - ndcY) * H / 2;
        float sx, sy, invD;
        GameProject(cam, p[0], p[1], p[2], sx, sy, invD);
        CHECK(std::fabs(px - sx) < 0.01f);
        CHECK(std::fabs(py - sy) < 0.01f);
        CHECK(std::fabs(c[3] * invD - 1.0f) < 1e-5f); // w is D
        CHECK(ndcZ > -1.0f);
        CHECK(ndcZ < 1.0f);
    }
}
