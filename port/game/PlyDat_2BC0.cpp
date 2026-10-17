#include "PlyDat_2BC0.hpp"
#include "GameSession_578.hpp"
#include "Game_0x40.hpp"
#include "Player.hpp"
#include "core/function.hpp"
#include "error.hpp"
#include "file.hpp"
#include <io.h>

DEFINE_GLOBAL(PlyDat*, gPlyDat_6FEAC0, 0x6FEAC0);
EXTERN_GLOBAL_ARRAY(wchar_t, gEmptyWStr_67DC8C, 32);
DEFINE_GLOBAL_ARRAY_INIT(ScoreTableLine,
                         gDefaultHiScores_6242B0,
                         10,
                         0x6242B0,
                         {L"ALISDAIR", 50000},
                         {L"BILLY", 40000},
                         {L"BRIAN", 30000},
                         {L"COLIN", 20000},
                         {L"IAIN", 10000},
                         {L"IAN", 9000},
                         {L"KEITH", 8000},
                         {L"MARTIN", 7000},
                         {L"STEPHEN", 6000},
                         {L"WILLIAM", 5000}, );
DEFINE_GLOBAL_ARRAY_INIT(ScoreTableLine,
                         gDefaultStageHiScores_6243A0,
                         120,
                         0x6243A0, //, , 3][4][10, 0xUNKNOWN);
                         {L"ALISDAIR", 50000},
                         {L"BILLY", 40000},
                         {L"BRIAN", 30000},
                         {L"COLIN", 20000},
                         {L"IAIN", 10000},
                         {L"IAN", 9000},
                         {L"KEITH", 8000},
                         {L"MARTIN", 7000},
                         {L"STEPHEN", 6000},
                         {L"WILLIAM", 5000},
                         {L"ALISDAIR", 50000},
                         {L"BILLY", 40000},
                         {L"BRIAN", 30000},
                         {L"COLIN", 20000},
                         {L"IAIN", 10000},
                         {L"IAN", 9000},
                         {L"KEITH", 8000},
                         {L"MARTIN", 7000},
                         {L"STEPHEN", 6000},
                         {L"WILLIAM", 5000},
                         {L"ALISDAIR", 50000},
                         {L"BILLY", 40000},
                         {L"BRIAN", 30000},
                         {L"COLIN", 20000},
                         {L"IAIN", 10000},
                         {L"IAN", 9000},
                         {L"KEITH", 8000},
                         {L"MARTIN", 7000},
                         {L"STEPHEN", 6000},
                         {L"WILLIAM", 5000},
                         {L"ALISDAIR", 50000},
                         {L"BILLY", 40000},
                         {L"BRIAN", 30000},
                         {L"COLIN", 20000},
                         {L"IAIN", 10000},
                         {L"IAN", 9000},
                         {L"KEITH", 8000},
                         {L"MARTIN", 7000},
                         {L"STEPHEN", 6000},
                         {L"WILLIAM", 5000},
                         {L"ALISDAIR", 50000},
                         {L"BILLY", 40000},
                         {L"BRIAN", 30000},
                         {L"COLIN", 20000},
                         {L"IAIN", 10000},
                         {L"IAN", 9000},
                         {L"KEITH", 8000},
                         {L"MARTIN", 7000},
                         {L"STEPHEN", 6000},
                         {L"WILLIAM", 5000},
                         {L"ALISDAIR", 50000},
                         {L"BILLY", 40000},
                         {L"BRIAN", 30000},
                         {L"COLIN", 20000},
                         {L"IAIN", 10000},
                         {L"IAN", 9000},
                         {L"KEITH", 8000},
                         {L"MARTIN", 7000},
                         {L"STEPHEN", 6000},
                         {L"WILLIAM", 5000},
                         {L"ALISDAIR", 50000},
                         {L"BILLY", 40000},
                         {L"BRIAN", 30000},
                         {L"COLIN", 20000},
                         {L"IAIN", 10000},
                         {L"IAN", 9000},
                         {L"KEITH", 8000},
                         {L"MARTIN", 7000},
                         {L"STEPHEN", 6000},
                         {L"WILLIAM", 5000},
                         {L"ALISDAIR", 50000},
                         {L"BILLY", 40000},
                         {L"BRIAN", 30000},
                         {L"COLIN", 20000},
                         {L"IAIN", 10000},
                         {L"IAN", 9000},
                         {L"KEITH", 8000},
                         {L"MARTIN", 7000},
                         {L"STEPHEN", 6000},
                         {L"WILLIAM", 5000},
                         {L"ALISDAIR", 50000},
                         {L"BILLY", 40000},
                         {L"BRIAN", 30000},
                         {L"COLIN", 20000},
                         {L"IAIN", 10000},
                         {L"IAN", 9000},
                         {L"KEITH", 8000},
                         {L"MARTIN", 7000},
                         {L"STEPHEN", 6000},
                         {L"WILLIAM", 5000},
                         {L"ALISDAIR", 50000},
                         {L"BILLY", 40000},
                         {L"BRIAN", 30000},
                         {L"COLIN", 20000},
                         {L"IAIN", 10000},
                         {L"IAN", 9000},
                         {L"KEITH", 8000},
                         {L"MARTIN", 7000},
                         {L"STEPHEN", 6000},
                         {L"WILLIAM", 5000},
                         {L"ALISDAIR", 50000},
                         {L"BILLY", 40000},
                         {L"BRIAN", 30000},
                         {L"COLIN", 20000},
                         {L"IAIN", 10000},
                         {L"IAN", 9000},
                         {L"KEITH", 8000},
                         {L"MARTIN", 7000},
                         {L"STEPHEN", 6000},
                         {L"WILLIAM", 5000},
                         {L"ALISDAIR", 50000},
                         {L"BILLY", 40000},
                         {L"BRIAN", 30000},
                         {L"COLIN", 20000},
                         {L"IAIN", 10000},
                         {L"IAN", 9000},
                         {L"KEITH", 8000},
                         {L"MARTIN", 7000},
                         {L"STEPHEN", 6000},
                         {L"WILLIAM", 5000}, );

