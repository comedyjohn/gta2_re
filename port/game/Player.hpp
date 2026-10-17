#pragma once

#include "BonusTracker_1C0.hpp"
#include "Camera.hpp"
#include "Draw.hpp"
#include "PlayerCrimeStats_3C.hpp"
#include "PlayerScoreTracker_36C.hpp"
#include "TagGame_28.hpp"
#include "core/ang16.hpp"
#include "core/fix16.hpp"
#include "core/function.hpp"
#include <windows.h>

class SoundObject_10;
class Ped;
class Weapon_30;
class Player;
struct SavedPlayerStats_90;

// TODO: add these later
class Car_BC;
class Gang_144;

namespace power_up_indices
{
enum
{
    Unk_0 = 0,
    Unk_1 = 1,
    Unk_2 = 2,
    Armor_3 = 3,
    JailCard_4 = 4,
    Unk_5 = 5,
    Invulnerability_6 = 6,
    DoubleDamage_7 = 7,
    FastReload_8 = 8,
    Electrofingers_9 = 9,
    Unk_10 = 10,
    Invisibility_11 = 11,
    InstantGang_12 = 12,
    Unk_13 = 13,
    Unk_14 = 14,
    Unk_15 = 15,
    Unk_16 = 16,
};
} // namespace power_up_indices

class Player
{
  public:
    bool has_player_ped_45B0C0()
    {
        return field_2C4_player_ped != nullptr;
    }

    Gang_144* get_gang_curr_location_4766D0()
    {
        return field_34_gang_curr_location;
    }

    void set_in_use_4A5170(bool in_use)
    {
        field_8E_in_use = in_use;
    }

    GmpMapZone* get_arrow_blocker_zone_4C7340()
    {
        return field_40_arrow_blocker_zone;
    }

    bool HasPowerUp_434920(s32 power_up_idx)
    {
        return field_6F4_power_up_timers[power_up_idx] != 0;
    }

    void DecPowerUp_434940(s32 power_up_idx)
    {
        field_6F4_power_up_timers[power_up_idx]--;
    }

    void ChangeMultipliers_4766B0(s32 amount)
    {
        field_6BC_multipliers.ChangeStatByAmount_4921B0(amount);
    }

    bool IsBustedNotObjective54_476700()
    {
        Ped* pPed;
        return field_28_wasted_or_busted && field_2C_death_countdown == 2 &&
            ((pPed = field_2C4_player_ped) == nullptr || pPed->get_objective_403A80() != Objectives::kObjective54);
    }

    bool IsBustedObjective54_476730()
    {
        Ped* pPed;
        return field_28_wasted_or_busted && field_2C_death_countdown == 2 && (pPed = field_2C4_player_ped) != nullptr &&
            pPed->get_objective_403A80() == Objectives::kObjective54;
    }

    s32 get_lives_4766C0()
    {
        return field_684_lives.get_value();
    }

    void AddCash_421990(s32 cash)
    {
        field_2D4_scores.AddCash_592620(cash);
    }

    s32 GetScore_421980()
    {
        return field_2D4_scores.GetScore_592370();
    }

    s32 get_multiplier_4766A0()
    {
        return field_6BC_multipliers.get_value();
    }

    void AddScore_41DC40(s32 score)
    {
        field_2D4_scores.AddCash_592620(score * field_6BC_multipliers.field_0_value);
    }

    Ped* GetCameraModePed()
    {
        return field_68_camera_mode == 2 ? field_2C8_aux_ped : field_2C4_player_ped;
    }

    // 0x4CCAE0
    s32 GetThrowCharge_4CCAE0()
    {
        return field_50_throw_charge;
    }

    // 0x4CCAB0
    void IncrementThrowCharge_4CCAB0()
    {
        s32 t = field_50_throw_charge;
        if (t >= 0)
        {
            this->field_50_throw_charge = t + 1;
        }

        if (this->field_50_throw_charge > 270)
        {
            this->field_50_throw_charge = 270;
        }
    }

    s32 GetThrowStrength_4CCAD0()
    {
        if (field_50_throw_charge <= 0x3C)
        {
            return field_50_throw_charge;
        }
        else
        {
            return 0x3C;
        }
    }

    bool GetInUse_461DB0()
    {
        return field_8E_in_use;
    }

    // 9.6f inline 0x41D020
    Ped* GetPlayerPed_41D020()
    {
        return field_2C4_player_ped;
    }

    bool IsThrowCharging_4CCB00()
    {
        return this->field_50_throw_charge >= 0;
    }

    void ResetThrowCharge_4A5180()
    {
        field_50_throw_charge = 0;
    }

    void SetDeathType_434950(s32 death_type)
    {
        field_44_death_type = death_type;
    }

    bool IsRemoteControlActive_4A5100()
    {
        s32 occupation;
        if (!field_2D0_aux_cam_active || !field_2C8_aux_ped ||
            (occupation = field_2C8_aux_ped->get_occupation_403980()) != PedOcupation::kEmpty)
        {
            return false;
        }
        return true;
    }

