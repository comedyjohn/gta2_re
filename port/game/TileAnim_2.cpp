#include "TileAnim_2.hpp"

#include "core/enums.hpp"
#include "data/gtx.hpp"
#include "error.hpp"
#include "map_0x370.hpp"
#include <cstdio>

DEFINE_GLOBAL(TileAnimSet*, gTileAnim_2_7052C4, 0x7052C4);
DEFINE_GLOBAL(TileAnimPool*, gTileAnimPool_7052C8, 0x7052C8);

// 0x5bc130
void TileAnimation::PoolAllocate()
{
    field_12_idx = gTileAnim_2_7052C4->field_0_count++;
}

// 0x5bc150
void TileAnimation::ApplyCurrentFrame_5BC150()
{
    if (field_C_ptr != nullptr)
    {
        gGtx->SetTileRemap(field_10_base, field_C_ptr->field_6_tiles[field_8_current_frame]);
        return;
    }
    gGtx->SetTileRemap(field_10_base, field_8_current_frame);
}

// 0x5bc190
void TileAnimation::InitFromGmpAnim_5BC190(const GmpTileAnimation* pStru)
{
    field_C_ptr = pStru;
    field_4_frame_rate = pStru->field_2_frame_rate;
    field_6_repeat = pStru->field_3_repeat;
    field_10_base = pStru->field_0_base;
    field_2_anim_length = pStru->field_4_anim_length - 1;
    field_0_start_frame = 0;
}

// 0x5bc1d0
void TileAnimation::Start_5BC1D0()
{
    field_A_frame_timer = field_4_frame_rate;
    field_8_current_frame = field_0_start_frame;
    ApplyCurrentFrame_5BC150();
}

// 0x5bc1f0
char_type TileAnimation::PoolUpdate()
{
    u8 bAtEnd;

    field_A_frame_timer--;
    if (field_A_frame_timer == 0)
    {
        if (field_2_anim_length < field_0_start_frame)
        {
            field_8_current_frame--;
            bAtEnd = field_8_current_frame < field_2_anim_length;
        }
        else
        {
            field_8_current_frame++;
            bAtEnd = field_8_current_frame > field_2_anim_length;
        }
        if (bAtEnd)
        {
            if (field_6_repeat > 0)
            {
                field_6_repeat--;
                if (field_6_repeat == 0)
                {
                    return 1;
                }
            }
            field_8_current_frame = field_0_start_frame;
        }
        ApplyCurrentFrame_5BC150();
        field_A_frame_timer = field_4_frame_rate;
    }
    return 0;
}

// 0x5bc260
void TileAnimSet::AddAnim_5BC260(s16 base, s16 f0, s16 length, s16 frame_rate, s16 repeat)
{
    TileAnimation* p18 = gTileAnimPool_7052C8->Allocate();
    p18->Init_4C33F0(base, f0, length, frame_rate, repeat);
    p18->Start_5BC1D0();
}

// 0x5bc2c0
void TileAnimSet::AddGmpAnim_5BC2C0(const GmpTileAnimation* pTileAnimation)
{
    TileAnimation* tmp = gTileAnimPool_7052C8->Allocate();
    tmp->InitFromGmpAnim_5BC190(pTileAnimation);
    tmp->Start_5BC1D0();
}

// 0x5bc300
void TileAnimSet::Empty_5BC300()
{
}

// 0x5bc310
void TileAnimSet::UpdateTileAnimations_5BC310()
{
    gTileAnimPool_7052C8->field_0_pool.UpdatePool();
}

// 0x5bc3a0
TileAnimSet::TileAnimSet()
{
    if (gTileAnimPool_7052C8 == nullptr)
    {
        gTileAnimPool_7052C8 = new TileAnimPool();
        if (gTileAnimPool_7052C8 == nullptr)
        {
            FatalError_4A38C0(Gta2Error::kInvalidMapObjectData, "C:\\Splitting\\Gta2\\Source\\tileanim.cpp", 220);
        }
    }
    field_0_count = 1;
}

// 0x5bc470
TileAnimSet::~TileAnimSet()
{
    if (gTileAnimPool_7052C8)
    {
        GTA2_DELETE_AND_NULL(gTileAnimPool_7052C8);
    }
}

// 0x5bc4a0
TileAnimPool::~TileAnimPool() = default;

// 0x5beba0
TileAnimation::TileAnimation()
{
    field_0_start_frame = 0;
    field_2_anim_length = 0;
    field_4_frame_rate = 0;
    field_6_repeat = 0;
    field_8_current_frame = 0;
    field_A_frame_timer = 0;
    field_C_ptr = nullptr;
    field_10_base = 0;
    field_12_idx = 0;
    next_ = nullptr;
}

// 0x5bebd0
TileAnimation::~TileAnimation()
{
    field_C_ptr = nullptr;
    next_ = nullptr;
}