// TODO
EXTERN_GLOBAL(s32, bStartNetworkGame_7081F0);

// 0x56B500
HighScoreTable::HighScoreTable()
{
    Init();
}

// 0x56B520
void HighScoreTable::Init()
{
    for (auto& i : score_table_line_)
    {
        wcscpy(i.field_0_player_name, gEmptyWStr_67DC8C);
        i.field_14_score = 0;
    }
}

// 0x56B550
char_type HighScoreTable::InsertScore(const wchar_t* pFindStr, u32 findScore)
{
    u16 startIdx = 10;
    for (s16 i = 9; i != -1; --i)
    {
        if (findScore > score_table_line_[i].field_14_score)
        {
            startIdx = i;
        }

        if (findScore == score_table_line_[i].field_14_score && wcscmp(pFindStr, score_table_line_[i].field_0_player_name) == 0)
        {
            return 0;
        }
    }

    if (startIdx != 10)
    {
        if (startIdx < 9U)
        {
            for (u16 k = 9; k > startIdx; --k)
            {
                wcsncpy(score_table_line_[k].field_0_player_name, score_table_line_[k - 1].field_0_player_name, 9U);
                score_table_line_[k].field_14_score = score_table_line_[k - 1].field_14_score;
            }
        }

        wcsncpy(score_table_line_[startIdx].field_0_player_name, pFindStr, 9U);
        score_table_line_[startIdx].field_14_score = findScore;

        return 1;
    }
    return 0;
}

// 0x56B610
PlySlot::PlySlot()
{
    ResetPlayerSlot();
}

// 0x56B630
void PlySlot::ResetPlayerSlot()
{
    for (wchar_t& k : str_player_name_)
    {
        k = 0;
    }

    for (auto& plyr_stage_stat : plyr_stage_stats_)
    {
        for (auto& stage_stat : plyr_stage_stat)
        {
            stage_stat.field_0_is_stage_unlocked = false;
            stage_stat.field_4_stage_best_score = 0;
            stage_stat.field_8_stage_latest_score = 0;
        }
    }
    plyr_stage_stats_[0][0].field_0_is_stage_unlocked = true;
}

// 0x56B680
s32 PlySlot::GetTotalLatestScore()
{
    s32 result = 0;

    for (auto& plyr_stage_stat : plyr_stage_stats_)
    {
        for (auto& stage_stat : plyr_stage_stat)
        {
            result += stage_stat.field_8_stage_latest_score;
        }
    }
    return result;
}