    // TODO: Ordering
    s32 ObjectTypeToWeaponType_443CB0(u8 object_type);

    u8 GetIdx_4881E0();
    void AddCarToHistory_5645B0(Car_BC* pNewCar);
    bool PromoteCarInHistory_564610(Car_BC* pCar, bool dont_modify);
    void PushCarInfo_564680(Car_BC* pCar);
    void SetKFCarWeapon_564710(Car_BC* pCar, s32 weapon_kind);
    void SetKFWeapon_564790(s32 weapon_idx);
    void ClearKFWeapon_5647D0();
    Weapon_30* GetCurrPlayerWeapon_5648F0();
    void SetWeapon_564910(Weapon_30* pWeapon);
    char_type HasAnyAmmo_564940();
    char_type AddWeaponWithAmmo_564960(s32 weapon_kind, u8 ammo);
    void SelectNextOrPrevWeapon_5649D0(char_type go_forwards, char_type go_backwards);
    void LoadCarWeapons_564AD0(Car_BC* pCar);
    void ClearCarWeapons_564B60();
    void CleanupEmptyAmmoWeapons_564B80();
    void UnloadCarWeapons_564C00();
    void RemovePlayerWeapons_564C50();
    void ClearPowerUps_564CC0();
    void ClearPowerUpsExceptJailCard_564CF0();
    char_type CollectPowerUp_564D60(s32 power_up_idx);
    void tick_down_powerups_565070();
    void RestorePowerUpsFromSave_5651F0(SavedPlayerStats_90* pSave);
    void TeleportToDebugCam_565310();
    void DebugWatchNearestCar_5653E0();
    void sub_565460();
    void InitPlayerPed_565490(Ped* pPed);
    void SetInputs_565740(u32 inputs);
    void IncrementGangRespectFromDebugKeys_565770(u8 count);
    void IncreaseWantedLevelFromDebugKeys_565860();
    void Hud_Controls_565890(u16 action);
    void HandleKeyRelease_566380(u16 key);
    void CharacterControls_566520();
    void ControlInputs_566820();
    void HandleControls_5668D0(Ped* pPed);
    void DoCarControlInputs_566C30(Car_BC* pCar);
    void DoPedControlInputs_566C80(Ped* pPed);
    void ShowDebugInfo_566EE0(char_type skip);
    void RespawnPlayer_5670B0();
    void Wasted_567130();
    void UpdateAuxPedDeath_567850();
    void Busted_5679E0();
    void UpdateCurrentZones_568520();
    void UpdateSoundListener_568630();
    void HandleDebugZoom_568670();
    void UpdateCamera_5686D0(Camera_0xBC* pCam);
    void Disconnect_568730();
    void Service_5687F0();
    void UpdatePaused_569410();
    void EndRemoteControl_569530();
    void ResetAuxCamera_5695A0();
    void StartRemoteControl_569600(Car_BC* pCar);
    void WatchCar_5696D0(Car_BC* pCar);
    void GetPosU8_569840(u8& x, u8& y, u8& z);
    Car_BC* GetPlayerCar_5698E0();
    void get_pos_569920(Fix16* out_x, Fix16* out_y, Fix16* out_z);
    void ChangeLifeCountByAmount_5699F0(s32 amount);
    void ColorScoreFromRemap_569A10();
    void SetScoreTextColour_569C20();
    void InitializePlayerState_569CB0();
    void DebugToggleRemoteControl_569E70();
    const char* GetDeathText_569F00();
    void DisableInputs_569F40();
    void DisableAllControls_569FF0();
    void EnableAllControls_56A000();
    void EnableKFMode_56A010();
    void DisableKFMode_56A020();
    void DisableEnterVehicles_56A030();
    void EnableEnterVehicles_56A040();
    void RestoreCarsFromSave_56A0F0();
    void CopyPlayerDataToSave_56A1A0(SavedPlayerStats_90* pSave);
    void UpdateGameFromSave_56A310(SavedPlayerStats_90* pSave);
    void ApplyCheats_56A490();
    void ClearInputs_56A6D0();

    // 0x56A740
    Player(u8 player_idx);

    // 0x56A940
    ~Player();

    u8 IsUser_41DC70()
    {
        return field_0_is_user;
    }

    Ped* GetPlayerPed_4A5130()
    {
        if (field_68_camera_mode == 2)
        {
            return field_2C8_aux_ped;
        }
        else
        {
            return field_2C4_player_ped;
        }
    }

    u8 get_idx_4219D0()
    {
        return field_2E_idx;
    }

    Ped* GetActivePed_4A5150()
    {
        return (field_68_camera_mode == 2 || field_68_camera_mode == 3) ? field_2C8_aux_ped : field_2C4_player_ped;
    }

    Camera_0xBC* get_camera_434900()
    {
        if (field_68_camera_mode == 2 || field_68_camera_mode == 3)
        {
            return &field_208_aux_game_camera;
        }
        else
        {
            return &field_90_game_camera;
        }
    }

