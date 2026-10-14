// The port's side of DirectPlay (dplay.cpp): settings the lobby (lobby.cpp) and the command line set
// before the game opens its connection, and what dplay.cpp reports back.
#pragma once
#include <stdint.h>

#include <string>

struct IUnknown;
struct _GUID;

struct PortNetConfig
{
    // Where to look for games: a host name or address (with an optional :port). Empty: this LAN.
    // A connection the game passes to InitializeConnection overrides it with its own address.
    std::string address;
    // Where a hosted game lives: empty for this machine (a server inside the game, at `port`), or a
    // gta2-server elsewhere that everybody can reach
    std::string hostServer;
    uint16_t port = 2302;
    bool verbose = false;
};
PortNetConfig& PortNet();

// Who this build can play with: the same game code, on any system (see dplay.cpp)
const char* PortNetCompat();
// This build, for messages: "linux x86-64, game 1a2b3c4d"
const char* PortNetBuild();
// The open session went away (its host left or the connection died)
bool PortNetSessionLost();
// The connection's state, for diagnostics
std::string PortNetDescribe();
// What went wrong last (empty if nothing), for the lobby to show
std::string PortNetLastError();

// CoCreateInstance(CLSID_DirectPlay) lands here
long PortDirectPlayCreate(const _GUID& iid, void** out);
