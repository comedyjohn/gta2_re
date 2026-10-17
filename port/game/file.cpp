#include "file.hpp"
#include "core/enums.hpp"
#include "core/function.hpp"
#include "core/memory.hpp"
#include "error.hpp"
#include <cstdio>
#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>

DEFINE_GLOBAL(s32, gbGlobalFileOpen_67D160, 0x67D160);
DEFINE_GLOBAL(FILE*, ghFile_67CFEC, 0x67CFEC);
DEFINE_GLOBAL_INIT(u16, gCdCheckFileCount_6252E0, 22, 0x6252E0);
DEFINE_GLOBAL_ARRAY_INIT(CdCheckFile,
                         gCdCheckFiles_6252E8,
                         22,
                         0x6252E8,
                         {"GTAudio\\1.wav", 31971388U},
                         {"GTAudio\\10.wav", 15985724U},
                         {"GTAudio\\101A.wav", 9838652U},
                         {"GTAudio\\10a.wav", 7992892U},
                         {"GTAudio\\11.wav", 15985724U},
                         {"GTAudio\\11a.wav", 7992892U},
                         {"GTAudio\\12.wav", 16932924U},
                         {"GTAudio\\2.wav", 15985724U},
                         {"GTAudio\\3.wav", 15985724U},
                         {"GTAudio\\4.wav", 15985724U},
                         {"GTAudio\\5.wav", 15985724U},
                         {"GTAudio\\5a.wav", 7992892U},
                         {"GTAudio\\6.wav", 15985724U},
                         {"GTAudio\\6a.wav", 7992892U},
                         {"GTAudio\\7.wav", 16003132U},
                         {"GTAudio\\7a.wav", 7992892U},
                         {"GTAudio\\8.wav", 15985724U},
                         {"GTAudio\\8a.wav", 7992892U},
                         {"GTAudio\\9.wav", 15985724U},
                         {"GTAudio\\9a.wav", 7992892U},
                         {"GTAudio\\A.wav", 23026196U},
                         {"GTAudio\\D.wav", 6532888U});

// 0x4A6B10
s32 File::GetFileSize(FILE* Stream)
{
    s32 oldPos = ::ftell(Stream);
    if (oldPos == -1)
    {
        FatalError_4A38C0(Gta2Error::kFtellError, "C:\\Splitting\\Gta2\\Source\\File.cpp", 56);
    }

    if (::fseek(Stream, 0, SEEK_END))
    {
        FatalError_4A38C0(Gta2Error::kFseekError, "C:\\Splitting\\Gta2\\Source\\File.cpp", 58);
    }

    s32 endPos = ::ftell(Stream);
    if (endPos == -1)
    {
        FatalError_4A38C0(Gta2Error::kFtellError, "C:\\Splitting\\Gta2\\Source\\File.cpp", 60);
    }

    if (::fseek(Stream, oldPos, SEEK_SET))
    {
        FatalError_4A38C0(Gta2Error::kFseekError, "C:\\Splitting\\Gta2\\Source\\File.cpp", 62);
    }

    return endPos;
}

// 0x4A6BB0
bool File::IsCdRomDrive(char_type driveLetter)
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
void File::CheckReadOnlyFile(const char_type* FileName, s32 expectedSize)
{
    FILE* hFile = ::fopen(FileName, "rb");
    if (!hFile)
    {
        FatalError_4A38C0(Gta2Error::kSecurityFail, "C:\\Splitting\\Gta2\\Source\\File.cpp", 104);
    }

    if (GetFileSize(hFile) != expectedSize)
    {
        FatalError_4A38C0(Gta2Error::kSecurityFail, "C:\\Splitting\\Gta2\\Source\\File.cpp", 108);
    }

    s32 closeRet = ::fclose(hFile);
    gbGlobalFileOpen_67D160 = 0;
    if (closeRet)
    {
        FatalError_4A38C0(Gta2Error::kSecurityFail, "C:\\Splitting\\Gta2\\Source\\File.cpp", 113);
    }

    if (::fopen(FileName, "wb"))
    {
        FatalError_4A38C0(Gta2Error::kSecurityFail, "C:\\Splitting\\Gta2\\Source\\File.cpp", 117);
    }
}

