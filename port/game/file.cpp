#include "file.hpp"
#include "Function.hpp"
#include "Globals.hpp"
#include "enums.hpp"
#include "error.hpp"
#include "memory.hpp"
#include <cstdio>
#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>

DEFINE_GLOBAL(s32, gbGlobalFileOpen_67D160, 0x67D160);
DEFINE_GLOBAL(FILE*, ghFile_67CFEC, 0x67CFEC);
DEFINE_GLOBAL_INIT(u16, gCdCheckFileCount_6252E0, 22, 0x6252E0);
DEFINE_GLOBAL_ARRAY_INIT(CdCheckFile_84,
                         gCdCheckFiles_6252E8,
                         22,
                         0x6252E8,
                         {"GTAudio\\1.wav", 31971388u},
                         {"GTAudio\\10.wav", 15985724u},
                         {"GTAudio\\101A.wav", 9838652u},
                         {"GTAudio\\10a.wav", 7992892u},
                         {"GTAudio\\11.wav", 15985724u},
                         {"GTAudio\\11a.wav", 7992892u},
                         {"GTAudio\\12.wav", 16932924u},
                         {"GTAudio\\2.wav", 15985724u},
                         {"GTAudio\\3.wav", 15985724u},
                         {"GTAudio\\4.wav", 15985724u},
                         {"GTAudio\\5.wav", 15985724u},
                         {"GTAudio\\5a.wav", 7992892u},
                         {"GTAudio\\6.wav", 15985724u},
                         {"GTAudio\\6a.wav", 7992892u},
                         {"GTAudio\\7.wav", 16003132u},
                         {"GTAudio\\7a.wav", 7992892u},
                         {"GTAudio\\8.wav", 15985724u},
                         {"GTAudio\\8a.wav", 7992892u},
                         {"GTAudio\\9.wav", 15985724u},
                         {"GTAudio\\9a.wav", 7992892u},
                         {"GTAudio\\A.wav", 23026196u},
                         {"GTAudio\\D.wav", 6532888u});

// 0x4A6B10
s32 File::GetFileSize_4A6B10(FILE* Stream)
{
    s32 oldPos = ::ftell(Stream);
    if (oldPos == -1)
    {
        FatalError_4A38C0(Gta2Error::FtellError, "C:\\Splitting\\Gta2\\Source\\File.cpp", 56);
    }

    if (::fseek(Stream, 0, SEEK_END))
    {
        FatalError_4A38C0(Gta2Error::FseekError, "C:\\Splitting\\Gta2\\Source\\File.cpp", 58);
    }

    s32 endPos = ::ftell(Stream);
    if (endPos == -1)
    {
        FatalError_4A38C0(Gta2Error::FtellError, "C:\\Splitting\\Gta2\\Source\\File.cpp", 60);
    }

    if (::fseek(Stream, oldPos, SEEK_SET))
    {
        FatalError_4A38C0(Gta2Error::FseekError, "C:\\Splitting\\Gta2\\Source\\File.cpp", 62);
    }

    return endPos;
}

// 0x4A6BB0
bool File::IsCdRomDrive_4A6BB0(char_type driveLetter)
{
    sprintf(gTmpBuffer_67C598, "%c:", driveLetter);
    // Silly return structure but needed to match (and somehow produces less code)
    if (GetDriveTypeA(gTmpBuffer_67C598) == DRIVE_CDROM)
    {
        return true;
    }
    return false;
}

// Copy protection: the file must have the expected size and must not be writable (so it is on the CD)
// 0x4A6BE0
void File::CheckReadOnlyFile_4A6BE0(const char_type* FileName, s32 expectedSize)
{
    FILE* hFile = ::fopen(FileName, "rb");
    if (!hFile)
    {
        FatalError_4A38C0(Gta2Error::SecurityFail, "C:\\Splitting\\Gta2\\Source\\File.cpp", 104);
    }

    if (GetFileSize_4A6B10(hFile) != expectedSize)
    {
        FatalError_4A38C0(Gta2Error::SecurityFail, "C:\\Splitting\\Gta2\\Source\\File.cpp", 108);
    }

    s32 closeRet = ::fclose(hFile);
    gbGlobalFileOpen_67D160 = 0;
    if (closeRet)
    {
        FatalError_4A38C0(Gta2Error::SecurityFail, "C:\\Splitting\\Gta2\\Source\\File.cpp", 113);
    }

    if (::fopen(FileName, "wb"))
    {
        FatalError_4A38C0(Gta2Error::SecurityFail, "C:\\Splitting\\Gta2\\Source\\File.cpp", 117);
    }
}

