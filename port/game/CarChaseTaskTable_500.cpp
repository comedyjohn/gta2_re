#include "CarChaseTaskTable_500.hpp"
#include "Ped.hpp"

DEFINE_GLOBAL(CarChaseTaskTable_500*, gCarChaseTaskTable_678E30, 0x678E30);

DEFINE_GLOBAL_INIT(Fix16, kFpZero_678D0C, Fix16(0), 0x678D0C);

// 0x4747b0
void CarChaseTask_40::ResetEntry_4747B0()
{
    field_0_in_use = 0;
    field_10_ram_target = 0;
    field_8_task_type = 0;
    field_14_target_x = kFpZero_678D0C;
    field_18_target_y = kFpZero_678D0C;
    field_1C_target_z = kFpZero_678D0C;
    field_20_can_snap_to_target = 0;
    field_21_unused = 0;
    field_22_following_route = 1;
    field_23_unused = 1;
    field_24 = 0;
    field_25_unused = 0;
    field_30_pTargetPed = nullptr;
    field_26_route_finished = 0;
    field_2A_settle_counter = 0;
    field_2C_side_counter = 0;
    field_2E_wait_counter = 0;
    field_C_chase_state = car_chase_state::follow_behind_0;
    field_34_snap_timer = 0;
    field_4_pDriver = nullptr;
    field_38_unused = 0;
    field_3C_block_counter = 0;
}

// 0x474810
CarChaseTask_40* CarChaseTaskTable_500::AllocateEntry_474810()
{
    for (auto& entry : field_0_entries)
    {
        if (!entry.field_0_in_use)
        {
            entry.field_0_in_use = 1;
            return &entry;
        }
    }
    return nullptr;
}

// 0x474850
bool CarChaseTaskTable_500::AreAllies_474850(Ped* pPed1, Ped* pPed2)
{
    if (pPed1->get_occupation_403980() < ped_ocupation_enum::police ||
        pPed1->get_occupation_403980() > ped_ocupation_enum::army_army) // ped 1 is not police
    {
        if (pPed2->field_17C_pGang == pPed1->field_17C_pGang) // they are from same gang (or both dont have any)
        {
            return true;
        }
    }
    else
    {
        if (pPed2->get_occupation_403980() >= ped_ocupation_enum::police &&
            pPed2->field_240_occupation <= ped_ocupation_enum::army_army) // both ped 1 and ped 2 are police feds
        {
            return true;
        }
    }
    return false;
}

// 0x4748a0
Ped* CarChaseTaskTable_500::FindDriverInState_4748A0(s32 chase_state, Ped* pTargetPed)
{
    for (auto& entry : field_0_entries)
    {
        if (entry.field_0_in_use == 1 && entry.field_30_pTargetPed == pTargetPed && AreAllies_474850(pTargetPed, entry.field_4_pDriver) &&
            chase_state == entry.field_C_chase_state)
        {
            return entry.field_4_pDriver;
        }
    }
    return nullptr;
}

// 0x474920
u8 CarChaseTaskTable_500::CountPursuers_474920(Ped* pTargetPed, Ped* pPed)
{
    u8 total = 0;
    for (auto& entry : field_0_entries)
    {
        if (entry.field_0_in_use == 1 && entry.field_30_pTargetPed == pTargetPed)
        {
            if (AreAllies_474850(pPed, entry.field_4_pDriver))
            {
                total++;
            }
        }
    }
    return total;
}

// 0x474970
bool CarChaseTaskTable_500::HasAnyPursuer_474970(Ped* pTargetPed)
{
    for (auto& entry : field_0_entries)
    {
        if (entry.field_0_in_use == 1 && entry.field_30_pTargetPed == pTargetPed)
        {
            return true;
        }
    }
    return false;
}

// 0x4749b0
bool CarChaseTaskTable_500::HasPursuerInFront_4749B0(Ped* pPed)
{
    for (auto& entry : field_0_entries)
    {
        if (entry.field_0_in_use == 1)
        {
            if (AreAllies_474850(pPed, entry.field_4_pDriver))
            {
                switch (entry.field_C_chase_state)
                {
                    case car_chase_state::in_front_stopping_13:
                    case car_chase_state::target_stopped_15:
                        return true;
                    default:
                        break;
                }
            }
        }
    }
    return false;
}

