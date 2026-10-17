#include "EmergencyCrewPool_1E0.hpp"
#include "CarChaseTaskTable_500.hpp"
#include "Car_BC.hpp"
#include "Ped.hpp"
#include "PedGroup.hpp"

DEFINE_GLOBAL(EmergencyCrewPool_1E0*, gEmergencyCrewPool_706280, 0x706280);
DEFINE_GLOBAL(Fix16, dword_706148, 0x706148);

// 0x4beb00
EmergencyCrew_30::EmergencyCrew_30()
{
    Init_5CBC00();
}

// 0x5cbb70
void EmergencyCrewPool_1E0::init_5CBB70()
{
}

// 0x5cbb80
EmergencyCrew_30* EmergencyCrewPool_1E0::AllocateSlot_5CBB80()
{
    for (auto& slot : field_0_slots)
    {
        if (!slot.field_1E_is_used)
        {
            return &slot;
        }
    }
    return nullptr;
}

// 0x5cbbd0
void EmergencyCrewPool_1E0::ServiceAll_5CBBD0()
{
    for (auto& slot : field_0_slots)
    {
        if (slot.field_1E_is_used)
        {
            if (slot.Service_5CC480())
            {
                slot.field_1E_is_used = 0;
            }
        }
    }
}

// 0x5cbc00
void EmergencyCrew_30::Init_5CBC00()
{
    field_1A_idle_limit = 150;
    field_1E_is_used = 0;
    field_20_crew_type = 0;
    field_24_phase = crew_phase::on_foot_0;
    field_0_car = nullptr;
    field_4_ped = nullptr;
    field_28_state = crew_state::idle_0;
    field_8_group = nullptr;
    field_2C_ready = 0;
    field_1C_spawn_wait_ticks = 0;
}

// 0x5cbc30
void EmergencyCrew_30::ReInit_5CBC30()
{
    Init_5CBC00();
}

// 0x5cbc40
void EmergencyCrew_30::RemovePed_5CBC40(Ped* pPed)
{
    field_8_group->RemovePed_4C9970(pPed);
    field_4_ped = field_8_group->field_2C_ped_leader;
}

// 0x5cbc60
bool EmergencyCrew_30::IsLeaderAlive_5CBC60()
{
    if (field_4_ped && field_4_ped->isDead_403B60())
    {
        return false;
    }
    // A switch whose cases all return true: VC6 drops the compares but keeps the
    // load of field_28_state (the stray mov 0x28(%ecx),%ecx). The case values are a guess.
    switch (field_28_state)
    {
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
            return true;
        default:
            break;
    }
    return true;
}

// 0x5cbc90
bool EmergencyCrew_30::ReplaceLeaderIfNeeded_5CBC90()
{
    PedGroup* pGroup = this->field_8_group;
    if (!pGroup)
    {
        return false;
    }

    u8 idx = 0;
    Ped* pPedAtIdx = pGroup->field_4_ped_list[idx];
    while (pPedAtIdx)
    {
        if (pPedAtIdx->GetPedState_403990() != ped_state_1::dead_9 && !pPedAtIdx->field_16C_car)
        {
            Ped* pLeaderPed = this->field_4_ped;
            if (pLeaderPed->field_16C_car)
            {
                RemovePed_5CBC40(pLeaderPed);
            }
            else
            {
                pGroup->PromoteMemberToLeader_4C9680(idx);
                Ped* pLeader = this->field_8_group->field_2C_ped_leader;
                this->field_4_ped = pLeader;
                pLeader->SetObjective(objectives_enum::no_obj_0);
                const s32 occupation = pPedAtIdx->get_occupation_403980();
                pPedAtIdx->set_occupation_403970(ped_ocupation_enum::dummy);
                pPedAtIdx->Kill_46F9D0();
                pPedAtIdx->set_occupation_403970(occupation);
                pPedAtIdx->SetObjective(objectives_enum::objective_28);
            }
            return true;
        }
        idx++;
        pPedAtIdx = pGroup->field_4_ped_list[idx];
    }
    return false;
}