// 0x4A6C80
void* File::ReadFileToBuffer_4A6C80(const char_type* FileName, size_t* pAllocatedBufferSize)
{
    Error_SetName_4A0770(FileName);
    FILE* hFileRead1 = ::fopen(FileName, "rb");
    if (!hFileRead1)
    {
        FatalError_4A38C0(Gta2Error::FreeloaderEpisodeUnknown, "C:\\Splitting\\Gta2\\Source\\File.cpp", 141);
    }

    *pAllocatedBufferSize = GetFileSize_4A6B10(hFileRead1);
    if (::fclose(hFileRead1))
    {
        FatalError_4A38C0(Gta2Error::FileCloseError, "C:\\Splitting\\Gta2\\Source\\File.cpp", 145);
    }

    void* pBuffer = Memory::malloc_4FE4D0(*pAllocatedBufferSize);

    FILE* hFileRead2 = ::fopen(FileName, "rb");
    if (!hFileRead2)
    {
        ::free(pBuffer);
        FatalError_4A38C0(Gta2Error::FreeloaderEpisodeUnknown, "C:\\Splitting\\Gta2\\Source\\File.cpp", 151);
    }

    if (Read_4A6D90(pBuffer, *pAllocatedBufferSize, 1u, hFileRead2) != 1)
    {
        ::free(pBuffer);
        ::fclose(hFileRead2);
        FatalError_4A38C0(Gta2Error::FileReadFailure, "C:\\Splitting\\Gta2\\Source\\File.cpp", 158);
    }

    if (::fclose(hFileRead2))
    {
        ::free(pBuffer);
        FatalError_4A38C0(Gta2Error::FileCloseError, "C:\\Splitting\\Gta2\\Source\\File.cpp", 164);
    }

    return pBuffer;
}

// 0x4A6D90
size_t File::Read_4A6D90(void* Buffer, size_t ElementSize, size_t ElementCount, FILE* Stream)
{
    size_t ret = ::fread(Buffer, ElementSize, ElementCount, Stream);
    return ret;
}

// Reads the whole file into the caller's buffer, which holds at most *pMaxSize bytes
// 0x4A6DB0
size_t File::ReadFileToFixedBuffer_4A6DB0(const char_type* FileName, void* pBuffer, size_t* pMaxSize)
{
    Error_SetName_4A0770(FileName);
    FILE* hFile = ::fopen(FileName, "rb");
    if (!hFile)
    {
        FatalError_4A38C0(Gta2Error::FreeloaderEpisodeUnknown, "C:\\Splitting\\Gta2\\Source\\File.cpp", 192);
    }

    size_t size = GetFileSize_4A6B10(hFile);
    if (size > *pMaxSize)
    {
        ::fclose(hFile);
        FatalError_4A38C0(Gta2Error::FileTooLarge, "C:\\Splitting\\Gta2\\Source\\File.cpp", 198, static_cast<s32>(size - *pMaxSize));
    }

    if (Read_4A6D90(pBuffer, size, 1u, hFile) != 1)
    {
        ::fclose(hFile);
        FatalError_4A38C0(Gta2Error::FileReadFailure, "C:\\Splitting\\Gta2\\Source\\File.cpp", 204);
    }

    if (::fclose(hFile))
    {
        FatalError_4A38C0(Gta2Error::FileCloseError, "C:\\Splitting\\Gta2\\Source\\File.cpp", 208);
    }
    return size;
}

