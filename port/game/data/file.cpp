#include "data/file.hpp"
#include "core/enums.hpp"
#include "core/function.hpp"
#include "core/memory.hpp"
#include "error.hpp"
#include <cstdio>
#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>

DEFINE_GLOBAL(s32, gGbGlobalFileOpen, 0x67D160);
DEFINE_GLOBAL(FILE*, gGhFile, 0x67CFEC);
DEFINE_GLOBAL_INIT(u16, gCdCheckFileCount, 22, 0x6252E0);
DEFINE_GLOBAL_ARRAY_INIT(CdCheckFile,
                         gCdCheckFiles,
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
s32 File::GetFileSize(FILE* stream)
{
    s32 old_pos = ::ftell(stream);
    if (old_pos == -1)
    {
        FatalError_4A38C0(Gta2Error::kFtellError, "C:\\Splitting\\Gta2\\Source\\File.cpp", 56);
    }

    if (::fseek(stream, 0, SEEK_END))
    {
        FatalError_4A38C0(Gta2Error::kFseekError, "C:\\Splitting\\Gta2\\Source\\File.cpp", 58);
    }

    s32 end_pos = ::ftell(stream);
    if (end_pos == -1)
    {
        FatalError_4A38C0(Gta2Error::kFtellError, "C:\\Splitting\\Gta2\\Source\\File.cpp", 60);
    }

    if (::fseek(stream, old_pos, SEEK_SET))
    {
        FatalError_4A38C0(Gta2Error::kFseekError, "C:\\Splitting\\Gta2\\Source\\File.cpp", 62);
    }

    return end_pos;
}

// 0x4A6BB0
bool File::IsCdRomDrive(char_type drive_letter)
{
    sprintf(gTmpBuffer_67C598, "%c:", drive_letter);
    // Silly return structure but needed to match (and somehow produces less code)
    if (GetDriveTypeA(gTmpBuffer_67C598) == DRIVE_CDROM)
    {
        return true;
    }
    return false;
}

// Copy protection: the file must have the expected size and must not be writable (so it is on the CD)
// 0x4A6BE0
void File::CheckReadOnlyFile(const char_type* file_name, s32 expected_size)
{
    FILE* h_file = ::fopen(file_name, "rb");
    if (!h_file)
    {
        FatalError_4A38C0(Gta2Error::kSecurityFail, "C:\\Splitting\\Gta2\\Source\\File.cpp", 104);
    }

    if (GetFileSize(h_file) != expected_size)
    {
        FatalError_4A38C0(Gta2Error::kSecurityFail, "C:\\Splitting\\Gta2\\Source\\File.cpp", 108);
    }

    s32 close_ret = ::fclose(h_file);
    gGbGlobalFileOpen = 0;
    if (close_ret)
    {
        FatalError_4A38C0(Gta2Error::kSecurityFail, "C:\\Splitting\\Gta2\\Source\\File.cpp", 113);
    }

    if (::fopen(file_name, "wb"))
    {
        FatalError_4A38C0(Gta2Error::kSecurityFail, "C:\\Splitting\\Gta2\\Source\\File.cpp", 117);
    }
}

// 0x4A6C80
void* File::ReadFileToBuffer(const char_type* file_name, size_t* allocated_buffer_size)
{
    Error_SetName_4A0770(file_name);
    FILE* h_file_read1 = ::fopen(file_name, "rb");
    if (!h_file_read1)
    {
        FatalError_4A38C0(Gta2Error::kFreeloaderEpisodeUnknown, "C:\\Splitting\\Gta2\\Source\\File.cpp", 141);
    }

    *allocated_buffer_size = GetFileSize(h_file_read1);
    if (::fclose(h_file_read1))
    {
        FatalError_4A38C0(Gta2Error::kFileCloseError, "C:\\Splitting\\Gta2\\Source\\File.cpp", 145);
    }

    void* buffer = Memory::Malloc(*allocated_buffer_size);

    FILE* h_file_read2 = ::fopen(file_name, "rb");
    if (!h_file_read2)
    {
        ::free(buffer);
        FatalError_4A38C0(Gta2Error::kFreeloaderEpisodeUnknown, "C:\\Splitting\\Gta2\\Source\\File.cpp", 151);
    }

    if (Read(buffer, *allocated_buffer_size, 1U, h_file_read2) != 1)
    {
        ::free(buffer);
        ::fclose(h_file_read2);
        FatalError_4A38C0(Gta2Error::kFileReadFailure, "C:\\Splitting\\Gta2\\Source\\File.cpp", 158);
    }

    if (::fclose(h_file_read2))
    {
        ::free(buffer);
        FatalError_4A38C0(Gta2Error::kFileCloseError, "C:\\Splitting\\Gta2\\Source\\File.cpp", 164);
    }

    return buffer;
}

// 0x4A6D90
size_t File::Read(void* buffer, size_t element_size, size_t element_count, FILE* stream)
{
    size_t ret = ::fread(buffer, element_size, element_count, stream);
    return ret;
}

// Reads the whole file into the caller's buffer, which holds at most *pMaxSize bytes
// 0x4A6DB0
size_t File::ReadFileToFixedBuffer(const char_type* file_name, void* buffer, size_t* max_size)
{
    Error_SetName_4A0770(file_name);
    FILE* h_file = ::fopen(file_name, "rb");
    if (!h_file)
    {
        FatalError_4A38C0(Gta2Error::kFreeloaderEpisodeUnknown, "C:\\Splitting\\Gta2\\Source\\File.cpp", 192);
    }

    size_t size = GetFileSize(h_file);
    if (size > *max_size)
    {
        ::fclose(h_file);
        FatalError_4A38C0(Gta2Error::kFileTooLarge, "C:\\Splitting\\Gta2\\Source\\File.cpp", 198, static_cast<s32>(size - *max_size));
    }

    if (Read(buffer, size, 1U, h_file) != 1)
    {
        ::fclose(h_file);
        FatalError_4A38C0(Gta2Error::kFileReadFailure, "C:\\Splitting\\Gta2\\Source\\File.cpp", 204);
    }

    if (::fclose(h_file))
    {
        FatalError_4A38C0(Gta2Error::kFileCloseError, "C:\\Splitting\\Gta2\\Source\\File.cpp", 208);
    }
    return size;
}

// 0x4A6E80
void File::WriteBufferToFile(const char_type* file_name, void* buffer, size_t* buffer_size)
{
    Error_SetName_4A0770(file_name);
    if (!*buffer_size)
    {
        FatalError_4A38C0(Gta2Error::kWritingZeroBytesToFile, "C:\\Splitting\\Gta2\\Source\\File.cpp", 228);
    }

    FILE* h_file = ::fopen(file_name, "wb");
    if (!h_file)
    {
        FatalError_4A38C0(Gta2Error::kFreeloaderEpisodeUnknown, "C:\\Splitting\\Gta2\\Source\\File.cpp", 231);
    }

    if (Write(buffer, *buffer_size, 1U, h_file) != 1)
    {
        ::fclose(h_file);
        FatalError_4A38C0(Gta2Error::kFileWriteFailure, "C:\\Splitting\\Gta2\\Source\\File.cpp", 237);
    }

    if (::fclose(h_file))
    {
        FatalError_4A38C0(Gta2Error::kFileCloseError, "C:\\Splitting\\Gta2\\Source\\File.cpp", 241);
    }
}

// 0x4A6F30
size_t File::Write(void* buffer, size_t element_size, size_t element_count, FILE* stream)
{
    return ::fwrite(buffer, element_size, element_count, stream);
}

// 0x4A6F50
void File::AppendBufferToFile(const char_type* file_name, void* buffer, u32* buffer_size)
{
    Error_SetName_4A0770(file_name);
    if (!*buffer_size)
    {
        FatalError_4A38C0(Gta2Error::kWritingZeroBytesToFile, "C:\\Splitting\\Gta2\\Source\\File.cpp", 261);
    }

    FILE* h_file = ::fopen(file_name, "ab"); // TODO: check
    if (!h_file)
    {
        FatalError_4A38C0(Gta2Error::kFreeloaderEpisodeUnknown, "C:\\Splitting\\Gta2\\Source\\File.cpp", 264);
    }

    if (Write(buffer, *buffer_size, 1U, h_file) != 1)
    {
        ::fclose(h_file);
        FatalError_4A38C0(Gta2Error::kFileWriteFailure, "C:\\Splitting\\Gta2\\Source\\File.cpp", 270);
    }

    if (::fclose(h_file))
    {
        FatalError_4A38C0(Gta2Error::kFileCloseError, "C:\\Splitting\\Gta2\\Source\\File.cpp", 274);
    }
}

// 0x4A7000
void File::CreateFile(const char_type* file_name)
{
    Error_SetName_4A0770(file_name);

    FILE* h_file = ::fopen(file_name, "wb");
    if (!h_file)
    {
        FatalError_4A38C0(Gta2Error::kFreeloaderEpisodeUnknown, "C:\\Splitting\\Gta2\\Source\\File.cpp", 296);
    }

    if (::fclose(h_file))
    {
        FatalError_4A38C0(Gta2Error::kFileCloseError, "C:\\Splitting\\Gta2\\Source\\File.cpp", 300);
    }
}

// 0x4A7060
void File::GlobalOpen(const char_type* file_name)
{
    if (gGbGlobalFileOpen)
    {
        GlobalClose();
    }

    Error_SetName_4A0770(file_name);

    gGhFile = ::fopen(file_name, "rb");
    if (!gGhFile)
    {
        FatalError_4A38C0(Gta2Error::kFreeloaderEpisodeUnknown, "C:\\Splitting\\Gta2\\Source\\File.cpp", 323);
    }

    gGbGlobalFileOpen = 1;
}

// 0x4A70C0
void File::GlobalClose()
{
    if (gGbGlobalFileOpen)
    {
        s32 close_result = ::fclose(gGhFile);
        gGbGlobalFileOpen = 0;
        if (close_result)
        {
            FatalError_4A38C0(Gta2Error::kFileCloseError, "C:\\Splitting\\Gta2\\Source\\File.cpp", 345);
        }
    }
}

// 0x4A7110
void File::GlobalCloseUnChecked()
{
    if (gGbGlobalFileOpen)
    {
        ::fclose(gGhFile);
        gGbGlobalFileOpen = 0;
    }
}

// 0x4A7140
void File::GlobalSeek(u32* offset)
{
    if (!gGbGlobalFileOpen)
    {
        FatalError_4A38C0(Gta2Error::kFileReadingNotOpenedFile, "C:\\Splitting\\Gta2\\Source\\File.cpp", 416);
    }

    if (::fseek(gGhFile, *offset, 1))
    {
        FileError(14, 0, 0);
    }
}

// 0x4A7190
void File::FileError(s32 code, s32 arg1, s32 arg2)
{
    GlobalCloseUnChecked();
    FatalError_4A38C0(code, "C:\\Splitting\\Gta2\\Source\\File.cpp", 398, arg1, arg2);
}

// 0x4A71C0
void File::GlobalRead(void* buffer, const u32& buffer_size)
{
    if (!gGbGlobalFileOpen)
    {
        FatalError_4A38C0(Gta2Error::kFileReadingNotOpenedFile, "C:\\Splitting\\Gta2\\Source\\File.cpp", 438);
    }

    if (Read(buffer, buffer_size, 1U, gGhFile) != 1)
    {
        FileError(15, 0, 0);
    }
}

// 0x4A7210
bool File::GlobalRead(void* buffer, u32* size)
{
    if (!gGbGlobalFileOpen)
    {
        FatalError_4A38C0(Gta2Error::kFileReadingNotOpenedFile, "C:\\Splitting\\Gta2\\Source\\File.cpp", 460);
    }
    return (Read(buffer, *size, 1U, gGhFile) == 1) ? true : false;
}

// 0x4A7250
size_t File::GetRemainderSize(void* buffer, u32* max_file_size)
{
    if (!gGbGlobalFileOpen)
    {
        FatalError_4A38C0(Gta2Error::kFileReadingNotOpenedFile, "C:\\Splitting\\Gta2\\Source\\File.cpp", 487);
    }

    s32 cur_pos = ::ftell(gGhFile);
    if (cur_pos == -1)
    {
        FileError(13, 0, 0);
    }

    if (::fseek(gGhFile, 0, SEEK_END))
    {
        FileError(14, 0, 0);
    }

    s32 end_pos = ::ftell(gGhFile);
    if (end_pos == -1)
    {
        FileError(13, 0, 0);
    }

    if (::fseek(gGhFile, cur_pos, 0))
    {
        FileError(14, 0, 0);
    }

    size_t remainder_size = end_pos - cur_pos;
    if (remainder_size > *max_file_size)
    {
        FileError(1022, remainder_size - *max_file_size, 0);
    }

    if (Read(buffer, remainder_size, 1U, gGhFile) != 1)
    {
        FileError(15, 0, 0);
    }

    return remainder_size;
}

// 0x4A7340
char_type File::SkipWhitespace(FILE* stream)
{
    char_type next_char = 0;

    while (true)
    {
        next_char = ::fgetc(stream);
        // note: feof = Stream->_flag & 0x10
        if (feof(stream) || next_char == '\n')
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