// 0x5cbd50
void EmergencyCrew_30::UpdateStateMachine_5CBD50()
{
    char_type bCarGone = 0;
    char_type bNoPedsAlive = 1;
    char_type bAllPedsAlive = 1;
    char_type bDriverIsOther = 0;
    u8 idx;
    if (field_8_group)
    {
        if (!field_8_group->field_34_count)
        {
            field_8_group->ClearGroupData_4C8E90();
            field_8_group = nullptr;
        }
    }

    if (field_24_phase == crew_phase::in_car_1)
    {
        if (field_0_car)
        {
            if (field_0_car->IsDespawning_4215B0())
            {
                bCarGone = 1;
            }
            if (field_0_car->IsMaxDamage_40F890())
            {
                bCarGone = 1;
            }
            if (field_0_car->HasSpriteZoom_43A230())
            {
                bCarGone = 1;
            }

            if (field_0_car->field_54_driver && field_0_car->field_54_driver != field_4_ped)
            {
                bDriverIsOther = 1;
                bCarGone = 1;
            }
        }
        else
        {
            bCarGone = 1;
        }

        if (bCarGone)
        {
            if (field_8_group)
            {
                idx = 0;
                for (Ped* pMember = field_8_group->field_4_ped_list[0]; pMember; pMember = field_8_group->field_4_ped_list[++idx])
                {
                    if (pMember->field_168_game_object)
                    {
                        bNoPedsAlive = 0;
                    }
                    else
                    {
                        bAllPedsAlive = 0;
                    }
                }
            }

            if (field_4_ped)
            {
                if (field_4_ped->field_168_game_object)
                {
                    bNoPedsAlive = 0;
                }
            }
            if (bDriverIsOther)
            {
                if (!bAllPedsAlive)
                {
                    return;
                }
                if (field_4_ped)
                {
                    if (!field_4_ped->field_168_game_object)
                    {
                        return;
                    }

                    if (Fix16::MaxAbsDistance_42A6B0(field_4_ped->get_cam_x(),
                                                     field_4_ped->get_cam_y(),
                                                     field_0_car->field_50_car_sprite->field_14_xy.x,
                                                     field_0_car->field_50_car_sprite->field_14_xy.y) > dword_706148)
                    {
                        bCarGone = 1;
                    }
                }
                else
                {
                    bCarGone = 1;
                }
            }
            if (!bCarGone)
            {
                return;
            }
            if (field_0_car)
            {
                field_0_car->field_76_last_seen_timer = 0;
                if (field_0_car->field_7C_ped_type != ped_type::player_2)
                {
                    field_0_car->field_7C_ped_type = ped_type::dummy_3;
                }
                if (field_0_car->field_60_pChaseTask)
                {
                    gCarChaseTaskTable_678E30->FreeEntry_474CC0(field_0_car->field_60_pChaseTask);
                    field_0_car->field_60_pChaseTask = nullptr;
                }
                field_0_car = nullptr;
            }
            if (bNoPedsAlive)
            {
                if (field_4_ped)
                {
                    field_4_ped->set_occupation_403970(ped_ocupation_enum::dummy);
                    field_4_ped->ClearGroupAndGroupIdx_403A30();
                    field_4_ped->Deallocate_45EB60();
                }
                idx = 0;
                if (field_8_group)
                {
                    for (Ped* j = field_8_group->field_4_ped_list[0]; j; j = field_8_group->field_4_ped_list[idx])
                    {
                        j->set_occupation_403970(ped_ocupation_enum::dummy);
                        j->ClearGroupAndGroupIdx_403A30();
                        j->Deallocate_45EB60();
                        ++idx;
                    }
                    field_8_group->ClearGroupData_4C8E90();
                }
                field_8_group = nullptr;
                field_4_ped = nullptr;
                field_24_phase = crew_phase::finished_2;
                return;
            }
            if (bAllPedsAlive)
            {
                if (field_4_ped)
                {
                    if (field_4_ped->field_168_game_object)
                    {
                        field_24_phase = crew_phase::on_foot_0;
                        if (field_8_group)
                        {
                            field_8_group->ResetGroupObjectives_4C8F20();
                        }
                        return;
                    }
                }
            }

            if (field_8_group)
            {
                if (field_8_group->PurgeMembersInCars_4C9040())
                {
                    if (field_8_group->field_2C_ped_leader)
                    {
                        field_24_phase = crew_phase::on_foot_0;
                        field_8_group->ResetGroupObjectives_4C8F20();
                    }
                    else if (field_8_group->field_34_count == 1)
                    {
                        field_4_ped = field_8_group->field_4_ped_list[0];
                        field_8_group->ClearGroupData_4C8E90();
                        field_8_group = nullptr;
                        field_4_ped->SetObjective(objectives_enum::no_obj_0);
                        field_4_ped->SetObjective2_463830(objectives_enum::no_obj_0);
                        field_24_phase = crew_phase::on_foot_0;
                    }
                    else
                    {
                        field_8_group->field_2C_ped_leader = field_8_group->field_4_ped_list[field_8_group->field_34_count];
                        field_8_group->field_4_ped_list[field_8_group->field_34_count] = nullptr;
                        --field_8_group->field_36_count;
                        --field_8_group->field_34_count;
                        field_4_ped = field_8_group->field_2C_ped_leader;
                        field_8_group->ResetGroupObjectives_4C8F20();
                        field_24_phase = crew_phase::on_foot_0;
                    }
                }
                else
                {
                    field_4_ped = field_8_group->field_2C_ped_leader;
                    field_8_group->DestroyGroup_4C93A0();
                    field_8_group = nullptr;
                    field_4_ped->SetObjective(objectives_enum::no_obj_0);
                    field_4_ped->SetObjective2_463830(objectives_enum::no_obj_0);
                    field_24_phase = crew_phase::on_foot_0;
                }
            }
            else if (field_4_ped)
            {
                if (field_4_ped->field_168_game_object)
                {
                    field_4_ped->SetObjective(objectives_enum::no_obj_0);
                    field_4_ped->SetObjective2_463830(objectives_enum::no_obj_0);
                    field_24_phase = crew_phase::on_foot_0;
                }
            }
            else if (!field_0_car)
            {
                field_24_phase = crew_phase::finished_2;
            }
        }
        else
        {
            bCarGone = 1;

            if (field_8_group)
            {
                idx = 0;
                for (Ped* pMember = field_8_group->field_4_ped_list[0]; pMember; pMember = field_8_group->field_4_ped_list[++idx])
                {
                    if (!pMember->isDead_403B60())
                    {
                        bCarGone = 0;
                    }
                }
            }

            if (bCarGone)
            {
                if (!field_4_ped || field_4_ped->isDead_403B60())
                {
                    field_24_phase = crew_phase::finished_2;
                }
            }
            if (field_24_phase == crew_phase::finished_2)
            {
                if (field_0_car)
                {
                    field_0_car->SetDriverPedType_421560(ped_type::dummy_3);
                    field_0_car->field_76_last_seen_timer = -200;
                }
            }
        }
    }
    else if (!field_24_phase)
    {
        bCarGone = 1;
        if (field_8_group)
        {
            idx = 0;
            for (Ped* pMember = field_8_group->field_4_ped_list[0]; pMember; pMember = field_8_group->field_4_ped_list[++idx])
            {
                if (!pMember->isDead_403B60())
                {
                    bCarGone = 0;
                }
            }
        }

        if (bCarGone)
        {
            if (!field_4_ped || field_4_ped->isDead_403B60())
            {
                field_24_phase = crew_phase::finished_2;
            }
        }
    }
    else
    {
        if (field_0_car)
        {
            if (field_0_car->field_60_pChaseTask)
            {
                gCarChaseTaskTable_678E30->FreeEntry_474CC0(field_0_car->field_60_pChaseTask);
                field_0_car->field_60_pChaseTask = nullptr;
            }
        }
        field_0_car = nullptr;
    }
}

