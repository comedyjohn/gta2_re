// Native entry point: starts the platform layer, then hands the command line to the game's WinMain
// (test.cpp), as Windows would.
#include "platform.h"
#include "types.hpp"

#include <string>

s32 APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, s32 nCmdShow);
int port_selftest();
int port_audiotest();

int main(int argc, char** argv)
{
    // Needs no display, so it runs before the platform layer opens one
    if (argc > 1 && std::string(argv[1]) == "--audiotest")
    {
        return port_audiotest();
    }

    if (!plat::Init())
    {
        return 1;
    }

    if (argc > 1 && std::string(argv[1]) == "--selftest")
    {
        const int result = port_selftest();
        plat::Shutdown();
        return result;
    }

    std::string cmdLine;
    for (int i = 1; i < argc; i++)
    {
        if (i > 1)
        {
            cmdLine += ' ';
        }
        cmdLine += argv[i];
    }
    const int result = WinMain(reinterpret_cast<HINSTANCE>(1), 0, &cmdLine[0], SW_SHOWNORMAL);

    plat::Shutdown();
    return result;
}
