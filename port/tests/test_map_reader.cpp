// Map_0x370's block queries on a synthetic two-column map built in memory (the .gmp loader itself,
// which reads chunks from a file, is covered by the replay corpus). Characterisation as ported.
#include <doctest.h>

#include <cstring>
#include <vector>

#include "map_0x370.hpp"

EXTERN_GLOBAL(gmp_block_info, gBlockInfo2_6F6028);

namespace
{
struct SyntheticMap
{
    // Column 0 (at base[0][0]): height 4, offset 1, blocks for z = 1, 2, 3.
    // Column 1 (at base[0][1]): height 2, offset 0, blocks for z = 0, 1.
    // Every other square uses column 0.
    SyntheticMap()
    {
        // A column is one header word (height, offset, 2 bytes padding) and one word per block.
        words = {4U | (1U << 8), 0, 1, 2, 2U | (0U << 8), 3, 4};
        blocks.resize(5);
        for (auto& b : blocks)
        {
            std::memset(&b, 0, sizeof(b));
        }
        blocks[0].field_B_slope_type = PAVEMENT;
        blocks[1].field_B_slope_type = AIR;
        blocks[2].field_B_slope_type = PARTIAL_BLOCK_LEFT | ROAD;
        blocks[3].field_B_slope_type = FIELD;
        blocks[4].field_B_slope_type = TRIANGULAR_SIDES_DIAGONAL_UP_LEFT | ROAD;

        dmap = new gmp_compressed_map_32();
        std::memset(dmap->field_0_base, 0, sizeof(dmap->field_0_base));
        dmap->field_0_base[0][1] = 4;
        dmap->field_40008_pColumn = reinterpret_cast<gmp_column_word*>(words.data());
        dmap->field_4000C_block = blocks.data();
        map.field_0_pDmap = dmap;
    }

    ~SyntheticMap()
    {
        map.field_0_pDmap = nullptr;
        delete dmap;
    }

    SyntheticMap(const SyntheticMap&) = delete;
    SyntheticMap& operator=(const SyntheticMap&) = delete;

    std::vector<u32> words;
    std::vector<gmp_block_info> blocks;
    gmp_compressed_map_32* dmap;
    Map_0x370 map;
};
} // namespace

TEST_CASE("get_block finds the block of a column at a height, or nothing outside the column")
{
    SyntheticMap m;
    CHECK(m.map.get_block_4DFE10(0, 0, 1) == &m.blocks[0]);
    CHECK(m.map.get_block_4DFE10(0, 0, 3) == &m.blocks[2]);
    CHECK(m.map.get_block_4DFE10(0, 0, 0) == nullptr); // below the column's offset
    CHECK(m.map.get_block_4DFE10(0, 0, 4) == nullptr); // at its height
    CHECK(m.map.get_block_4DFE10(1, 0, 0) == &m.blocks[3]);
    CHECK(m.map.get_block_4DFE10(1, 0, 2) == nullptr);
    CHECK(m.map.get_block_4DFE10(9, 9, 2) == &m.blocks[1]); // every other square: column 0

    CHECK(m.map.get_block_452980(1, 0, 1) == &m.blocks[4]);
    CHECK(m.map.get_block_452980(1, 0, 2) == nullptr);
}

TEST_CASE("GetBlockClamped keeps the coordinates on the map")
{
    SyntheticMap m;
    CHECK(m.map.GetBlockClamped_4DFEE0(-5, -5, 1) == &m.blocks[0]);
    CHECK(m.map.GetBlockClamped_4DFEE0(1000, 1000, 2) == &m.blocks[1]);
    CHECK(m.map.GetBlockClamped_4DFEE0(0, -1, 3) == &m.blocks[2]);
    CHECK(m.map.GetBlockClamped_4DFEE0(0, 0, 0) == nullptr);
}

TEST_CASE("GetEffectiveBlock hides partial blocks and makes triangular ones solid")
{
    SyntheticMap m;
    CHECK(m.map.GetEffectiveBlock_4DFE60(0, 0, 1) == &m.blocks[0]);
    CHECK(m.map.GetEffectiveBlock_4DFE60(0, 0, 3) == nullptr); // partial
    CHECK(m.map.GetEffectiveBlock_4DFE60(1, 0, 1) == &gBlockInfo2_6F6028); // triangular
    CHECK(m.map.GetEffectiveBlock_4DFE60(1, 0, 5) == nullptr);
}

TEST_CASE("FindHighestBlockForCoord returns the topmost block that is not air")
{
    SyntheticMap m;
    s32 z = -1;
    CHECK(m.map.FindHighestBlockForCoord_4E4C30(0, 0, &z) == &m.blocks[2]);
    CHECK(z == 3);
    CHECK(m.map.FindHighestBlockForCoord_4E4C30(1, 0, &z) == &m.blocks[4]);
    CHECK(z == 1);
}

TEST_CASE("FindNonAirBlockAtOrBelowZ walks down past air and reports the height it stopped at")
{
    SyntheticMap m;
    s32 z = 2;
    CHECK(m.map.FindNonAirBlockAtOrBelowZ_4E4CB0(0, 0, z) == &m.blocks[0]); // z = 2 is air
    CHECK(z == 1);

    z = 9; // above the column: starts at its top
    CHECK(m.map.FindNonAirBlockAtOrBelowZ_4E4CB0(0, 0, z) == &m.blocks[2]);
    CHECK(z == 3);

    z = 0; // below the column
    CHECK(m.map.FindNonAirBlockAtOrBelowZ_4E4CB0(0, 0, z) == nullptr);
}
