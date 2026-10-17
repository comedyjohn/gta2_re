#pragma once

#include "core/function.hpp"
#include <windows.h>

class Registry
{
  public:
    // todo: fix ordering

    bool OpenSoundRoot(PHKEY phkResult);

    bool GetSoundSettting(const char_type* lpValueName);

    s32 GetOrCreateSoundSetting(const char_type* lpValueName, s32 value);

    void SetSoundSetting(const char_type* lpValueName, s32 Data);

    void ClearOrDeleteSoundSetting(const char_type* lpValueName, char_type bClear);

    char_type OpenDebugRoot(PHKEY phkResult);

    char_type OpenOptionsRoot(PHKEY phkResult);

    char_type OpenControlRoot(PHKEY phkResult);

    char_type OpenScreenRoot(PHKEY phkResult);

    bool GetDebugSetting(const char_type* lpValueName);

    void GetDebugSetting(const char_type* lpValueName, LPBYTE lpData, s32 data_size);

    void GetOption(const char_type* lpValueName, BYTE* pBuffer, s32 bufferSize);

    s32 SetOption(const char_type* lpValueName, s32 value);

    s32 SetControlSetting(s32 control_idx, s32 value);
    // Port: overwrites a control setting (Set_Control_Setting_587010 only fills in a missing one)
    void WriteControlSetting(s32 control_idx, s32 value);

    s32 GetScreenSetting(const char_type* lpValueName, s32 default_value);

    void Null();

    char_type SetBinary(HKEY hKey, const char_type* base_name, BYTE* lpData, u32 Data);

    bool GetBinary(HKEY hKey, const char_type* keyPath, s32 value, LPBYTE lpData);

    s32 GetInt(HKEY hKey, const char_type* subKey);

    char_type CreateNetworkRoot(PHKEY phkResult);

    DWORD GetIntSetting(HKEY hKey, const char_type* lpValueName);

    static s32 GetBinaryValue(HKEY hKey, const char_type* keyPath, LPBYTE lpData);

    static s32 SetWStringValue(HKEY hKey, const char_type* keyPath, const wchar_t* pStr);

    char_type NetworkSettingExists(const char_type* lpValueName);

    s32 SetNetworkSetting(const char_type* lpValueName, s32 value);

    void SetNetworkSetting(const char_type* lpValueName, BYTE Data);

    char_type OpenPlayerRoot(PHKEY phkResult);

    DWORD CreatePlayerSetting(const char_type* lpValueName);

    void SetPlayerSetting(const char_type* lpValueName, DWORD Data);

    void SetScreenSetting(const char_type* lpValueName, s32 Data);

    void SetScreenSetting(const char_type* lpValueName, BYTE* lpData, u16 maxLen);
};

EXTERN_GLOBAL(Registry, gRegistry);