// 0x4A6E80
void File::WriteBufferToFile_4A6E80(const char_type* FileName, void* Buffer, size_t* pBufferSize)
{
    Error_SetName_4A0770(FileName);
    if (!*pBufferSize)
    {
        FatalError_4A38C0(Gta2Error::WritingZeroBytesToFile, "C:\\Splitting\\Gta2\\Source\\File.cpp", 228);
    }

    FILE* hFile = ::fopen(FileName, "wb");
    if (!hFile)
    {
        FatalError_4A38C0(Gta2Error::FreeloaderEpisodeUnknown, "C:\\Splitting\\Gta2\\Source\\File.cpp", 231);
    }

    if (Write_4A6F30(Buffer, *pBufferSize, 1u, hFile) != 1)
    {
        ::fclose(hFile);
        FatalError_4A38C0(Gta2Error::FileWriteFailure, "C:\\Splitting\\Gta2\\Source\\File.cpp", 237);
    }

    if (::fclose(hFile))
    {
        FatalError_4A38C0(Gta2Error::FileCloseError, "C:\\Splitting\\Gta2\\Source\\File.cpp", 241);
    }
}

// 0x4A6F30
size_t File::Write_4A6F30(void* Buffer, size_t ElementSize, size_t ElementCount, FILE* Stream)
{
    return ::fwrite(Buffer, ElementSize, ElementCount, Stream);
}

// 0x4A6F50
void File::AppendBufferToFile_4A6F50(const char_type* FileName, void* pBuffer, u32* pBufferSize)
{
    Error_SetName_4A0770(FileName);
    if (!*pBufferSize)
    {
        FatalError_4A38C0(Gta2Error::WritingZeroBytesToFile, "C:\\Splitting\\Gta2\\Source\\File.cpp", 261);
    }

    FILE* hFile = ::fopen(FileName, "ab"); // TODO: check
    if (!hFile)
    {
        FatalError_4A38C0(Gta2Error::FreeloaderEpisodeUnknown, "C:\\Splitting\\Gta2\\Source\\File.cpp", 264);
    }

    if (Write_4A6F30(pBuffer, *pBufferSize, 1u, hFile) != 1)
    {
        ::fclose(hFile);
        FatalError_4A38C0(Gta2Error::FileWriteFailure, "C:\\Splitting\\Gta2\\Source\\File.cpp", 270);
    }

    if (::fclose(hFile))
    {
        FatalError_4A38C0(Gta2Error::FileCloseError, "C:\\Splitting\\Gta2\\Source\\File.cpp", 274);
    }
}

// 0x4A7000
void File::CreateFile_4A7000(const char_type* FileName)
{
    Error_SetName_4A0770(FileName);

    FILE* hFile = ::fopen(FileName, "wb");
    if (!hFile)
    {
        FatalError_4A38C0(Gta2Error::FreeloaderEpisodeUnknown, "C:\\Splitting\\Gta2\\Source\\File.cpp", 296);
    }

    if (::fclose(hFile))
    {
        FatalError_4A38C0(Gta2Error::FileCloseError, "C:\\Splitting\\Gta2\\Source\\File.cpp", 300);
    }
}

// 0x4A7060
void File::Global_Open_4A7060(const char_type* FileName)
{
    if (gbGlobalFileOpen_67D160)
    {
        Global_Close_4A70C0();
    }

    Error_SetName_4A0770(FileName);

    ghFile_67CFEC = ::fopen(FileName, "rb");
    if (!ghFile_67CFEC)
    {
        FatalError_4A38C0(Gta2Error::FreeloaderEpisodeUnknown, "C:\\Splitting\\Gta2\\Source\\File.cpp", 323);
    }

    gbGlobalFileOpen_67D160 = 1;
}

// 0x4A70C0
void File::Global_Close_4A70C0()
{
    if (gbGlobalFileOpen_67D160)
    {
        s32 close_result = ::fclose(ghFile_67CFEC);
        gbGlobalFileOpen_67D160 = 0;
        if (close_result)
        {
            FatalError_4A38C0(Gta2Error::FileCloseError, "C:\\Splitting\\Gta2\\Source\\File.cpp", 345);
        }
    }
}

