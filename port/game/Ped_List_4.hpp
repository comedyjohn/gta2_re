#pragma once

#include "Function.hpp"
#include "fix16.hpp"

class Ped;
class Char_8;
class Car_BC;

class Ped_List_4
{
  public:
    Ped_List_4()
    {
        ClearList_420E90();
    }

    Char_8* AddPedToFront_471140(Ped* pPed);
    void AddPedToBackIfMissing_471160(Ped* pPed);
    void AddPedToFrontIfMissing_4711B0(Ped* pPed);
    void RemovePed_4711F0(Ped* pPed);
    // Functionally the same as above? Hmmm
    void RemovePed_471240(Ped* pPed);
    // Keeps the active dead peds (the bodies waiting for an ambulance) and frees every other node.
    // Returns how many were removed.
    u8 RemoveNonCorpses_471290();
    void ClearPeds_4712F0();
    Ped* RemoveFirstPed_471320();
    Ped* FindClosestActivePed_471340(Fix16 x, Fix16 y);
    // facing: the view direction, half_cone: how far either side of it counts as in view
    Ped* FindClosestPedInViewCone_4713C0(Fix16 x, Fix16 y, Ang16 facing, Ang16 half_cone);
    void KillAndClearAllPeds_4715A0();
    void KillAndClearAllPedsResetCarRef_4715E0();
    void ApplyPassengerBusStopBehavior_471630();
    void ForceTaxiPassengersToExit_471680();

    u16 GetPedsCount_4716B0();
    void SyncPassengersWithCarState_4716D0(Car_BC* pCar);
    bool HasScriptCreatedPed_471710();

    // inline
    void ClearList_420E90()
    {
        field_0_pFirstPed = nullptr;
    }

    bool IsEmpty_420EA0()
    {
        return field_0_pFirstPed == nullptr;
    }

    Char_8* field_0_pFirstPed;
};

EXTERN_GLOBAL(Ped_List_4, gThreateningPedsList_678468);
