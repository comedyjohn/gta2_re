#pragma once

#include "core/function.hpp"
#include <windows.h>

struct TextEntry
{
    wchar_t* field_0_pwstr;
    const char_type* field_4_pKey;
    s32 field_8;
};

class TextTdat
{
  public:
    TextTdat() // inline 0x4C2420
    {
        field_0_data = nullptr;
        field_4_len = 0;
    }

    ~TextTdat() // inline 0x4C2430
    {

        delete[] field_0_data;

        field_0_data = nullptr;
    }
    BYTE* field_0_data;
    s32 field_4_len;
    void TDATLoad(u32 size);
};

class TextTkey
{
  public:
    TextTkey() // inline 4C23F0
    {
        field_0_tKey = nullptr;
        field_4_tKey_count = 0;
    }

    ~TextTkey() // inline 0x4C2400
    {

        delete[] field_0_tKey;

        field_0_tKey = nullptr;
    }
    TextEntry* field_0_tKey;
    u32 field_4_tKey_count;
};

class GameText
{
  public:
    bool LangIsJapanese()
    {
        return field_10_lang_code == 106;
    }

    TextTkey field_0_tKey;
    TextTdat field_8_tDat;
    char_type field_10_lang_code;
    char_type field_11_pad[3];

    u16 RemapExtendedChar(u16 ch);

    void RemapExtendedCharsIfNotJapanese(wchar_t* pText);

    void RemapExtendedChars(wchar_t* pStr);

    void TKEYLoad(u32 size);

    void TDATOffsets2Strings(BYTE* pDat);

    bool HasKey(const char_type* pIdStr);

    static s32 TKeySearch(const void* a, const void* b);

    wchar_t* FindText(const char_type* pIdStr);

    u16 ToUpper(wchar_t ch);

    wchar_t* StrToUpper(wchar_t* pWideStr);

    static s32 InsertLineBreaksAndGetNumLines(wchar_t* pDestStr, const wchar_t* pSrcStr, s32 max_line_width, u16 font_type);

    static char_type* Wide2PesudoAscii(const wchar_t* pText);

    static wchar_t* Ascii2Wide(const char_type* pStr);

    void LoadChunk(const char_type* chunk_type, u32 chunk_len);

    void Load();

    wchar_t* Find(const char_type* pIdStr);

    bool KeyExists(const char_type* pIdStr);

    // 0x5B5FB0
    GameText();

    // 0x5B6050
    ~GameText();
};

EXTERN_GLOBAL(GameText*, gText_0x14_704DFC);

EXTERN_GLOBAL(GameText*, gtext_0x14_6F87F0);
