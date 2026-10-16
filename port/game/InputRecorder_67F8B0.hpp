#pragma once

#include "Function.hpp"
#include <windows.h>

struct DIDEVICEOBJECTDATA;

struct AttractFile
{
    char field_0_path[256];
};

// InputRecorder_67F8B0::field_38_replay_state
enum class ReplayState : s32
{
    Live_0 = 0,     // reads the real input devices and records them (test\replay.rep)
    Playback_1 = 1, // plays back test\replay.rep, goes back to Live_0 when it ends
    Disabled_2 = 2, // nothing is recorded
    Attract_3 = 3,  // plays an attract replay, any input exits
};

#pragma pack(push)
#pragma pack(1)
// DirectInput keyboard and game pads (the object has no data of its own)
class InputDevices_1
{
  public:
    // TODO: Probably this call
    void free_input_devices_4987A0();

    void read_keyboard_and_gamepad_498CC0();

    void get_registry_controls_498C00();
    bool game_pads_init_498BA0();
    BOOL make_input_devices_498800(HINSTANCE hInstance);
    void set_game_pad_device_properties_4989C0();
    void input_devices_init_498C40(HINSTANCE hInstance);
    void SetAltKeyState_498CB0(u32 alt_down);
    bool acquire_input_device_498730(struct IDirectInputDeviceA* pGamePadDevice);
    bool game_pad_read_498D20();
    void AddKeyToInputBits_498C80(s32* pInputBits, DIDEVICEOBJECTDATA* device_data_keys);
    void read_input_device_498DA0(s32* input_bits, u8 bUnknown);

    u8 field_0;
};
#pragma pack(pop)

// One recorded input change: the rng index it happened at, the input bits and the random number
struct InputRecord_C
{
    s32 field_0_rng_idx;
    s32 field_4_inputs;
    s32 field_8_rng_rnd;
};

// The text header at the start of a replay file
struct ReplayHeader_10C
{
    char_type field_0_version[8];
    char_type field_8_date[30];
    char_type field_26_computer_name[30];
    char_type field_44_map_name[40];
    char_type field_6C_style_name[40];
    char_type field_94_script_name[40];
    char_type field_BC_debug_str[40];
    char_type field_E4_flags[40];
};

// Reads the inputs (get_input_bits_4CEAC0) and records / plays back replays
class InputRecorder_67F8B0
{
  public:
    enum
    {
        k_max_records = 40000
    };

    void StaticShutdown_4CDCD0(); // static dtor
    void clear_inputs_4CDCE0();
    void set_input_4CDCF0(s32 mask_idx);
    void clear_input_4CDD10(s32 mask_idx);
    bool should_ignore_input_4CDD80(s32 dinput_key);
    bool should_ignore_input_4CDDF0(s32 dinput_key);
    void save_replay_record_4CDE20(u32 inputs);
    void SaveReplay_4CDED0();
    void modify_inputs_4CDF30(s32 match_mask);
    void AppendReplayHeader_4CDF70();
    void LoadReplayHeader_4CE380(char_type bLoadDebug);
    void VerifyAttractFilesExist_4CE650();
    void GetNextAttrReplay_4CE6E0(char_type* pAttrPathOut);
    void input_init_replay_4CE740(HINSTANCE hInstance);
    void input_init_live_4CE880(HINSTANCE hInstance);
    void Shutdown_4CEA00();
    void replay_save_4CEA40(u32* input_bits);
    u32 get_input_bits_4CEAC0();
    void save_replay_inputs_4CED00(s32 input_old, s32 input_new);
    void DisplayInputBits_4CED90();
    bool RecOrPlayBackState_4CEDF0();
    void ShowInput_4CEE10();

    // 9.6f 0x44AA60
    bool inlined_check()
    {
        if (field_38_replay_state == ReplayState::Playback_1 || field_38_replay_state == ReplayState::Attract_3)
        {
            return true;
        }
        return false;
    }

    // 9.6f 0x44C050
    inline bool IsInputSet_44C050(s32 mask_idx)
    {
        return static_cast<u32>(field_4_input_bits & field_8_input_masks[mask_idx]) > 0;
    }

    // 9.6f 0x44AA80
    inline s32 GetLastRecRngIdx_44AA80()
    {
        return field_3C_rec_buff[field_7533C_used_recs_count - 1].field_0_rng_idx;
    }

    char_type field_0_bShutDown;
    s32 field_4_input_bits;
    s32 field_8_input_masks[12];
    ReplayState field_38_replay_state;
    InputRecord_C field_3C_rec_buff[k_max_records];
    u32 field_7533C_used_recs_count;
    u32 field_75340_rec_buf_idx;
    // front end input on/off ??
    char_type field_75344_bInputEnabled;
    u8 field_75345_attract_idx;
};

EXTERN_GLOBAL(InputRecorder_67F8B0, gInputRecorder_67F8B0);

EXTERN_GLOBAL(InputDevices_1*, gInputDevices_67B990);

// Control i's device (0 keyboard, 1 game pad) and binding: a DirectInput key code, or for a game pad
// a button number, or 224..227 for the stick or d-pad left, right, up, down. Default keys per control.
EXTERN_GLOBAL_ARRAY(s32, gMaybeDeviceType_67B91C, 12);
EXTERN_GLOBAL_ARRAY(u32, gPlayerControlsBinding_67B6E8, 12);
EXTERN_GLOBAL_ARRAY(s32, gDefaultControls_61A9E4, 12);
