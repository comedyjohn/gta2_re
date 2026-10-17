#include "text_0x14.hpp"
#include "chunk.hpp"
#include "core/enums.hpp"
#include "core/function.hpp"
#include "error.hpp"
#include "file.hpp"
#include "gtx_0x106C.hpp"
#include "registry.hpp"

DEFINE_GLOBAL(GameText*, gText, 0x704DFC);
DEFINE_GLOBAL(GameText*, gGtext, 0x6F87F0);
DEFINE_GLOBAL_ARRAY(wchar_t, gTmpAscii2WideStr, 640, 0x70488C);
DEFINE_GLOBAL_ARRAY(char_type, gTmpWide2AsciiStr, 80, 0x70462C);

const wchar_t kExtendedCharMapPart2[256] = {
    128U, 129U, 130U, 131U, 132U, 133U, 134U, 135U, 136U, 137U, 138U, 139U, 140U, 141U, 142U, 143U, 144U, 145U, 146U, 147U, 148U, 149U,
    150U, 128U, 129U, 130U, 131U, 132U, 133U, 134U, 135U, 136U, 137U, 138U, 139U, 140U, 141U, 142U, 143U, 144U, 145U, 146U, 147U, 148U,
    149U, 173U, 173U, 175U, 176U, 177U, 178U, 179U, 180U, 181U, 182U, 183U, 184U, 185U, 186U, 187U, 188U, 189U, 190U, 191U, 192U, 193U,
    194U, 195U, 196U, 197U, 198U, 199U, 200U, 201U, 202U, 203U, 204U, 205U, 206U, 207U, 208U, 209U, 177U, 178U, 179U, 180U, 181U, 182U,
    183U, 184U, 185U, 186U, 187U, 188U, 189U, 190U, 191U, 192U, 193U, 194U, 195U, 196U, 197U, 198U, 199U, 200U, 201U, 202U, 203U, 204U,
    205U, 206U, 207U, 208U, 209U, 243U, 244U, 245U, 246U, 247U, 248U, 249U, 250U, 251U, 252U, 253U, 254U, 255U, 128U, 129U, 130U, 131U,
    132U, 133U, 134U, 135U, 136U, 137U, 138U, 139U, 140U, 141U, 142U, 143U, 144U, 145U, 146U, 147U, 148U, 149U, 150U, 65U,  65U,  65U,
    65U,  132U, 133U, 69U,  69U,  69U,  69U,  73U,  73U,  73U,  73U,  79U,  79U,  79U,  79U,  85U,  85U,  85U,  85U,  173U, 173U, 175U,
    176U, 177U, 178U, 179U, 180U, 181U, 182U, 183U, 184U, 185U, 186U, 187U, 188U, 189U, 190U, 191U, 192U, 193U, 194U, 195U, 196U, 197U,
    198U, 199U, 200U, 201U, 202U, 203U, 204U, 205U, 206U, 207U, 208U, 209U, 210U, 211U, 212U, 213U, 214U, 215U, 216U, 217U, 218U, 219U,
    220U, 221U, 222U, 223U, 224U, 225U, 226U, 227U, 228U, 229U, 230U, 231U, 232U, 233U, 234U, 235U, 236U, 237U, 238U, 239U, 240U, 241U,
    242U, 243U, 244U, 245U, 246U, 247U, 248U, 249U, 250U, 251U, 252U, 253U, 254U, 255U};

const s16 kExtendedRussianCharMap[128] = {
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   177, 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   210, 0,   0,   0,   0,   0,   0,   0,   193, 208, 192, 184, 198, 182, 199, 187, 205, 178, 181, 197, 204, 183,
    196, 194, 195, 203, 206, 180, 190, 188, 179, 202, 185, 186, 189, 191, 207, 200, 209, 201, 226, 241, 225, 217, 231, 215, 232, 220,
    238, 211, 214, 230, 237, 216, 229, 227, 228, 236, 239, 213, 223, 221, 212, 235, 218, 219, 222, 224, 240, 233, 242, 234};

// Extended characters 128-255, corresponding to original data at 0x626390.
namespace
{
const u16 kExtendedWesternCharMap[128] = {
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   176, 0,   0,   0,   0,   0,   0,   0,   0, 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   175, 128, 129, 130, 0, 131, 0,   132, 133, 134, 135, 136, 137, 138, 139,
    140, 141, 0,   173, 142, 143, 144, 0,   145, 0,   0,   146, 147, 148, 149, 0, 0,   150, 151, 152, 153, 0,   154, 0,   155, 156,
    157, 158, 159, 160, 161, 162, 163, 164, 0,   174, 165, 166, 167, 0,   168, 0, 0,   169, 170, 171, 172, 0,   0,   0};
} // namespace

