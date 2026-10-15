#pragma once

#include "fix16.hpp"
#include "Function.hpp"
#include "Pool.hpp"

class ExplodingScore_50
{
  public:
    ExplodingScore_50();
    ~ExplodingScore_50();
    void PoolAllocate();
    void PoolDeallocate();
    void ClearNumbersArrayFrom_596A70(s32 start_idx);
    void InitScore_596A90(Fix16 xpos, Fix16 ypos, Fix16 zpos, u32 score);
    void DrawNumbers_596C90();
    char_type PoolUpdate();
    void DrawSingleNumber_597100(s32 number_to_draw, s32 xpos_sub);

    s32 field_0_numbers_count;
    s32 field_4_digits[9]; // Number components
    Fix16 field_28_x;
    Fix16 field_2C_y;
    Fix16 field_30_z;
    s16 field_34_palette;
    u8 field_36; // draw figure param
    char_type field_37_unused; // pad ?
    s32 field_38; // scale factor?
    s32 field_3C; // xoff ?
    s32 field_40; // yoff ?
    // NOTE: Used by a template so the name maters
    ExplodingScore_50* mpNext;
    s16 field_48_unused; // set to 0, never used ??
    s16 field_4A_unused; // pad?
    u32 field_4C_score;
};

class ExplodingScorePool
{
  public:
    ExplodingScorePool();
    ~ExplodingScorePool();
    s16 NextPalette_596860();
    void IncrementFreeScores_596880();
    void PushScore_596890(Fix16 xpos, Fix16 ypos, Fix16 zpos, u32 score);
    void Update_596940();
    void DrawExplodingScores_5969E0();

    u16 field_0_last_palette;
    u16 field_2_free_scores;

    // NOTE: Nearly all other pools start at offset 0, this object is some 
    // suggestion its composition and not inheritance unless this object 
    // has an additional unknown base that been melted by the compiler :']
    Pool<ExplodingScore_50, 3> field_4_pool;
};

EXTERN_GLOBAL(ExplodingScorePool*, gExplodingScorePool);
