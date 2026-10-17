#pragma once

#include "core/function.hpp"
#include "core/pool.hpp"
#include <cstdio>

struct GmpTileAnimation;

class TileAnimSet
{
  public:
    void AddAnim_5BC260(s16 base, s16 f0, s16 length, s16 frame_rate, s16 repeat);
    void AddGmpAnim_5BC2C0(const GmpTileAnimation* pTileAnimation);
    void Empty_5BC300();
    void UpdateTileAnimations_5BC310();
    TileAnimSet();
    ~TileAnimSet();
    s16 field_0_count;
};

class TileAnimation
{
  public:
    void Init_4C33F0(s16 base, s16 f0, s16 length, s16 frame_rate, s16 repeat)
    {
        field_10_base = base;
        field_0_start_frame = f0;
        field_2_anim_length = length;
        field_4_frame_rate = frame_rate;
        field_6_repeat = repeat;
        field_C_ptr = nullptr;
    }

    void PoolAllocate();
    void PoolDeallocate()
    {
    }
    void ApplyCurrentFrame_5BC150();
    void InitFromGmpAnim_5BC190(const GmpTileAnimation* pStru);
    void Start_5BC1D0();
    char_type PoolUpdate();
    TileAnimation();
    ~TileAnimation();

    u16 field_0_start_frame;
    u16 field_2_anim_length;
    s16 field_4_frame_rate;
    u16 field_6_repeat;
    u16 field_8_current_frame;
    s16 field_A_frame_timer;
    const GmpTileAnimation* field_C_ptr;
    s16 field_10_base;
    s16 field_12_idx;
    TileAnimation* next_;
};

class TileAnimPool
{
  public:
    // Inlined 0x4c34b0
    TileAnimPool() = default;

    ~TileAnimPool();

    TileAnimation* Allocate()
    {
        return field_0_pool.Allocate();
    }

    Pool<TileAnimation, 50> field_0_pool;
};

EXTERN_GLOBAL(TileAnimSet*, gTileAnim_2_7052C4);
