#include "game_view.h"

#include "Camera.hpp"
#include "GameSession_578.hpp"
#include "MapRenderer.hpp"
#include "gtx_0x106C.hpp"
#include "map_0x370.hpp"
#include "TextureCache_15D8.hpp"

namespace world3d
{
static_assert(sizeof(gmp_block_info) == 12, "upstream's Map::LoadFromDmap reads 12-byte blocks");

const void* MapIdentity()
{
    if (!gMap_0x370_6F6268 || !gMap_0x370_6F6268->field_0_pDmap)
    {
        return nullptr;
    }
    return gMap_0x370_6F6268->field_0_pDmap;
}

bool ReadMap(gta2::Map& out, std::string* error)
{
    if (!MapIdentity())
    {
        if (error)
        {
            *error = "no map loaded";
        }
        return false;
    }
    const gmp_compressed_map_32* dmap = gMap_0x370_6F6268->field_0_pDmap;
    return out.LoadFromDmap(reinterpret_cast<const uint8_t*>(dmap->field_0_base),
                            reinterpret_cast<const uint8_t*>(dmap->field_40008_pColumn),
                            static_cast<size_t>(dmap->field_40000_column_words) * 4,
                            reinterpret_cast<const uint8_t*>(dmap->field_4000C_block),
                            static_cast<size_t>(dmap->field_40004_num_blocks) * 12, error);
}

void ReadSlopes(gta2::SlopeInfo out[gta2::kSlopeTypeCount])
{
    for (int i = 0; i < gta2::kSlopeTypeCount; i++)
    {
        out[i].direction = gGmpSlopes_6F5BA8[i].field_0_gradient_direction;
        out[i].steps = gGmpSlopes_6F5BA8[i].field_1_gradient_size;
        out[i].step = gGmpSlopes_6F5BA8[i].field_2_gradient_level;
    }
}

std::string StylePath()
{
    const char* name = gGameSession_67E8E0.GetStyleName_4C5950();
    return name ? name : "";
}
} // namespace world3d
