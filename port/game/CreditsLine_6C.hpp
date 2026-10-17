#pragma once

#include "core/function.hpp"
#include <cwchar>

class CreditsLine_6C
{
  public:
    CreditsLine_6C(); // 483E30
    ~CreditsLine_6C() = default; // 0x483E60

    bool field_0_bLoaded;
    s16 field_2_unused; // set to 300 by LoadCredits_483F20, never read
    u16 field_4_y_gap; // extra space above this line
    u16 field_6_string_category; // credits_line_category (credits_line_category.hpp)
    wchar_t field_8_text[50];
};