// 0x56B6B0
s32 PlySlot::GetTotalBestScore()
{
    s32 result = 0;

    for (auto& plyr_stage_stat : plyr_stage_stats_)
    {
        for (auto& stage_stat : plyr_stage_stat)
        {
            result += stage_stat.field_4_stage_best_score;
        }
    }
    return result;
}

/*
s32 len;
    s32 k3Counter;
    s32 k4Counter;

    char_type FileName[256];
    GetHiScoreHscFileName_56BCF0(FileName);
    File::Global_Open_4A7060(FileName);

    len = 240;
    File::Global_Read_4A71C0(&field_23D0_total_scores.field_0, &len);


    for (k3Counter = 0; k3Counter < 3; k3Counter++)
    {
        for (k4Counter = 0; k4Counter < 4; k4Counter++)
        {
            len = 240;
            File::Global_Read_4A71C0(&field_1890_stage_scores[k3Counter][k4Counter].field_0, &len);
        }

        len = 40;
        File::Global_Read_4A71C0(&field_1800_best_stats[k3Counter], &len); // 3 40 byte objs

        len = 4;
        File::Global_Read_4A71C0(&field_1878_best_car_damage_cost[k3Counter], &len);

        len = 4;
        File::Global_Read_4A71C0(&field_1884_best_evasion_rating[k3Counter], &len);
    }

    File::Global_Close_4A70C0();
*/

// 0x56B6E0
PlyDat::PlyDat()
{
    for (s32 i = 0; i < 3; i++)
    {
        memset(&best_stats_[i], 0, sizeof(BestStageStats));
        best_car_damage_cost_[i] = 0;
        best_evasion_rating_[i] = 0;
    }

    for (u32 j = 0; static_cast<u16>(j) < 8; j++)
    {
        if (PlySlotDatExists(j))
        {
            LoadPlySlotDat(j);
        }
        else
        {
            SavePlySlotDat(j);
        }
    }

    if (HiScoreHscExists())
    {
        LoadHiScores();
    }
    else
    {
        InitDefaultHiScores();
        SaveHiScores();
    }

    InitAltHiScores();
}

// 0x56B8A0
void PlyDat::GetPlySlotDatName(u16 slot_idx, char_type* pName)
{
    char_type Buffer[8];
    _itoa(slot_idx, Buffer, 10);
    strcpy(pName, "player\\plyslot");
    strcat(pName, Buffer);
    strcat(pName, ".dat");
}

// 0x56B940
char_type PlyDat::PlySlotDatExists(s32 slot_idx)
{
    char_type FileName[356];
    GetPlySlotDatName(slot_idx, FileName);

    _finddata_t findData;
    long hFind = _findfirst(FileName, &findData);
    if (hFind == -1)
    {
        return 0;
    }
    _findclose(hFind);
    return 1;
}

// 0x56B990
void PlyDat::LoadPlySlotDat(u16 slotIdx)
{
    char_type FileName[356];

    PlySlot* pTmp = &plyr_stats_[slotIdx];

    GetPlySlotDatName(slotIdx, FileName);
    File::GlobalOpen(FileName);

    for (wchar_t& i : pTmp->str_player_name_)
    {
        u32 len_read = 2;
        File::GlobalRead(&i, len_read);
    }

    for (auto& plyr_stage_stat : plyr_stats_[slotIdx].plyr_stage_stats_)
    {
        for (auto& stage_stat : plyr_stage_stat)
        {
            u32 len_read = 1;
            File::GlobalRead(&stage_stat.field_0_is_stage_unlocked, len_read);

            len_read = 4;
            File::GlobalRead(&stage_stat.field_4_stage_best_score, len_read);

            len_read = 4;
            File::GlobalRead(&stage_stat.field_8_stage_latest_score, len_read);
        }
    }
    File::GlobalClose();
}

