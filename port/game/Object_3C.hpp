#pragma once

#include "Fix16_Point.hpp"
#include "Function.hpp"
#include "ang16.hpp"
#include "fix16.hpp"

EXTERN_GLOBAL(u32, gObj3C_id_6F8E54);
EXTERN_GLOBAL(u32, gObj3C_dealloc_count_6F8F0C);
EXTERN_GLOBAL(Ang16, kZeroAng_6F8F68);

class Sprite;
class AttachedSprite_18;
class Object_2C;

class SpriteList_4
{
  public:
    SpriteList_4() // 424620 inline
    {
        ResetHead_4207E0();
    }

    void ResetHead_4207E0() // inline
    {
        field_0_p18 = nullptr;
    }

    AttachedSprite_18* GetSpriteForModel_5A6A50(s32 obj_type);
    Object_2C* FindObject2CByModel_5A6A90(s32 obj_type);
    AttachedSprite_18* FindFirstActiveObject_5A6AD0();
    void RemoveSprite_5A6B10(Sprite* toFind);
    void RemoveSpriteSafe_5A6B60(Sprite* toFind);
    void sub_5A6BB0();
    void sub_5A6BD0();
    void DispatchCarImpactEvents_5A6BF0(Sprite* pSprite);
    char_type TagSpriteWithRng_5A6C10(Sprite* toFind);
    void RemoveByRngValue_5A6C40(s32 toFind);
    Sprite* FirstSpriteOfType_5A6CA0(s32 sprite_type);
    void AddSprite_5A6CD0(Sprite* pSprite);
    void PushImpactEvent_5A6D00(Sprite* pSprite1, Fix16 x, Fix16 y, Ang16 angle);
    void PushSprite_5A6D40(Sprite* pToFind);
    char_type SpriteExists_5A6D80(Sprite* pToFind);
    Sprite* PopFrontSprite_5A6DA0();
    Sprite* PopBackSprite_5A6DC0();
    AttachedSprite_18* ClearList_5A6E10();
    Sprite* FindClosestSprite_5A6E40(Fix16 xOff, Fix16 yOff);
    Sprite* TakeClosestSprite_5A6EA0(Fix16 xpos, Fix16 ypos);

    void PoolUpdate_5A6F70(Sprite* pSprite);
    void DestroyAllSprites_5A7010();
    void CleanupSpriteList_5A7080();
    void ClearGangIconSprite_5A7110();
    s32 GetGangIdxFromSpriteIfAny_5A71A0();
    void sub_5A71F0();
    void PruneNonCollidingSprites_5A7240(Sprite* pSprite);
    void PropagateMaxZLayer_5A72B0(Sprite* pSprite, char_type bUnknown);
    char_type sub_5A7310();

    AttachedSprite_18* field_0_p18;
};

class Object_3C
{
  public:
    Object_3C(); // 0x52ad80
    ~Object_3C(); // 0x52ade0

    // 9.6f 0x482B20
    void PoolAllocate()
    {
        ++gObj3C_id_6F8E54;
        field_C_speed = 0;
        field_4_angle = kZeroAng_6F8F68;
        field_18_friction = Fix16(0);
        field_28_next_definition_timer = 0;
        field_38_conveyor_speed = 0;
        field_34 = 2;
        field_24 = 0;
        field_2F_bOnSlope = 0;
        field_30_bSkipAnim = 0;
    }

    // 9.6f 0x482B80
    void PoolDeallocate()
    {
        --gObj3C_id_6F8E54;
        ++gObj3C_dealloc_count_6F8F0C;
    }

    void ClearF38_482BD0()
    {
        field_38_conveyor_speed = 0;
    }

    // ?? not sure if this is O3C either :skull:
    Fix16_Point GetSpeedVector_52ADF0();

    // 9.6f 0x482BA0; GetSpeedVector_52ADF0 is its out-of-line copy. It returns a Fix16_Point (which has a
    // destructor), so VC6 never expands it: it is charged to the caller's inline budget and called out of
    // line (GetMovementSpeedAndAngle_521FD0)
    Fix16_Point GetSpeedVector_482BA0()
    {
        Fix16_Point p;
        p.FromPolar_41E210(field_C_speed, field_4_angle);
        return p;
    }

    void GetMovementSpeedAndAngle_521FD0(Fix16& Speed, Ang16& Angle);

    SpriteList_4 field_0;
    Ang16 field_4_angle;
    s16 field_6_unused;
    Object_3C* mpNext;
    Fix16 field_C_speed;
    Fix16 field_10_z_speed;
    Fix16 field_14;
    Fix16 field_18_friction;
    Fix16 field_1C_z_accel;
    s32 field_20_obj2c_id;
    s32 field_24;
    s16 field_28_next_definition_timer;
    s16 field_2A_bAirborne;
    s16 field_2C_unused;
    char_type field_2E_unused;
    char_type field_2F_bOnSlope;
    u8 field_30_bSkipAnim;
    u8 field_31_unused;
    u16 field_32_unused;
    s32 field_34;
    char_type field_38_conveyor_speed;
    char_type field_39_unused;
    char_type field_3A_unused;
    char_type field_3B_unused;
};

EXTERN_GLOBAL(SpriteList_4, stru_67727C);
