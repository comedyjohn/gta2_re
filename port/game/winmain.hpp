#pragma once

#include "core/function.hpp"
#include <windows.h>

void Start_GTA2Manager_5E4DE0();

void ErrorMsgBox_5E4EC0(LPCSTR lpText);

LRESULT WindowProc_5E4EE0(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

void laughing_blackwell_0x1EB54_sub_5E53C0(BYTE* a1);

void LoadBeginSceneCBPtr();

int WinMain_5E53F0(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd);

char Start_NetworkGame_5E5A30(HINSTANCE hInstance);

void GetGTA2Version_5E5D60(int* pVerMinor, int* pVerMajor);

void ResetFrameTimer_4DA830();

void UpdateGameScreenSize_5D8E00();

EXTERN_GLOBAL(s32, bStartNetworkGame_7081F0);
EXTERN_GLOBAL_ARRAY(u8, gSyncCheckData_6F58E0, 0x20);
void MakeInitialSyncData_4DB2E0(u8* pSyncData);
void CompareRemotePlayers_4DB440(u8* pLocalSyncData, u8* pRemoteSyncData);

EXTERN_GLOBAL_ARRAY(char_type, gWorkingDir_707F64, 652);

EXTERN_GLOBAL(u32, gLightingDrawFlag_7068F4);

EXTERN_GLOBAL(u32, gBufferMode_706B34);

EXTERN_GLOBAL(char_type, gLighting_626A09);

static inline u8 IsNetworkGame_434B10()
{
    return bStartNetworkGame_7081F0;
}
