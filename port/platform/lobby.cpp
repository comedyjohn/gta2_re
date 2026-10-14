// The network lobby, without a window.
//
// On Windows `gta2 -n` opens dialog 103 from the original executable (Network_20324::DialogFunc):
// host or join, pick the map and rules, wait for players, start. The port doesn't have that dialog,
// so this does the same steps through the same calls (Network_20324 and NetPlay, with their
// callbacks), driven by the command line and the terminal:
//
//   gta2 --host [--map N] [--players N] ...    host a game here, start once N players are in
//   gta2 --join [address]                      join the first game on this network, or at `address`
//
// Network_20324's own logic still decides everything the dialog would: the map list, the saved
// settings, the settings a joining player takes from the host, the "disable joining" handshake that
// starts the game.
#include "dplay_port.h"
#include "lobby_port.h"

#include "NetPlay.hpp"
#include "Network_20324.hpp"
#include "registry.hpp"

#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <string>
#include <vector>

PortLobbyOptions& PortLobby()
{
    static PortLobbyOptions options;
    return options;
}

const char* PortNetUsage()
{
    return "Network game (every player needs the same build of the game and the same game data):\n"
           "  --host               host a game on this computer (UDP port 2302 must be reachable)\n"
           "  --join [ADDRESS]     join a game: the first one on this network, or the one at ADDRESS\n"
           "                       (a name or IP, optionally :PORT)\n"
           "  --server ADDRESS     with --host: host on a gta2-server there instead of this computer;\n"
           "                       the others then --join ADDRESS\n"
           "  --name NAME          your player name (default: the last one used)\n"
           "  --port N             UDP port (default 2302)\n"
           "  --game NAME          with --join: join the game whose name contains NAME\n"
           "Host only:\n"
           "  --players N          start as soon as N players are in (default 2; Enter starts at once)\n"
           "  --map MAP            map number or part of its name (default: the last one used)\n"
           "  --mode frags|points|tag\n"
           "  --limit N            frags or points to win\n"
           "  --time-limit MIN     0 for none\n"
           "  --police on|off\n"
           "  --speed slow|normal|fast\n"
           "  --net-log            log what the network does\n";
}

static bool ParseInt(const char* s, int& out)
{
    char* end = nullptr;
    const long v = strtol(s, &end, 10);
    if (!*s || *end)
    {
        return false;
    }
    out = static_cast<int>(v);
    return true;
}

bool PortParseNetArgs(int argc, char** argv, std::vector<std::string>& rest, std::string& error)
{
    PortLobbyOptions& o = PortLobby();
    PortNetConfig& net = PortNet();
    bool network = false;
    for (int i = 1; i < argc; i++)
    {
        const std::string a = argv[i];
        auto value = [&](std::string& out)
        {
            if (i + 1 >= argc)
            {
                error = a + " needs a value";
                return false;
            }
            out = argv[++i];
            return true;
        };
        auto number = [&](int& out)
        {
            std::string v;
            if (!value(v))
            {
                return false;
            }
            if (!ParseInt(v.c_str(), out) || out < 0)
            {
                error = a + ": " + v + " isn't a number";
                return false;
            }
            return true;
        };
        std::string v;
        if (a == "--host")
        {
            o.mode = PortLobbyOptions::Host;
            network = true;
        }
        else if (a == "--join")
        {
            o.mode = PortLobbyOptions::Join;
            network = true;
            if (i + 1 < argc && argv[i + 1][0] != '-')
            {
                net.address = argv[++i];
            }
        }
        else if (a == "--server")
        {
            if (!value(v))
            {
                return false;
            }
            net.hostServer = v;
            net.address = v;
        }
        else if (a == "--name")
        {
            if (!value(o.name))
            {
                return false;
            }
        }
        else if (a == "--game")
        {
            if (!value(o.session))
            {
                return false;
            }
        }
        else if (a == "--port")
        {
            int port;
            if (!number(port))
            {
                return false;
            }
            if (port < 1 || port > 65535)
            {
                error = "--port: out of range";
                return false;
            }
            net.port = static_cast<uint16_t>(port);
        }
        else if (a == "--players")
        {
            if (!number(o.players))
            {
                return false;
            }
            if (o.players < 1 || o.players > 6)
            {
                error = "--players: GTA2 has 1 to 6 players";
                return false;
            }
        }
        else if (a == "--map")
        {
            if (!value(o.map))
            {
                return false;
            }
        }
        else if (a == "--mode")
        {
            if (!value(v))
            {
                return false;
            }
            o.gameType = v == "frags" ? 1 : v == "points" ? 2 : v == "tag" ? 3 : 0;
            if (!o.gameType)
            {
                error = "--mode: frags, points or tag";
                return false;
            }
        }
        else if (a == "--limit")
        {
            if (!number(o.limit))
            {
                return false;
            }
        }
        else if (a == "--time-limit")
        {
            if (!number(o.timeLimit))
            {
                return false;
            }
        }
        else if (a == "--police")
        {
            if (!value(v))
            {
                return false;
            }
            o.police = v == "on" ? 1 : v == "off" ? 0 : -1;
            if (o.police < 0)
            {
                error = "--police: on or off";
                return false;
            }
        }
        else if (a == "--speed")
        {
            if (!value(v))
            {
                return false;
            }
            o.speed = v == "slow" ? 0 : v == "normal" ? 1 : v == "fast" ? 2 : -1;
            if (o.speed < 0)
            {
                error = "--speed: slow, normal or fast";
                return false;
            }
        }
        else if (a == "--net-log")
        {
            net.verbose = true;
        }
        else
        {
            // The game's own: -n starts a network game, asking on the terminal
            if (a == "-n" || a == "-N" || a == "/n" || a == "/N")
            {
                network = true;
            }
            else
            {
                rest.push_back(a);
            }
        }
    }
    if (network)
    {
        rest.insert(rest.begin(), "-n");
    }
    return true;
}

