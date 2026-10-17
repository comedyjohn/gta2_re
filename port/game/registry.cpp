#include "registry.hpp"
#include "core/enums.hpp"
#include "core/function.hpp"
#include "error.hpp"
#include <cstdio>
#include <windows.h>

HKEY kRegistryRootKey = HKEY_LOCAL_MACHINE; // Note: should be const, but can't be

DEFINE_GLOBAL(Registry, gRegistry, 0x6FF968);

// 0x586A00
bool Registry::OpenSoundRoot(PHKEY phk_result)
{
    if (::RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\DMA Design Ltd\\GTA2\\Sound", 0, KEY_ALL_ACCESS, phk_result) != ERROR_SUCCESS)
    {
        DWORD disposition;

        if (::RegCreateKeyExA(HKEY_LOCAL_MACHINE,
                              "SOFTWARE\\DMA Design Ltd\\GTA2\\Sound",
                              0,
                              const_cast<LPSTR>(""), // lpClass: the API takes a writable string, byte_67DC88
                              0,
                              KEY_ALL_ACCESS,
                              nullptr,
                              phk_result,
                              &disposition) != ERROR_SUCCESS)
        {
            FatalError_4A38C0(Gta2Error::kCreateRegistryKeyFail, "C:\\Splitting\\Gta2\\Source\\registry.cpp", 58);
        }
        return false;
    }
    return true;
}

// 0x586A70
bool Registry::GetSoundSettting(const char_type* lp_value_name)
{
    HKEY h_key;
    DWORD cb_data;
    u8 data[4];

    bool setting_read_ok = false;
    if (OpenSoundRoot(&h_key))
    {
        cb_data = sizeof(DWORD);
        if (::RegQueryValueExA(h_key, lp_value_name, nullptr, nullptr, data, &cb_data) == ERROR_SUCCESS)
        {
            setting_read_ok = true;
        }
    }

    if (::RegCloseKey(h_key) != ERROR_SUCCESS)
    {
        FatalError_4A38C0(Gta2Error::kCloseRegistryKeyFail, "C:\\Splitting\\Gta2\\Source\\registry.cpp", 109);
    }
    return setting_read_ok;
}

// 0x586AE0
s32 Registry::GetOrCreateSoundSetting(const char_type* lp_value_name, s32 value)
{
    HKEY h_key;
    OpenSoundRoot(&h_key);

    DWORD cb_data = sizeof(DWORD);
    DWORD data;
    if (::RegQueryValueExA(h_key, lp_value_name, nullptr, nullptr, reinterpret_cast<BYTE*>(&data), &cb_data) != ERROR_SUCCESS)
    {
        if (::RegSetValueExA(h_key, lp_value_name, 0, REG_DWORD, reinterpret_cast<const BYTE*>(&value), sizeof(DWORD)) != ERROR_SUCCESS)
        {
            FatalError_4A38C0(Gta2Error::kSetRegistryValueFail, "C:\\Splitting\\Gta2\\Source\\registry.cpp", 138);
        }

        data = value;
    }

    if (::RegCloseKey(h_key) != ERROR_SUCCESS)
    {
        FatalError_4A38C0(Gta2Error::kCloseRegistryKeyFail, "C:\\Splitting\\Gta2\\Source\\registry.cpp", 146);
    }

    return data;
}

// 0x586B80
void Registry::SetSoundSetting(const char_type* lp_value_name, s32 data)
{
    HKEY h_key;
    if (OpenSoundRoot(&h_key))
    {
        if (RegSetValueExA(h_key, lp_value_name, 0, REG_DWORD, reinterpret_cast<const BYTE*>(&data), sizeof(DWORD)) != ERROR_SUCCESS)
        {
            FatalError_4A38C0(Gta2Error::kSetRegistryValueFail, "C:\\Splitting\\Gta2\\Source\\registry.cpp", 169);
        }
    }

    if (RegCloseKey(h_key) != ERROR_SUCCESS)
    {
        FatalError_4A38C0(Gta2Error::kCloseRegistryKeyFail, "C:\\Splitting\\Gta2\\Source\\registry.cpp", 175);
    }
}

// 0x586BF0
void Registry::ClearOrDeleteSoundSetting(const char_type* lp_value_name, char_type clear)
{
    HKEY h_key;
    u32 data = 0;
    if (OpenSoundRoot(&h_key))
    {
        if (clear)
        {
            RegSetValueExA(h_key, lp_value_name, 0, REG_DWORD, reinterpret_cast<const BYTE*>(&data), sizeof(DWORD));
            // note: wrong register fixed by adding else instead of early return + dup'ed close call
        }
        else
        {
            RegDeleteValueA(h_key, lp_value_name);
        }
    }
    RegCloseKey(h_key); // bug: closes even if open fails
}

// 0x586C60
char_type Registry::OpenDebugRoot(PHKEY phk_result)
{
    DWORD disposition;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\DMA Design Ltd\\GTA2\\Debug", 0, KEY_ALL_ACCESS, phk_result) != ERROR_SUCCESS)
    {
        if (RegCreateKeyExA(HKEY_LOCAL_MACHINE,
                            "SOFTWARE\\DMA Design Ltd\\GTA2\\Debug",
                            0,
                            const_cast<LPSTR>(""), // lpClass: the API takes a writable string, byte_67DC88
                            0,
                            KEY_ALL_ACCESS,
                            nullptr,
                            phk_result,
                            &disposition) != ERROR_SUCCESS)
        {
            FatalError_4A38C0(Gta2Error::kCreateRegistryKeyFail, "C:\\Splitting\\Gta2\\Source\\registry.cpp", 232);
        }
        return 0;
    }
    return 1;
}

// 0x586CD0
char_type Registry::OpenOptionsRoot(PHKEY phk_result)
{
    DWORD disposition;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\DMA Design Ltd\\GTA2\\Option", 0, KEY_ALL_ACCESS, phk_result) != ERROR_SUCCESS)
    {
        if (RegCreateKeyExA(HKEY_LOCAL_MACHINE,
                            "SOFTWARE\\DMA Design Ltd\\GTA2\\Option",
                            0,
                            const_cast<LPSTR>(""), // lpClass: the API takes a writable string, byte_67DC88
                            0,
                            KEY_ALL_ACCESS,
                            nullptr,
                            phk_result,
                            &disposition) != ERROR_SUCCESS)
        {
            FatalError_4A38C0(Gta2Error::kCreateRegistryKeyFail, "C:\\Splitting\\Gta2\\Source\\registry.cpp", 265);
        }
        return 0;
    }
    return 1;
}

// 0x586D40
char_type Registry::OpenControlRoot(PHKEY phk_result)
{
    DWORD disposition;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\DMA Design Ltd\\GTA2\\Control", 0, KEY_ALL_ACCESS, phk_result) != ERROR_SUCCESS)
    {
        if (RegCreateKeyExA(HKEY_LOCAL_MACHINE,
                            "SOFTWARE\\DMA Design Ltd\\GTA2\\Control",
                            0,
                            const_cast<LPSTR>(""), // lpClass: the API takes a writable string, byte_67DC88
                            0,
                            KEY_ALL_ACCESS,
                            nullptr,
                            phk_result,
                            &disposition) != ERROR_SUCCESS)
        {
            FatalError_4A38C0(Gta2Error::kCreateRegistryKeyFail, "C:\\Splitting\\Gta2\\Source\\registry.cpp", 298);
        }
        return 0;
    }
    return 1;
}

// 0x586DB0
char_type Registry::OpenScreenRoot(PHKEY phk_result)
{
    DWORD disposition;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\DMA Design Ltd\\GTA2\\Screen", 0, KEY_ALL_ACCESS, phk_result) != ERROR_SUCCESS)
    {
        if (RegCreateKeyExA(HKEY_LOCAL_MACHINE,
                            "SOFTWARE\\DMA Design Ltd\\GTA2\\Screen",
                            0,
                            const_cast<LPSTR>(""), // lpClass: the API takes a writable string, byte_67DC88
                            0,
                            KEY_ALL_ACCESS,
                            nullptr,
                            phk_result,
                            &disposition))
        {
            FatalError_4A38C0(Gta2Error::kCreateRegistryKeyFail, "C:\\Splitting\\Gta2\\Source\\registry.cpp", 331);
        }
        return 0;
    }
    return 1;
}

// 0x586E20
bool Registry::GetDebugSetting(const char_type* lp_value_name)
{
    bool was_read = false;
    HKEY h_key;
    DWORD data;
    if (OpenDebugRoot(&h_key))
    {
        DWORD cb_data = sizeof(DWORD);
        if (RegQueryValueExA(h_key, lp_value_name, nullptr, nullptr, reinterpret_cast<BYTE*>(&data), &cb_data) == ERROR_SUCCESS)
        {
            was_read = true;
        }
    }

    if (RegCloseKey(h_key) != ERROR_SUCCESS)
    {
        FatalError_4A38C0(Gta2Error::kCloseRegistryKeyFail, "C:\\Splitting\\Gta2\\Source\\registry.cpp", 424);
    }

    return was_read;
}

// 0x586E90
void Registry::GetDebugSetting(const char_type* lp_value_name, LPBYTE lp_data, s32 data_size)
{
    HKEY h_key;
    DWORD cb_data;
    if (OpenDebugRoot(&h_key))
    {
        cb_data = static_cast<u16>(data_size);
        RegQueryValueExA(h_key, lp_value_name, nullptr, nullptr, lp_data, &cb_data);
    }

    if (RegCloseKey(h_key) != ERROR_SUCCESS)
    {
        FatalError_4A38C0(Gta2Error::kCloseRegistryKeyFail, "C:\\Splitting\\Gta2\\Source\\registry.cpp", 531);
    }
}

// 0x586F00
void Registry::GetOption(const char_type* lp_value_name, BYTE* buffer, s32 buffer_size)
{
    DWORD cb_data;
    HKEY h_key;
    *buffer = 0;

    if (OpenOptionsRoot(&h_key))
    {
        cb_data = static_cast<u16>(buffer_size);
        RegQueryValueExA(h_key, lp_value_name, nullptr, nullptr, buffer, &cb_data);
    }

    // bug: closing key even if not opened
    if (RegCloseKey(h_key) != ERROR_SUCCESS)
    {
        FatalError_4A38C0(Gta2Error::kCloseRegistryKeyFail, "C:\\Splitting\\Gta2\\Source\\registry.cpp", 586);
    }
}

// 0x586F70
s32 Registry::SetOption(const char_type* lp_value_name, s32 value)
{
    HKEY h_key;
    DWORD data;

    OpenOptionsRoot(&h_key);

    DWORD cb_data = sizeof(DWORD);

    if (RegQueryValueExA(h_key, lp_value_name, nullptr, nullptr, reinterpret_cast<BYTE*>(&data), &cb_data) != ERROR_SUCCESS)
    {
        if (RegSetValueExA(h_key, lp_value_name, 0, REG_DWORD, reinterpret_cast<const BYTE*>(&value), sizeof(DWORD)) != ERROR_SUCCESS)
        {
            FatalError_4A38C0(Gta2Error::kSetRegistryValueFail, "C:\\Splitting\\Gta2\\Source\\registry.cpp", 622);
        }
        data = value;
    }

    if (RegCloseKey(h_key) != ERROR_SUCCESS)
    {
        FatalError_4A38C0(Gta2Error::kCloseRegistryKeyFail, "C:\\Splitting\\Gta2\\Source\\registry.cpp", 629);
    }

    return data;
}

// 0x587010
s32 Registry::SetControlSetting(s32 control_idx, s32 value)
{
    HKEY h_key;
    DWORD data;
    OpenControlRoot(&h_key);

    DWORD cb_data = sizeof(DWORD);
    char_type value_name[4];
    sprintf(value_name, "%d", static_cast<BYTE>(control_idx));

    if (RegQueryValueExA(h_key, value_name, nullptr, nullptr, reinterpret_cast<BYTE*>(&data), &cb_data) != ERROR_SUCCESS)
    {
        if (RegSetValueExA(h_key, value_name, 0, REG_DWORD, reinterpret_cast<const BYTE*>(&value), sizeof(DWORD)) != ERROR_SUCCESS)
        {
            FatalError_4A38C0(Gta2Error::kSetRegistryValueFail, "C:\\Splitting\\Gta2\\Source\\registry.cpp", 667);
        }
        data = value;
    }

    if (RegCloseKey(h_key) != ERROR_SUCCESS)
    {
        FatalError_4A38C0(Gta2Error::kCloseRegistryKeyFail, "C:\\Splitting\\Gta2\\Source\\registry.cpp", 674);
    }

    return data;
}

void Registry::WriteControlSetting(s32 control_idx, s32 value)
{
    HKEY h_key;
    OpenControlRoot(&h_key);

    char_type value_name[4];
    sprintf(value_name, "%d", static_cast<BYTE>(control_idx));
    const DWORD data = value;
    if (RegSetValueExA(h_key, value_name, 0, REG_DWORD, reinterpret_cast<const BYTE*>(&data), sizeof(DWORD)) != ERROR_SUCCESS)
    {
        FatalError_4A38C0(Gta2Error::kSetRegistryValueFail, "C:\\Splitting\\Gta2\\Source\\registry.cpp", 0);
    }
    RegCloseKey(h_key);
}

// 0x5870D0
s32 Registry::GetScreenSetting(const char_type* lp_value_name, s32 default_value)
{
    HKEY h_key;
    OpenScreenRoot(&h_key);

    DWORD data;
    DWORD cb_data = sizeof(DWORD);
    if (RegQueryValueExA(h_key, lp_value_name, nullptr, nullptr, reinterpret_cast<BYTE*>(&data), &cb_data) != ERROR_SUCCESS)
    {
        if (RegSetValueExA(h_key, lp_value_name, 0, REG_DWORD, reinterpret_cast<const BYTE*>(&default_value), sizeof(DWORD)) !=
            ERROR_SUCCESS)
        {
            FatalError_4A38C0(Gta2Error::kSetRegistryValueFail, "C:\\Splitting\\Gta2\\Source\\registry.cpp", 709);
        }
        data = default_value;
    }

    if (RegCloseKey(h_key) != ERROR_SUCCESS)
    {
        FatalError_4A38C0(Gta2Error::kCloseRegistryKeyFail, "C:\\Splitting\\Gta2\\Source\\registry.cpp", 716);
    }

    return data;
}

// 0x587170
void Registry::SetScreenSetting(const char_type* lp_value_name, s32 data)
{
    HKEY h_key;
    OpenScreenRoot(&h_key);

    if (RegSetValueExA(h_key, lp_value_name, 0, REG_DWORD, reinterpret_cast<BYTE*>(&data), sizeof(DWORD)) != ERROR_SUCCESS)
    {
        FatalError_4A38C0(Gta2Error::kSetRegistryValueFail, "C:\\Splitting\\Gta2\\Source\\registry.cpp", 743);
    }

    if (RegCloseKey(h_key) != ERROR_SUCCESS)
    {
        FatalError_4A38C0(Gta2Error::kCloseRegistryKeyFail, "C:\\Splitting\\Gta2\\Source\\registry.cpp", 745);
    }
}

// 0x5871E0
void Registry::SetScreenSetting(const char_type* lp_value_name, BYTE* lp_data, u16 max_len)
{
    HKEY h_key; // [esp+8h] [ebp-8h] BYREF
    DWORD cb_data; // [esp+Ch] [ebp-4h] BYREF

    OpenScreenRoot(&h_key);

    cb_data = max_len;
    if (RegQueryValueExA(h_key, lp_value_name, nullptr, nullptr, lp_data, &cb_data) &&
        RegSetValueExA(h_key, lp_value_name, 0, 1U, lp_data, strlen(reinterpret_cast<const char_type*>(lp_data)) + 1))
    {
        FatalError_4A38C0(Gta2Error::kSetRegistryValueFail, "C:\\Splitting\\Gta2\\Source\\registry.cpp", 781);
    }

    if (RegCloseKey(h_key))
    {
        FatalError_4A38C0(Gta2Error::kCloseRegistryKeyFail, "C:\\Splitting\\Gta2\\Source\\registry.cpp", 787);
    }
}

// 0x587290
void Registry::Null()
{
}

// 0x5872A0
char_type Registry::SetBinary(HKEY h_key, const char_type* base_name, BYTE* lp_data, u32 data)
{
    char_type buffer[260];
    char_type ret = 0;

    sprintf(buffer, "%ss", base_name);
    if (RegSetValueExA(h_key, buffer, 0, REG_DWORD, reinterpret_cast<const BYTE*>(&data), sizeof(DWORD)) == ERROR_SUCCESS)
    {
        sprintf(buffer, "%sd", base_name);
        if (RegSetValueExA(h_key, buffer, 0, REG_BINARY, lp_data, data) == ERROR_SUCCESS)
        {
            ret = 1;
        }
    }
    return ret;
}

// 0x587340
bool Registry::GetBinary(HKEY h_key, const char_type* key_path, s32 value, LPBYTE lp_data)
{
    char_type value_name[260]; // [esp+14h] [ebp-104h] BYREF
    bool ret = false;

    sprintf(value_name, "%ss", key_path);
    DWORD v = GetIntSetting(h_key, value_name);
    if (v == static_cast<DWORD>(value))
    {
        sprintf(value_name, "%sd", key_path);
        ret = RegQueryValueExA(h_key, value_name, nullptr, nullptr, lp_data, &v) == ERROR_SUCCESS;
    }
    return ret;
}

// 0x5873E0
s32 Registry::GetInt(HKEY h_key, const char_type* sub_key)
{
    char_type key_path[260];
    sprintf(key_path, "%ss", sub_key);
    return GetIntSetting(h_key, key_path);
}

// 0x587420
char_type Registry::CreateNetworkRoot(PHKEY phk_result)
{
    DWORD disposition;
    char_type sub_key[260];

    strcpy(sub_key, "SOFTWARE\\DMA Design Ltd\\GTA2");
    strcat(sub_key, "\\Network");

    if (RegCreateKeyExA(kRegistryRootKey, sub_key, 0, const_cast<LPSTR>(""), 0, KEY_ALL_ACCESS, nullptr, phk_result, &disposition) !=
        ERROR_SUCCESS)
    {
        FatalError_4A38C0(Gta2Error::kCreateRegistryKeyFail, "C:\\Splitting\\Gta2\\Source\\registry.cpp", 910);
    }

    return 1;
}

// 0x5874E0
DWORD Registry::GetIntSetting(HKEY h_key, const char_type* lp_value_name)
{
    DWORD cb_data = 4;

    if (RegQueryValueExA(h_key,
                         lp_value_name,
                         nullptr,
                         reinterpret_cast<LPDWORD>(&h_key),
                         reinterpret_cast<LPBYTE>(&lp_value_name),
                         &cb_data) != ERROR_SUCCESS)
    {
        // Zeroing the value instead of returning 0 keeps VC6 from going branchless
        lp_value_name = nullptr;
    }

    return static_cast<DWORD>(reinterpret_cast<uintptr_t>(lp_value_name));
}

// 0x5875A0
s32 Registry::GetBinaryValue(HKEY h_key, const char_type* key_path, LPBYTE lp_data)
{
    s32 ret = 0;
    s32 size = gRegistry.GetInt(h_key, key_path);
    if (size)
    {
        if (gRegistry.GetBinary(h_key, key_path, size, lp_data))
        {
            ret = 1;
        }
    }
    return ret;
}

// 0x5875E0
s32 Registry::SetWStringValue(HKEY h_key, const char_type* key_path, const wchar_t* str)
{
    s32 ret = 0;
    u32 size = 2 * wcslen(str) + 2;
    if (gRegistry.SetBinary(h_key, key_path, reinterpret_cast<BYTE*>(const_cast<wchar_t*>(str)), size))
    {
        ret = 1;
    }
    return ret;
}

// 0x587620
char_type Registry::NetworkSettingExists(const char_type* lp_value_name)
{
    HKEY h_key;
    u32 data;
    DWORD cb_data;
    char_type ret = 0;
    if (CreateNetworkRoot(&h_key))
    {
        cb_data = sizeof(DWORD);
        if (RegQueryValueExA(h_key, lp_value_name, nullptr, nullptr, reinterpret_cast<BYTE*>(&data), &cb_data) == ERROR_SUCCESS)
        {
            ret = 1;
        }
    }

    if (RegCloseKey(h_key) != ERROR_SUCCESS)
    {
        FatalError_4A38C0(Gta2Error::kCloseRegistryKeyFail, "C:\\Splitting\\Gta2\\Source\\registry.cpp", 1020);
    }
    return ret;
}

// 0x587690
s32 Registry::SetNetworkSetting(const char_type* lp_value_name, s32 value)
{
    HKEY h_key;
    CreateNetworkRoot(&h_key);

    u32 stored_data;
    DWORD cb_data = sizeof(DWORD);
    if (RegQueryValueExA(h_key, lp_value_name, nullptr, nullptr, reinterpret_cast<BYTE*>(&stored_data), &cb_data) != ERROR_SUCCESS)
    {
        if (RegSetValueExA(h_key, lp_value_name, 0, REG_DWORD, reinterpret_cast<const BYTE*>(&value), sizeof(DWORD)) != ERROR_SUCCESS)
        {
            FatalError_4A38C0(Gta2Error::kSetRegistryValueFail, "C:\\Splitting\\Gta2\\Source\\registry.cpp", 1051);
        }
        stored_data = value;
    }

    if (RegCloseKey(h_key) != ERROR_SUCCESS)
    {
        FatalError_4A38C0(Gta2Error::kCloseRegistryKeyFail, "C:\\Splitting\\Gta2\\Source\\registry.cpp", 1058);
    }

    return stored_data;
}

// 0x587730
void Registry::SetNetworkSetting(const char_type* lp_value_name, BYTE data)
{
    HKEY h_key;
    if (CreateNetworkRoot(&h_key))
    {
        if (RegSetValueExA(h_key, lp_value_name, 0, REG_DWORD, &data, sizeof(DWORD)))
        {
            FatalError_4A38C0(Gta2Error::kSetRegistryValueFail, "C:\\Splitting\\Gta2\\Source\\registry.cpp", 1080);
        }
    }

    if (RegCloseKey(h_key) != ERROR_SUCCESS)
    {
        FatalError_4A38C0(Gta2Error::kCloseRegistryKeyFail, "C:\\Splitting\\Gta2\\Source\\registry.cpp", 1086);
    }
}

// 0x5877A0
char_type Registry::OpenPlayerRoot(PHKEY phk_result)
{
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\DMA Design Ltd\\GTA2\\Player", 0, KEY_ALL_ACCESS, phk_result) != ERROR_SUCCESS)
    {
        DWORD disposition;
        if (RegCreateKeyExA(HKEY_LOCAL_MACHINE,
                            "SOFTWARE\\DMA Design Ltd\\GTA2\\Player",
                            0,
                            const_cast<LPSTR>(""),
                            0,
                            KEY_ALL_ACCESS,
                            nullptr,
                            phk_result,
                            &disposition) != ERROR_SUCCESS)
        {
            FatalError_4A38C0(Gta2Error::kCreateRegistryKeyFail, "C:\\Splitting\\Gta2\\Source\\registry.cpp", 1110);
        }
        return 0;
    }
    return 1;
}

// 0x587810
DWORD Registry::CreatePlayerSetting(const char_type* lp_value_name)
{
    HKEY h_key;
    DWORD data;
    if (OpenPlayerRoot(&h_key))
    {
        DWORD cb_data = sizeof(DWORD);
        if (RegQueryValueExA(h_key, lp_value_name, nullptr, nullptr, reinterpret_cast<BYTE*>(&data), &cb_data) != ERROR_SUCCESS)
        {
            data = 0;
        }
    }
    else
    {
        data = 0;
        if (RegSetValueExA(h_key, lp_value_name, 0, REG_DWORD, reinterpret_cast<BYTE*>(&data), sizeof(DWORD)) != ERROR_SUCCESS)
        {
            FatalError_4A38C0(Gta2Error::kSetRegistryValueFail, "C:\\Splitting\\Gta2\\Source\\registry.cpp", 1143);
        }
    }

    if (RegCloseKey(h_key) != ERROR_SUCCESS)
    {
        FatalError_4A38C0(Gta2Error::kCloseRegistryKeyFail, "C:\\Splitting\\Gta2\\Source\\registry.cpp", 1149);
    }

    return data;
}

// 0x5878C0
void Registry::SetPlayerSetting(const char_type* lp_value_name, DWORD data)
{
    HKEY h_key;
    if (OpenPlayerRoot(&h_key))
    {
        if (RegSetValueExA(h_key, lp_value_name, 0, REG_DWORD, reinterpret_cast<BYTE*>(&data), sizeof(DWORD)) != ERROR_SUCCESS)
        {
            FatalError_4A38C0(Gta2Error::kSetRegistryValueFail, "C:\\Splitting\\Gta2\\Source\\registry.cpp", 1171);
        }
    }

    if (RegCloseKey(h_key) != ERROR_SUCCESS)
    {
        FatalError_4A38C0(Gta2Error::kCloseRegistryKeyFail, "C:\\Splitting\\Gta2\\Source\\registry.cpp", 1177);
    }
}