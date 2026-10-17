#pragma once

#include "core/ang16.hpp"
#include "core/function.hpp"
#include "particle_flags.hpp"

class Sprite;
class Explosion_30;

class Particle_4C
{
  public:
    char_type UpdateFloatingParticle_state_6_15_16_17_538060();
    char_type UpdateDirectedProjectile_state_3_12_5384C0();
    char_type UpdateBeamSegment_state_43_538A40();
    char_type UpdateObjectBeamLink_state_38_538AC0();
    char_type UpdateDirectedBurstSweep_state_4_539040();
    char_type UpdateDirectedBurst_state_13_14_36_539480();
    char_type UpdateCircularBurst_state_5_539890();
    char_type UpdateStaticAnim_state_39_53A180();
    char_type UpdateSkidOrScrapeSpark_state_40_41_53A280();
    char_type Empty_state_42_53AB70();
    char_type UpdateSimpleBallisticMotion_state_1_53ABA0();
    char_type UpdateLargeBallisticDebris_state_35_53AE60();
    char_type Empty_state_44_53B170();
    bool UpdateDebrisArc_state_7_53B1A0();
    char_type UpdateShortAnim_state_37_53B580();
    char_type UpdateAttachedEmitter_state_9_10_53B670();
    char_type UpdateBurstAnimation_state_29_30_53B9F0();
    char_type UpdateCollisionBurst_state_31_34_53BAC0();
    char_type PoolUpdate();
    void PoolAllocate();
    void PoolDeallocate();

    s32 field_0_id;
    union
    {
        s32 field_4_flags;
        ParticleFlags32 field_4_bf;
    };
    Fix16 field_8_speed_x;
    Fix16 field_C_speed_y;
    Fix16 field_10;
    Fix16 field_14_additional_speed_x;
    Fix16 field_18_additional_speed_y;
    Fix16 field_1C;
    Fix16 field_20_speed;
    Ang16 field_24_angle;
    s16 field_26_unused;
    Sprite* field_28_pSprite;
    u16 field_2C_counter;
    u16 field_2E;
    Sprite* field_30_pNext;
    s32 field_34_unused;
    s32 field_38_state;
    Particle_4C* mpNext; // 0x3C
    Explosion_30* field_40_pExplosion;
    s16 field_44;
    u8 field_46_sub_state;
    char_type field_47_unused;
    u8 field_48_timer;
    char_type field_49_unused;
    char_type field_4A_unused;
    char_type field_4B_unused;
};