// 0x56BA60
void PlyDat::SavePlySlotDat(s16 slotIdx)
{
    char_type FileName[356];
    size_t len;

    GetPlySlotDatName(slotIdx, FileName);

    // The start of this object is reused as the buffer for the player slot .dat file
    memcpy(static_cast<void*>(this), plyr_stats_[static_cast<u16>(slotIdx)].str_player_name_, sizeof(plyr_stats_[0].str_player_name_));
    len = sizeof(plyr_stats_[0].str_player_name_);

    // len is accumulated like in SaveHiScores_56BF20: VC6 folds its final value (126) into one store
    // that it emits after the loop counter init, which a plain "len = 126" never does.
    u8* pDst = reinterpret_cast<u8*>(this) + sizeof(plyr_stats_[0].str_player_name_);
    for (auto& plyr_stage_stat : plyr_stats_[static_cast<u16>(slotIdx)].plyr_stage_stats_)
    {
        for (auto& stage_stat : plyr_stage_stat)
        {
            *pDst = stage_stat.field_0_is_stage_unlocked;
            pDst++;
            len++;
            *reinterpret_cast<u32*>(pDst) = stage_stat.field_4_stage_best_score;
            pDst += 4;
            len += 4;
            *reinterpret_cast<s32*>(pDst) = stage_stat.field_8_stage_latest_score;
            pDst += 4;
            len += 4;
        }
    }

    File::WriteBufferToFile(FileName, this, &len);
}

// 0x56BB10
void PlyDat::UpdateStageScore(Player* pPlayer)
{
    const u8 slot_idx = gGameSession_67E8E0.GetPlySlotIdx_4C59B0();
    PlySlot* pPlayerStats = &plyr_stats_[slot_idx];
    u8 map_num;
    u8 bonus_num;
    if (!gGameSession_67E8E0.IsBonusStage_4C59A0())
    {
        map_num = gGameSession_67E8E0.GetMainStageIdx_4C5980();
        bonus_num = 0;
    }
    else
    {
        gGameSession_67E8E0.DecodeStage_453A60(gGameSession_67E8E0.GetStage_4C5990(), &map_num, &bonus_num);
    }

    StageStats* pStageStats = &pPlayerStats->plyr_stage_stats_[map_num][bonus_num];
    const u32 latest_score = pPlayer->field_2D4_scores.GetScore_592370();
    if (latest_score > pStageStats->field_4_stage_best_score)
    {
        pStageStats->field_4_stage_best_score = latest_score;
    }
    pStageStats->field_8_stage_latest_score = latest_score;
    SavePlySlotDat(slot_idx);
}

// 0x56BBD0
void PlyDat::UnlockStage(u8 map_num, u8 bonus_num)
{
    const u8 slot_idx = gGameSession_67E8E0.GetPlySlotIdx_4C59B0();
    this->plyr_stats_[slot_idx].plyr_stage_stats_[map_num][bonus_num].field_0_is_stage_unlocked = 1;
    if (!bStartNetworkGame_7081F0)
    {
        SavePlySlotDat(slot_idx);
    }
}

// 0x56BC40
void PlyDat::UnlockAllStages()
{
    const u8 slot_idx = gGameSession_67E8E0.GetPlySlotIdx_4C59B0();
    PlySlot* pStats = &this->plyr_stats_[slot_idx];
    for (auto& plyr_stage_stat : pStats->plyr_stage_stats_)
    {
        for (auto& k4 : plyr_stage_stat)
        {
            k4.field_0_is_stage_unlocked = 1;
        }
    }

    if (!bStartNetworkGame_7081F0)
    {
        SavePlySlotDat(slot_idx);
    }
}

// =====================================================================

// 0x56BCA0
char_type PlyDat::HiScoreHscExists()
{
    char_type FileName[356];
    GetHiScoreHscFileName(FileName);

    _finddata_t findData;
    long hFind = _findfirst(FileName, &findData);
    if (hFind == -1)
    {
        return 0;
    }
    _findclose(hFind);
    return 1;
}

// 0x56BCF0
void PlyDat::GetHiScoreHscFileName(char_type* pName)
{
    strcpy(pName, "player\\hiscores.hsc");
}

