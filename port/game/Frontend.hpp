#pragma once

#include "CreditsLine_6C.hpp"
#include "CreditsText_FD22.hpp"
#include "TagGame_28.hpp"
#include "core/function.hpp"
#include "multiplayer_game_type.hpp"
#include <windows.h>

#ifndef DIRECTINPUT_VERSION
    #define DIRECTINPUT_VERSION 0x0500
#endif

#include <dinput.h>

class Player;
class Ang16;
struct ScoreTableLine;

struct SaveSlotInfo_52
{
    char_type field_0_map_name[25];
    char_type field_19_style_name[25];
    char_type field_32_script_name[25];
    char_type field_4B_last_saved_stage;
    char_type field_4C_last_saved_bonus_stage_code;
    char_type field_4D_last_saved_is_bonus;
    short field_4E_unused;
    s16 field_50_unused;
};

// todo: move
struct SaveSlotSummary_4
{
    char_type field_0_save_exists;
    char_type field_1_last_saved_stage;
    char_type field_2_last_saved_bonus_stage_code;
    char_type field_3_last_saved_is_bonus;

    SaveSlotSummary_4();

    ~SaveSlotSummary_4();

    void LoadPlySlotSvg_4B6480(const char_type* FileName);
};

// todo: move
struct menu_element_0x6E
{
    menu_element_0x6E();

    ~menu_element_0x6E();

    u8 field_0_element_type;
    char_type field_1_is_it_displayed;
    s16 field_2_xpos;
    s16 field_4_ypos;
    union
    {
        struct // if field_0_element_type == STRING_TEXT_1 or STRING_TEXT_2
        {
            wchar_t field_6_element_name_str[50];
            u16 field_6A_font_type;
            u16 field_6C_font_palette;
        };
        u16 field_6_geometric_shape_type; // if field_0_element_type == GEOMETRIC_SHAPE_3
    };
};

// todo: move
struct menu_option_0x82
{
  public:
    menu_option_0x82();

    ~menu_option_0x82();

    bool SelectNextHorizontalIdx_4B6330();

    bool SelectPrevHorizontalIdx_4B6390();

    char_type field_0_option_type;
    char_type field_1_is_unlocked;
    s16 field_2_x_pos;
    s16 field_4_y_pos;
    wchar_t field_6_option_name_str[50];
    u16 field_6A_font_type;
    u16 field_6C_palette;
    u16 field_6E_horizontal_selected_idx;
    s16 field_70;
    char_type field_72_horizontal_idx_enabled[12];
    u16 field_7E_horizontal_max_idx;
    s16 field_80_menu_page_target;
};

// todo: move
struct MenuOption_6
{
  public:
    MenuOption_6();

    ~MenuOption_6();

    s16 field_0;
    s16 field_2_unused;
    char_type field_4_is_option_unlocked; // this control if the menu selection will skip a option or not
    char_type field_5_unused;
};

// todo: move
struct MenuPage_0xBCA
{
    MenuPage_0xBCA();

    ~MenuPage_0xBCA();

    bool SelectPrevOption_4B61B0();

    bool SelectNextOption_4B6200();

    u16 field_0_number_of_options;
    u16 field_2_number_of_elements;
    menu_option_0x82 field_4_options_array[10];
    menu_element_0x6E field_518_elements_array[15];
    MenuOption_6 field_B8A[10];
    u16 field_BC6_current_option_idx;
    s16 field_BC8_default_option_idx;
};

struct PlySlot;

enum WinMainState
{
    Quit_1 = 1,
    Run_Frontend_2 = 2,
    Start_Game_3 = 3,
    Start_Replay_4 = 4,
};

namespace FrontendState
{
enum FrontendState
{
    Unknown_1 = 1,
    Booting_Map_2 = 2,
    User_Typing_New_Player_Name_3 = 3,
    Unknown_4 = 4,
    Unknown_5 = 5,
};
} // namespace FrontendState

