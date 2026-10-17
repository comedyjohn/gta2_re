// text_0x14: the in-game text (GXT) lookups and character remapping. Characterisation of the code
// as ported; the file loading is covered by the replay corpus, since it needs the game's data.
#include <doctest.h>

#include <cstddef>
#include <cstring>

#include "data/game_text.hpp"

namespace
{
// A table of keys sorted as the GXT files have them, with strings in one data block.
struct Fixture
{
    static constexpr u32 kCount = 3;
    BYTE data[64] = {};
    TextEntry keys[kCount] = {};
    GameText text;

    explicit Fixture(char_type lang = 'e')
    {
        auto* wide = reinterpret_cast<wchar_t*>(data);
        wide[0] = L'A';
        wide[1] = 0;
        wide[2] = L'B';
        wide[3] = L'C';
        wide[4] = 0;
        // Offsets in bytes, as the file stores them.
        const char* names[kCount] = {"alpha", "bravo", "charlie"};
        const sptr offsets[kCount] = {0, static_cast<sptr>(2 * sizeof(wchar_t)), static_cast<sptr>(4 * sizeof(wchar_t))};
        for (u32 i = 0; i < kCount; i++)
        {
            keys[i].field_0_pwstr = reinterpret_cast<wchar_t*>(offsets[i]);
            // The key's characters run on from field_4_pKey into field_8 (the file's 8-byte key).
            char* key = reinterpret_cast<char*>(&keys[i]) + offsetof(TextEntry, key);
            std::memcpy(key, names[i], std::strlen(names[i]) + 1);
        }
        text.t_key_.t_key_ = keys;
        text.t_key_.t_key_count_ = kCount;
        text.lang_code_ = lang;
        text.TDATOffsets2Strings(data);
    }
    // The fixture's arrays are not owned by the text object.
    ~Fixture()
    {
        text.t_key_.t_key_ = nullptr;
        text.t_key_.t_key_count_ = 0;
    }
};
} // namespace

TEST_CASE("TDAT_Offsets2Strings turns byte offsets into string pointers")
{
    Fixture f;
    CHECK(reinterpret_cast<BYTE*>(f.keys[0].field_0_pwstr) == f.data);
    CHECK(reinterpret_cast<BYTE*>(f.keys[2].field_0_pwstr) == f.data + 4 * sizeof(wchar_t));
}

TEST_CASE("Keys are found by binary search; a missing key gives a single space")
{
    Fixture f;
    CHECK(f.text.HasKey("bravo"));
    CHECK(f.text.HasKey("alpha"));
    CHECK(f.text.HasKey("charlie"));
    CHECK_FALSE(f.text.HasKey("delta"));
    CHECK_FALSE(f.text.HasKey("alph"));

    CHECK(wcscmp(f.text.FindText("alpha"), L"A") == 0);
    CHECK(wcscmp(f.text.FindText("bravo"), L"BC") == 0);
    CHECK(wcscmp(f.text.FindText("zulu"), L" ") == 0);
    CHECK(f.text.KeyExists("charlie"));
    CHECK(wcscmp(f.text.Find("charlie"), L"") == 0);
}

TEST_CASE("Extended characters are remapped per language; unmapped ones become '?'")
{
    Fixture west('e');
    CHECK(west.text.RemapExtendedChar('a') == 'a');
    CHECK(west.text.RemapExtendedChar(127) == 127);
    CHECK(west.text.RemapExtendedChar(0xC0) == 128);
    CHECK(west.text.RemapExtendedChar(128) == '?');
    CHECK(west.text.RemapExtendedChar(256) == '?');
    CHECK(west.text.RemapExtendedChar(0x1234) == '?');

    Fixture russian('r');
    CHECK(russian.text.RemapExtendedChar(0xC0) == 193);
    CHECK(russian.text.RemapExtendedChar(128) == '?');
}

TEST_CASE("RemapExtendedChars leaves Japanese text alone")
{
    wchar_t s[] = {L'a', 0xC0, 0};
    Fixture jp('j');
    jp.text.RemapExtendedCharsIfNotJapanese(s);
    CHECK(s[1] == 0xC0);

    Fixture west('e');
    west.text.RemapExtendedCharsIfNotJapanese(s);
    CHECK(s[0] == L'a');
    CHECK(s[1] == 128);
}

TEST_CASE("StrToUpper upper-cases ASCII, in the languages that have it")
{
    wchar_t a[] = L"hello, World 1";
    Fixture west('e');
    CHECK(west.text.StrToUpper(a) == a);
    CHECK(wcscmp(a, L"HELLO, WORLD 1") == 0);

    wchar_t b[] = L"hello";
    Fixture other('x');
    other.text.StrToUpper(b);
    CHECK(wcscmp(b, L"hello") == 0);

    Fixture french('f');
    CHECK(french.text.ToUpper('q') == 'Q');
    CHECK(french.text.ToUpper('Q') == 'Q');
}

TEST_CASE("Ascii2Wide widens, Wide2PesudoAscii narrows with '#' for anything above 127")
{
    char_type narrow[] = "Hi 42";
    CHECK(wcscmp(GameText::Ascii2Wide(narrow), L"Hi 42") == 0);

    const wchar_t wide[] = {L'o', L'k', 0x100, L'!', 0};
    const char_type* out = GameText::Wide2PesudoAscii(wide);
    CHECK(out[0] == 'o');
    CHECK(out[1] == 'k');
    CHECK(out[2] == '#');
    CHECK(out[3] == '!');
    CHECK(out[4] == 0);
}