// 0x5cc1c0
void EmergencyCrew_30::CleanupExpiredEntities_5CC1C0()
{
    bool bClearRouteAndTryClearOthers = false;
    bool bClearPedAndGroup = true;
    bool bClearCharB4F24 = true;

    if (field_8_group)
    {
        if (field_8_group->field_34_count == 0)
        {
            field_8_group->ClearGroupData_4C8E90();
            this->field_8_group = nullptr;
        }
    }

    if (this->field_24_phase == crew_phase::in_car_1)
    {
        if (this->field_0_car && !field_0_car->IsDespawning_4215B0() && !field_0_car->IsMaxDamage_40F890())
        {
            if (field_0_car->GetLastSeenTimer_4A9AD0() > this->field_1A_idle_limit)
            {
                field_0_car->MarkForDespawn_421470();
                bClearRouteAndTryClearOthers = true;
            }
        }
        else
        {
            bClearRouteAndTryClearOthers = true;
        }

        if (field_4_ped)
        {
            if (field_4_ped->field_168_game_object)
            {
                if (field_4_ped->GetOffscreenCounter_4039F0() < this->field_1A_idle_limit)
                {
                    bClearPedAndGroup = false;
                }
            }
            else if (field_4_ped->isDead_403B60())
            {
                this->field_4_ped = nullptr;
            }
        }

        if (field_4_ped)
        {
            if (field_4_ped->field_168_game_object)
            {
                bClearPedAndGroup = false;
            }
        }

        if (field_8_group)
        {
            u8 i = 0;
            for (Ped* pPedListIter = field_8_group->field_4_ped_list[0]; pPedListIter;)
            {
                if (pPedListIter->field_168_game_object)
                {
                    if (pPedListIter->GetOffscreenCounter_4039F0() < this->field_1A_idle_limit)
                    {
                        bClearPedAndGroup = false;
                    }
                }
                else
                {
                    bClearCharB4F24 = false;
                }
                i++;
                pPedListIter = field_8_group->field_4_ped_list[i];
            }
        }

        if (bClearRouteAndTryClearOthers)
        {
            if (field_0_car)
            {
                if (field_0_car->field_60_pChaseTask)
                {
                    gCarChaseTaskTable_678E30->FreeEntry_474CC0(field_0_car->field_60_pChaseTask); // something to do with car route
                    field_0_car->field_60_pChaseTask = nullptr;
                }
            }
            field_0_car = nullptr;

            if (bClearPedAndGroup)
            {
                if (field_4_ped)
                {
                    field_4_ped->set_occupation_403970(ped_ocupation_enum::dummy);
                    field_4_ped->ClearGroupAndGroupIdx_403A30();
                    field_4_ped->Deallocate_45EB60();
                }

                u8 i = 0;
                if (field_8_group)
                {
                    for (Ped* pPedListIter = field_8_group->field_4_ped_list[0]; pPedListIter;)
                    {
                        pPedListIter->set_occupation_403970(ped_ocupation_enum::dummy);
                        pPedListIter->ClearGroupAndGroupIdx_403A30();
                        pPedListIter->Deallocate_45EB60();
                        i++;
                        pPedListIter = field_8_group->field_4_ped_list[i];
                    }
                    field_8_group->ClearGroupData_4C8E90();
                }
                this->field_8_group = nullptr;
                this->field_2C_ready = 1;
            }
            else if (bClearCharB4F24)
            {
                if (this->field_4_ped->field_168_game_object)
                {
                    this->field_24_phase = crew_phase::on_foot_0;
                }
            }
        }
        return;
    }

    if (field_4_ped)
    {
        if (field_4_ped->GetOffscreenCounter_4039F0() < this->field_1A_idle_limit)
        {
            bClearPedAndGroup = false;
        }
    }

    if (field_8_group)
    {
        u8 i = 0;
        for (Ped* pPedListIter = field_8_group->field_4_ped_list[0]; pPedListIter;)
        {
            if (pPedListIter->GetOffscreenCounter_4039F0() < this->field_1A_idle_limit)
            {
                bClearPedAndGroup = false;
            }
            i++;
            pPedListIter = field_8_group->field_4_ped_list[i];
        }
    }

    if (bClearPedAndGroup)
    {
        if (field_4_ped)
        {
            field_4_ped->ClearGroupAndGroupIdx_403A30();
            field_4_ped->Deallocate_45EB60();
        }

        u8 i = 0;
        if (field_8_group)
        {
            Ped* pPedListIter = field_8_group->field_4_ped_list[0];
            while (pPedListIter)
            {
                pPedListIter->ClearGroupAndGroupIdx_403A30();
                pPedListIter->Deallocate_45EB60();
                i++;
                pPedListIter = field_8_group->field_4_ped_list[i];
            }
            field_8_group->ClearGroupData_4C8E90();
        }
        field_2C_ready = 1;
    }
}

