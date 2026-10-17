#pragma once

#include "core/function.hpp"
#include <stdio.h>
#include <windows.h>

class File
{
  public:
    // or static with explicit
    static s32 GetFileSize_4A6B10(FILE* Stream);

    static bool IsCdRomDrive_4A6BB0(char_type driveLetter);

    static void CheckReadOnlyFile_4A6BE0(const char_type* FileName, s32 expectedSize);
    static void* ReadFileToBuffer_4A6C80(const char_type* FileName, size_t* pAllocatedBufferSize);
    static size_t ReadFileToFixedBuffer_4A6DB0(const char_type* FileName, void* pBuffer, size_t* pMaxSize);

    static size_t Read_4A6D90(void* Buffer, size_t ElementSize, size_t ElementCount, FILE* Stream);

    static void WriteBufferToFile_4A6E80(const char_type* FileName, void* Buffer, size_t* pBufferSize);

    static size_t Write_4A6F30(void* Buffer, size_t ElementSize, size_t ElementCount, FILE* Stream);

    static void AppendBufferToFile_4A6F50(const char_type* FileName, void* Buffer, u32* pBufferSize);

    static void CreateFile_4A7000(const char_type* FileName);

    static void Global_Open_4A7060(const char_type* FileName);

    static void Global_Close_4A70C0();

    static void Global_Close_UnChecked_4A7110();

    static void Global_Seek_4A7140(u32* pOffset);

    static void File_Error_4A7190(s32 Code, s32 arg1, s32 arg2);

    static void Global_Read_4A71C0(void* pBuffer, const u32& pBufferSize);

    static bool Global_Read_4A7210(void* Buffer, u32* pSize);

    static size_t GetRemainderSize_4A7250(void* Buffer, u32* pMaxFileSize);

    static char_type SkipWhitespace_4A7340(FILE* Stream);
};

// Copy protection: a file of the game CD with its expected size
struct CdCheckFile_84
{
    char_type field_0_name[128];
    u32 field_80_size;
};

EXTERN_GLOBAL(u16, gCdCheckFileCount_6252E0);
EXTERN_GLOBAL_ARRAY(CdCheckFile_84, gCdCheckFiles_6252E8, 22);
