// Native entry point: starts the platform layer, then hands the command line to the game's WinMain
// (test.cpp), as Windows would.
#include "lobby_port.h"
#include "platform.h"
#include "types.hpp"

#include <stdio.h>

#include <string>
#include <vector>

s32 APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, s32 nCmdShow);
int port_selftest();
int port_audiotest();
int port_nettest(int argc, char** argv);
int port_simcheck();

int main(int argc, char** argv)
{
    const std::string first = argc > 1 ? argv[1] : "";
    // These need no display, so they run before the platform layer opens one
    if (first == "--audiotest")
    {
        return port_audiotest();
    }
    if (first == "--nettest")
    {
        return port_nettest(argc, argv);
    }
    if (first == "--simcheck")
    {
        return port_simcheck();
    }
    if (first == "--help" || first == "-h")
    {
        printf("usage: gta2 [options]\n"
               "  --selftest           check the renderer\n"
               "  --audiotest          check the sound mixer\n"
               "  --nettest ...        check the network layer (see port/tools/nettest.sh)\n"
               "  --simcheck           print what network games compare between systems\n\n%s",
               PortNetUsage());
        return 0;
    }

    // The network options are the port's (the lobby reads them); the rest is the game's
    std::vector<std::string> gameArgs;
    std::string error;
    if (!PortParseNetArgs(argc, argv, gameArgs, error))
    {
        fprintf(stderr, "gta2: %s (see gta2 --help)\n", error.c_str());
        return 2;
    }

    if (!plat::Init())
    {
        return 1;
    }

    if (first == "--selftest")
    {
        const int result = port_selftest();
        plat::Shutdown();
        return result;
    }

    std::string cmdLine;
    for (size_t i = 0; i < gameArgs.size(); i++)
    {
        if (i > 0)
        {
            cmdLine += ' ';
        }
        cmdLine += gameArgs[i];
    }
    const int result = WinMain(reinterpret_cast<HINSTANCE>(1), 0, &cmdLine[0], SW_SHOWNORMAL);

    plat::Shutdown();
    return result;
}
