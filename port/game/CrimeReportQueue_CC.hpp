#pragma once

#include "Function.hpp"
#include "fix16.hpp"
#include "Fix16_Point.hpp"

class Ped;
class Player;
class Car_BC;
class Char_B4;

// One reported crime: its type, the ped that committed it and where that ped was
class CrimeReport_14
{
  public:
    CrimeReport_14();
    ~CrimeReport_14() = default; // 0x484ce0
    void SetReport(s32 crime_type, s32 ped_id);
    void GetReport(s32* pCrimeType, Fix16* pXPos, Fix16* yPos, Fix16* zPos);

    s32 field_0_crime_type; // crime_stats_type
    s32 field_4_ped_id;
    Fix16_Vec field_8_pos;
};

// Ring buffer of the last 10 reported crimes, drained by the police sound/AI code (TryPopOldestReport).
// field_2_read_idx is the oldest report still queued, field_0_write_idx the next free slot.
class CrimeReportQueue_CC
{
  public:
    CrimeReportQueue_CC();
    ~CrimeReportQueue_CC() = default; // 0x484db0
    void QueueReport(s32 crime_type, s32 ped_id);
    bool TryPopOldestReport(s32* pCrimeType, Fix16* pXPos, Fix16* pYPos, Fix16* pZPos);
    bool IsCrimeQueued(s32 crime_type);
    CrimeReportQueue_CC* ctor_484FC0();
    void dtor_484FD0();
    void ReportCrimeForPed(u32 crime_type, Ped* pPed);
    bool ShouldReportCarCrime_485090(Car_BC* pCar, Player* pPlayer);
    bool ShouldReportCharCrime_4850F0(Char_B4* pB4, Player* pPlayer);
    bool ShouldReportPedCrime_485140(Ped* pPed, Player* pPlayer);

    u16 field_0_write_idx;
    u16 field_2_read_idx;
    CrimeReport_14 field_4_reports[10];
};

class CrimeReportQueue_CC_Sub : public CrimeReportQueue_CC {
  public:
    CrimeReportQueue_CC_Sub();
    ~CrimeReportQueue_CC_Sub() = default; // 0x484fd0
};

EXTERN_GLOBAL(CrimeReportQueue_CC*, gCrimeReportQueue_67A4B8);
