// Upstream's mesh builder on tiny maps made in memory (port/world3d/scene.h). The style has no tiles
// loaded, so the faces use tile 1: the builder keeps two extra slots past the style's tile count.
#include <doctest.h>

#include "../scene.h"

#include <cstring>
#include <vector>

using namespace world3d;

namespace
{
// A map with one column at `cell` holding `block` at level 0, built the way the game keeps the DMAP:
// base[256*256] column-word indices, a column pool and 12-byte blocks.
struct TinyMap
{
    std::vector<uint32_t> base = std::vector<uint32_t>(256 * 256, 0);
    std::vector<uint32_t> columns;
    std::vector<uint8_t> blocks;

    // `faces`: left, right, top, bottom, lid; slope byte
    TinyMap(bool withColumn, int x, int y, const uint16_t faces[5], uint8_t slope)
    {
        columns = {0, 0}; // word 0: an empty column (height 0, offset 0), padded
        blocks.assign(12 * 2, 0); // block 0: empty
        if (withColumn)
        {
            columns[1] = 0; // placeholder so word 1 starts the real column
            columns.push_back(0x00000001); // height 1, offset 0 (bytes 0, 1, 2, 3)
            columns.push_back(1); // level 0 holds block 1
            base[y * 256 + x] = 2;
            uint8_t b[12] = {};
            std::memcpy(b, faces, 10);
            b[11] = slope;
            std::memcpy(&blocks[12], b, 12);
        }
    }

    bool Load(gta2::Map& map, std::string* error)
    {
        return map.LoadFromDmap(reinterpret_cast<const uint8_t*>(base.data()),
                                reinterpret_cast<const uint8_t*>(columns.data()), columns.size() * 4, blocks.data(),
                                blocks.size(), error);
    }
};

gta2::WorldMesh Build(const gta2::Map& map, const gta2::SlopeInfo* slopes = nullptr)
{
    gta2::Style style;
    gta2::SetWorldSeal(false);
    gta2::SetRiverbedDepth(0);
    gta2::WorldMesh mesh;
    gta2::BuildWorldMesh(map, style, slopes, gta2::PartialCuts(), &mesh);
    for (gta2::Vertex& v : mesh.vertices)
    {
        MeshToGame(v);
    }
    return mesh;
}
} // namespace

TEST_CASE("mesh: an empty map has no triangles")
{
    const uint16_t none[5] = {};
    TinyMap tiny(false, 0, 0, none, 0);
    gta2::Map map;
    std::string error;
    REQUIRE(tiny.Load(map, &error));
    CHECK(Build(map).indices.empty());
}

TEST_CASE("mesh: a solid block stays inside its cell after the conversion to game coordinates")
{
    const uint16_t all[5] = {1, 1, 1, 1, 1};
    TinyMap tiny(true, 10, 20, all, 0);
    gta2::Map map;
    std::string error;
    REQUIRE(tiny.Load(map, &error));
    const gta2::WorldMesh mesh = Build(map);
    REQUIRE(!mesh.indices.empty());
    for (const gta2::Vertex& v : mesh.vertices)
    {
        CHECK(v.x >= 10.0f);
        CHECK(v.x <= 11.0f);
        CHECK(v.y >= 20.0f);
        CHECK(v.y <= 21.0f);
        CHECK(v.z >= 0.0f);
        CHECK(v.z <= 1.0f);
    }
}

TEST_CASE("mesh: a lid-only block faces up")
{
    const uint16_t lidOnly[5] = {0, 0, 0, 0, 1};
    TinyMap tiny(true, 3, 4, lidOnly, 0);
    gta2::Map map;
    std::string error;
    REQUIRE(tiny.Load(map, &error));
    const gta2::WorldMesh mesh = Build(map);
    REQUIRE(!mesh.vertices.empty());
    for (const gta2::Vertex& v : mesh.vertices)
    {
        CHECK(v.nz > 0.99f);
    }
}

TEST_CASE("mesh: a ramp rising north shares the edge height between its steps")
{
    gta2::SlopeInfo step0{1, 2, 0};
    gta2::SlopeInfo step1{1, 2, 1};
    const gta2::CornerHeights a = gta2::LidHeights(step0);
    const gta2::CornerHeights b = gta2::LidHeights(step1);
    // corners are NW, NE, SE, SW: the south edge of one step is the north edge of the next
    const bool aSouthIsBNorth = (a[2] == b[1] && a[3] == b[0]);
    const bool bSouthIsANorth = (b[2] == a[1] && b[3] == a[0]);
    CHECK((aSouthIsBNorth || bSouthIsANorth));
    CHECK(a[0] == a[1]); // north edge level
    CHECK(a[2] == a[3]); // south edge level
}