enum MenuScreen
{
    Options_0 = 0,
    Play_1 = 1,
    Quit_2 = 2,
    BonusAC_3 = 3,
    BonusDF_4 = 4,
    BonusGI_5 = 5,
    ViewHiScore_6 = 6,
    PlayArea1_7 = 7,
    PlayArea2_8 = 8,
    PlayArea3_9 = 9,
    EnterPlayerName_10 = 10,
    ResumeLoadSave_11 = 11,
    HiScoresDisplay_12 = 12,
    GameOver_13 = 13,
    BlueBar_14 = 14, // for game over?
    Loading_15 = 15, // seems to have some progress bar? prob never seen due to fast loading
    RedBar_16 = 16, // for player quit ?
    Credits_17 = 17,
};

enum MenuPages // enum for field_132_f136_idx
{
    MENUPAGE_NONE = -1,
    MENUPAGE_START_MENU = 0,
    MENUPAGE_PLAY = 1,
    MENUPAGE_DEAD = 2,
    MENUPAGE_AREA_COMPLETE = 3,
    MENUPAGE_GAME_COMPLETE = 4,
    MENUPAGE_VIEW_HIGH_SCORE = 5,
    MENUPAGE_BONUS_AREA = 6,
    MENUPAGE_MULTIPLAYER_RESULTS = 7,
    MENUPAGE_PLAY_INTRO = 8,
    MENUPAGE_CREDITS = 9,
    MENUPAGE_NICE_TRY = 10,
    MENUPAGE_RESULTS_PLAYER_QUIT = 11,
    MENUPAGE_12 = 12,
    MENUPAGE_13 = 13,
    MENUPAGE_PARENTAL_CONTROL = 14,
    MENUPAGE_15 = 15,
    NUM_MENUPAGES,

    // Not really pages
    MENUPAGE_GTA2MANAGER = 257,
    MENUPAGE_QUIT = 258,
    MENUPAGE_REPLAY_PREVIOUS_AREA = 259,
    MENUPAGE_LOADING_SAVE = 260,
    MENUPAGE_PLAY_NEXT_AREA = 261,
    MENUPAGE_GET_READY_TO_PLAY = 264,
    MENUPAGE_GET_READY_TO_PLAY_BONUS = 265,
    MENUPAGE_CONTINUE_NEXT_STAGE = 266,
};

enum ElementType
{
    NULL_TYPE_0 = 0, //  i.e. don't exists
    STRING_TEXT_1 = 1,
    STRING_TEXT_2 = 2,
    GEOMETRIC_SHAPE_3 = 3, //  triangles, circles
};

struct MainBlockStrings
{
    char field_0_debug_str[256];
    char field_100_map_name[256];
    char field_200_sty_name[256];
};

