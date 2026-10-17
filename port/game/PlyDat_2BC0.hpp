#pragma once

#include "core/function.hpp"
#include <windows.h>

class Player;

// Progress of a player slot in one stage
struct StageStats
{
    char_type field_0_is_stage_unlocked;
    u32 field_4_stage_best_score;
    s32 field_8_stage_latest_score;
};

// One player slot: name and the stats of the 3 main stages x 4 (main + 3 bonus) stages
struct PlySlot
{
    StageStats plyr_stage_stats_[3][4];
    wchar_t str_player_name_[9];

    PlySlot();
    ~PlySlot() = default; // 0x56B620

    void ResetPlayerSlot();
    s32 GetTotalLatestScore();
    s32 GetTotalBestScore();
};

// One line (name, score) of a high score table
struct ScoreTableLine
{
    wchar_t field_0_player_name[10];
    u32 field_14_score;
};

// Top 10 scores
struct HighScoreTable
{
    HighScoreTable();
    ~HighScoreTable() = default; // 0x56B510

    void Init();

    char_type InsertScore(const wchar_t* pFindStr, u32 findScore);

    ScoreTableLine score_table_line_[10];
};

// The best of each of the 10 statistics (GameSession statistics) in a main stage
struct BestStageStats
{
    BYTE field_0[40];
};

// Player profile and high score data (plydat.cpp): the 8 player slots (player\plyslotN.dat) and the high score tables
// (player\hiscores.hsc)
struct PlyDat
{
    u8 file_buffer_[0x1800]; // the start of the object is reused as the buffer when saving the files
    BestStageStats best_stats_[3];
    s32 best_car_damage_cost_[3];
    s32 best_evasion_rating_[3];
    HighScoreTable stage_scores_[3][4];
    HighScoreTable total_scores_;
    HighScoreTable alt_scores_;
    HighScoreTable extra_scores_;
    PlySlot plyr_stats_[8];

    PlyDat();
    ~PlyDat() = default; // 0x56B810

    // todo: ordering
    void UpdateStageScore(Player* pPlayer);

    void UpdateHiScores();

    void GetPlySlotDatName(u16 slot_idx, char_type* pName);

    char_type PlySlotDatExists(s32 slot_idx);

    void GetHiScoreHscFileName(char_type* pName);

    char_type HiScoreHscExists();

    void LoadPlySlotDat(u16 slotIdx);

    void SavePlySlotDat(s16 slotIdx);

    void LoadHiScores();

    void BuildMergedHiScores();

    void InitDefaultHiScores();

    void SaveHiScores();

    void InitAltHiScores();

    void UnlockStage(u8 map_num, u8 bonus_num);

    void UnlockAllStages();

    void DoMuchCashCheat();
    static void Create();
    static void Destroy();
};

EXTERN_GLOBAL(PlyDat*, gPlyDat_6FEAC0);
