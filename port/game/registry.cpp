#include "registry.hpp"
#include "core/enums.hpp"
#include "core/function.hpp"
#include "error.hpp"
#include <cstdio>
#include <windows.h>

HKEY kRegistryRootKey = HKEY_LOCAL_MACHINE; // Note: should be const, but can't be

DEFINE_GLOBAL(Registry, gRegistry, 0x6FF968);

// 0x586A00
bool Registry::OpenSoundRoot(PHKEY phkResult)
{
    if (::RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\DMA Design Ltd\\GTA2\\Sound", 0, KEY_ALL_ACCESS, phkResult) != ERROR_SUCCESS)
    {
        DWORD dwDisposition;

        if (::RegCreateKeyExA(HKEY_LOCAL_MACHINE,
                              "SOFTWARE\\DMA Design Ltd\\GTA2\\Sound",
                              0,
                              const_cast<LPSTR>(""), // lpClass: the API takes a writable string, byte_67DC88
                              0,
                              KEY_ALL_ACCESS,
                              nullptr,
                              phkResult,
                              &dwDisposition) != ERROR_SUCCESS)
        {
            FatalError_4A38C0(Gta2Error::kCreateRegistryKeyFail, "C:\\Splitting\\Gta2\\Source\\registry.cpp", 58);
        }
        return false;
    }
    return true;
}

// 0x586A70
bool Registry::GetSoundSettting(const char_type* lpValueName)
{
    HKEY hKey;
    DWORD cbData;
    u8 data[4];

    bool settingReadOK = false;
    if (OpenSoundRoot(&hKey))
    {
        cbData = sizeof(DWORD);
        if (::RegQueryValueExA(hKey, lpValueName, nullptr, nullptr, data, &cbData) == ERROR_SUCCESS)
        {
            settingReadOK = true;
        }
    }

    if (::RegCloseKey(hKey) != ERROR_SUCCESS)
    {
        FatalError_4A38C0(Gta2Error::kCloseRegistryKeyFail, "C:\\Splitting\\Gta2\\Source\\registry.cpp", 109);
    }
    return settingReadOK;
}

// 0x586AE0
s32 Registry::GetOrCreateSoundSetting(const char_type* lpValueName, s32 value)
{
    HKEY hKey;
    OpenSoundRoot(&hKey);

    DWORD cbData = sizeof(DWORD);
    DWORD data;
    if (::RegQueryValueExA(hKey, lpValueName, nullptr, nullptr, reinterpret_cast<BYTE*>(&data), &cbData) != ERROR_SUCCESS)
    {
        if (::RegSetValueExA(hKey, lpValueName, 0, REG_DWORD, reinterpret_cast<const BYTE*>(&value), sizeof(DWORD)) != ERROR_SUCCESS)
        {
            FatalError_4A38C0(Gta2Error::kSetRegistryValueFail, "C:\\Splitting\\Gta2\\Source\\registry.cpp", 138);
        }

        data = value;
    }

    if (::RegCloseKey(hKey) != ERROR_SUCCESS)
    {
        FatalError_4A38C0(Gta2Error::kCloseRegistryKeyFail, "C:\\Splitting\\Gta2\\Source\\registry.cpp", 146);
    }

    return data;
}

// 0x586B80
void Registry::SetSoundSetting(const char_type* lpValueName, s32 Data)
{
    HKEY hKey;
    if (OpenSoundRoot(&hKey))
    {
        if (RegSetValueExA(hKey, lpValueName, 0, REG_DWORD, reinterpret_cast<const BYTE*>(&Data), sizeof(DWORD)) != ERROR_SUCCESS)
        {
            FatalError_4A38C0(Gta2Error::kSetRegistryValueFail, "C:\\Splitting\\Gta2\\Source\\registry.cpp", 169);
        }
    }

    if (RegCloseKey(hKey) != ERROR_SUCCESS)
    {
        FatalError_4A38C0(Gta2Error::kCloseRegistryKeyFail, "C:\\Splitting\\Gta2\\Source\\registry.cpp", 175);
    }
}

// 0x586BF0
void Registry::ClearOrDeleteSoundSetting(const char_type* lpValueName, char_type bClear)
{
    HKEY hKey;
    u32 Data = 0;
    if (OpenSoundRoot(&hKey))
    {
        if (bClear)
        {
            RegSetValueExA(hKey, lpValueName, 0, REG_DWORD, reinterpret_cast<const BYTE*>(&Data), sizeof(DWORD));
            // note: wrong register fixed by adding else instead of early return + dup'ed close call
        }
        else
        {
            RegDeleteValueA(hKey, lpValueName);
        }
    }
    RegCloseKey(hKey); // bug: closes even if open fails
}

// 0x586C60
char_type Registry::OpenDebugRoot(PHKEY phkResult)
{
    DWORD dwDisposition;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\DMA Design Ltd\\GTA2\\Debug", 0, KEY_ALL_ACCESS, phkResult) != ERROR_SUCCESS)
    {
        if (RegCreateKeyExA(HKEY_LOCAL_MACHINE,
                            "SOFTWARE\\DMA Design Ltd\\GTA2\\Debug",
                            0,
                            const_cast<LPSTR>(""), // lpClass: the API takes a writable string, byte_67DC88
                            0,
                            KEY_ALL_ACCESS,
                            nullptr,
                            phkResult,
                            &dwDisposition) != ERROR_SUCCESS)
        {
            FatalError_4A38C0(Gta2Error::kCreateRegistryKeyFail, "C:\\Splitting\\Gta2\\Source\\registry.cpp", 232);
        }
        return 0;
    }
    return 1;
}

// 0x586CD0
char_type Registry::OpenOptionsRoot(PHKEY phkResult)
{
    DWORD dwDisposition;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\DMA Design Ltd\\GTA2\\Option", 0, KEY_ALL_ACCESS, phkResult) != ERROR_SUCCESS)
    {
        if (RegCreateKeyExA(HKEY_LOCAL_MACHINE,
                            "SOFTWARE\\DMA Design Ltd\\GTA2\\Option",
                            0,
                            const_cast<LPSTR>(""), // lpClass: the API takes a writable string, byte_67DC88
                            0,
                            KEY_ALL_ACCESS,
                            nullptr,
                            phkResult,
                            &dwDisposition) != ERROR_SUCCESS)
        {
            FatalError_4A38C0(Gta2Error::kCreateRegistryKeyFail, "C:\\Splitting\\Gta2\\Source\\registry.cpp", 265);
        }
        return 0;
    }
    return 1;
}

// 0x586D40
char_type Registry::OpenControlRoot(PHKEY phkResult)
{
    DWORD dwDisposition;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\DMA Design Ltd\\GTA2\\Control", 0, KEY_ALL_ACCESS, phkResult) != ERROR_SUCCESS)
    {
        if (RegCreateKeyExA(HKEY_LOCAL_MACHINE,
                            "SOFTWARE\\DMA Design Ltd\\GTA2\\Control",
                            0,
                            const_cast<LPSTR>(""), // lpClass: the API takes a writable string, byte_67DC88
                            0,
                            KEY_ALL_ACCESS,
                            nullptr,
                            phkResult,
                            &dwDisposition) != ERROR_SUCCESS)
        {
            FatalError_4A38C0(Gta2Error::kCreateRegistryKeyFail, "C:\\Splitting\\Gta2\\Source\\registry.cpp", 298);
        }
        return 0;
    }
    return 1;
}

// 0x586DB0
char_type Registry::OpenScreenRoot(PHKEY phkResult)
{
    DWORD dwDisposition;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\DMA Design Ltd\\GTA2\\Screen", 0, KEY_ALL_ACCESS, phkResult) != ERROR_SUCCESS)
    {
        if (RegCreateKeyExA(HKEY_LOCAL_MACHINE,
                            "SOFTWARE\\DMA Design Ltd\\GTA2\\Screen",
                            0,
                            const_cast<LPSTR>(""), // lpClass: the API takes a writable string, byte_67DC88
                            0,
                            KEY_ALL_ACCESS,
                            nullptr,
                            phkResult,
                            &dwDisposition))
        {
            FatalError_4A38C0(Gta2Error::kCreateRegistryKeyFail, "C:\\Splitting\\Gta2\\Source\\registry.cpp", 331);
        }
        return 0;
    }
    return 1;
}

// 0x586E20
bool Registry::GetDebugSetting(const char_type* lpValueName)
{
    bool bRead = false;
    HKEY hKey;
    DWORD Data;
    if (OpenDebugRoot(&hKey))
    {
        DWORD cbData = sizeof(DWORD);
        if (RegQueryValueExA(hKey, lpValueName, nullptr, nullptr, reinterpret_cast<BYTE*>(&Data), &cbData) == ERROR_SUCCESS)
        {
            bRead = true;
        }
    }

    if (RegCloseKey(hKey) != ERROR_SUCCESS)
    {
        FatalError_4A38C0(Gta2Error::kCloseRegistryKeyFail, "C:\\Splitting\\Gta2\\Source\\registry.cpp", 424);
    }

    return bRead;
}

// 0x586E90
void Registry::GetDebugSetting(const char_type* lpValueName, LPBYTE lpData, s32 data_size)
{
    HKEY hKey;
    DWORD cbData;
    if (OpenDebugRoot(&hKey))
    {
        cbData = static_cast<u16>(data_size);
        RegQueryValueExA(hKey, lpValueName, nullptr, nullptr, lpData, &cbData);
    }

    if (RegCloseKey(hKey) != ERROR_SUCCESS)
    {
        FatalError_4A38C0(Gta2Error::kCloseRegistryKeyFail, "C:\\Splitting\\Gta2\\Source\\registry.cpp", 531);
    }
}

// 0x586F00
void Registry::GetOption(const char_type* lpValueName, BYTE* pBuffer, s32 bufferSize)
{
    DWORD cbData;
    HKEY hKey;
    *pBuffer = 0;

    if (OpenOptionsRoot(&hKey))
    {
        cbData = static_cast<u16>(bufferSize);
        RegQueryValueExA(hKey, lpValueName, nullptr, nullptr, pBuffer, &cbData);
    }

    // bug: closing key even if not opened
    if (RegCloseKey(hKey) != ERROR_SUCCESS)
    {
        FatalError_4A38C0(Gta2Error::kCloseRegistryKeyFail, "C:\\Splitting\\Gta2\\Source\\registry.cpp", 586);
    }
}

// 0x586F70
s32 Registry::SetOption(const char_type* lpValueName, s32 value)
{
    HKEY hKey;
    DWORD Data;

    OpenOptionsRoot(&hKey);

    DWORD cbData = sizeof(DWORD);

    if (RegQueryValueExA(hKey, lpValueName, nullptr, nullptr, reinterpret_cast<BYTE*>(&Data), &cbData) != ERROR_SUCCESS)
    {
        if (RegSetValueExA(hKey, lpValueName, 0, REG_DWORD, reinterpret_cast<const BYTE*>(&value), sizeof(DWORD)) != ERROR_SUCCESS)
        {
            FatalError_4A38C0(Gta2Error::kSetRegistryValueFail, "C:\\Splitting\\Gta2\\Source\\registry.cpp", 622);
        }
        Data = value;
    }

    if (RegCloseKey(hKey) != ERROR_SUCCESS)
    {
        FatalError_4A38C0(Gta2Error::kCloseRegistryKeyFail, "C:\\Splitting\\Gta2\\Source\\registry.cpp", 629);
    }

    return Data;
}

// 0x587010
s32 Registry::SetControlSetting(s32 control_idx, s32 value)
{
    HKEY hKey;
    DWORD Data;
    OpenControlRoot(&hKey);

    DWORD cbData = sizeof(DWORD);
    char_type ValueName[4];
    sprintf(ValueName, "%d", static_cast<BYTE>(control_idx));

    if (RegQueryValueExA(hKey, ValueName, nullptr, nullptr, reinterpret_cast<BYTE*>(&Data), &cbData) != ERROR_SUCCESS)
    {
        if (RegSetValueExA(hKey, ValueName, 0, REG_DWORD, reinterpret_cast<const BYTE*>(&value), sizeof(DWORD)) != ERROR_SUCCESS)
        {
            FatalError_4A38C0(Gta2Error::kSetRegistryValueFail, "C:\\Splitting\\Gta2\\Source\\registry.cpp", 667);
        }
        Data = value;
    }

    if (RegCloseKey(hKey) != ERROR_SUCCESS)
    {
        FatalError_4A38C0(Gta2Error::kCloseRegistryKeyFail, "C:\\Splitting\\Gta2\\Source\\registry.cpp", 674);
    }

    return Data;
}

void Registry::WriteControlSetting(s32 control_idx, s32 value)
{
    HKEY hKey;
    OpenControlRoot(&hKey);

    char_type ValueName[4];
    sprintf(ValueName, "%d", static_cast<BYTE>(control_idx));
    const DWORD data = value;
    if (RegSetValueExA(hKey, ValueName, 0, REG_DWORD, reinterpret_cast<const BYTE*>(&data), sizeof(DWORD)) != ERROR_SUCCESS)
    {
        FatalError_4A38C0(Gta2Error::kSetRegistryValueFail, "C:\\Splitting\\Gta2\\Source\\registry.cpp", 0);
    }
    RegCloseKey(hKey);
}

// 0x5870D0
s32 Registry::GetScreenSetting(const char_type* lpValueName, s32 default_value)
{
    HKEY hKey;
    OpenScreenRoot(&hKey);

    DWORD Data;
    DWORD cbData = sizeof(DWORD);
    if (RegQueryValueExA(hKey, lpValueName, nullptr, nullptr, reinterpret_cast<BYTE*>(&Data), &cbData) != ERROR_SUCCESS)
    {
        if (RegSetValueExA(hKey, lpValueName, 0, REG_DWORD, reinterpret_cast<const BYTE*>(&default_value), sizeof(DWORD)) != ERROR_SUCCESS)
        {
            FatalError_4A38C0(Gta2Error::kSetRegistryValueFail, "C:\\Splitting\\Gta2\\Source\\registry.cpp", 709);
        }
        Data = default_value;
    }

    if (RegCloseKey(hKey) != ERROR_SUCCESS)
    {
        FatalError_4A38C0(Gta2Error::kCloseRegistryKeyFail, "C:\\Splitting\\Gta2\\Source\\registry.cpp", 716);
    }

    return Data;
}

// 0x587170
void Registry::SetScreenSetting(const char_type* lpValueName, s32 Data)
{
    HKEY hKey;
    OpenScreenRoot(&hKey);

    if (RegSetValueExA(hKey, lpValueName, 0, REG_DWORD, reinterpret_cast<BYTE*>(&Data), sizeof(DWORD)) != ERROR_SUCCESS)
    {
        FatalError_4A38C0(Gta2Error::kSetRegistryValueFail, "C:\\Splitting\\Gta2\\Source\\registry.cpp", 743);
    }

    if (RegCloseKey(hKey) != ERROR_SUCCESS)
    {
        FatalError_4A38C0(Gta2Error::kCloseRegistryKeyFail, "C:\\Splitting\\Gta2\\Source\\registry.cpp", 745);
    }
}

// 0x5871E0
void Registry::SetScreenSetting(const char_type* lpValueName, BYTE* lpData, u16 maxLen)
{
    HKEY hKey; // [esp+8h] [ebp-8h] BYREF
    DWORD cbData; // [esp+Ch] [ebp-4h] BYREF

    OpenScreenRoot(&hKey);

    cbData = maxLen;
    if (RegQueryValueExA(hKey, lpValueName, nullptr, nullptr, lpData, &cbData) &&
        RegSetValueExA(hKey, lpValueName, 0, 1U, lpData, strlen(reinterpret_cast<const char_type*>(lpData)) + 1))
    {
        FatalError_4A38C0(Gta2Error::kSetRegistryValueFail, "C:\\Splitting\\Gta2\\Source\\registry.cpp", 781);
    }

    if (RegCloseKey(hKey))
    {
        FatalError_4A38C0(Gta2Error::kCloseRegistryKeyFail, "C:\\Splitting\\Gta2\\Source\\registry.cpp", 787);
    }
}

// 0x587290
void Registry::Null()
{
}

// 0x5872A0
char_type Registry::SetBinary(HKEY hKey, const char_type* base_name, BYTE* lpData, u32 Data)
{
    char_type Buffer[260];
    char_type ret = 0;

    sprintf(Buffer, "%ss", base_name);
    if (RegSetValueExA(hKey, Buffer, 0, REG_DWORD, reinterpret_cast<const BYTE*>(&Data), sizeof(DWORD)) == ERROR_SUCCESS)
    {
        sprintf(Buffer, "%sd", base_name);
        if (RegSetValueExA(hKey, Buffer, 0, REG_BINARY, lpData, Data) == ERROR_SUCCESS)
        {
            ret = 1;
        }
    }
    return ret;
}

// 0x587340
bool Registry::GetBinary(HKEY hKey, const char_type* keyPath, s32 value, LPBYTE lpData)
{
    char_type ValueName[260]; // [esp+14h] [ebp-104h] BYREF
    bool ret = false;

    sprintf(ValueName, "%ss", keyPath);
    DWORD v = GetIntSetting(hKey, ValueName);
    if (v == static_cast<DWORD>(value))
    {
        sprintf(ValueName, "%sd", keyPath);
        ret = RegQueryValueExA(hKey, ValueName, nullptr, nullptr, lpData, &v) == ERROR_SUCCESS;
    }
    return ret;
}

// 0x5873E0
s32 Registry::GetInt(HKEY hKey, const char_type* subKey)
{
    char_type keyPath[260];
    sprintf(keyPath, "%ss", subKey);
    return GetIntSetting(hKey, keyPath);
}

// 0x587420
char_type Registry::CreateNetworkRoot(PHKEY phkResult)
{
    DWORD dwDisposition;
    char_type SubKey[260];

    strcpy(SubKey, "SOFTWARE\\DMA Design Ltd\\GTA2");
    strcat(SubKey, "\\Network");

    if (RegCreateKeyExA(kRegistryRootKey, SubKey, 0, const_cast<LPSTR>(""), 0, KEY_ALL_ACCESS, nullptr, phkResult, &dwDisposition) !=
        ERROR_SUCCESS)
    {
        FatalError_4A38C0(Gta2Error::kCreateRegistryKeyFail, "C:\\Splitting\\Gta2\\Source\\registry.cpp", 910);
    }

    return 1;
}

// 0x5874E0
DWORD Registry::GetIntSetting(HKEY hKey, const char_type* lpValueName)
{
    DWORD cbData = 4;

    if (RegQueryValueExA(hKey, lpValueName, nullptr, reinterpret_cast<LPDWORD>(&hKey), reinterpret_cast<LPBYTE>(&lpValueName), &cbData) !=
        ERROR_SUCCESS)
    {
        // Zeroing the value instead of returning 0 keeps VC6 from going branchless
        lpValueName = nullptr;
    }

    return static_cast<DWORD>(reinterpret_cast<uintptr_t>(lpValueName));
}

// 0x5875A0
s32 Registry::GetBinaryValue(HKEY hKey, const char_type* keyPath, LPBYTE lpData)
{
    s32 ret = 0;
    s32 size = gRegistry.GetInt(hKey, keyPath);
    if (size)
    {
        if (gRegistry.GetBinary(hKey, keyPath, size, lpData))
        {
            ret = 1;
        }
    }
    return ret;
}

// 0x5875E0
s32 Registry::SetWStringValue(HKEY hKey, const char_type* keyPath, const wchar_t* pStr)
{
    s32 ret = 0;
    u32 size = 2 * wcslen(pStr) + 2;
    if (gRegistry.SetBinary(hKey, keyPath, reinterpret_cast<BYTE*>(const_cast<wchar_t*>(pStr)), size))
    {
        ret = 1;
    }
    return ret;
}

// 0x587620
char_type Registry::NetworkSettingExists(const char_type* lpValueName)
{
    HKEY hKey;
    u32 Data;
    DWORD cbData;
    char_type ret = 0;
    if (CreateNetworkRoot(&hKey))
    {
        cbData = sizeof(DWORD);
        if (RegQueryValueExA(hKey, lpValueName, nullptr, nullptr, reinterpret_cast<BYTE*>(&Data), &cbData) == ERROR_SUCCESS)
        {
            ret = 1;
        }
    }

    if (RegCloseKey(hKey) != ERROR_SUCCESS)
    {
        FatalError_4A38C0(Gta2Error::kCloseRegistryKeyFail, "C:\\Splitting\\Gta2\\Source\\registry.cpp", 1020);
    }
    return ret;
}

// 0x587690
s32 Registry::SetNetworkSetting(const char_type* lpValueName, s32 value)
{
    HKEY hKey;
    CreateNetworkRoot(&hKey);

    u32 Data;
    DWORD cbData = sizeof(DWORD);
    if (RegQueryValueExA(hKey, lpValueName, nullptr, nullptr, reinterpret_cast<BYTE*>(&Data), &cbData) != ERROR_SUCCESS)
    {
        if (RegSetValueExA(hKey, lpValueName, 0, REG_DWORD, reinterpret_cast<const BYTE*>(&value), sizeof(DWORD)) != ERROR_SUCCESS)
        {
            FatalError_4A38C0(Gta2Error::kSetRegistryValueFail, "C:\\Splitting\\Gta2\\Source\\registry.cpp", 1051);
        }
        Data = value;
    }

    if (RegCloseKey(hKey) != ERROR_SUCCESS)
    {
        FatalError_4A38C0(Gta2Error::kCloseRegistryKeyFail, "C:\\Splitting\\Gta2\\Source\\registry.cpp", 1058);
    }

    return Data;
}

// 0x587730
void Registry::SetNetworkSetting(const char_type* lpValueName, BYTE Data)
{
    HKEY hKey;
    if (CreateNetworkRoot(&hKey))
    {
        if (RegSetValueExA(hKey, lpValueName, 0, REG_DWORD, &Data, sizeof(DWORD)))
        {
            FatalError_4A38C0(Gta2Error::kSetRegistryValueFail, "C:\\Splitting\\Gta2\\Source\\registry.cpp", 1080);
        }
    }

    if (RegCloseKey(hKey) != ERROR_SUCCESS)
    {
        FatalError_4A38C0(Gta2Error::kCloseRegistryKeyFail, "C:\\Splitting\\Gta2\\Source\\registry.cpp", 1086);
    }
}

// 0x5877A0
char_type Registry::OpenPlayerRoot(PHKEY phkResult)
{
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\DMA Design Ltd\\GTA2\\Player", 0, KEY_ALL_ACCESS, phkResult) != ERROR_SUCCESS)
    {
        DWORD dwDisposition;
        if (RegCreateKeyExA(HKEY_LOCAL_MACHINE,
                            "SOFTWARE\\DMA Design Ltd\\GTA2\\Player",
                            0,
                            const_cast<LPSTR>(""),
                            0,
                            KEY_ALL_ACCESS,
                            nullptr,
                            phkResult,
                            &dwDisposition) != ERROR_SUCCESS)
        {
            FatalError_4A38C0(Gta2Error::kCreateRegistryKeyFail, "C:\\Splitting\\Gta2\\Source\\registry.cpp", 1110);
        }
        return 0;
    }
    return 1;
}

// 0x587810
DWORD Registry::CreatePlayerSetting(const char_type* lpValueName)
{
    HKEY hKey;
    DWORD Data;
    if (OpenPlayerRoot(&hKey))
    {
        DWORD cbData = sizeof(DWORD);
        if (RegQueryValueExA(hKey, lpValueName, nullptr, nullptr, reinterpret_cast<BYTE*>(&Data), &cbData) != ERROR_SUCCESS)
        {
            Data = 0;
        }
    }
    else
    {
        Data = 0;
        if (RegSetValueExA(hKey, lpValueName, 0, REG_DWORD, reinterpret_cast<BYTE*>(&Data), sizeof(DWORD)) != ERROR_SUCCESS)
        {
            FatalError_4A38C0(Gta2Error::kSetRegistryValueFail, "C:\\Splitting\\Gta2\\Source\\registry.cpp", 1143);
        }
    }

    if (RegCloseKey(hKey) != ERROR_SUCCESS)
    {
        FatalError_4A38C0(Gta2Error::kCloseRegistryKeyFail, "C:\\Splitting\\Gta2\\Source\\registry.cpp", 1149);
    }

    return Data;
}

// 0x5878C0
void Registry::SetPlayerSetting(const char_type* lpValueName, DWORD Data)
{
    HKEY hKey;
    if (OpenPlayerRoot(&hKey))
    {
        if (RegSetValueExA(hKey, lpValueName, 0, REG_DWORD, reinterpret_cast<BYTE*>(&Data), sizeof(DWORD)) != ERROR_SUCCESS)
        {
            FatalError_4A38C0(Gta2Error::kSetRegistryValueFail, "C:\\Splitting\\Gta2\\Source\\registry.cpp", 1171);
        }
    }

    if (RegCloseKey(hKey) != ERROR_SUCCESS)
    {
        FatalError_4A38C0(Gta2Error::kCloseRegistryKeyFail, "C:\\Splitting\\Gta2\\Source\\registry.cpp", 1177);
    }
}