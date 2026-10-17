#include "input.hpp"
#include "Function.hpp"

#include "error.hpp"

DEFINE_GLOBAL(LPDIRECTINPUTA, gpDInput_67B804, 0x67B804);
DEFINE_GLOBAL(LPDIRECTINPUTDEVICEA, gKeyboardDevice_67B5C0, 0x67B5C0);
DEFINE_GLOBAL(LPDIRECTINPUTDEVICEA, gGamePadDevice_67B6C0, 0x67B6C0);

// 0x4986D0
void Input::DirectInputCreate_4986D0(HINSTANCE hInstance)
{
    // VC6-compatible path
    if (DirectInputCreateA(hInstance, 1792, &gpDInput_67B804, nullptr) < 0)
    {
        FatalError_4A38C0(8, "C:\\Splitting\\Gta2\\Source\\diutil.cpp", 129);
    }
}

// 0x498710
void Input::DInputRelease_498710()
{
    if (gpDInput_67B804)
    {
        gpDInput_67B804->Release();
        gpDInput_67B804 = nullptr;
    }
}