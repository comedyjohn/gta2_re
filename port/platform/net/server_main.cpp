// gta2-server: a GTA2 session server on its own. Run it on a machine every player can reach (one with
// a public address, or the one with the forwarded port) and start games with
// `gta2-launch --host --server <its address>`; the others join with `--join <its address>`. It needs
// no game data, relays every message and runs any number of games at once.
#include "net.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

namespace
{
volatile sig_atomic_t gStop = 0;

void OnSignal(int)
{
    gStop = 1;
}

void Usage()
{
    printf("usage: gta2-server [--port N] [--quiet]\n"
           "  --port N   UDP port to listen on (default %u)\n"
           "  --quiet    don't log connections and games\n",
           net::kDefaultPort);
}
} // namespace

int main(int argc, char** argv)
{
    unsigned port = net::kDefaultPort;
    bool verbose = true;
    for (int i = 1; i < argc; i++)
    {
        if (!strcmp(argv[i], "--port") && i + 1 < argc)
        {
            port = static_cast<unsigned>(atoi(argv[++i]));
        }
        else if (!strcmp(argv[i], "--quiet"))
        {
            verbose = false;
        }
        else
        {
            Usage();
            return strcmp(argv[i], "--help") && strcmp(argv[i], "-h") ? 2 : 0;
        }
    }
    if (port == 0 || port > 65535)
    {
        fprintf(stderr, "gta2-server: bad port\n");
        return 2;
    }

    signal(SIGINT, OnSignal);
    signal(SIGTERM, OnSignal);
    signal(SIGPIPE, SIG_IGN);

    net::Server server;
    std::string error;
    if (!server.Start(static_cast<uint16_t>(port), verbose, error))
    {
        fprintf(stderr, "gta2-server: %s\n", error.c_str());
        return 1;
    }
    printf("gta2-server: running on UDP port %u (network protocol %u). Ctrl+C stops it.\n", server.Port(), net::kProtocolVersion);
    fflush(stdout);
    while (!gStop)
    {
        usleep(100000);
    }
    server.Stop();
    return 0;
}