    u8 field_0_is_user;
    char_type field_1_unused;
    char_type field_2_unused;
    char_type field_3_pad;
    u32 field_4_inputs;
    Ang16 field_8_turn_speed;
    Ang16 field_A_turn_accel;
    Fix16 field_C_move_direction;
    s32 field_10_unused;
    s16 field_14_saved_ped_weapon_idx;
    s16 field_16_saved_car_weapon_idx;

    s16 field_18_pre_kf_weapon_kind;
    s16 field_1A_pre_kf_ammo;
    s32 field_1C_kf_weapon_kind;
    Car_BC* field_20_kf_car;
    s32 field_24_kf_car_id;

    char_type field_28_wasted_or_busted;
    char_type field_29_aux_ped_dying;
    char_type field_2A_pad;
    char_type field_2B_pad;
    s16 field_2C_death_countdown;
    u8 field_2E_idx;
    char_type field_2F_disable_all_controls;
    char_type field_30_disable_enter_vehicles;
    char_type field_31_kf_weapon_mode;
    char_type field_32_pad;
    char_type field_33_pad;
    Gang_144* field_34_gang_curr_location;
    GmpMapZone* field_38_local_navigation_zone;
    GmpMapZone* field_3C_navigation_zone;
    GmpMapZone* field_40_arrow_blocker_zone;
    s32 field_44_death_type;
    char_type field_48_dbg_cam_follow_player;
    char_type field_49_pad;
    char_type field_4A_pad;
    char_type field_4B_pad;
    SoundObject_10* field_4C_sound_obj;
    s32 field_50_throw_charge;
    Car_BC* field_54_car_history[3];
    s32 field_60_finish_score_reached;
    char_type field_64_jumping;
    char_type field_65_pad;
    char_type field_66_pad;
    char_type field_67_pad;
    s32 field_68_camera_mode;
    s32 field_6C_in_debug_cam_mode;
    char_type field_70_dbg_cam_north;
    char_type field_71_dbg_cam_south;
    char_type field_72_dbg_cam_west;
    char_type field_73_dbg_cam_east;
    char_type field_74_dbg_cam_zooming_out;
    char_type field_75_dbg_cam_zooming_in;
    char_type field_76_dbg_zoom_in;
    char_type field_77_dbg_zoom_out;

    // Current inputs state
    bool field_78_now_forward_pressed;
    bool field_79_now_down_pressed;
    bool field_7A_now_left_pressed;
    bool field_7B_now_right_pressed;
    bool field_7C_now_attack_pressed;
    bool field_7D_now_enter_exit_pressed;
    bool field_7E_now_hand_brake_or_jump_pressed;
    bool field_7F_now_prev_weapon_pressed;
    bool field_80_now_next_weapon_pressed;
    bool field_81_now_special_1_pressed;
    bool field_82_now_special_2_pressed;
    bool field_83_now_special_3_pressed;

    // Previous inputs state
    bool field_84_was_special_1_pressed;
    bool field_85_was_special_2_pressed;
    bool field_86_was_special_3_pressed;
    bool field_87_was_next_weapon_pressed;
    bool field_88_was_prev_weapon_pressed;
    bool field_89_was_enter_exit_pressed;
    bool field_8A_was_hand_brake_or_jump_pressed;
    bool field_8B_was_forward_pressed;
    bool field_8C_was_down_pressed;
    bool field_8D_was_attack_pressed;

    bool field_8E_in_use;
    char_type field_8F_block_attack;
    Camera_0xBC field_90_game_camera;
    Camera_0xBC field_14C_view_camera;
    Camera_0xBC field_208_aux_game_camera;
    Ped* field_2C4_player_ped;
    Ped* field_2C8_aux_ped;
    Car_BC* field_2CC_watched_car;
    char_type field_2D0_aux_cam_active;
    char_type field_2D1_pad;
    char_type field_2D2_pad;
    char_type field_2D3_pad;
    PlayerScoreTracker_36C field_2D4_scores;
    char_type field_640_busted;
    char_type field_641_pad;
    char_type field_642_pad;
    char_type field_643_pad;
    PlayerCrimeStats_3C field_644_crime_stats;
    u16 field_680_traffic_spawn_counter;
    u16 field_682_traffic_spawn_threshold;
    RollingDigitCounter_38 field_684_lives;
    RollingDigitCounter_38 field_6BC_multipliers;
    u16 field_6F4_power_up_timers[17];
    s16 field_716_unused;
    Weapon_30* field_718_weapons[28];
    s16 field_788_curr_weapon_idx;
    char_type field_78A_show_quit_message;
    char_type field_78B_pad;
    s32 field_78C_hud_palette_type; // Usage: 2 = default, 7 = multiplayer if using gang remap
    u16 field_790_hud_palette;
    s16 field_792_unused;
    char_type field_794_is_chatting;
    char_type field_795_pad;
    s16 field_796_chat_text[79];
    s32 field_834_unused;
    s32 field_838_chat_text_idx;
    wchar_t field_83C_player_name[16];
};

inline bool TagGame_28::IsFugitivePed_434B60(Ped* pPed)
{
    return field_0_fugitive && field_0_fugitive->GetPlayerPed_41D020() == pPed;
}
