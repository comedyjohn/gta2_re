#pragma once

#include "core/function.hpp"
#include <stdio.h>
#include <windows.h>

class File
{
  public:
    // or static with explicit
    static s32 GetFileSize(FILE* stream);

    static bool IsCdRomDrive(char_type drive_letter);

    static void CheckReadOnlyFile(const char_type* file_name, s32 expected_size);
    static void* ReadFileToBuffer(const char_type* file_name, size_t* allocated_buffer_size);
    static size_t ReadFileToFixedBuffer(const char_type* file_name, void* buffer, size_t* max_size);

    static size_t Read(void* buffer, size_t element_size, size_t element_count, FILE* stream);

    static void WriteBufferToFile(const char_type* file_name, void* buffer, size_t* buffer_size);

    static size_t Write(void* buffer, size_t element_size, size_t element_count, FILE* stream);

    static void AppendBufferToFile(const char_type* file_name, void* buffer, u32* buffer_size);

    static void CreateFile(const char_type* file_name);

    static void GlobalOpen(const char_type* file_name);

    static void GlobalClose();

    static void GlobalCloseUnChecked();

    static void GlobalSeek(u32* offset);

    static void FileError(s32 code, s32 arg1, s32 arg2);

    static void GlobalRead(void* buffer, const u32& buffer_size);

    static bool GlobalRead(void* buffer, u32* size);

    static size_t GetRemainderSize(void* buffer, u32* max_file_size);

    static char_type SkipWhitespace(FILE* stream);
};

// Copy protection: a file of the game CD with its expected size
struct CdCheckFile
{
    char_type field_0_name[128];
    u32 field_80_size;
};

EXTERN_GLOBAL(u16, gCdCheckFileCount);
EXTERN_GLOBAL_ARRAY(CdCheckFile, gCdCheckFiles, 22);
