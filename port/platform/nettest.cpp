// gta2 --nettest: plays the network part of a game through the port's DirectPlay, the way NetPlay
// does, without game data or a window. port/tools/nettest.sh runs several of these against each other
// (with lost datagrams, players leaving, a host that dies, builds of other architectures) and CI runs
// that script.
//
//   gta2 --nettest host [--players N] [--frames N] [--port P] [--server ADDR] [--name NAME]
//   gta2 --nettest join [--address ADDR] [--frames N] [--port P] [--name NAME]
//                       [--leave-after N] [--expect-lost] [--expect-refused]
//
// After everyone is in, each player sends one small message a frame to all the others and waits for
// every other player's message of that frame, as the game's lockstep does (winmain.cpp's
// Net_Receive_Players_Inputs_4DA9F0). Every message is checked: from whom, which frame, what it holds.
#include "dplay_port.h"

#include <VC98/Include/DPLAY.H>
#include <VC98/Include/DPLOBBY.H>

#include <execinfo.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <map>
#include <set>
#include <string>
#include <vector>

namespace
{
const GUID kTestApplication = {0x12345678, 0x1234, 0x5678, {1, 2, 3, 4, 5, 6, 7, 8}};

struct Options
{
    bool host = false;
    int players = 2;
    int frames = 300;
    int leaveAfter = -1;
    bool expectLost = false;
    bool expectRefused = false;
    std::string name;
    std::string address;
};

IDirectPlay3* gDp = nullptr;
DPID gGroup = 0;
DPID gMe = 0;
std::set<DPID> gInGroup; // players in the group, as the system messages tell it
std::map<DPID, std::string> gNames;
std::set<DPID> gLeft;
bool gStarted = false;
bool gLost = false;
int gChats = 0;
uint32_t gStart;
// Next frame expected from each player
std::map<DPID, uint32_t> gNextFrame;

[[noreturn]] void Fail(const std::string& why)
{
    for (auto& kv : gNames)
    {
        fprintf(stderr,
                "  known: %lu %s%s\n",
                static_cast<unsigned long>(kv.first),
                kv.second.c_str(),
                gInGroup.count(kv.first) ? " (in the group)" : "");
    }
    for (DPID id : gInGroup)
    {
        fprintf(stderr, "  in the group: %lu\n", static_cast<unsigned long>(id));
    }
    for (auto& kv : gNextFrame)
    {
        fprintf(stderr,
                "  next frame from %lu: %u%s\n",
                static_cast<unsigned long>(kv.first),
                kv.second,
                gLeft.count(kv.first) ? " (left)" : "");
    }
    fprintf(stderr, "%s", PortNetDescribe().c_str());
    fprintf(stderr, "nettest %s: FAILED: %s\n", gNames.count(gMe) ? gNames[gMe].c_str() : "?", why.c_str());
    fflush(stderr);
    exit(1);
}

void Check(bool ok, const std::string& what)
{
    if (!ok)
    {
        Fail(what);
    }
}

std::string Narrow(const wchar_t* w)
{
    std::string s;
    for (; w && *w; w++)
    {
        s.push_back(static_cast<char>(*w));
    }
    return s;
}

std::u16string Wide(const std::string& s)
{
    return std::u16string(s.begin(), s.end());
}

uint32_t Expected(DPID from, uint32_t frame)
{
    // Something both ends compute, different for every sender and frame
    uint32_t v = from * 2654435761u ^ frame * 40503u;
    return v ^ (v >> 13);
}

struct Frame
{
    uint8_t tag; // 'F': a frame, 'S': the host starts the game
    uint32_t frame;
    uint32_t value;
};

std::vector<uint8_t> Pack(uint8_t tag, uint32_t frame, uint32_t value)
{
    std::vector<uint8_t> b(9);
    b[0] = tag;
    memcpy(&b[1], &frame, 4);
    memcpy(&b[5], &value, 4);
    return b;
}

// The game starts with whoever is in the group
void BeginPlay()
{
    gStarted = true;
    for (DPID id : gInGroup)
    {
        if (id != gMe)
        {
            gNextFrame[id] = 0;
        }
    }
}

void Handle(DPID from, const uint8_t* data, DWORD size)
{
    if (from == DPID_SYSMSG)
    {
        const DPMSG_GENERIC* m = reinterpret_cast<const DPMSG_GENERIC*>(data);
        switch (m->dwType)
        {
            case DPSYS_CREATEPLAYERORGROUP:
            {
                const DPMSG_CREATEPLAYERORGROUP* c = reinterpret_cast<const DPMSG_CREATEPLAYERORGROUP*>(data);
                if (c->dwPlayerType == DPPLAYERTYPE_PLAYER)
                {
                    gNames[c->dpId] = Narrow(c->dpnName.lpszShortName);
                }
                break;
            }
            case DPSYS_ADDPLAYERTOGROUP:
            {
                const DPMSG_ADDPLAYERTOGROUP* a = reinterpret_cast<const DPMSG_ADDPLAYERTOGROUP*>(data);
                if (a->dpIdGroup == gGroup)
                {
                    gInGroup.insert(a->dpIdPlayer);
                }
                break;
            }
            case DPSYS_DELETEPLAYERFROMGROUP:
            {
                const DPMSG_DELETEPLAYERFROMGROUP* d = reinterpret_cast<const DPMSG_DELETEPLAYERFROMGROUP*>(data);
                if (d->dpIdGroup == gGroup)
                {
                    gInGroup.erase(d->dpIdPlayer);
                    gLeft.insert(d->dpIdPlayer);
                }
                break;
            }
            case DPSYS_DESTROYPLAYERORGROUP:
                gLeft.insert(reinterpret_cast<const DPMSG_DESTROYPLAYERORGROUP*>(data)->dpId);
                break;
            case DPSYS_CHAT:
            {
                const DPMSG_CHAT* c = reinterpret_cast<const DPMSG_CHAT*>(data);
                Check(Narrow(c->lpChat->lpszMessage) == "hello from " + gNames[c->idFromPlayer],
                      "chat text \"" + Narrow(c->lpChat->lpszMessage) + "\" from " + std::to_string(c->idFromPlayer) + " (" +
                          gNames[c->idFromPlayer] + ")");
                gChats++;
                break;
            }
            case DPSYS_SESSIONLOST:
                gLost = true;
                break;
        }
        return;
    }
    Check(size == 9, "message size " + std::to_string(size));
    uint32_t frame;
    uint32_t value;
    memcpy(&frame, data + 1, 4);
    memcpy(&value, data + 5, 4);
    if (data[0] == 'S')
    {
        // Frames from the other players can follow in the same batch
        BeginPlay();
        return;
    }
    Check(data[0] == 'F', "message tag");
    Check(value == Expected(from, frame), "message contents");
    auto it = gNextFrame.find(from);
    Check(it != gNextFrame.end(), "message from a player outside the game");
    Check(frame == it->second,
          "out of order: frame " + std::to_string(frame) + " from " + gNames[from] + ", expected " + std::to_string(it->second));
    it->second++;
}

void Pump(int sleepMs)
{
    uint8_t buffer[0x1800];
    for (;;)
    {
        DPID from = 0;
        DPID to = 0;
        DWORD size = sizeof(buffer);
        const HRESULT hr = gDp->Receive(&from, &to, DPRECEIVE_ALL, buffer, &size);
        if (hr == DPERR_NOMESSAGES)
        {
            break;
        }
        Check(hr == DP_OK, "Receive failed");
        if (from != DPID_SYSMSG)
        {
            Check(to == gMe, "message addressed to someone else");
        }
        Handle(from, buffer, size);
    }
    Check(timeGetTime() - gStart < 60000, "took over a minute");
    if (sleepMs)
    {
        Sleep(sleepMs);
    }
}

BOOL OnConnection(LPCGUID sp, LPVOID, DWORD, LPCDPNAME, DWORD, LPVOID ctx)
{
    if (*sp == DPSPGUID_TCPIP)
    {
        *static_cast<bool*>(ctx) = true;
    }
    return TRUE;
}

std::vector<DPSESSIONDESC2> gFound;
std::vector<std::u16string> gFoundNames;
BOOL OnSession(LPCDPSESSIONDESC2 desc, LPDWORD, DWORD flags, LPVOID)
{
    if (flags & DPESC_TIMEDOUT)
    {
        return FALSE;
    }
    gFound.push_back(*desc);
    gFoundNames.push_back(std::u16string(reinterpret_cast<const char16_t*>(desc->lpszSessionName)));
    return TRUE;
}

BOOL OnGroup(DPID id, DWORD type, LPCDPNAME, DWORD, LPVOID)
{
    if (type == DPPLAYERTYPE_GROUP)
    {
        gGroup = id;
    }
    return TRUE;
}

BOOL OnGroupPlayer(DPID id, DWORD, LPCDPNAME name, DWORD, LPVOID)
{
    gInGroup.insert(id);
    gNames[id] = Narrow(name->lpszShortName);
    return TRUE;
}

const char kSettings[] = "the host's game settings";

int Run(const Options& o)
{
    gStart = timeGetTime();
    // As NetPlay::DirectPlayCreate_51DCD0 and SetProtoAndConnection_51DAE0 do
    Check(CoCreateInstance(CLSID_DirectPlay, 0, 1, IID_IDirectPlay3, reinterpret_cast<void**>(&gDp)) == S_OK, "CoCreateInstance");
    IDirectPlayLobby* lobby1 = nullptr;
    Check(DirectPlayLobbyCreateW(0, &lobby1, 0, 0, 0) == DP_OK, "DirectPlayLobbyCreateW");
    IDirectPlayLobby2* lobby = nullptr;
    Check(lobby1->QueryInterface(IID_IDirectPlayLobby2, reinterpret_cast<void**>(&lobby)) == S_OK, "IDirectPlayLobby2");
    lobby1->Release();

    bool haveTcp = false;
    Check(gDp->EnumConnections(&kTestApplication, OnConnection, &haveTcp, 0) == DP_OK && haveTcp, "EnumConnections");

    // The connection, with the address to look at (NetPlay::CreateTcpIpAddress_51E140)
    std::u16string address = Wide(o.address);
    DPCOMPOUNDADDRESSELEMENT elements[2];
    elements[0].guidDataType = DPAID_ServiceProvider;
    elements[0].dwDataSize = sizeof(GUID);
    elements[0].lpData = const_cast<GUID*>(&DPSPGUID_TCPIP);
    elements[1].guidDataType = DPAID_INetW;
    elements[1].dwDataSize = static_cast<DWORD>(address.size() * 2 + 2);
    elements[1].lpData = &address[0];
    DWORD size = 0;
    Check(lobby->CreateCompoundAddress(elements, 2, nullptr, &size) == DPERR_BUFFERTOOSMALL, "CreateCompoundAddress size");
    std::vector<uint8_t> connection(size);
    Check(lobby->CreateCompoundAddress(elements, 2, connection.data(), &size) == DP_OK, "CreateCompoundAddress");
    Check(gDp->InitializeConnection(connection.data(), 0) == DP_OK, "InitializeConnection");

    std::u16string name = Wide(o.name);
    DPNAME dpname;
    memset(&dpname, 0, sizeof(dpname));
    dpname.dwSize = sizeof(dpname);
    dpname.lpszShortName = reinterpret_cast<LPWSTR>(&name[0]);

    if (o.host)
    {
        // NetPlay::HostSession_51E7A0
        DPSESSIONDESC2 desc;
        memset(&desc, 0, sizeof(desc));
        desc.dwSize = sizeof(desc);
        desc.guidApplication = kTestApplication;
        desc.dwMaxPlayers = 6;
        std::u16string session = name + u"'s game";
        desc.lpszSessionName = reinterpret_cast<LPWSTR>(&session[0]);
        Check(gDp->Open(&desc, DPOPEN_CREATE) == DP_OK, "Open(create): " + PortNetLastError());
        Check(gDp->CreatePlayer(&gMe, &dpname, 0, nullptr, 0, 0) == DP_OK, "CreatePlayer");
        gNames[gMe] = o.name;
        DPNAME groupName;
        memset(&groupName, 0, sizeof(groupName));
        groupName.dwSize = sizeof(groupName);
        groupName.lpszShortName = const_cast<LPWSTR>(L"GBH Group");
        Check(gDp->CreateGroup(&gGroup, &groupName, const_cast<char*>(kSettings), sizeof(kSettings), 0) == DP_OK, "CreateGroup");
        Check(gDp->AddPlayerToGroup(gGroup, gMe) == DP_OK, "AddPlayerToGroup");
        printf("nettest %s: hosting, waiting for %d players\n", o.name.c_str(), o.players);
        fflush(stdout);
        while (static_cast<int>(gInGroup.size()) < o.players)
        {
            Pump(5);
        }
        // Starts the game and closes it to others (NetPlay::DisableJoining_521220)
        std::vector<uint8_t> start = Pack('S', 0, 0);
        Check(gDp->Send(gMe, gGroup, DPSEND_GUARANTEED, start.data(), static_cast<DWORD>(start.size())) == DP_OK, "Send start");
        DWORD descSize = 0;
        gDp->GetSessionDesc(nullptr, &descSize);
        std::vector<uint8_t> descBuffer(descSize);
        Check(gDp->GetSessionDesc(descBuffer.data(), &descSize) == DP_OK, "GetSessionDesc");
        DPSESSIONDESC2* current = reinterpret_cast<DPSESSIONDESC2*>(descBuffer.data());
        current->dwFlags |= DPSESSION_JOINDISABLED;
        Check(gDp->SetSessionDesc(current, 0) == DP_OK, "SetSessionDesc");
        BeginPlay();
    }
    else
    {
        // NetPlay::EnumSessions_51E650 and the lobby's browsing: asynchronous, again each second
        DPSESSIONDESC2 desc;
        memset(&desc, 0, sizeof(desc));
        desc.dwSize = sizeof(desc);
        desc.guidApplication = kTestApplication;
        uint32_t lastEnum = 0;
        while (gFound.empty())
        {
            if (!lastEnum || timeGetTime() - lastEnum > 300)
            {
                gFound.clear();
                gFoundNames.clear();
                Check(gDp->EnumSessions(&desc, 0, OnSession, nullptr, DPENUMSESSIONS_AVAILABLE | DPENUMSESSIONS_ASYNC) == DP_OK,
                      "EnumSessions");
                lastEnum = timeGetTime();
            }
            Check(timeGetTime() - gStart < 15000, "no session found");
            Sleep(20);
        }

        // NetPlay::JoinSession_520570
        DPSESSIONDESC2 join = gFound[0];
        join.lpszSessionName = reinterpret_cast<LPWSTR>(&gFoundNames[0][0]);
        const HRESULT open = gDp->Open(&join, DPOPEN_JOIN | DPOPEN_RETURNSTATUS);
        if (o.expectRefused)
        {
            Check(open != DP_OK, "a different build was let in");
            Check(PortNetLastError().find("different version") != std::string::npos, "no reason given: " + PortNetLastError());
            printf("nettest %s: OK, refused (%s)\n", o.name.c_str(), PortNetLastError().c_str());
            return 0;
        }
        Check(open == DP_OK, "Open(join): " + PortNetLastError());
        Check(gDp->CreatePlayer(&gMe, &dpname, 0, nullptr, 0, 0) == DP_OK, "CreatePlayer");
        gNames[gMe] = o.name;
        Check(gDp->EnumGroups(&join.guidInstance, OnGroup, nullptr, 0) == DP_OK && gGroup, "EnumGroups");
        Check(gDp->EnumGroupPlayers(gGroup, &join.guidInstance, OnGroupPlayer, nullptr, 0) == DP_OK, "EnumGroupPlayers");
        Check(!gInGroup.empty(), "the host isn't in the group");
        Check(gDp->AddPlayerToGroup(gGroup, gMe) == DP_OK, "AddPlayerToGroup");
        DWORD dataSize = 0;
        Check(gDp->GetGroupData(gGroup, nullptr, &dataSize, 0) == DPERR_BUFFERTOOSMALL && dataSize == sizeof(kSettings),
              "GetGroupData size");
        std::vector<char> data(dataSize);
        Check(gDp->GetGroupData(gGroup, data.data(), &dataSize, 0) == DP_OK && !memcmp(data.data(), kSettings, dataSize), "GetGroupData");
        printf("nettest %s: joined, waiting for the start\n", o.name.c_str());
        fflush(stdout);
        while (!gStarted)
        {
            Pump(5);
            Check(!gLost, "session lost before the start");
        }
    }

    // We are in the group too: our own adding came back as a system message
    Check(gInGroup.count(gMe) == 1, "our own DPSYS_ADDPLAYERTOGROUP never came");
    printf("nettest %s: playing with %zu others\n", o.name.c_str(), gNextFrame.size());
    fflush(stdout);

    std::u16string chat = Wide("hello from " + o.name);
    DPCHAT dpchat;
    memset(&dpchat, 0, sizeof(dpchat));
    dpchat.dwSize = sizeof(dpchat);
    dpchat.lpszMessage = reinterpret_cast<LPWSTR>(&chat[0]);
    Check(gDp->SendChatMessage(gMe, gGroup, 0, &dpchat) == DP_OK, "SendChatMessage");

    for (int frame = 0; frame < o.frames; frame++)
    {
        if (frame == o.leaveAfter)
        {
            printf("nettest %s: OK, leaving after %d frames\n", o.name.c_str(), frame);
            fflush(stdout);
            gDp->DestroyPlayer(gMe);
            gDp->Close();
            gDp->Release();
            return 0;
        }
        std::vector<uint8_t> m = Pack('F', frame, Expected(gMe, frame));
        if (gDp->Send(gMe, DPID_ALLPLAYERS, 0, m.data(), static_cast<DWORD>(m.size())) != DP_OK)
        {
            // Only once the session is gone, which the next message says
            Pump(0);
            Check(gLost, "Send");
        }
        // Lockstep: wait for every other player's message of this frame
        for (;;)
        {
            if (gLost)
            {
                if (o.expectLost)
                {
                    printf("nettest %s: OK, the session was lost as expected\n", o.name.c_str());
                    return 0;
                }
                Fail("session lost");
            }
            bool all = true;
            for (auto& kv : gNextFrame)
            {
                if (!gLeft.count(kv.first) && kv.second <= static_cast<uint32_t>(frame))
                {
                    all = false;
                }
            }
            if (all)
            {
                break;
            }
            Pump(0);
            Sleep(1);
        }
    }
    Check(!o.expectLost, "the session wasn't lost");

    // Wait for the chats of the players who are still here
    const uint32_t chatWait = timeGetTime();
    size_t stayed = 0;
    for (auto& kv : gNextFrame)
    {
        stayed += !gLeft.count(kv.first);
    }
    while (gChats < static_cast<int>(stayed) && timeGetTime() - chatWait < 5000)
    {
        Pump(5);
    }
    Check(gChats >= static_cast<int>(stayed), "chat messages missing");
    // Stay a moment so slower players get their last frames from us before we go
    const uint32_t linger = timeGetTime();
    while (timeGetTime() - linger < 1000)
    {
        Pump(10);
    }
    printf("nettest %s: OK, %d frames with %zu others (%zu left during the game), %s\n",
           o.name.c_str(),
           o.frames,
           gNextFrame.size(),
           gLeft.size(),
           PortNetBuild());
    fflush(stdout);
    gDp->DestroyPlayer(gMe);
    gDp->Close();
    gDp->Release();
    return 0;
}
} // namespace

