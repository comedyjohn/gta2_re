#pragma once

#include "Function.hpp"
#include "fix16.hpp"

class Car_BC;
class Ped;

// Firefighter_28::field_8_state (stored as s32)
namespace firefighter_state
{
enum
{
    idle_0 = 0,
    spawn_truck_1 = 1, // waiting for a fire truck to spawn at the dispatch point
    drive_to_fire_2 = 2,
    arrived_3 = 3,
    put_out_fire_4 = 4,
    finished_5 = 5, // truck goes back to normal AI, slot is reset
    abort_6 = 6, // truck lost, despawned or the target is gone: deinit
};
} // namespace firefighter_state

class Firefighter_28
{
  public:
    // inline
    void Clear_450C10()
    {
        field_C_target_car = nullptr;
        field_8_state = firefighter_state::idle_0;
        field_4_bActive = 0;
        field_1C_car = nullptr;
        field_20_ped = nullptr;
        field_24_next_state_timer = 0;
    }

    bool sub_4A7FC0();
    void deinit_4A81A0();
    void Update_4A81F0();
    void init_4A85C0();
    void Reset_4A85E0();

    s16 field_0_id;
    s16 field_2_unused;
    s32 field_4_bActive;
    s32 field_8_state; // firefighter_state
    Car_BC* field_C_target_car;
    Fix16 field_10_xpos;
    Fix16 field_14_ypos;
    Fix16 field_18_zpos;
    Car_BC* field_1C_car;
    Ped* field_20_ped;
    u16 field_24_next_state_timer;
    s16 field_26_unused;
};

class FirefighterPool_54
{
  public:
    FirefighterPool_54()
    {
        for (auto& firefighter : field_0_firefighters)
        {
            firefighter.init_4A85C0();
        }
        ResetCount_4A88D0();
    }

    ~FirefighterPool_54()
    {
        // TODO: Should this be empty?
    }

    void FireEnginesService_4A85F0();
    Firefighter_28* DispatchFirefighters_4A8620(Car_BC* pCar, Fix16 x, Fix16 y, Fix16 z);
    Firefighter_28* New28_4A8800();
    char_type TryDispatchFirefightersToCar_4A8820(Car_BC* pCar);
    char_type IsFireEngineSentToCar_4A8890(Car_BC* pFireEngine, Car_BC* pTargetCar);
    void ResetCount_4A88D0();

    Firefighter_28 field_0_firefighters[2];
    s16 field_50_count;
    s16 field_52_unused;
};

EXTERN_GLOBAL(FirefighterPool_54*, gFirefighterPool_54_67D4C0);