// 0x56BD20
void PlyDat::InitAltHiScores()
{
    wcsncpy(alt_scores_.score_table_line_[0].field_0_player_name, L"ALAN", 9U);
    alt_scores_.score_table_line_[0].field_14_score = 1000000;
    wcsncpy(alt_scores_.score_table_line_[1].field_0_player_name, L"BRIAN", 9U);
    alt_scores_.score_table_line_[1].field_14_score = 500000;
    wcsncpy(alt_scores_.score_table_line_[2].field_0_player_name, L"COLIN", 9U);
    alt_scores_.score_table_line_[2].field_14_score = 400000;
    wcsncpy(alt_scores_.score_table_line_[3].field_0_player_name, L"DAVE", 9U);
    alt_scores_.score_table_line_[3].field_14_score = 300000;
    wcsncpy(alt_scores_.score_table_line_[4].field_0_player_name, L"ERIC", 9U);
    alt_scores_.score_table_line_[4].field_14_score = 250000;
    wcsncpy(alt_scores_.score_table_line_[5].field_0_player_name, L"FRANK", 9U);
    alt_scores_.score_table_line_[5].field_14_score = 200000;
    wcsncpy(alt_scores_.score_table_line_[6].field_0_player_name, L"GRAEME", 9U);
    alt_scores_.score_table_line_[6].field_14_score = 100000;
    wcsncpy(alt_scores_.score_table_line_[7].field_0_player_name, L"HECTOR", 9U);
    alt_scores_.score_table_line_[7].field_14_score = 50000;
    wcsncpy(alt_scores_.score_table_line_[8].field_0_player_name, L"IMOGEN", 9U);
    alt_scores_.score_table_line_[8].field_14_score = 25000;
    wcsncpy(alt_scores_.score_table_line_[9].field_0_player_name, L"JACKSON", 9U);
    alt_scores_.score_table_line_[9].field_14_score = 10000;
}

// 0x56BE50
void PlyDat::LoadHiScores()
{
    char_type FileName[256];
    GetHiScoreHscFileName(FileName);
    File::GlobalOpen(FileName);

    File::GlobalRead(&total_scores_.score_table_line_, 240);

    for (s32 k3Counter = 0; k3Counter < 3; k3Counter++)
    {
        for (s32 k4Counter = 0; k4Counter < 4; k4Counter++)
        {
            File::GlobalRead(&stage_scores_[k3Counter][k4Counter], 240);
        }

        File::GlobalRead(&best_stats_[k3Counter], 40); // 3 40 byte objs

        File::GlobalRead(&best_car_damage_cost_[k3Counter], 4);

        File::GlobalRead(&best_evasion_rating_[k3Counter], 4);
    }

    File::GlobalClose();
}

// 0x56BF20
void PlyDat::SaveHiScores()
{
    char_type FileName[256];
    size_t len;

    GetHiScoreHscFileName(FileName);

    // The start of this object is reused as the buffer for the high score file
    u8* pDst = reinterpret_cast<u8*>(this);
    memcpy(pDst, &total_scores_, sizeof(HighScoreTable));
    pDst += sizeof(HighScoreTable);
    len = sizeof(HighScoreTable);

    for (s32 k = 0; k < 3; k++)
    {
        for (s32 j = 0; j < 4; j++)
        {
            memcpy(pDst, &stage_scores_[k][j], sizeof(HighScoreTable));
            pDst += sizeof(HighScoreTable);
            len += sizeof(HighScoreTable);
        }

        memcpy(pDst, &best_stats_[k], sizeof(BestStageStats));
        pDst += sizeof(BestStageStats);
        len += sizeof(BestStageStats);

        *reinterpret_cast<s32*>(pDst) = best_car_damage_cost_[k];
        pDst += 4;
        len += 4;

        *reinterpret_cast<s32*>(pDst) = best_evasion_rating_[k];
        pDst += 4;
        len += 4;
    }

    File::WriteBufferToFile(FileName, this, &len);
}