struct Frontend
{
    LPDIRECTINPUTA field_0_pDInput;
    LPDIRECTINPUTDEVICEA field_4_pKeyboardDevice;
    char_type field_8_keys[256];
    s32 field_108_winmain_next_state;
    char_type field_10C_bKeyboardAcquired;
    char_type field_10D_bInputEnabled;
    char_type field_10E_unused;
    char_type field_10F_unused;
    s32 field_110_state;
    s32 field_114_cursor_blink;
    s32 field_118_cursor_blink_timer;
    u16 field_11C_normal_font;
    s16 field_11E;
    s16 field_120_selected_font;
    s16 field_122_unused;
    u16 field_124_font_type;
    s16 field_126;
    s16 field_128;
    s16 field_12A_score_font;
    s16 field_12C;
    s16 field_12E_unused;
    s16 field_130;
    u16 field_132_f136_idx;
    s16 field_134_unused;
    MenuPage_0xBCA field_136_menu_pages_array[17];
    wchar_t field_C9A0_curr_plyr_name[9];
    u8 field_C9B2_curr_plyr_name_length;
    char_type field_C9B3_key_held;
    s16 field_C9B4_last_key;
    s16 field_C9B6_key_repeat_timer;
    wchar_t field_C9B8_password[9];
    //char_type field_C9C8;
    //char_type field_C9C9;
    u8 field_C9CA_password_length;
    char_type field_C9CB_wrong_password_shown;
    char_type field_C9CC_left_pressed;
    char_type field_C9CD_right_pressed;
    char_type field_C9CE_up_pressed;
    char_type field_C9CF_down_pressed;
    char_type field_C9D0_return_pressed;
    char_type field_C9D1_escape_pressed;
    char_type field_C9D2_delete_pressed;
    char_type field_C9D3_left_key_down;
    char_type field_C9D4_right_key_down;
    char_type field_C9D5_up_key_down;
    char_type field_C9D6_down_key_down;
    char_type field_C9D7_return_key_down;
    char_type field_C9D8_escape_key_down;
    char_type field_C9D9_delete_key_down;
    char_type field_C9DA_unused;
    char_type field_C9DB_unused;
    u32 field_C9DC_next_frame_time;
    char_type field_C9E0_updates_since_render;
    char_type field_C9E1_bCheatsEnabled;
    char_type field_C9E2_unused;
    char_type field_C9E3_unused;
    s32 field_C9E4_last_input_time;
    MainBlockStrings field_C9E8_blocks[3][4];
    SaveSlotSummary_4 field_EDE8_plySlots[8];
    char_type field_EE08_menu_screen;
    char_type field_EE09_unused;
    u16 field_EE0A_dialog_cursor_ypos;
    u8 field_EE0C_dialog_type;
    u8 field_EE0D_hiscore_table_idx;
    CreditsText_FD22 field_EE0E_credits;
    u16 field_1EB30_credits_scroll_timer;
    s16 field_1EB32_unused;
    Fix16 field_1EB34_credits_ypos;
    u16 field_1EB38_credits_line_idx;
    u8 field_1EB3A_selected_main_stage[8];
    u8 field_1EB42_selected_bonus_stage[8];
    char_type field_1EB4A_has_prev_player_slot;
    char_type field_1EB4B_has_next_player_slot;
    char_type field_1EB4C_has_prev_main_stage;
    char_type field_1EB4D_has_next_main_stage;
    char_type field_1EB4E_has_prev_bonus_stage;
    char_type field_1EB4F_has_next_bonus_stage;
    u8 field_1EB50_num_main_stages;
    u8 field_1EB51_num_bonus_stages[3];

    void SetField10D_453A30(const char_type& value)
    {
        field_10D_bInputEnabled = value;
    }

    // todo: ordering
    void LoadStringsFromStage_4B4C60(u16 mainBlockIdx, u16 bounusBlockIdx, char* pDebugStr, char* pMapName, char* pStyName);
    void StoreStringsForStage_4B4BC0(u16 mainBlockIdx,
                                     u16 bounusBlockIdx,
                                     const char* pDebugStr,
                                     const char* pMapName,
                                     const char* pStyName);
    void DrawSavedStage_4B5270();

    // todo move to another file for ordering
    static void create_4ACFA0();

    static void destroy_4AD070();

    void ChangeMenuPage_4B3170(u16 menu_page_idx);

    void InitSound_4B8680();

    const char_type* intro_bik_4B5E50();

    static bool intro_bik_exists_4B5FF0();

    void Update_4AEC00();

    void read_menu_input_4AFEB0();

    void GetOptionText_4B3AF0(u16 menu_page_idx, u16 option_idx, wchar_t** w_buffer);

    void UpdateMenuScreen_4B6780();

    void ManageCredits_4B7A10();

    void UpdatePageFromUserInput_4AE2D0();

    void SetWinMainStateToBootMap_4AE990();

    void HandlePlayerNameTyping_4B2F60();

    void HandleDeletePlayerDialog_4AE9A0();

    void HandlePasswordTyping_4B8280();

    void DeleteCurrentPlayer_4B4410();

