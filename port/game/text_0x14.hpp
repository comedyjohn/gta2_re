#pragma once

#include "Function.hpp"
#include <windows.h>

struct text_0xC
{
    wchar_t* field_0_pwstr;
    const char_type* field_4_pKey;
    s32 field_8;
};

class text_tdat
{
  public:
    text_tdat() // inline 0x4C2420
    {
        field_0_data = nullptr;
        field_4_len = 0;
    }

    ~text_tdat() // inline 0x4C2430
    {
        if (field_0_data)
        {
            delete[] field_0_data;
        }
        field_0_data = nullptr;
    }
    BYTE* field_0_data;
    s32 field_4_len;
    void TDAT_Load_5B5A80(u32 size);
};

class text_tkey
{
  public:
    text_tkey() // inline 4C23F0
    {
        field_0_tKey = nullptr;
        field_4_tKey_count = 0;
    }

    ~text_tkey() // inline 0x4C2400
    {
        if (field_0_tKey)
        {
            delete[] field_0_tKey;
        }
        field_0_tKey = nullptr;
    }
    text_0xC* field_0_tKey;
    u32 field_4_tKey_count;
};

class text_0x14
{
  public:
    // 9.6f 0x452E60
    inline bool LangIsJapanese_452E60()
    {
        return field_10_lang_code == 106;
    }

    text_tkey field_0_tKey;
    text_tdat field_8_tDat;
    char_type field_10_lang_code;
    char_type field_11_pad[3];

    u16 RemapExtendedChar_5B58D0(u16 ch);

    void RemapExtendedCharsIfNotJapanese_5B5910(wchar_t* pText);

    void RemapExtendedChars_5B5930(wchar_t* pStr);

    void TKEY_Load_5B5960(u32 size);

    void TDAT_Offsets2Strings_5B59B0(BYTE* pDat);

    bool TKeyFind_5B59E0(const char_type* pIdStr);

    static s32 TKeySearch_5B5A10(const void* a, const void* b);

    wchar_t* TKeyFind_5B5A50(const char_type* pIdStr);

    u16 ToUpper_5B5AD0(wchar_t ch);

    wchar_t* StrToUpper_5B5B80(wchar_t* pWideStr);

    static s32 InsertLineBreaksAndGetNumLines_5B5BC0(wchar_t* pDestStr, const wchar_t* pSrcStr, s32 max_line_width, u16 font_type);

    static char_type* Wide2PesudoAscii_5B5D10(const wchar_t* pText);

    static wchar_t* Ascii2Wide_5B5DF0(const char_type* pStr);

    void load_chunk_5B5E20(const char_type* chunk_type, u32 chunk_len);

    void Load_5B5E90();

    wchar_t* Find_5B5F90(const char_type* pIdStr);

    bool KeyExists_5B5FA0(const char_type* pIdStr);

    // 0x5B5FB0
    text_0x14();

    // 0x5B6050
    ~text_0x14();
};

EXTERN_GLOBAL(text_0x14*, gText_0x14_704DFC);

EXTERN_GLOBAL(text_0x14*, gtext_0x14_6F87F0);