namespace
{
const HWND kLobbyWindow = reinterpret_cast<HWND>(0x103);

// Names and chat as the game's UTF-16 (std::wstring would be glibc's 4-byte wchar_t underneath)
std::u16string Wide(const std::string& s)
{
    std::u16string w;
    for (unsigned char c : s)
    {
        w.push_back(static_cast<char16_t>(c));
    }
    return w;
}

wchar_t* W(std::u16string& s)
{
    return reinterpret_cast<wchar_t*>(&s[0]);
}

const wchar_t* W(const std::u16string& s)
{
    return reinterpret_cast<const wchar_t*>(s.c_str());
}

std::string Narrow(const wchar_t* w)
{
    std::string s;
    for (; w && *w; w++)
    {
        s.push_back(*w < 0x80 ? static_cast<char>(*w) : '?');
    }
    return s;
}

bool Interactive()
{
    return isatty(0) != 0;
}

// A line typed on the terminal, if one is waiting
bool ReadLine(std::string& line)
{
    if (!Interactive())
    {
        return false;
    }
    pollfd p {0, POLLIN, 0};
    if (poll(&p, 1, 0) <= 0 || !(p.revents & POLLIN))
    {
        return false;
    }
    char buffer[256];
    if (!fgets(buffer, sizeof(buffer), stdin))
    {
        return false;
    }
    line = buffer;
    while (!line.empty() && (line.back() == '\n' || line.back() == '\r'))
    {
        line.pop_back();
    }
    return true;
}

std::string Ask(const std::string& question, const std::string& def)
{
    printf("%s%s%s%s: ", question.c_str(), def.empty() ? "" : " [", def.c_str(), def.empty() ? "" : "]");
    fflush(stdout);
    char buffer[256];
    if (!fgets(buffer, sizeof(buffer), stdin))
    {
        return def;
    }
    std::string s = buffer;
    while (!s.empty() && (s.back() == '\n' || s.back() == '\r' || s.back() == ' '))
    {
        s.pop_back();
    }
    return s.empty() ? def : s;
}

void Pump()
{
    gNetPlay_7071E8.NetworkTick_51ED00();
    Sleep(10);
}

// Prints the player list when it changes
std::string gLastPlayers;
int PrintPlayers()
{
    std::string list;
    int count = 0;
    Network_Game_Session& s = gNetPlay_7071E8.field_758_n2;
    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        if (s.field_10_players[i].field_0_in_use)
        {
            if (count++)
            {
                list += ", ";
            }
            list += Narrow(s.field_10_players[i].field_1C_player_name);
            if (i == gNetPlay_7071E8.field_5D4_player_idx)
            {
                list += " (you)";
            }
        }
    }
    if (list != gLastPlayers)
    {
        printf("Players (%d): %s\n", count, list.c_str());
        fflush(stdout);
        gLastPlayers = list;
    }
    return count;
}

// NetPlay's chat callback (slot 7), printing instead of filling the dialog's edit box
void __stdcall OnChat(void*, sptr message)
{
    const Network_ChatMessage* m = reinterpret_cast<const Network_ChatMessage*>(message);
    printf("%s: %s\n", Narrow(m->field_100_name).c_str(), Narrow(m->field_0_message).c_str());
    fflush(stdout);
}

