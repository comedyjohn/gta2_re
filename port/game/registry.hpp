#pragma once

#include "Function.hpp"
#include <windows.h>

class Registry
{
  public:
    // todo: fix ordering

    bool Open_Sound_Root_586A00(PHKEY phkResult);

    bool Get_Sound_Settting_586A70(const char_type* lpValueName);

    s32 Set_Sound_Setting_586AE0(const char_type* lpValueName, s32 value);

    void Set_Sound_Setting_586B80(const char_type* lpValueName, s32 Data);

    void Clear_Or_Delete_Sound_Setting_586BF0(const char_type* lpValueName, char_type bClear);

    char_type Open_Debug_Root_586C60(PHKEY phkResult);

    char_type Open_Options_Root_586CD0(PHKEY phkResult);

    char_type Open_Control_Root_586D40(PHKEY phkResult);

    char_type Open_Screen_Root_586DB0(PHKEY phkResult);

    bool Get_Debug_Setting_586E20(const char_type* lpValueName);

    void Get_Debug_Setting_586E90(const char_type* lpValueName, LPBYTE lpData, s32 data_size);

    void Get_Option_586F00(const char_type* lpValueName, BYTE* pBuffer, s32 bufferSize);

    s32 Set_Option_586F70(const char_type* lpValueName, s32 value);

    s32 Set_Control_Setting_587010(s32 control_idx, s32 value);
    // Port: overwrites a control setting (Set_Control_Setting_587010 only fills in a missing one)
    void Write_Control_Setting(s32 control_idx, s32 value);

    s32 Get_Screen_Setting_5870D0(const char_type* lpValueName, s32 default_value);

    void null_587290();

    char_type Set_Binary_5872A0(HKEY hKey, const char_type* base_name, BYTE* lpData, u32 Data);

    bool Get_Binary_587340(HKEY hKey, const char_type* keyPath, s32 value, LPBYTE lpData);

    s32 Get_Int_5873E0(HKEY hKey, const char_type* subKey);

    char_type CreateNetworkRoot_587420(PHKEY phkResult);

    DWORD Get_Int_Setting_5874E0(HKEY hKey, const char_type* lpValueName);

    static s32 Get_Binary_Value_5875A0(HKEY hKey, const char_type* keyPath, LPBYTE lpData);

    static s32 Set_WString_Value_5875E0(HKEY hKey, const char_type* keyPath, const wchar_t* pStr);

    char_type Network_Setting_Exists_587620(const char_type* lpValueName);

    s32 Set_Network_Setting_587690(const char_type* lpValueName, s32 value);

    void Set_Network_Setting_587730(const char_type* lpValueName, BYTE Data);

    char_type Open_Player_Root_5877A0(PHKEY phkResult);

    DWORD Create_Player_Setting_587810(const char_type* lpValueName);

    void Set_Player_Setting_5878C0(const char_type* lpValueName, DWORD Data);

    void Set_Screen_Setting_587170(const char_type* lpValueName, s32 Data);

    void Set_Screen_Setting_5871E0(const char_type* lpValueName, BYTE* lpData, u16 maxLen);
};

EXTERN_GLOBAL(Registry, gRegistry_6FF968);
