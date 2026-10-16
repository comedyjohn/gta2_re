#include "scene.h"

#include <chrono>
#include <cstdio>

namespace world3d
{
void MeshToGame(gta2::Vertex& v)
{
    const float y = v.y, z = v.z, ny = v.ny, nz = v.nz;
    v.y = static_cast<float>(gta2::kMapHeight) - z;
    v.z = y;
    v.ny = -nz;
    v.nz = ny;
}

bool UpdateScene(Scene& scene, bool dirty)
{
    const void* identity = MapIdentity();
    if (!identity)
    {
        scene.valid = false;
        scene.identity = nullptr;
        return false;
    }
    std::string error;
    const std::string path = StylePath();
    if (path != scene.stylePath)
    {
        scene.valid = false;
        scene.stylePath = path;
        dirty = true;
        if (!scene.style.Load(path, &error))
        {
            std::fprintf(stderr, "world3d: style %s: %s\n", path.c_str(), error.c_str());
            scene.stylePath.clear();
            return false;
        }
    }
    if (!dirty && identity == scene.identity && scene.valid)
    {
        return false;
    }
    const auto t0 = std::chrono::steady_clock::now();
    if (!ReadMap(scene.map, &error))
    {
        std::fprintf(stderr, "world3d: map: %s\n", error.c_str());
        scene.valid = false;
        return false;
    }
    ReadSlopes(scene.slopes);
    gta2::SetWorldSeal(false);
    gta2::SetRiverbedDepth(0); // the black background already does what they are for
    scene.mesh = gta2::WorldMesh();
    gta2::BuildWorldMesh(scene.map, scene.style, scene.slopes, gta2::PartialCuts(), &scene.mesh);
    for (gta2::Vertex& v : scene.mesh.vertices)
    {
        MeshToGame(v);
    }
    scene.identity = identity;
    scene.valid = true;
    scene.buildMs = std::chrono::duration<float, std::milli>(std::chrono::steady_clock::now() - t0).count();
    std::fprintf(stderr, "world3d: mesh built in %.0f ms: %zu vertices, %zu triangles, %zu batches\n", scene.buildMs,
                 scene.mesh.vertices.size(), scene.mesh.indices.size() / 3, scene.mesh.batches.size());
    return true;
}
} // namespace world3d