// 0x5B58D0
u16 GameText::RemapExtendedChar(u16 ch)
{
    u16 result = ch;
    if (ch >= 128U)
    {
        if (ch >= 256U)
        {
            result = 63;
        }
        else
        {
            result = lang_code_ == 'r' ? kExtendedRussianCharMap[ch - 128] : kExtendedWesternCharMap[ch - 128];
            if (!result)
            {
                result = 63;
            }
        }
    }
    return result;
}

// 0x5B5910
void GameText::RemapExtendedCharsIfNotJapanese(wchar_t* pText)
{
    if (lang_code_ != 'j')
    {
        RemapExtendedChars(pText);
    }
}

// 0x5B5930
void GameText::RemapExtendedChars(wchar_t* pStr)
{
    for (wchar_t* pStrIter = pStr; *pStrIter; ++pStrIter)
    {
        *pStrIter = RemapExtendedChar(*pStrIter);
    }
}

// 0x5B5960
void GameText::TKEYLoad(u32 chunkSize)
{
#if defined(GTA2_PORT_64BIT)
    // The file's 12-byte entries hold a 4-byte offset where text_0xC has a pointer, then the 8-byte
    // key. Read them as they are and widen them; TDAT_Offsets2Strings_5B59B0 turns the offsets into
    // pointers later. field_8 stays zero, so a key of all 8 characters still ends there.
    struct Text0xCFile
    {
        u32 field_0_offset;
        char_type field_4_key[8];
    };
    t_key_.t_key_count_ = chunkSize / sizeof(Text0xCFile);
    t_key_.t_key_ = new TextEntry[t_key_.t_key_count_];
    auto* pFileEntries = new Text0xCFile[t_key_.t_key_count_];
    File::GlobalRead(pFileEntries, chunkSize);
    for (u32 i = 0; i < t_key_.t_key_count_; i++)
    {
        TextEntry& entry = t_key_.t_key_[i];
        memset(&entry, 0, sizeof(entry));
        entry.field_0_pwstr = reinterpret_cast<wchar_t*>(static_cast<sptr>(pFileEntries[i].field_0_offset));
        memcpy(reinterpret_cast<void*>(&entry.key),
               reinterpret_cast<const void*>(pFileEntries[i].field_4_key),
               sizeof(pFileEntries[i].field_4_key));
    }
    delete[] pFileEntries;
#else
    // TODO: probably an inline
    t_key_.t_key_count_ = chunkSize / sizeof(TextEntry);
    t_key_.t_key_ = new TextEntry[t_key_.t_key_count_];
    if (!t_key_.t_key_)
    {
        FatalError_4A38C0(Gta2Error::kOutOfMemoryNewOperator, "C:\\Splitting\\Gta2\\Source\\text.cpp", 218);
    }

    File::GlobalRead(t_key_.t_key_, chunkSize);
#endif
}

// 0x5B59B0
void GameText::TDATOffsets2Strings(BYTE* pDat)
{
    TextEntry* ptKeyIter = t_key_.t_key_;

    for (u32 idx = 0; idx < t_key_.t_key_count_; idx++)
    {
        // The raw file record is an offset, convert it to pos32 to the string data in the TDAT
        const s32 offsetInBytes = static_cast<s32>(reinterpret_cast<uintptr_t>(ptKeyIter->field_0_pwstr));
        ptKeyIter->field_0_pwstr = reinterpret_cast<wchar_t*>(&pDat[offsetInBytes]);
        ptKeyIter++;
    }
}

// 0x5B59E0
bool GameText::HasKey(const char_type* pIdStr)
{
    return bsearch(pIdStr, // search key
                   t_key_.t_key_, // array to search
                   t_key_.t_key_count_, // array count
                   sizeof(TextEntry), // element size
                   TKeySearch) ?
        true :
        false;
}

// 0x5B5A10
s32 GameText::TKeySearch(const void* a, const void* b)
{
    const auto* pIdStr = static_cast<const char_type*>(a);
    const auto* a2 = static_cast<const TextEntry*>(b);
    return strcmp(pIdStr, reinterpret_cast<const char_type*>(&a2->key)); // TODO: ???
}