void SendChat(const std::string& line)
{
    std::u16string w = Wide(line.substr(0, 120));
    gNetPlay_7071E8.SendChatMessage_521060(W(w), -1);
}

void Fail(const std::string& what)
{
    const std::string why = PortNetLastError();
    fprintf(stderr, "%s%s%s\n", what.c_str(), why.empty() ? "" : ": ", why.c_str());
    fflush(stderr);
}

const char* GameTypeName(u32 type)
{
    switch (type)
    {
        case 1:
            return "frags";
        case 2:
            return "points";
        case 3:
            return "tag";
        default:
            return "?";
    }
}

int FindMap(Network_20324* ui, const std::string& wanted)
{
    int n;
    if (ParseInt(wanted.c_str(), n))
    {
        return n >= 1 && n <= ui->field_1FD64_total_map_count ? n - 1 : -1;
    }
    std::string w = wanted;
    for (char& c : w)
    {
        c = static_cast<char>(tolower(c));
    }
    for (int i = 0; i < ui->field_1FD64_total_map_count; i++)
    {
        const Network_Enumerated_Map& m = ui->field_4_maps[i];
        for (const char* field : {m.field_30C_description, m.field_410_mmp_name, m.field_0_map_name})
        {
            std::string f = field;
            for (char& c : f)
            {
                c = static_cast<char>(tolower(c));
            }
            if (f.find(w) != std::string::npos)
            {
                return i;
            }
        }
    }
    return -1;
}

void ListMaps(Network_20324* ui)
{
    for (int i = 0; i < ui->field_1FD64_total_map_count; i++)
    {
        printf("  %d: %s (%u players)\n", i + 1, ui->field_4_maps[i].field_30C_description, ui->field_4_maps[i].field_514_player_count);
    }
}

INT_PTR Host(Network_20324* ui, const std::string& name, const std::u16string& wname)
{
    PortLobbyOptions& o = PortLobby();
    if (ui->field_1FD64_total_map_count == 0)
    {
        fprintf(stderr, "No multiplayer maps (data/*.mmp) found\n");
        return 0;
    }

    // The settings saved last time, then what the command line changes (as the dialog's controls would)
    ui->LoadGameSettingsFromRegistry_51C830();
    NetworkGameSettings& s = ui->field_20088_game_settings;
    if (!o.map.empty())
    {
        const int map = FindMap(ui, o.map);
        if (map < 0)
        {
            fprintf(stderr, "No map matches \"%s\". The maps:\n", o.map.c_str());
            ListMaps(ui);
            return 0;
        }
        s.field_2018C_map_idx = map;
        gRegistry_6FF968.Set_Network_Setting_587730("map_index", map);
    }
    strcpy(s.field_20088_default_map, ui->field_4_maps[s.field_2018C_map_idx].field_410_mmp_name);
    if (o.gameType)
    {
        s.field_20198_game_type = o.gameType;
        gRegistry_6FF968.Set_Network_Setting_587730("game_type", o.gameType);
        s.field_20194_frag_limit = o.gameType == 1 ? gRegistry_6FF968.Set_Network_Setting_587690("f_limit", 3) :
                                                     gRegistry_6FF968.Set_Network_Setting_587690("s_limit", 10000);
    }
    if (o.limit >= 0)
    {
        s.field_20194_frag_limit = o.limit;
        gRegistry_6FF968.Set_Network_Setting_587730(s.field_20198_game_type == 1 ? "f_limit" : "s_limit", o.limit);
    }
    if (o.timeLimit >= 0)
    {
        s.field_201A4_game_time_limit = o.timeLimit;
        gRegistry_6FF968.Set_Network_Setting_587730("game_time_limit", o.timeLimit);
    }
    if (o.police >= 0)
    {
        s.field_201A0_police_on = o.police;
        gRegistry_6FF968.Set_Network_Setting_587730("police", o.police);
    }
    if (o.speed >= 0)
    {
        s.field_20190_game_speed = o.speed;
        gRegistry_6FF968.Set_Network_Setting_587730("game_speed", o.speed);
    }

    // The dialog's "Create" button (Network_20324::OnWmCommand_519FE0, 1002)
    NetworkGameSettings settings = s;
    Network_8 groupData;
    groupData.field_0_data = &settings;
    groupData.field_4_len = sizeof(NetworkGameSettings);
    Network_8 playerData;
    playerData.field_0_data = 0;
    playerData.field_4_len = 0;
    std::u16string session = wname + u"'s game";
    std::u16string hostName = wname;
    if (!gNetPlay_7071E8.HostSession_51E7A0(W(session), W(hostName), &groupData, &playerData))
    {
        Fail("Couldn't host a game");
        return 0;
    }
    ui->SetPlayNameAndSaveToRegistry_51BD40(W(wname), name.c_str());
    ui->field_202D4_showing_specific_window_idx = 1; // the host's page, for the callbacks

    const Network_Enumerated_Map& map = ui->field_4_maps[s.field_2018C_map_idx];
    const int wanted = o.players ? o.players : 2;
    printf("Hosting \"%s\": %s, %s to %u, time limit %u, police %s.\n",
           Narrow(W(session)).c_str(),
           map.field_30C_description,
           GameTypeName(s.field_20198_game_type),
           s.field_20194_frag_limit,
           s.field_201A4_game_time_limit,
           s.field_201A0_police_on ? "on" : "off");
    if (PortNet().hostServer.empty())
    {
        printf("Others join with: gta2-launch --join <this computer's address>  (UDP port %u)\n", PortNet().port);
    }
    else
    {
        printf("Others join with: gta2-launch --join %s\n", PortNet().hostServer.c_str());
    }
    printf("The game starts when %d players are in%s.\n", wanted, Interactive() ? ". Press Enter to start sooner, or type to chat" : "");
    fflush(stdout);

    uint32_t enoughSince = 0;
    for (;;)
    {
        Pump();
        const int count = PrintPlayers();
        std::string line;
        bool start = false;
        if (ReadLine(line))
        {
            if (line.empty())
            {
                start = true;
            }
            else
            {
                SendChat(line);
            }
        }
        if (count >= wanted)
        {
            // A moment for the last one to finish joining
            if (!enoughSince)
            {
                enoughSince = timeGetTime();
            }
            start = start || timeGetTime() - enoughSince > 1500;
        }
        else
        {
            enoughSince = 0;
        }
        if (start)
        {
            break;
        }
    }

    // The dialog's "Start" button (1021)
    printf("Starting.\n");
    fflush(stdout);
    gNetPlay_7071E8.NetworkTick_51ED00();
    Sleep(100);
    ui->SetPlayNameAndSaveToRegistry_51BD40(W(wname), name.c_str());
    gNetPlay_7071E8.DisableJoining_521220();
    return 1;
}