static void OnCrash(int sig)
{
    void* frames[64];
    const int n = backtrace(frames, 64);
    fprintf(stderr, "nettest: crashed (signal %d)\n", sig);
    backtrace_symbols_fd(frames, n, 2);
    _exit(128 + sig);
}

int port_nettest(int argc, char** argv)
{
    signal(SIGSEGV, OnCrash);
    signal(SIGABRT, OnCrash);
    Options o;
    for (int i = 2; i < argc; i++)
    {
        const std::string a = argv[i];
        const char* v = i + 1 < argc ? argv[i + 1] : "";
        if (a == "host")
        {
            o.host = true;
        }
        else if (a == "join")
        {
            o.host = false;
        }
        else if (a == "--players")
        {
            o.players = atoi(v), i++;
        }
        else if (a == "--frames")
        {
            o.frames = atoi(v), i++;
        }
        else if (a == "--leave-after")
        {
            o.leaveAfter = atoi(v), i++;
        }
        else if (a == "--name")
        {
            o.name = v, i++;
        }
        else if (a == "--address")
        {
            o.address = v, i++;
        }
        else if (a == "--port")
        {
            PortNet().port = static_cast<uint16_t>(atoi(v)), i++;
        }
        else if (a == "--server")
        {
            PortNet().hostServer = v, i++;
        }
        else if (a == "--expect-lost")
        {
            o.expectLost = true;
        }
        else if (a == "--expect-refused")
        {
            o.expectRefused = true;
        }
        else if (a == "--net-log")
        {
            PortNet().verbose = true;
        }
        else
        {
            fprintf(stderr, "nettest: unknown option %s\n", a.c_str());
            return 2;
        }
    }
    if (o.name.empty())
    {
        o.name = o.host ? "host" : "player";
    }
    return Run(o);
}