// 0x4A6C80
void* File::ReadFileToBuffer(const char_type* FileName, size_t* pAllocatedBufferSize)
{
    Error_SetName_4A0770(FileName);
    FILE* hFileRead1 = ::fopen(FileName, "rb");
    if (!hFileRead1)
    {
        FatalError_4A38C0(Gta2Error::kFreeloaderEpisodeUnknown, "C:\\Splitting\\Gta2\\Source\\File.cpp", 141);
    }

    *pAllocatedBufferSize = GetFileSize(hFileRead1);
    if (::fclose(hFileRead1))
    {
        FatalError_4A38C0(Gta2Error::kFileCloseError, "C:\\Splitting\\Gta2\\Source\\File.cpp", 145);
    }

    void* pBuffer = Memory::Malloc(*pAllocatedBufferSize);

    FILE* hFileRead2 = ::fopen(FileName, "rb");
    if (!hFileRead2)
    {
        ::free(pBuffer);
        FatalError_4A38C0(Gta2Error::kFreeloaderEpisodeUnknown, "C:\\Splitting\\Gta2\\Source\\File.cpp", 151);
    }

    if (Read(pBuffer, *pAllocatedBufferSize, 1U, hFileRead2) != 1)
    {
        ::free(pBuffer);
        ::fclose(hFileRead2);
        FatalError_4A38C0(Gta2Error::kFileReadFailure, "C:\\Splitting\\Gta2\\Source\\File.cpp", 158);
    }

    if (::fclose(hFileRead2))
    {
        ::free(pBuffer);
        FatalError_4A38C0(Gta2Error::kFileCloseError, "C:\\Splitting\\Gta2\\Source\\File.cpp", 164);
    }

    return pBuffer;
}

// 0x4A6D90
size_t File::Read(void* Buffer, size_t ElementSize, size_t ElementCount, FILE* Stream)
{
    size_t ret = ::fread(Buffer, ElementSize, ElementCount, Stream);
    return ret;
}

// Reads the whole file into the caller's buffer, which holds at most *pMaxSize bytes
// 0x4A6DB0
size_t File::ReadFileToFixedBuffer(const char_type* FileName, void* pBuffer, size_t* pMaxSize)
{
    Error_SetName_4A0770(FileName);
    FILE* hFile = ::fopen(FileName, "rb");
    if (!hFile)
    {
        FatalError_4A38C0(Gta2Error::kFreeloaderEpisodeUnknown, "C:\\Splitting\\Gta2\\Source\\File.cpp", 192);
    }

    size_t size = GetFileSize(hFile);
    if (size > *pMaxSize)
    {
        ::fclose(hFile);
        FatalError_4A38C0(Gta2Error::kFileTooLarge, "C:\\Splitting\\Gta2\\Source\\File.cpp", 198, static_cast<s32>(size - *pMaxSize));
    }

    if (Read(pBuffer, size, 1U, hFile) != 1)
    {
        ::fclose(hFile);
        FatalError_4A38C0(Gta2Error::kFileReadFailure, "C:\\Splitting\\Gta2\\Source\\File.cpp", 204);
    }

    if (::fclose(hFile))
    {
        FatalError_4A38C0(Gta2Error::kFileCloseError, "C:\\Splitting\\Gta2\\Source\\File.cpp", 208);
    }
    return size;
}

// 0x4A6E80
void File::WriteBufferToFile(const char_type* FileName, void* Buffer, size_t* pBufferSize)
{
    Error_SetName_4A0770(FileName);
    if (!*pBufferSize)
    {
        FatalError_4A38C0(Gta2Error::kWritingZeroBytesToFile, "C:\\Splitting\\Gta2\\Source\\File.cpp", 228);
    }

    FILE* hFile = ::fopen(FileName, "wb");
    if (!hFile)
    {
        FatalError_4A38C0(Gta2Error::kFreeloaderEpisodeUnknown, "C:\\Splitting\\Gta2\\Source\\File.cpp", 231);
    }

    if (Write(Buffer, *pBufferSize, 1U, hFile) != 1)
    {
        ::fclose(hFile);
        FatalError_4A38C0(Gta2Error::kFileWriteFailure, "C:\\Splitting\\Gta2\\Source\\File.cpp", 237);
    }

    if (::fclose(hFile))
    {
        FatalError_4A38C0(Gta2Error::kFileCloseError, "C:\\Splitting\\Gta2\\Source\\File.cpp", 241);
    }
}

// 0x4A6F30
size_t File::Write(void* Buffer, size_t ElementSize, size_t ElementCount, FILE* Stream)
{
    return ::fwrite(Buffer, ElementSize, ElementCount, Stream);
}

// 0x4A6F50
void File::AppendBufferToFile(const char_type* FileName, void* pBuffer, u32* pBufferSize)
{
    Error_SetName_4A0770(FileName);
    if (!*pBufferSize)
    {
        FatalError_4A38C0(Gta2Error::kWritingZeroBytesToFile, "C:\\Splitting\\Gta2\\Source\\File.cpp", 261);
    }

    FILE* hFile = ::fopen(FileName, "ab"); // TODO: check
    if (!hFile)
    {
        FatalError_4A38C0(Gta2Error::kFreeloaderEpisodeUnknown, "C:\\Splitting\\Gta2\\Source\\File.cpp", 264);
    }

    if (Write(pBuffer, *pBufferSize, 1U, hFile) != 1)
    {
        ::fclose(hFile);
        FatalError_4A38C0(Gta2Error::kFileWriteFailure, "C:\\Splitting\\Gta2\\Source\\File.cpp", 270);
    }

    if (::fclose(hFile))
    {
        FatalError_4A38C0(Gta2Error::kFileCloseError, "C:\\Splitting\\Gta2\\Source\\File.cpp", 274);
    }
}