INT_PTR Join(Network_20324* ui, const std::string& name, const std::u16string& wname)
{
    PortLobbyOptions& o = PortLobby();
    const std::string where = PortNet().address.empty() ? std::string("on this network") : "at " + PortNet().address;
    printf("Looking for a game %s...\n", where.c_str());
    fflush(stdout);

    // NetworkTick re-lists the sessions every second while browsing
    const uint32_t start = timeGetTime();
    int index = -1;
    std::string shown;
    while (index < 0)
    {
        Pump();
        Network_504& sessions = gNetPlay_7071E8.field_C4_sessions;
        std::string list;
        for (u32 i = 0; i < sessions.field_5C4_session_count; i++)
        {
            const std::string sessionName = Narrow(sessions.field_C4_sessions[i].lpszSessionName);
            list += "  " + sessionName + "\n";
            if (index < 0 && (o.session.empty() || sessionName.find(o.session) != std::string::npos))
            {
                index = static_cast<int>(i);
            }
        }
        if (list != shown && !list.empty())
        {
            printf("Found:\n%s", list.c_str());
            fflush(stdout);
            shown = list;
        }
        if (index < 0 && timeGetTime() - start > 15000)
        {
            fprintf(stderr,
                    "No game found %s%s. Is it hosted, and %s?\n",
                    where.c_str(),
                    o.session.empty() ? "" : (" named like \"" + o.session + "\"").c_str(),
                    PortNet().address.empty() ? "on the same network" : "is its UDP port reachable");
            const std::string why = PortNetLastError();
            if (!why.empty())
            {
                fprintf(stderr, "(%s)\n", why.c_str());
            }
            return 0;
        }
    }

    // The dialog's "Join" button (1001)
    const std::string sessionName = Narrow(gNetPlay_7071E8.field_C4_sessions.field_C4_sessions[index].lpszSessionName);
    printf("Joining \"%s\"...\n", sessionName.c_str());
    fflush(stdout);
    Network_8 playerData;
    playerData.field_0_data = 0;
    playerData.field_4_len = 0;
    Network_8 groupData;
    groupData.field_0_data = 0;
    groupData.field_4_len = 0;
    std::u16string playerName = wname;
    ui->SetPlayNameAndSaveToRegistry_51BD40(W(wname), name.c_str());
    ui->field_202D4_showing_specific_window_idx = 2; // the joined page, for the callbacks
    if (!gNetPlay_7071E8.JoinSession_520570(index, W(playerName), &playerData, &groupData))
    {
        Fail("Couldn't join \"" + sessionName + "\"");
        return 0;
    }
    ui->ApplyHostGameSettings_51B810(static_cast<const char_type*>(groupData.field_0_data));

    // The host's map has to be here too
    const NetworkGameSettings& s = ui->field_20088_game_settings;
    bool haveMap = false;
    for (int i = 0; i < ui->field_1FD64_total_map_count; i++)
    {
        haveMap = haveMap || !strcmp(ui->field_4_maps[i].field_410_mmp_name, s.field_20088_default_map);
    }
    if (!haveMap)
    {
        fprintf(stderr, "The host plays %s, which isn't in your data folder\n", s.field_20088_default_map);
        gNetPlay_7071E8.Disconnect_520D10();
        return 0;
    }
    printf("Joined: %s, %s to %u, time limit %u, police %s. Waiting for the host to start%s.\n",
           ui->field_4_maps[s.field_2018C_map_idx].field_30C_description,
           GameTypeName(s.field_20198_game_type),
           s.field_20194_frag_limit,
           s.field_201A4_game_time_limit,
           s.field_201A0_police_on ? "on" : "off",
           Interactive() ? " (type to chat)" : "");
    fflush(stdout);

    // The host's "disable joining" packet ends the dialog (NetPlay callback 9, cb_SavePlayerName_51BC00)
    while (!ui->GetEndDialog_51BC80())
    {
        Pump();
        PrintPlayers();
        std::string line;
        if (ReadLine(line) && !line.empty())
        {
            SendChat(line);
        }
        if (ui->field_202D4_showing_specific_window_idx == 0 || PortNetSessionLost())
        {
            // NetPlay's disconnect callback went back to the first page, or the host is gone
            fprintf(stderr, "The host closed the game\n");
            return 0;
        }
    }
    printf("Starting.\n");
    fflush(stdout);
    return 1;
}
} // namespace

