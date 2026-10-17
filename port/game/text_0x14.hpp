#pragma once

#include "core/function.hpp"
#include <windows.h>

struct TextEntry
{
    wchar_t* field_0_pwstr;
    const char_type* key;
    s32 field_8;
};

class TextTdat
{
  public:
    TextTdat() // inline 0x4C2420
    {
        data_ = nullptr;
        len_ = 0;
    }

    ~TextTdat() // inline 0x4C2430
    {

        delete[] data_;

        data_ = nullptr;
    }
    BYTE* data_;
    s32 len_;
    void TDATLoad(u32 size);
};

class TextTkey
{
  public:
    TextTkey() // inline 4C23F0
    {
        t_key_ = nullptr;
        t_key_count_ = 0;
    }

    ~TextTkey() // inline 0x4C2400
    {

        delete[] t_key_;

        t_key_ = nullptr;
    }
    TextEntry* t_key_;
    u32 t_key_count_;
};

class GameText
{
  public:
    bool LangIsJapanese()
    {
        return lang_code_ == 106;
    }

    TextTkey t_key_;
    TextTdat t_dat_;
    char_type lang_code_;
    char_type pad_[3];

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
