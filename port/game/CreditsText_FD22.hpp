#pragma once

#include "CreditsLine_6C.hpp"
#include "Function.hpp"

#pragma pack(push)
#pragma pack(1)
class CreditsText_FD22
{
  public:
    CreditsText_FD22(); // 483E70
    ~CreditsText_FD22() = default; // 0x483EA0

    wchar_t ReadNextNonBlankChar_483EC0(const wchar_t* pStr, u16* pStartPos, bool bStopAtSpace);
    void LoadCredits_483F20();

    s16 field_0_line_count;
    CreditsLine_6C field_2_lines[600];
};
#pragma pack(pop)