// 0x5cc480
bool EmergencyCrew_30::Service_5CC480()
{
    if (field_18_spawn_delay > 0)
    {
        this->field_18_spawn_delay--;
        return false;
    }

    if (field_18_spawn_delay > -80)
    {
        this->field_18_spawn_delay--;
    }

    switch (this->field_28_state)
    {
        case crew_state::clean_up_5:
            CleanupExpiredEntities_5CC1C0();
            return false;

        case crew_state::update_6:
            UpdateStateMachine_5CBD50();
            return false;

        case crew_state::spawn_car_3:
            // fall through to default below
            break;

        default:
            return false;
    }

    switch (field_20_crew_type)
    {
        case crew_type::paramedic_1:
            if (gCarManager_6C_677930->CanAllocateCarOfKind_446930(car_kind::paramedic_car_4))
            {
                this->field_0_car = gCarManager_6C_677930->SpawnCarAtRoadDirection_444CF0(car_model_enum::MEDICAR,
                                                                                          this->field_C_spawn_x,
                                                                                          this->field_10_spawn_y,
                                                                                          this->field_14_spawn_z);
            }
            else
            {
                this->field_0_car = nullptr;
            }

            if (this->field_0_car)
            {
                this->field_2C_ready = 1;
                field_0_car->IncrementCarStats_443D70(car_kind::paramedic_car_4);
            }
            break;

        case crew_type::police_3:
            if (gCarManager_6C_677930->CanAllocateCarOfKind_446930(car_kind::police_6))
            {
                this->field_0_car = gCarManager_6C_677930->SpawnCarAtRoadDirection_444CF0(car_model_enum::COPCAR,
                                                                                          this->field_C_spawn_x,
                                                                                          this->field_10_spawn_y,
                                                                                          this->field_14_spawn_z);
            }
            else
            {
                this->field_0_car = nullptr;
            }
            if (this->field_0_car)
            {
                this->field_2C_ready = 1;
                field_0_car->IncrementCarStats_443D70(car_kind::police_6);
            }
            break;

        case crew_type::swat_5:
            if (gCarManager_6C_677930->CanAllocateCarOfKind_446930(car_kind::police_6))
            {
                this->field_0_car = gCarManager_6C_677930->SpawnCarAtRoadDirection_444CF0(car_model_enum::SWATVAN,
                                                                                          this->field_C_spawn_x,
                                                                                          this->field_10_spawn_y,
                                                                                          this->field_14_spawn_z);
            }
            else
            {
                this->field_0_car = nullptr;
            }

            if (this->field_0_car)
            {
                this->field_2C_ready = 1;
                field_0_car->IncrementCarStats_443D70(car_kind::police_6);
            }
            break;

        default:
            field_0_car = nullptr;
            break;
    }

    return false;
}