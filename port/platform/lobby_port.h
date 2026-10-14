// The network lobby without a window (lobby.cpp): what the command line asked for.
#pragma once
#include <string>
#include <vector>

struct PortLobbyOptions
{
    enum Mode
    {
        Ask, // -n alone: ask on the terminal
        Host,
        Join,
    };
    Mode mode = Ask;
    std::string name; // the player's name (empty: the one saved last time)
    std::string session; // join: part of the game's name to look for (empty: the first one found)
    // Host: the game to set up. Empty/negative values keep what was used last time.
    std::string map; // a number from the list, or part of the map's name or file
    int players = 0; // start once this many are in (0: 2, or Enter on the terminal)
    int gameType = 0; // 1 frags, 2 points, 3 tag
    int limit = -1; // frags or points to win
    int timeLimit = -1; // minutes, 0 for none
    int police = -1;
    int speed = -1; // 0 slow, 1 normal, 2 fast
};
PortLobbyOptions& PortLobby();

// Takes the network options out of the command line (--host, --join, --server, --name, ...; see
// `gta2 --help`). `rest` is what the game gets; it starts with -n when a network game was asked for.
// Returns false with a message for a bad option.
bool PortParseNetArgs(int argc, char** argv, std::vector<std::string>& rest, std::string& error);
const char* PortNetUsage();
