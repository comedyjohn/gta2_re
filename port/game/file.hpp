#pragma once

#include "core/function.hpp"
#include <stdio.h>
#include <windows.h>

class File
{
  public:
    // or static with explicit
    static s32 GetFileSize(FILE* Stream);

    static bool IsCdRomDrive(char_type driveLetter);

    static void CheckReadOnlyFile(const char_type* FileName, s32 expectedSize);
    static void* ReadFileToBuffer(const char_type* FileName, size_t* pAllocatedBufferSize);
    static size_t ReadFileToFixedBuffer(const char_type* FileName, void* pBuffer, size_t* pMaxSize);

    static size_t Read(void* Buffer, size_t ElementSize, size_t ElementCount, FILE* Stream);

    static void WriteBufferToFile(const char_type* FileName, void* Buffer, size_t* pBufferSize);

    static size_t Write(void* Buffer, size_t ElementSize, size_t ElementCount, FILE* Stream);

    static void AppendBufferToFile(const char_type* FileName, void* Buffer, u32* pBufferSize);

    static void CreateFile(const char_type* FileName);

    static void GlobalOpen(const char_type* FileName);

    static void GlobalClose();

    static void GlobalCloseUnChecked();

    static void GlobalSeek(u32* pOffset);

    static void FileError(s32 Code, s32 arg1, s32 arg2);

    static void GlobalRead(void* pBuffer, const u32& pBufferSize);

    static bool GlobalRead(void* Buffer, u32* pSize);

    static size_t GetRemainderSize(void* Buffer, u32* pMaxFileSize);

    static char_type SkipWhitespace(FILE* Stream);
};

// Copy protection: a file of the game CD with its expected size
struct CdCheckFile
{
    char_type field_0_name[128];
    u32 field_80_size;
};

EXTERN_GLOBAL(u16, gCdCheckFileCount_6252E0);
EXTERN_GLOBAL_ARRAY(CdCheckFile, gCdCheckFiles_6252E8, 22);
