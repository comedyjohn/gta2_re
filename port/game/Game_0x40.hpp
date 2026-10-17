#pragma once

#include "Function.hpp"
#include "fix16.hpp"

class Player;
class Sprite;
class Car_BC;
class Camera_0xBC;
class Fix16_Rect;

namespace GameState
{
enum GameState
{
    ExecuteFrameAndPause_0 = 0,
    Running_1 = 1,
    Paused_2 = 2,
};
} // namespace GameState

enum class GameExitType : s32
{
    None = 0,
    CloseGameImmediately_1 = 1,
    PlayerQuit_2 = 2,
    GameOverRIP_3 = 3,
    AreaCompleted_4 = 4,
    MultiplayerExit_5 = 5,
    ReplayExit_6 = 6,
};

class Game_0x40
{
  public:
    void ToggleFastForward_4A4750()
    {
        field_30_fast_forward = field_30_fast_forward == 0;
    }

    bool IsPaused_416BC0()
    {
        return field_0_game_state == GameState::Paused_2;
    }

    bool IsExitTimerSet_4B7590()
    {
        return field_28_exit_timer != -1;
    }

    u8 GetLocalPlayerIdx_476790()
    {
        return field_24_local_player_idx;
    }

    GameExitType GetExitType_4D09C0()
    {
        return field_2C_game_exit_type;
    }

    s32 GetFrameDurationMs_4B8BB0();
    void ExitGame_4B8BD0(s32 new_timer, GameExitType exit_type, s8 bonus_type);
    void ExitGameNoBonus_4B8C00(s32 new_timer, GameExitType exit_type);
    bool UpdateExitTimer_4B8C20();
    void LoadGameFiles_4B8C40();
    void SetScreenSize_4B8E00(u32 width, u32 height);
    void ApplyNetworkGameSettings_4B8E50();
    void BootGame_4B8EB0();
    void ShowCounters_4B8FF0();
    void ShowDebugOverlay_4B9270();
    void Draw_4B92D0();
    void UpdateAllPlayersControls_4B9380();
    void UpdatePaused_4B93C0();
    void UpdateGame_4B9410();
    bool ExecuteGame_4B9640();
    void Pause_4B96B0();
    void Unpause_4B96C0();
    void TogglePause_4B9700();
    void SinglePlayerStepFrame_4B9710();
    void ReinitAfterVideoModeChange_4B9720();
    Player* GetFirstPlayerWithoutPed_4B9750();
    void ShakeCamerasAtPos_4B9790(Fix16 amount, Fix16 x, Fix16 y);
    bool IsSpriteInAnyPlayerBounds_4B97E0(Sprite* pSprite, Fix16 margin);
    bool IsSpriteInBoundsOrFollowedByAnyPlayer_4B9830(Sprite* pSprite, Fix16 margin);
    bool IsSpriteInAnyPlayerView_4B9890(Sprite* pSprite, s32 wide_view);
    bool IsSpriteInPlayerView_4B98E0(Sprite* pSprite, u8 player_idx, s32 wide_view);
    bool IsSpriteInPlayerBounds_4B9950(Sprite* pSprite, u8 player_idx, Fix16 margin);
    bool IsSpriteCameraSubjectForPlayer_4B9A10(Sprite* pSprite, u8 player_idx);
    bool IsPointOnScreen_4B9A80(Fix16 x, Fix16 y);
    bool IsRectVisibleToAnyPlayer_4B9B10(Fix16_Rect* pBounds);
    Camera_0xBC* IteratePlayerCamera_4B9BC0();
    bool IsCarInAnyPlayerHistory_4B9C10(Car_BC* pCar);
    Camera_0xBC* IterateNextPlayerCamera_4B9C50();
    Player* IterateFirstPlayer_4B9CD0();
    Player* IterateNextPlayer_4B9D10();
    void ReduceTrafficSpawnCounterOfOtherViewers_4B9D60(Sprite* pSprite, Player* pExclude);
    Game_0x40(u8 max_players, s8 player_idx); // 4B9DE0
    ~Game_0x40(); // 4BAE30

    Player* GetPlayer_4219E0(u8 idx)
    {
        return field_4_players[idx];
    }

    s32 field_0_game_state;
    Player* field_4_players[MAX_PLAYERS];
    Player*
        field_1C_draw_player; // set to field_38_local_player in the ctor and never changed; only Draw_4B92D0 reads it (gViewCamera_676978)
    u8 field_20_player_iter_idx;
    u8 field_21_camera_iter_idx;
    bool field_22_aux_cam_iterated;
    u8 field_23_num_players;
    u8 field_24_local_player_idx;
    char_type field_25_unused;
    char_type field_26_unused;
    char_type field_27_unused;
    s32 field_28_exit_timer;
    GameExitType field_2C_game_exit_type;
    bool field_30_fast_forward;
    char_type field_31_unused;
    char_type field_32_unused;
    char_type field_33_unused;
    s32 field_34_network_frame_ms;
    Player* field_38_local_player; // field_4_players[field_24_local_player_idx], the player controlled on this machine
    bool field_3C_skip_police;
    char_type field_3D_unused;
    char_type field_3E_unused;
    char_type field_3F_unused;
};

EXTERN_GLOBAL(Game_0x40*, gGame_0x40_67E008);

EXTERN_GLOBAL(Fix16, kInitialAmbient_67DCCC);

EXTERN_GLOBAL(s32, gCachedScreenDimension_706C58);

EXTERN_GLOBAL(class CarChaseTaskTable_500*, gCarChaseTaskTable_678E30);
