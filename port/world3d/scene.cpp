#include "scene.h"

#include <chrono>
#include <cmath>
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

namespace
{
// The block a triangle belongs to, from a point just inside it
const gta2::Block* BlockOf(const gta2::Map& map, float x, float y, float z)
{
    const int cx = static_cast<int>(std::floor(x)), cy = static_cast<int>(std::floor(y));
    const int level = static_cast<int>(std::floor(z));
    if (cx < 0 || cy < 0 || cx >= gta2::kMapWidth || cy >= gta2::kMapHeight)
    {
        return nullptr;
    }
    const gta2::Column& column = map.ColumnAt(cx, cy);
    if (level < column.offset || level >= column.height || level - column.offset >= static_cast<int>(column.blocks.size()))
    {
        return nullptr;
    }
    return &map.BlockAt(column.blocks[level - column.offset]);
}

uint8_t KindOfTriangle(const gta2::Map& map, const gta2::Vertex& a, const gta2::Vertex& b, const gta2::Vertex& c)
{
    const float cx = (a.x + b.x + c.x) / 3, cy = (a.y + b.y + c.y) / 3, cz = (a.z + b.z + c.z) / 3;
    const float nx = a.nx, ny = a.ny, nz = a.nz;
    const gta2::Block* block = BlockOf(map, cx - nx * 0.01f, cy - ny * 0.01f, cz - nz * 0.01f);
    if (!block)
    {
        return kShadeLid0;
    }
    const int slope = block->SlopeType();
    if (slope >= 49 && slope <= 61)
    {
        return kShadeClassic;
    }
    if (nz > 0.5f)
    {
        const int sel = (block->lid.raw >> 10) & 3;
        if (sel == 0)
        {
            return slope >= 1 && slope <= 44 ? kShadeRamp : kShadeLid0;
        }
        return static_cast<uint8_t>(sel); // kShadeT1..3
    }
    if (nz < -0.5f)
    {
        return kShadeLid0;
    }
    if (std::fabs(nx) > 0.9f)
    {
        return nx < 0 ? kShadeT1 : kShadeEast;
    }
    if (std::fabs(ny) > 0.9f)
    {
        return ny < 0 ? kShadeT3 : kShadeT2;
    }
    if (nx < 0)
    {
        return ny < 0 ? kShadeDiagUpLeft : kShadeDiagDownLeft;
    }
    return ny < 0 ? kShadeDiagUpRight : kShadeDiagDownRight;
}
} // namespace

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
    scene.kind.assign(scene.mesh.vertices.size(), kShadeLid0);
    for (size_t i = 0; i + 2 < scene.mesh.indices.size(); i += 3)
    {
        const uint32_t i0 = scene.mesh.indices[i], i1 = scene.mesh.indices[i + 1], i2 = scene.mesh.indices[i + 2];
        const uint8_t k = KindOfTriangle(scene.map, scene.mesh.vertices[i0], scene.mesh.vertices[i1], scene.mesh.vertices[i2]);
        scene.kind[i0] = scene.kind[i1] = scene.kind[i2] = k;
    }
    scene.identity = identity;
    scene.valid = true;
    scene.buildMs = std::chrono::duration<float, std::milli>(std::chrono::steady_clock::now() - t0).count();
    std::fprintf(stderr, "world3d: mesh built in %.0f ms: %zu vertices, %zu triangles, %zu batches\n", scene.buildMs,
                 scene.mesh.vertices.size(), scene.mesh.indices.size() / 3, scene.mesh.batches.size());
    return true;
}
} // namespace world3d
