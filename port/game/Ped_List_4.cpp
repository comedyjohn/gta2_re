#include "Ped_List_4.hpp"
#include "Car_BC.hpp"
#include "Char_Pool.hpp"
#include "Ped.hpp"

DEFINE_GLOBAL(Ped_List_4, gThreateningPedsList_678468, 0x678468);

DEFINE_GLOBAL_INIT(Ang16, kAng0_678B40, Ang16(0), 0x678B40);

// 0x471140
Char_8* Ped_List_4::AddPedToFront_471140(Ped* pPed)
{
    Char_8* pNew = gChar_8_Pool_678b50->Allocate_445EF0();

    pNew->field_0_char_ped = pPed;

    pNew->next_ = field_0_pFirstPed;
    field_0_pFirstPed = pNew;

    return pNew;
}

// 0x471160
void Ped_List_4::AddPedToBackIfMissing_471160(Ped* pPed)
{
    Char_8* pIter = field_0_pFirstPed;
    Char_8* pTail = nullptr;
    if (pIter)
    {
        while (pIter)
        {
            if (pIter->field_0_char_ped == pPed)
            {
                // Already exists
                return;
            }

            pTail = pIter;
            pIter = pIter->next_;
        }
    }

    Char_8* pNew = gChar_8_Pool_678b50->Allocate_445EF0();
    pNew->field_0_char_ped = pPed;

    if (pTail)
    {
        pTail->next_ = pNew;
        pNew->next_ = nullptr;
    }
    else
    {
        // List was empty
        field_0_pFirstPed = pNew;
        pNew->next_ = nullptr;
    }
}

// 0x4711B0
void Ped_List_4::AddPedToFrontIfMissing_4711B0(Ped* pPed)
{
    Char_8* pIter = field_0_pFirstPed;
    if (pIter)
    {
        while (pIter)
        {
            if (pIter->field_0_char_ped == pPed)
            {
                // Already exists
                return;
            }

            pIter = pIter->next_;
        }
    }

    Char_8* pNew = gChar_8_Pool_678b50->Allocate_445EF0();
    pNew->field_0_char_ped = pPed;

    pNew->next_ = field_0_pFirstPed;
    field_0_pFirstPed = pNew;
}

// 0x4711f0
void Ped_List_4::RemovePed_4711F0(Ped* pPed)
{
    Char_8* pIter = field_0_pFirstPed;
    Char_8* pPrev = nullptr;
    while (pIter)
    {
        if (pIter->field_0_char_ped == pPed)
        {
            if (pPrev)
            {
                pPrev->next_ = pIter->next_;
            }
            else
            {
                field_0_pFirstPed = pIter->next_;
            }
            gChar_8_Pool_678b50->DeAllocate_445F00(pIter);
            break;
        }
        pPrev = pIter;
        pIter = pIter->next_;
    }
}

// 0x471240
void Ped_List_4::RemovePed_471240(Ped* pPed)
{
    Char_8* pIter = field_0_pFirstPed;
    Char_8* pPrev = nullptr;
    while (pIter)
    {
        if (pIter->field_0_char_ped == pPed)
        {
            if (pPrev)
            {
                pPrev->next_ = pIter->next_;
            }
            else
            {
                field_0_pFirstPed = pIter->next_;
            }
            gChar_8_Pool_678b50->DeAllocate_445F00(pIter);
            break;
        }
        pPrev = pIter;
        pIter = pIter->next_;
    }
}

// 0x471290
u8 Ped_List_4::RemoveNonCorpses_471290()
{
    Char_8* pPrevKept = nullptr;
    Char_8* pIter = field_0_pFirstPed;
    u8 removedCount = 0;
    while (pIter)
    {
        if (pIter->field_0_char_ped->IsActive_433B40() && pIter->field_0_char_ped->field_278_ped_state_1 == PedState1::kDead)
        {
            pPrevKept = pIter;
            pIter = pIter->next_;
        }
        else
        {
            if (pPrevKept)
            {
                pPrevKept->next_ = pIter->next_;
            }
            else
            {
                field_0_pFirstPed = pIter->next_;
            }

            Char_8* pNext = pIter->next_;

            gChar_8_Pool_678b50->DeAllocate_445F00(pIter);
            pIter = pNext;

            ++removedCount;
        }
    }
    return removedCount;
}

// 0x4712F0
void Ped_List_4::ClearPeds_4712F0()
{
    Char_8* pIter = field_0_pFirstPed;
    while (pIter)
    {
        Char_8* pFreed = pIter;
        pIter = pIter->next_;
        gChar_8_Pool_678b50->field_0_pool.DeAllocate(pFreed);
    }

    field_0_pFirstPed = nullptr;
}

// 0x471320
Ped* Ped_List_4::RemoveFirstPed_471320()
{
    Char_8* pIter = field_0_pFirstPed;
    if (!pIter)
    {
        return nullptr;
    }

    Ped* pPed = pIter->field_0_char_ped;
    field_0_pFirstPed = pIter->next_;
    gChar_8_Pool_678b50->DeAllocate_445F00(pIter);
    return pPed;
}