// 0x5B5A50
wchar_t* GameText::FindText(const char_type* pIdStr)
{
    auto* pFound = static_cast<TextEntry*>(bsearch(pIdStr, // key
                                                   t_key_.t_key_, // array
                                                   t_key_.t_key_count_, // array count
                                                   sizeof(TextEntry), // element size
                                                   TKeySearch));
    if (!pFound)
    {
        static wchar_t kBlank[] = L" "; // writable, like the text the callers usually get
        return kBlank;
    }
    else
    {
        return pFound->field_0_pwstr;
    }
}

// 0x5B5A80
void TextTdat::TDATLoad(u32 size)
{
    len_ = size / sizeof(wchar_t);
    data_ = new BYTE[len_ * sizeof(wchar_t)];

    if (!data_)
    {
        FatalError_4A38C0(Gta2Error::kOutOfMemoryNewOperator, "C:\\Splitting\\Gta2\\Source\\text.cpp", 378);
    }

    File::GlobalRead(data_, size);
}

// 0x5B5AD0
u16 GameText::ToUpper(wchar_t ch)
{
    // The original indexed both tables with the raw char (0x80-0xFF), so its 'g'/'i'/'r'/'s' lookup read
    // past kExtendedRussianCharMap_626490 into word_626590[0..127], which follows it in memory. That
    // read is spelled out below; 'f' reads word_626590[128..255].
    // TODO: the contents of these arrays are very wrong
    switch (lang_code_)
    {
        case 'e':
            if (ch >= 'a' && ch <= 'z')
            {
                return ch - ' ';
            }
            break;

        case 'f':
            if (ch >= 'a' && ch <= 'z')
            {
                return ch - ' ';
            }
            if (ch >= 0x80 && ch <= 0xFF)
            {
                return kExtendedCharMapPart2[ch];
            }
            break;

        case 'g':
        case 'i':
        case 'r':
        case 's':
            if (ch >= 'a' && ch <= 'z')
            {
                return ch - ' ';
            }
            if (ch >= 0x80 && ch <= 0xFF)
            {
                return kExtendedCharMapPart2[ch - 0x80];
            }
            break;
        default:
            break;
    }
    return ch;
}

// 0x5B5B80
wchar_t* GameText::StrToUpper(wchar_t* pWideStr)
{
    for (wchar_t* pStrIter = pWideStr; *pStrIter; ++pStrIter)
    {
        *pStrIter = ToUpper(*pStrIter);
    }
    return pWideStr;
}

// 9.6f 0x4539D0, file-local copy of GetCharWidth_4539D0 (Draw.hpp): including Draw.hpp changes this file's static init code
namespace
{
inline s32 GetCharWidthLocal(u16 font_type, wchar_t chr)
{
    return gGtx->GetFontWidth(&font_type, &chr);
}
} // namespace

// 0x5B5BC0
s32 GameText::InsertLineBreaksAndGetNumLines(wchar_t* pDestStr, const wchar_t* pSrcStr, s32 max_line_width, u16 font_type)
{
    const wchar_t* pSrcStrCopy = pSrcStr;
    wchar_t* pDestStrCopy = pDestStr;
    s32 num_of_lines = 1;
    s32 space_width = GetSpaceWidth(font_type);
    s32 current_width = 0;
    const wchar_t* pPrevSrcCheckpoint = nullptr;
    wchar_t* pPrevDestCheckpoint = nullptr;

    for (; *pSrcStrCopy; pSrcStrCopy++, pDestStrCopy++)
    {
        *pDestStrCopy = *pSrcStrCopy;

        switch (*pSrcStrCopy)
        {
            case '\n':
                pPrevSrcCheckpoint = nullptr;
                pPrevDestCheckpoint = nullptr;
                current_width = 0;
                num_of_lines += 1;
                break;
            case ' ':
                current_width += space_width;
                pPrevSrcCheckpoint = pSrcStrCopy;
                pPrevDestCheckpoint = pDestStrCopy;
                break;
            case '#':
                break;
            default:
                current_width += GetCharWidthLocal(font_type, *pSrcStrCopy);
                break;
        }

        if (current_width > max_line_width)
        {
            if (pPrevSrcCheckpoint && pPrevDestCheckpoint)
            {
                current_width = 0;
                pDestStrCopy = pPrevDestCheckpoint; // retrocede
                pSrcStrCopy = pPrevSrcCheckpoint;
                *pDestStrCopy = '\n';

                pPrevSrcCheckpoint = nullptr;
                pPrevDestCheckpoint = nullptr;
            }
            else
            {
                *pDestStrCopy = '\n';
                pDestStrCopy++;

                current_width = GetCharWidthLocal(font_type, *pSrcStrCopy);
                *pDestStrCopy = *pSrcStrCopy;
            }
            num_of_lines += 1;
        }
    }
    *pDestStrCopy = 0; // terminate string

    return num_of_lines;
}

