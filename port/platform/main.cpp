// Native entry point: hands the command line to the game's WinMain (test.cpp), as Windows would.
#include "types.hpp"
#include <string>

s32 APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, s32 nCmdShow);

int main(int argc, char** argv)
{
    std::string cmdLine;
    for (int i = 1; i < argc; i++)
    {
        if (i > 1)
        {
            cmdLine += ' ';
        }
        cmdLine += argv[i];
    }
    return WinMain(reinterpret_cast<HINSTANCE>(1), 0, &cmdLine[0], SW_SHOWNORMAL);
}