    PlySlot* GetCurrPlayerStats_4B43E0();

    void UpdateMenuForCurrPlayer_4B42E0();

    void SaveAndUpdatePlayerName_4B4230();

    void HandleCheatCode_4B3DD0(const wchar_t* String);

    void LoadCurrPlayerName_4B4280();

    u8 GetPrevUnlockedStageIndex_4B77B0(PlySlot* pStats);

    u8 GetPrevUnlockedStageBonusCode_4B7800(PlySlot* pStats);

    void StripPasswordToCurrLength_4B8530();

    void CheckPassword_4B8560();

    void ContinueToNextStage_4B8020();

    void DrawBonusRating_4B7D60();
    int sub_4B7E10(u8 str_id_idx, u16 text_xpos, u16 text_ypos, u16 fontType, s32 palette);

    char_type AreAllStagesUnlocked_4B7FB0();

    void LoadMapFilenames_4B4D00(u8 mainBlockIdx, u8 bonusBlockIdx);

    void DrawLoading_4AD0D0();

    void DrawDeletePlayerDialog_4ADDE0();

    void DrawCurrentState_4ADF50();

    void Render_4ADFB0();

    void DrawBackground_4B6E10();

    void GetTgaIdxsForMenuScreen_4B6B00(u8 screen, BYTE* pTgaIdx, BYTE* pTgaIdx2);

    void Load_tga_4B6520(u16 idx);

    Frontend();

    ~Frontend();

    void AcquireKeyBoard_4AFD70();

    void FreeKeyBoardDevice_4AFD00();

    void FreeImageTable_4B6750();

    char_type KeyBoard_GetKeyStates_4AFDD0();

    void InitKeyBoardDevice_4AFBE0();

    void SetFontTypes_4AF0E0();

    void SetupMenuStringsOptionsElements_4B0220();

    void GetMainAndBonusStagesFromSeqFile_4B4440();

    void GetSeqItem_4B48D0(s32 type, char_type* ppRet, FILE* hSeqFile);

    void LoadPlySlotSvgs_4B53C0();

    void Load_tgas_4B66B0();

    void GetPlySlotSvgName_4B51D0(u8 idx, char_type* pStr);

    char_type PlySlotSvgExists_4B5370(u8 idx);

    static void SetInputEnabled_5E53C0(BYTE* pEnabled);

    static s32 GetMaxTextWidth_5D8990(wchar_t* pStr, u16 font_type);

    u16 GetCenteredXPos_4B0190(wchar_t* pText, u16 fontType, s32 width);

    u8 GetPreviousUnlockedMainStage_4B7060(u8 stage_idx);

    u8 GetNextUnlockedMainStage_4B7270(char_type main_stage_idx);

    bool ExistsPreviousMainStage_4B7490();

    bool ExistsNextMainStage_4B74C0();

    void UpdateMainStageArrows_4B7550();

    bool ChangeMainStageToPrevious_4B6FF0();

    void StripPlayerNameToCurrLength_4B42B0();

    char_type GetPreviousUnlockedBonusStage_4B7120(u8 stage);

    void UpdateBonusStageArrows_4B7610();

    bool ChangeBonusStageToPrevious_4B70B0();

    bool ExistsPreviousBonusStage_4B74F0();

    char_type GetNextUnlockedBonusStage_4B7360(u8 stage);

    bool ExistsNextBonusStage_4B7520();

    bool ChangeBonusStageToNext_4B72F0();

    bool ChangeMainStageToNext_4B7200();

    void sub_4B4EC0();

    s32 Run_4AEDB0();

    void DrawMenu_4AD140();

    void DrawCredits_4B7AE0();

    void FreeSound_4B8650();

    static bool pre_intro_bik_exists_4B6030();

    char_type* pre_intro_bik_4B5F20();

    void GetElementText_4B3CC0(u16 page_idx, u16 element_idx, wchar_t** ppText);

