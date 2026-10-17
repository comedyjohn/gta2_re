#include "game_view.h"
#include "camera.h"

#include "Camera.hpp"
#include "GameSession_578.hpp"
#include "MapRenderer.hpp"
#include "TextureCache_15D8.hpp"
#include "data/gtx.hpp"
#include "map_0x370.hpp"
#include "renderer.h"
#include "winmain.hpp"

namespace world3d
{
static_assert(sizeof(GmpBlockInfo) == 12, "upstream's Map::LoadFromDmap reads 12-byte blocks");

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
    const GmpCompressedMap* dmap = gMap_0x370_6F6268->field_0_pDmap;
    return out.LoadFromDmap(reinterpret_cast<const uint8_t*>(dmap->field_0_base),
                            reinterpret_cast<const uint8_t*>(dmap->field_40008_pColumn),
                            static_cast<size_t>(dmap->field_40000_column_words) * 4,
                            reinterpret_cast<const uint8_t*>(dmap->field_4000C_block),
                            static_cast<size_t>(dmap->field_40004_num_blocks) * 12,
                            error);
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

int BlockSlopeType(int x, int y, int level)
{
    if (!MapIdentity())
    {
        return -1;
    }
    const GmpBlockInfo* block = gMap_0x370_6F6268->GetBlockClamped_4DFEE0(x, y, level);
    return block ? block->field_B_slope_type >> 2 : -1;
}

Shades ReadShades()
{
    Shades s;
    const MapRenderer* m = gpMapRenderer_6F66E4;
    s.lit = (gLightingDrawFlag_7068F4 & 0x8000) != 0;
    if (!m)
    {
        return s;
    }
    s.level[0] = 255;
    s.level[1] = m->field_C_colour_t1;
    s.level[2] = m->field_E_colour_t2;
    s.level[3] = m->field_F_colour_t3;
    s.level[4] = m->field_D_right_colour;
    s.level[5] = m->field_10_diag_up_left_colour;
    s.level[6] = m->field_11_diag_up_right_colour;
    s.level[7] = m->field_12_diag_down_left_colour;
    s.level[8] = m->field_13_diag_down_right_colour;
    s.level[9] = m->field_14_dcolour;
    return s;
}

unsigned TileTexture(int tile)
{
    if (!gGtx || !gTextureCache_15D8_705064)
    {
        return 0;
    }
    u16 idx = gGtx->GetTileMapping(static_cast<u16>(tile));
    if (!idx)
    {
        return 0;
    }
    return port_renderer_texture_id(gTextureCache_15D8_705064->GetTexture_46BB50(idx));
}

std::string StylePath()
{
    const char* name = gGameSession_67E8E0.GetStyleName_4C5950();
    return name ? name : "";
}
} // namespace world3d

namespace world3d
{
GameCamera ReadCamera()
{
    GameCamera cam;
    const Camera_0xBC* c = gViewCamera_676978;
    if (!c)
    {
        return cam;
    }
    cam.camX = c->field_98_cur_pos.field_0_x.ToFloat();
    cam.camY = c->field_98_cur_pos.field_4_y.ToFloat();
    cam.camZ = c->field_98_cur_pos.field_8_z.ToFloat();
    cam.f = c->field_60_screen_scale.x.ToFloat();
    cam.cx = static_cast<float>(c->field_70_screen_px_center_x);
    cam.cy = static_cast<float>(c->field_74_screen_px_center_y);
    cam.valid = true;
    return cam;
}
} // namespace world3d