extern "C" INT_PTR PortRunDialog(HINSTANCE, LPCSTR templateName, DLGPROC, LPARAM param)
{
    if (templateName != reinterpret_cast<LPCSTR>(103))
    {
        return -1;
    }
    Network_20324* ui = reinterpret_cast<Network_20324*>(param);
    PortLobbyOptions& o = PortLobby();
    ui->SetDlgHwnd_519E10(kLobbyWindow);

    // Chat goes to the terminal
    gNetPlay_7071E8.field_4C_func_ptrs_and_params[7].field_0_param_fn_callback = reinterpret_cast<void*>(OnChat);
    gNetPlay_7071E8.field_4C_func_ptrs_and_params[7].field_4_param_context = ui;

    std::string name = o.name.empty() ? ui->field_1FF80_player_name : o.name;
    if (o.mode == PortLobbyOptions::Ask)
    {
        if (!Interactive())
        {
            fprintf(stderr, "Start a network game with --host or --join:\n%s", PortNetUsage());
            return 0;
        }
        const std::string answer = Ask("Host a game or join one? (h/j)", "j");
        o.mode = !answer.empty() && (answer[0] == 'h' || answer[0] == 'H') ? PortLobbyOptions::Host : PortLobbyOptions::Join;
        if (o.mode == PortLobbyOptions::Join && PortNet().address.empty())
        {
            PortNet().address = Ask("Address of the game (empty: look on this network)", "");
        }
        if (o.mode == PortLobbyOptions::Host && o.map.empty())
        {
            ListMaps(ui);
            o.map = Ask("Map", std::to_string(ui->field_20088_game_settings.field_2018C_map_idx + 1));
        }
        if (o.name.empty())
        {
            name = Ask("Your name", name.empty() ? "Player" : name);
        }
    }
    if (name.empty())
    {
        const char* user = getenv("USER");
        name = user && *user ? user : "Player";
    }
    name = name.substr(0, 15); // the game keeps 15 characters
    SetDlgItemTextA(kLobbyWindow, 1004, name.c_str()); // the dialog's name box, which the game reads back
    const std::u16string wname = Wide(name);

    printf("GTA2 network game (%s)\n", PortNetBuild());
    fflush(stdout);
    const INT_PTR result = o.mode == PortLobbyOptions::Host ? Host(ui, name, wname) : Join(ui, name, wname);
    if (!result)
    {
        gNetPlay_7071E8.field_4C_func_ptrs_and_params[7].field_0_param_fn_callback = 0;
    }
    return result;
}