// 0x474a20
bool CarChaseTaskTable_500::HasPursuerAlongsideLeft_474A20(Ped* pPed)
{
    for (auto& entry : field_0_entries)
    {
        if (entry.field_0_in_use == 1)
        {
            if (AreAllies_474850(pPed, entry.field_4_pDriver) && entry.field_C_chase_state == car_chase_state::alongside_left_8)
            {
                return true;
            }
        }
    }
    return false;
}

// 0x474a80
bool CarChaseTaskTable_500::HasPursuerOnLeft_474A80(Ped* pPed)
{
    for (auto& entry : field_0_entries)
    {
        if (entry.field_0_in_use == 1)
        {
            if (AreAllies_474850(pPed, entry.field_4_pDriver))
            {
                switch (entry.field_C_chase_state)
                {
                    case car_chase_state::approach_left_6:
                    case car_chase_state::alongside_left_8:
                    case car_chase_state::pull_ahead_left_10:
                        return true;
                    default:
                        break;
                }
            }
        }
    }
    return false;
}

// 0x474af0
bool CarChaseTaskTable_500::HasPursuerAlongsideRight_474AF0(Ped* pPed)
{
    for (auto& entry : field_0_entries)
    {
        if (entry.field_0_in_use == 1)
        {
            if (AreAllies_474850(pPed, entry.field_4_pDriver) && entry.field_C_chase_state == car_chase_state::alongside_right_9)
            {
                return true;
            }
        }
    }
    return false;
}

// 0x474b50
bool CarChaseTaskTable_500::HasPursuerOnRight_474B50(Ped* pPed)
{
    for (auto& entry : field_0_entries)
    {
        if (entry.field_0_in_use == 1)
        {
            if (AreAllies_474850(pPed, entry.field_4_pDriver))
            {
                switch (entry.field_C_chase_state)
                {
                    case car_chase_state::approach_right_7:
                        return true;
                    case car_chase_state::alongside_right_9:
                    case car_chase_state::pull_ahead_right_11:
                        return true;
                    default:
                        break;
                }
            }
        }
    }
    return false;
}

// 0x474bc0
bool CarChaseTaskTable_500::HasPursuerAlongside_474BC0(Ped* pPed)
{
    for (auto& entry : field_0_entries)
    {
        if (entry.field_0_in_use == 1)
        {
            if (AreAllies_474850(pPed, entry.field_4_pDriver))
            {
                if (entry.field_C_chase_state >= car_chase_state::approach_left_6 &&
                    (entry.field_C_chase_state <= car_chase_state::alongside_right_9 ||
                     entry.field_C_chase_state == car_chase_state::in_front_stopping_13))
                {
                    return true;
                }
            }
        }
    }
    return false;
}

// 0x474c30
char_type CarChaseTaskTable_500::HasPursuerChoosingSide_474C30(Ped* pPed)
{
    for (auto& entry : field_0_entries)
    {
        if (entry.field_0_in_use == 1)
        {
            if (AreAllies_474850(pPed, entry.field_4_pDriver))
            {
                if (entry.field_C_chase_state < car_chase_state::move_right_4)
                {
                    continue;
                }

                if (entry.field_C_chase_state <= car_chase_state::choose_side_5)
                {
                    return 1;
                }
            }
        }
    }
    return 0;
}

// 0x474ca0
CarChaseTask_40::CarChaseTask_40()
{
    ResetEntry_4747B0();
}

// 0x474cc0
void CarChaseTaskTable_500::FreeEntry_474CC0(CarChaseTask_40* toFind)
{
    for (auto& entry : field_0_entries)
    {
        if (&entry == toFind)
        {
            entry.field_0_in_use = 0;
            entry.ResetEntry_4747B0();
            return;
        }
    }
}

// 0x474d10
CarChaseTaskTable_500::CarChaseTaskTable_500() = default;