// 0x5B5D10
char* GameText::Wide2PesudoAscii(const wchar_t* pText)
{
    const wchar_t* pSrc = pText;
    // jnb forced to jge by changing u32 loop index to int
    s32 dstIdx = 0;
    while (*pSrc) // the original tested the pointer: it read up to 79 characters past the end of a short string
    {
        // jnb also used if comparing to sizeof() value instead of
        // numerical literal! cast to s32 fixes (sizeof returns unsigned?)
        if (dstIdx >= static_cast<int>(sizeof(gTmpWide2AsciiStr) - 1))
        {
            break;
        }

        if (*pSrc >= 0x80U)
        {
            gTmpWide2AsciiStr[dstIdx] = '#';
        }
        else
        {
            gTmpWide2AsciiStr[dstIdx] = static_cast<char>(*pSrc);
        }

        pSrc++;
        dstIdx++;
    }

    // add null terminator
    gTmpWide2AsciiStr[dstIdx] = 0;
    return gTmpWide2AsciiStr;
}

// 0x5B5DF0
wchar_t* GameText::Ascii2Wide(const char_type* pStr)
{
    // movsx vs movzx caused by unsigned vs s8
    wchar_t* pDst = gTmpAscii2WideStr;
    while (*pStr)
    {
        *pDst = *pStr;
        pDst++;
        pStr++;
    }

    // null terminate
    *pDst = 0;
    return gTmpAscii2WideStr;
}

// 0x5B5E20
void GameText::LoadChunk(const char_type* chunk_type, u32 chunk_len)
{
    if (!strncmp(chunk_type, "TKEY", 4U))
    {
        TKEYLoad(chunk_len);
    }
    else if (!strncmp(chunk_type, "TDAT", 4U))
    {
        t_dat_.TDATLoad(chunk_len);
    }
    else
    {
        File::GlobalSeek(&chunk_len);
    }
}

// 0x5B5E90
void GameText::Load()
{
    char_type expected_code[5];

    ChunkHeader chunkHeader;
    char_type gxtFileName[20];
    sprintf(gxtFileName, "data\\%c.gxt", lang_code_);

    sprintf(expected_code, "GBL%c", lang_code_ - ' ');

    File::GlobalOpen(gxtFileName);

    FileHeader gtxHeader;
    u32 len = sizeof(FileHeader);
    File::GlobalRead(&gtxHeader, len);

    gtxHeader.VerifyType(expected_code);
    gtxHeader.VerifyVersion(100);

    for (len = sizeof(ChunkHeader); File::GlobalRead(&chunkHeader, &len); len = sizeof(ChunkHeader))
    {
        if (chunkHeader.size_ != 0)
        {
            LoadChunk(chunkHeader.type_, chunkHeader.size_);
        }
    }
    File::GlobalClose();
    TDATOffsets2Strings(t_dat_.data_);
}

// 0x5B5F90
wchar_t* GameText::Find(const char_type* pIdStr)
{
    return FindText(pIdStr);
}

// 0x5B5FA0
bool GameText::KeyExists(const char_type* pIdStr)
{
    return HasKey(pIdStr);
}

// 0x5B5FB0
GameText::GameText()
{
    gRegistry.GetOption("language", reinterpret_cast<BYTE*>(gTmpBuffer_67C598), sizeof(gTmpBuffer_67C598));

    if (gTmpBuffer_67C598[0] == 'e' || gTmpBuffer_67C598[0] == 'f' || gTmpBuffer_67C598[0] == 'g' || gTmpBuffer_67C598[0] == 'i' ||
        gTmpBuffer_67C598[0] == 's' || gTmpBuffer_67C598[0] == 'j' || gTmpBuffer_67C598[0] == 'r')
    {
        lang_code_ = gTmpBuffer_67C598[0];
    }
    else
    {
        lang_code_ = 'e';
    }
}

// Only gets its EH frame when <new> isn't included, see Globals.hpp
// 0x5B6050
GameText::~GameText() = default;