    void DrawMultiplayerScores_4B55F0();

    void DrawLastAndBestStats_4B57B0(u16 text_xpos, u16 text_ypos);

    void DrawTextFixedWidth_4B78B0(wchar_t* pString,
                                   u16 text_xpos,
                                   u16 text_ypos,
                                   u16 font_type,
                                   u16 palette,
                                   u16 scale,
                                   u16 char_width,
                                   u8 pStr);

    void DrawScoreTable_4B5430(ScoreTableLine* pStrings,
                               u16 text_xpos,
                               u16 text_ypos,
                               u16 num_entries,
                               u16 arg_fontType,
                               u16 palette,
                               u8 spacing_type);
};

class FreeLoader
{
  public:
    static s32 GetCityInstalled_4AE0F0();
    static char_type CheckCityInstalled_4AE1F0(u8 city_idx);
    static void MessageBox_4AE170(UINT uType, LPCSTR lpCaption, LPCSTR lpFormat, ...);
    static LPCSTR GetRegDword_4AE010(HKEY hKey, LPCSTR lpValueName, LPCSTR default_value);
};

s32 IsFullScreen_5D98D0();
s32 SetGamma_5D9910(s32 gamma);

EXTERN_GLOBAL(Frontend*, gFrontend_67DC84);

EXTERN_GLOBAL_ARRAY(wchar_t, gTmpBuff, 640);

void MakeScreenTableAndSetWindow_5D7D30();
void FreeSurface_5D7DC0();
void DrawDebugLine_5D7DD0(s32 x1, s32 y1, s32 x2, s32 y2, u16 colour);
void ConvertColourBanks_5D7CB0();

EXTERN_GLOBAL(short, font_type_703C14);

EXTERN_GLOBAL(s16, word_703C3C);

EXTERN_GLOBAL(s16, word_703D0C);

EXTERN_GLOBAL(s16, word_703C16);

EXTERN_GLOBAL(s16, word_703C8C);

EXTERN_GLOBAL(s16, word_703C8A);

EXTERN_GLOBAL(s16, word_703BE2);

EXTERN_GLOBAL(s16, word_703B88);

EXTERN_GLOBAL(s16, word_703DAC);

EXTERN_GLOBAL(s16, word_703B9C);

EXTERN_GLOBAL(u32, counter_706C4C);

EXTERN_GLOBAL(bool, gCheatOnlyMuggerPeds_67D5A4);
EXTERN_GLOBAL(bool, gCheatUnlimitedElectroGun);
EXTERN_GLOBAL(bool, gCheatAllGangMaxRespect_67D587);
EXTERN_GLOBAL(bool, gCheatOnlyElvisPeds);
EXTERN_GLOBAL(bool, gCheatNakedPeds_67D5E8);
EXTERN_GLOBAL(bool, gCheatGetBasicWeaponsMaxAmmo_67D545);
EXTERN_GLOBAL(bool, gCheatGet99Lives);
EXTERN_GLOBAL(bool, gCheatGetPlayerPoints_67D4C8);
EXTERN_GLOBAL(bool, gCheatUnlimitedFlameThrower_67D6CC);
EXTERN_GLOBAL(bool, gCheatUnknown);
EXTERN_GLOBAL(bool, gCheatGet10MillionMoney_67D6CE);
EXTERN_GLOBAL(bool, gCheat10xMultiplier_67D589);
EXTERN_GLOBAL(bool, gCheatUnlockThreeLevels_67D6CB);
EXTERN_GLOBAL(bool, gCheatUnlockLevelsOneAndTwo_67D584);
EXTERN_GLOBAL(bool, gCheatUnlockAllLevels_67D538);
EXTERN_GLOBAL(bool, gCheatUnlimitedDoubleDamage_67D57C);
EXTERN_GLOBAL(bool, gCheatInvisibility_67D539);
EXTERN_GLOBAL(bool, gCheatMiniCars_67D6C8);