// 9.6f calls MaxAbsDistance_42A6B0 here. By value, with the y difference first, VC6 keeps
// both differences and the max in registers as the original does.
// 0x471340
Ped* Ped_List_4::FindClosestActivePed_471340(Fix16 x, Fix16 y)
{
    Fix16 closestDistance(99999);
    Char_8* pIter = this->field_0_pFirstPed;
    Ped* pClosest = nullptr;
    while (pIter)
    {
        Ped* pPed = pIter->field_0_char_ped;
        if (pIter->field_0_char_ped->IsActive_433B40() == 1)
        {
            Fix16 distance = Fix16::MaxAbsDistance(x, y, pPed->get_cam_x(), pPed->get_cam_y());
            if (distance < closestDistance)
            {
                pClosest = pPed;
                closestDistance = distance;
            }
        }
        pIter = pIter->next_;
    }
    return pClosest;
}

// 0x4713C0
Ped* Ped_List_4::FindClosestPedInViewCone_4713C0(Fix16 x, Fix16 y, Ang16 facing, Ang16 half_cone)
{
    Fix16 distance;
    Ang16 angle_diff;
    Ang16 neg_half_cone;

    auto closestDistance = Fix16(99999);
    Char_8* pIter = field_0_pFirstPed;
    Ped* pClosest = nullptr;

    for (; pIter; pIter = pIter->next_)
    {
        if (pIter->field_0_char_ped->IsActive_433B40() == 1)
        {
            bool withinCone = false;
            distance = Fix16::MaxAbsDistance(x, y, pIter->field_0_char_ped->get_cam_x(), pIter->field_0_char_ped->get_cam_y());

            angle_diff = Fix16::Atan2Fixed(pIter->field_0_char_ped->get_cam_y() - y, pIter->field_0_char_ped->get_cam_x() - x) - facing;

            neg_half_cone = kAng0_678B40 - half_cone;

            if (angle_diff < half_cone || angle_diff > neg_half_cone)
            {
                withinCone = true;
            }
            if (distance < closestDistance && withinCone)
            {
                pClosest = pIter->field_0_char_ped;
                closestDistance = distance;
            }
        }
    }
    return pClosest;
}

// 0x4715a0
void Ped_List_4::KillAndClearAllPeds_4715A0()
{
    Char_8* pIter = field_0_pFirstPed;
    while (pIter)
    {
        Char_8* pFreed = pIter;
        pIter->field_0_char_ped->Kill_46F9D0();
        pIter = pIter->next_;
        gChar_8_Pool_678b50->DeAllocate_445F00(pFreed);
    }

    field_0_pFirstPed = nullptr;
}

// 0x4715e0
void Ped_List_4::KillAndClearAllPedsResetCarRef_4715E0()
{
    Char_8* pIter = field_0_pFirstPed;
    while (pIter)
    {
        Char_8* pFreed = pIter;
        pIter->field_0_char_ped->Kill_46F9D0();
        pIter->field_0_char_ped->field_16C_car = nullptr;
        pIter = pIter->next_;
        gChar_8_Pool_678b50->DeAllocate_445F00(pFreed);
    }

    field_0_pFirstPed = nullptr;
}

// 0x471630
void Ped_List_4::ApplyPassengerBusStopBehavior_471630()
{
    for (Char_8* pIter = field_0_pFirstPed; pIter; pIter = pIter->next_)
    {
        if (pIter->field_0_char_ped->get_occupation_403980() == PedOcupation::kBusCustomer)
        {
            pIter->field_0_char_ped->SetObjective2_463830(Objectives::kNoObj);
            pIter->field_0_char_ped->SetObjective(Objectives::kObjective34);
            pIter->field_0_char_ped->SetTargetObjectiveCar(pIter->field_0_char_ped->field_16C_car);
        }
    }
}

// 0x471680
void Ped_List_4::ForceTaxiPassengersToExit_471680()
{
    for (Char_8* pIter = field_0_pFirstPed; pIter; pIter = pIter->next_)
    {
        if (pIter->field_0_char_ped->field_240_occupation == PedOcupation::kTaxiCustomer)
        {
            pIter->field_0_char_ped->field_21C_bf.bForcedOutOfTaxi = true;
        }
    }
}

// 0x4716b0
u16 Ped_List_4::GetPedsCount_4716B0()
{
    u16 passengerCount;
    Char_8* pIter = field_0_pFirstPed;
    for (passengerCount = 0; pIter; ++passengerCount)
    {
        pIter = pIter->next_;
    }
    return passengerCount;
}

// 0x4716d0
void Ped_List_4::SyncPassengersWithCarState_4716D0(Car_BC* pCar)
{
    for (Char_8* pIter = field_0_pFirstPed; pIter; pIter = pIter->next_)
    {
        pIter->field_0_char_ped->field_204_killer_id = pCar->field_70_exploder_ped_id;
        pIter->field_0_char_ped->field_290_death_cause = pCar->field_90_death_cause;
        pIter->field_0_char_ped->field_264_killer_id_timer = 50;
    }
}

// 0x471710
bool Ped_List_4::HasScriptCreatedPed_471710()
{
    Char_8* pIter = field_0_pFirstPed;
    while (pIter)
    {
        if (pIter->field_0_char_ped->field_238_ped_type == PedType::kScriptCreated)
        {
            return true;
        }
        pIter = pIter->next_;
    }
    return false;
}
