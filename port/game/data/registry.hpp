#pragma once

#include "core/function.hpp"
#include <windows.h>

class Registry
{
  public:
    // todo: fix ordering

    bool OpenSoundRoot(PHKEY phk_result);

    bool GetSoundSettting(const char_type* lp_value_name);

    s32 GetOrCreateSoundSetting(const char_type* lp_value_name, s32 value);

    void SetSoundSetting(const char_type* lp_value_name, s32 data);

    void ClearOrDeleteSoundSetting(const char_type* lp_value_name, char_type clear);

    char_type OpenDebugRoot(PHKEY phk_result);

    char_type OpenOptionsRoot(PHKEY phk_result);

    char_type OpenControlRoot(PHKEY phk_result);

    char_type OpenScreenRoot(PHKEY phk_result);

    bool GetDebugSetting(const char_type* lp_value_name);

    void GetDebugSetting(const char_type* lp_value_name, LPBYTE lp_data, s32 data_size);

    void GetOption(const char_type* lp_value_name, BYTE* buffer, s32 buffer_size);

    s32 SetOption(const char_type* lp_value_name, s32 value);

    s32 SetControlSetting(s32 control_idx, s32 value);
    // Port: overwrites a control setting (Set_Control_Setting_587010 only fills in a missing one)
    void WriteControlSetting(s32 control_idx, s32 value);

    s32 GetScreenSetting(const char_type* lp_value_name, s32 default_value);

    void Null();

    char_type SetBinary(HKEY h_key, const char_type* base_name, BYTE* lp_data, u32 data);

    bool GetBinary(HKEY h_key, const char_type* key_path, s32 value, LPBYTE lp_data);

    s32 GetInt(HKEY h_key, const char_type* sub_key);

    char_type CreateNetworkRoot(PHKEY phk_result);

    DWORD GetIntSetting(HKEY h_key, const char_type* lp_value_name);

    static s32 GetBinaryValue(HKEY h_key, const char_type* key_path, LPBYTE lp_data);

    static s32 SetWStringValue(HKEY h_key, const char_type* key_path, const wchar_t* str);

    char_type NetworkSettingExists(const char_type* lp_value_name);

    s32 SetNetworkSetting(const char_type* lp_value_name, s32 value);

    void SetNetworkSetting(const char_type* lp_value_name, BYTE data);

    char_type OpenPlayerRoot(PHKEY phk_result);

    DWORD CreatePlayerSetting(const char_type* lp_value_name);

    void SetPlayerSetting(const char_type* lp_value_name, DWORD data);

    void SetScreenSetting(const char_type* lp_value_name, s32 data);

    void SetScreenSetting(const char_type* lp_value_name, BYTE* lp_data, u16 max_len);
};

EXTERN_GLOBAL(Registry, gRegistry);