// TODO: logic matches, only register allocation differs
// 0x56C010
void PlyDat::UpdateHiScores()
{
    u8 map_num;
    u8 bonus_num;
    char_type bBestStatsChanged = 0;

    if (!gGameSession_67E8E0.IsBonusStage_4C59A0())
    {
        map_num = gGameSession_67E8E0.GetMainStageIdx_4C5980();
        bonus_num = 0;
    }
    else
    {
        gGameSession_67E8E0.DecodeStage_453A60(gGameSession_67E8E0.GetStage_4C5990(), &map_num, &bonus_num);
    }

    PlySlot* pPlayerStats = &plyr_stats_[gGameSession_67E8E0.GetPlySlotIdx_4C59B0()];
    s32 latest = pPlayerStats->plyr_stage_stats_[map_num][bonus_num].field_8_stage_latest_score;
    const char_type bNewStageScore = stage_scores_[map_num][bonus_num].InsertScore(pPlayerStats->str_player_name_, latest);
    const char_type bNewTotalScore = total_scores_.InsertScore(pPlayerStats->str_player_name_, pPlayerStats->GetTotalBestScore());

    if (!bonus_num)
    {
        u32* pBestStats = reinterpret_cast<u32*>(&best_stats_[map_num]);
        for (u8 i = 0; i < 10; i++, pBestStats++)
        {
            const u32 value = gGameSession_67E8E0.GetStatistic_4C59F0(i);
            if (value > *pBestStats)
            {
                *pBestStats = value;
                bBestStatsChanged = 1;
            }
        }

        const u32 value1 = gGameSession_67E8E0.GetCarDamageCost_4C5A80();
        if (value1 > static_cast<u32>(best_car_damage_cost_[map_num]))
        {
            best_car_damage_cost_[map_num] = value1;
            bBestStatsChanged = 1;
        }

        const u32 value2 = gGameSession_67E8E0.GetEvasionRating_4C5AA0();
        if (value2 > static_cast<u32>(best_evasion_rating_[map_num]))
        {
            best_evasion_rating_[map_num] = value2;
            bBestStatsChanged = 1;
        }
    }

    if (bNewStageScore || bNewTotalScore || bBestStatsChanged)
    {
        SaveHiScores();
    }
}

// 0x56C170
void PlyDat::BuildMergedHiScores()
{
    extra_scores_.Init();
    ScoreTableLine* pLine = alt_scores_.score_table_line_;
    for (s32 k10 = 0; k10 < 10; k10++)
    {
        extra_scores_.InsertScore(pLine->field_0_player_name, pLine->field_14_score);
        ++pLine;
    }

    for (auto& plyr_stat : plyr_stats_)
    {
        PlySlot* pSlot = &plyr_stat;
        extra_scores_.InsertScore(pSlot->str_player_name_, pSlot->GetTotalLatestScore());
    }
}

// 0x56C1D0
void PlyDat::InitDefaultHiScores()
{
    total_scores_.Init();
    ScoreTableLine* p10StruIter = gDefaultHiScores_6242B0;
    for (s32 k10 = 0; k10 < 10; k10++)
    {
        total_scores_.InsertScore(p10StruIter->field_0_player_name, p10StruIter->field_14_score);
        ++p10StruIter;
    }

    ScoreTableLine* pSruIter = gDefaultStageHiScores_6243A0;
    for (auto& stage_score : stage_scores_)
    {
        for (auto& k4 : stage_score)
        {
            k4.Init();
            for (s32 k10 = 0; k10 < 10; k10++)
            {
                k4.InsertScore(pSruIter->field_0_player_name, pSruIter->field_14_score);
                ++pSruIter;
            }
        }
    }
}

// 0x56C250
void PlyDat::DoMuchCashCheat()
{
    if (!bStartNetworkGame_7081F0)
    {
        PlySlot* pStats = &this->plyr_stats_[gGameSession_67E8E0.GetPlySlotIdx_4C59B0()];
        if (wcscmp(pStats->str_player_name_, L"MUCHCASH") == 0)
        {
            Player* pPlayer = gGame_0x40_67E008->field_38_local_player;
            pPlayer->field_2D4_scores.AddCash_592620(pPlayer->field_6BC_multipliers.field_0_value * 500000);
        }
    }
}

// 0x56C2C0
void PlyDat::Create()
{
    if (!gPlyDat_6FEAC0)
    {
        gPlyDat_6FEAC0 = new PlyDat();
        if (!gPlyDat_6FEAC0)
        {
            FatalError_4A38C0(Gta2Error::kOutOfMemoryNewOperator, "C:\\Splitting\\Gta2\\Source\\plydat.cpp", 1269);
        }
    }
}

// 0x56C340
void PlyDat::Destroy()
{
    if (gPlyDat_6FEAC0)
    {
        delete gPlyDat_6FEAC0;
        gPlyDat_6FEAC0 = nullptr;
    }
}