// 0x4A7000
void File::CreateFile(const char_type* FileName)
{
    Error_SetName_4A0770(FileName);

    FILE* hFile = ::fopen(FileName, "wb");
    if (!hFile)
    {
        FatalError_4A38C0(Gta2Error::kFreeloaderEpisodeUnknown, "C:\\Splitting\\Gta2\\Source\\File.cpp", 296);
    }

    if (::fclose(hFile))
    {
        FatalError_4A38C0(Gta2Error::kFileCloseError, "C:\\Splitting\\Gta2\\Source\\File.cpp", 300);
    }
}

// 0x4A7060
void File::GlobalOpen(const char_type* FileName)
{
    if (gbGlobalFileOpen_67D160)
    {
        GlobalClose();
    }

    Error_SetName_4A0770(FileName);

    ghFile_67CFEC = ::fopen(FileName, "rb");
    if (!ghFile_67CFEC)
    {
        FatalError_4A38C0(Gta2Error::kFreeloaderEpisodeUnknown, "C:\\Splitting\\Gta2\\Source\\File.cpp", 323);
    }

    gbGlobalFileOpen_67D160 = 1;
}

// 0x4A70C0
void File::GlobalClose()
{
    if (gbGlobalFileOpen_67D160)
    {
        s32 close_result = ::fclose(ghFile_67CFEC);
        gbGlobalFileOpen_67D160 = 0;
        if (close_result)
        {
            FatalError_4A38C0(Gta2Error::kFileCloseError, "C:\\Splitting\\Gta2\\Source\\File.cpp", 345);
        }
    }
}

// 0x4A7110
void File::GlobalCloseUnChecked()
{
    if (gbGlobalFileOpen_67D160)
    {
        ::fclose(ghFile_67CFEC);
        gbGlobalFileOpen_67D160 = 0;
    }
}

// 0x4A7140
void File::GlobalSeek(u32* pOffset)
{
    if (!gbGlobalFileOpen_67D160)
    {
        FatalError_4A38C0(Gta2Error::kFileReadingNotOpenedFile, "C:\\Splitting\\Gta2\\Source\\File.cpp", 416);
    }

    if (::fseek(ghFile_67CFEC, *pOffset, 1))
    {
        FileError(14, 0, 0);
    }
}

// 0x4A7190
void File::FileError(s32 Code, s32 arg1, s32 arg2)
{
    GlobalCloseUnChecked();
    FatalError_4A38C0(Code, "C:\\Splitting\\Gta2\\Source\\File.cpp", 398, arg1, arg2);
}

// 0x4A71C0
void File::GlobalRead(void* pBuffer, const u32& pBufferSize)
{
    if (!gbGlobalFileOpen_67D160)
    {
        FatalError_4A38C0(Gta2Error::kFileReadingNotOpenedFile, "C:\\Splitting\\Gta2\\Source\\File.cpp", 438);
    }

    if (Read(pBuffer, pBufferSize, 1U, ghFile_67CFEC) != 1)
    {
        FileError(15, 0, 0);
    }
}

// 0x4A7210
bool File::GlobalRead(void* Buffer, u32* pSize)
{
    if (!gbGlobalFileOpen_67D160)
    {
        FatalError_4A38C0(Gta2Error::kFileReadingNotOpenedFile, "C:\\Splitting\\Gta2\\Source\\File.cpp", 460);
    }
    return (Read(Buffer, *pSize, 1U, ghFile_67CFEC) == 1) ? true : false;
}

// 0x4A7250
size_t File::GetRemainderSize(void* Buffer, u32* pMaxFileSize)
{
    if (!gbGlobalFileOpen_67D160)
    {
        FatalError_4A38C0(Gta2Error::kFileReadingNotOpenedFile, "C:\\Splitting\\Gta2\\Source\\File.cpp", 487);
    }

    s32 curPos = ::ftell(ghFile_67CFEC);
    if (curPos == -1)
    {
        FileError(13, 0, 0);
    }

    if (::fseek(ghFile_67CFEC, 0, SEEK_END))
    {
        FileError(14, 0, 0);
    }

    s32 endPos = ::ftell(ghFile_67CFEC);
    if (endPos == -1)
    {
        FileError(13, 0, 0);
    }

    if (::fseek(ghFile_67CFEC, curPos, 0))
    {
        FileError(14, 0, 0);
    }

    size_t remainderSize = endPos - curPos;
    if (remainderSize > *pMaxFileSize)
    {
        FileError(1022, remainderSize - *pMaxFileSize, 0);
    }

    if (Read(Buffer, remainderSize, 1U, ghFile_67CFEC) != 1)
    {
        FileError(15, 0, 0);
    }

    return remainderSize;
}

// 0x4A7340
char_type File::SkipWhitespace(FILE* Stream)
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