// 0x4A7110
void File::Global_Close_UnChecked_4A7110()
{
    if (gbGlobalFileOpen_67D160)
    {
        ::fclose(ghFile_67CFEC);
        gbGlobalFileOpen_67D160 = 0;
    }
}

// 0x4A7140
void File::Global_Seek_4A7140(u32* pOffset)
{
    if (!gbGlobalFileOpen_67D160)
    {
        FatalError_4A38C0(Gta2Error::FileReadingNotOpenedFile, "C:\\Splitting\\Gta2\\Source\\File.cpp", 416);
    }

    if (::fseek(ghFile_67CFEC, *pOffset, 1))
    {
        File_Error_4A7190(14, 0, 0);
    }
}

// 0x4A7190
void File::File_Error_4A7190(s32 Code, s32 arg1, s32 arg2)
{
    Global_Close_UnChecked_4A7110();
    FatalError_4A38C0(Code, "C:\\Splitting\\Gta2\\Source\\File.cpp", 398, arg1, arg2);
}

// 0x4A71C0
void File::Global_Read_4A71C0(void* pBuffer, const u32& pBufferSize)
{
    if (!gbGlobalFileOpen_67D160)
    {
        FatalError_4A38C0(Gta2Error::FileReadingNotOpenedFile, "C:\\Splitting\\Gta2\\Source\\File.cpp", 438);
    }

    if (Read_4A6D90(pBuffer, pBufferSize, 1u, ghFile_67CFEC) != 1)
    {
        File_Error_4A7190(15, 0, 0);
    }
}

// 0x4A7210
bool File::Global_Read_4A7210(void* Buffer, u32* pSize)
{
    if (!gbGlobalFileOpen_67D160)
    {
        FatalError_4A38C0(Gta2Error::FileReadingNotOpenedFile, "C:\\Splitting\\Gta2\\Source\\File.cpp", 460);
    }
    return (Read_4A6D90(Buffer, *pSize, 1u, ghFile_67CFEC) == 1) ? true : false;
}

// 0x4A7250
size_t File::GetRemainderSize_4A7250(void* Buffer, u32* pMaxFileSize)
{
    if (!gbGlobalFileOpen_67D160)
    {
        FatalError_4A38C0(Gta2Error::FileReadingNotOpenedFile, "C:\\Splitting\\Gta2\\Source\\File.cpp", 487);
    }

    s32 curPos = ::ftell(ghFile_67CFEC);
    if (curPos == -1)
    {
        File_Error_4A7190(13, 0, 0);
    }

    if (::fseek(ghFile_67CFEC, 0, SEEK_END))
    {
        File_Error_4A7190(14, 0, 0);
    }

    s32 endPos = ::ftell(ghFile_67CFEC);
    if (endPos == -1)
    {
        File_Error_4A7190(13, 0, 0);
    }

    if (::fseek(ghFile_67CFEC, curPos, 0))
    {
        File_Error_4A7190(14, 0, 0);
    }

    size_t remainderSize = endPos - curPos;
    if (remainderSize > *pMaxFileSize)
    {
        File_Error_4A7190(1022, remainderSize - *pMaxFileSize, 0);
    }

    if (Read_4A6D90(Buffer, remainderSize, 1u, ghFile_67CFEC) != 1)
    {
        File_Error_4A7190(15, 0, 0);
    }

    return remainderSize;
}

// 0x4A7340
char_type File::SkipWhitespace_4A7340(FILE* Stream)
{
    char_type next_char = 0;

    while (true)
    {
        next_char = ::fgetc(Stream);
        // note: feof = Stream->_flag & 0x10
        if (feof(Stream) || next_char == '\n')
        {
            next_char = 0;
            break;
        }

        if (next_char == ' ' || next_char == '\r' || next_char == '\t')
        {
            next_char = 0;
        }
        else
        {
            break;
        }
    }
    return next_char;
}