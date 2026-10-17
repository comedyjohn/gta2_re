#pragma once

#include "Function.hpp"

class Ped;

class Car_Door_10
{
  public:
    // 9.6f 0x4341B0
    Ped* get_pObj_4341B0()
    {
        return field_8_pObj;
    }

    void AnimateOpening_439CD0(u32* pRet);
    void AnimateClosing_439D40(u32* pDeltaMask);
    void Service_439DA0(u32* a3a);
    void Init_439E40(u8 door_idx);
    void Open_439E60();
    void Close_439EA0();
    Car_Door_10(); // 447330
    ~Car_Door_10(); // 447350

    // 9.6f 0x421360
    bool IsStateActive_421360()
    {
        return field_4_state != 0 && field_4_state != 6;
    }

    // 9.6f 0x421340
    void ResetState_421340()
    {
        field_4_state = 0;
    }

    // Inlined in sound_obj::HandleCarDoorSounds_4182E0
    bool IsStartingToOpen()
    {
        return field_4_state == 2 && field_0_animation_frame == 0 && field_1_frame_delay == 0;
    }

    // Inlined in sound_obj::HandleCarDoorSounds_4182E0
    bool IsStartingToClose()
    {
        return field_4_state == 3 && field_0_animation_frame == 1;
    }

    // 9.6f inline 0x421380
    void set_ped_421380(Ped* pPed)
    {
        field_8_pObj = pPed;
    }

    s8 field_0_animation_frame;
    s8 field_1_frame_delay;
    s8 field_2_first_delta_idx;
    s8 field_3_unused;
    s32 field_4_state;
    Ped* field_8_pObj;
    u8 field_C_stay_open_timer;
    s8 field_D_unused;
    s8 field_E_unused;
    s8 field_F_unused;
};
