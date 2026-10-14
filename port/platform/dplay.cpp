// DirectPlay for the native build: the IDirectPlay3 and IDirectPlayLobby2 calls the game makes
// (Source/NetPlay.cpp), over the port's own UDP session protocol (net/net.h).
//
// The game sees the DirectPlay it was written for: one connection ("Internet TCP/IP"), sessions it
// finds and opens, players and their group, system messages and player messages from Receive. What
// travels is the port's: a session server (inside the hosting game, or a separate gta2-server) that
// relays everything reliably and in order. Machines of any architecture and either pointer size play
// together, as long as they run the same game code (PortNetCompat).
#include "dplay_port.h"
#include "gta2_compat.h"
// windows.h defines SAL's __out as nothing, which libstdc++'s <thread> uses as a name
#pragma push_macro("__out")
#undef __out
#include "net/net.h"
#pragma pop_macro("__out")

#include <VC98/Include/DPLAY.H>
#include <VC98/Include/DPLOBBY.H>

#include <stdio.h>
#include <string.h>

#include <algorithm>
#include <map>
#include <memory>

namespace
{
std::string gLastError;

void SetError(const std::string& text)
{
    gLastError = text;
    if (!text.empty())
    {
        fprintf(stderr, "network: %s\n", text.c_str());
        fflush(stderr);
    }
}

void Log(const std::string& text)
{
    if (PortNet().verbose)
    {
        fprintf(stderr, "network: %s\n", text.c_str());
        fflush(stderr);
    }
}

std::string BuildDescription()
{
#if defined(__linux__)
    std::string os = "linux";
#elif defined(__APPLE__)
    std::string os = "macos";
#elif defined(__FreeBSD__)
    std::string os = "freebsd";
#else
    std::string os = "unix";
#endif
#if defined(__x86_64__)
    const char* arch = "x86-64";
#elif defined(__i386__)
    const char* arch = "x86";
#elif defined(__aarch64__)
    const char* arch = "arm64";
#elif defined(__arm__)
    const char* arch = "arm";
#elif defined(__riscv)
    const char* arch = "riscv";
#else
    const char* arch = "cpu";
#endif
    return os + " " + arch + ", game " + std::string(GTA2_GAME_HASH).substr(0, 8);
}

std::u16string ToU16(const wchar_t* s)
{
    std::u16string out;
    if (s)
    {
        for (; *s; s++)
        {
            out.push_back(static_cast<char16_t>(*s));
        }
    }
    return out;
}

std::string ToUtf8(const std::u16string& s)
{
    std::string out;
    for (char16_t c : s)
    {
        out += c < 0x80 ? static_cast<char>(c) : '?';
    }
    return out;
}

// One connection for the whole game: the game creates several DirectPlay objects over its life
// (and a second one just to ask about addresses), and they all speak for the same machine
net::Client* gClient = nullptr;
net::Server* gServer = nullptr;
// What EnumSessions last listed, to open by instance guid
std::map<std::string, net::SessionInfo> gSessions;
// The address InitializeConnection was given (overrides PortNet().address when not empty)
std::string gConnectionAddress;

net::Client* Client()
{
    if (!gClient)
    {
        gClient = new net::Client(PortNetCompat(), PortNetBuild());
        std::string error;
        if (!gClient->Start(error))
        {
            SetError("can't open a network socket: " + error);
            delete gClient;
            gClient = nullptr;
        }
    }
    return gClient;
}

std::string LookAddress()
{
    return gConnectionAddress.empty() ? PortNet().address : gConnectionAddress;
}

HRESULT ToHresult(net::Result r)
{
    switch (r)
    {
        case net::kOk:
            return DP_OK;
        case net::kErrNoSession:
            return DPERR_NOSESSIONS;
        case net::kErrFull:
            return DPERR_CANTADDPLAYER;
        case net::kErrIncompatible:
            return DPERR_INVALIDINTERFACE;
        case net::kErrPassword:
            return DPERR_INVALIDPASSWORD;
        case net::kErrInvalidPlayer:
            return DPERR_INVALIDPLAYER;
        case net::kErrInvalidGroup:
            return DPERR_INVALIDGROUP;
        case net::kErrTimeout:
            return DPERR_TIMEOUT;
        case net::kErrNotConnected:
            return DPERR_NOCONNECTION;
        default:
            return DPERR_GENERIC;
    }
}

// Lays out a system message in the caller's buffer the way DirectPlay does: the structure first,
// then the strings and data its pointers point at
class MessageBuilder
{
  public:
    explicit MessageBuilder(std::vector<uint8_t>& bytes) : mBytes(bytes)
    {
    }
    template<typename T>
    T* Struct()
    {
        mBytes.assign(sizeof(T), 0);
        return nullptr;
    }
    size_t Append(const void* data, size_t size)
    {
        while (mBytes.size() % 8)
        {
            mBytes.push_back(0);
        }
        const size_t at = mBytes.size();
        const uint8_t* p = static_cast<const uint8_t*>(data);
        mBytes.insert(mBytes.end(), p, p + size);
        return at;
    }
    size_t AppendText(const std::u16string& s)
    {
        std::u16string z = s;
        z.push_back(0);
        return Append(z.data(), z.size() * 2);
    }
    std::vector<uint8_t>& mBytes;
};

template<typename T>
T* At(uint8_t* base)
{
    return reinterpret_cast<T*>(base);
}

void FillName(DPNAME& name, uint8_t* base, size_t shortAt, size_t longAt, bool hasLong)
{
    name.dwSize = sizeof(DPNAME);
    name.dwFlags = 0;
    name.lpszShortName = reinterpret_cast<LPWSTR>(base + shortAt);
    name.lpszLongName = hasLong ? reinterpret_cast<LPWSTR>(base + longAt) : nullptr;
}

DPID gLocalPlayerForMessages = 0;

// Turns an event into the bytes Receive returns, and says who it is from and to. Pointers in the
// message point into `out`.
void Encode(const net::Event& e, DPID& from, DPID& to, std::vector<uint8_t>& out)
{
    out.clear();
    MessageBuilder b(out);
    from = DPID_SYSMSG;
    to = gLocalPlayerForMessages;
    switch (e.type)
    {
        case net::Event::Message:
            from = e.from;
            to = e.to;
            out = e.data;
            return;

        case net::Event::CreateMember:
        {
            b.Struct<DPMSG_CREATEPLAYERORGROUP>();
            const size_t data = e.member.data.empty() ? 0 : b.Append(e.member.data.data(), e.member.data.size());
            const size_t shortName = b.AppendText(e.member.shortName);
            const size_t longName = b.AppendText(e.member.longName);
            uint8_t* base = b.mBytes.data();
            DPMSG_CREATEPLAYERORGROUP* m = At<DPMSG_CREATEPLAYERORGROUP>(base);
            m->dwType = DPSYS_CREATEPLAYERORGROUP;
            m->dwPlayerType = e.member.isGroup ? DPPLAYERTYPE_GROUP : DPPLAYERTYPE_PLAYER;
            m->dpId = e.member.id;
            m->dwCurrentPlayers = gClient ? gClient->Session().currentPlayers : 0;
            m->lpData = e.member.data.empty() ? nullptr : base + data;
            m->dwDataSize = static_cast<DWORD>(e.member.data.size());
            FillName(m->dpnName, base, shortName, longName, !e.member.longName.empty());
            m->dpIdParent = 0;
            m->dwFlags = e.member.flags;
            return;
        }

        case net::Event::DestroyMember:
        {
            b.Struct<DPMSG_DESTROYPLAYERORGROUP>();
            const size_t data = e.member.data.empty() ? 0 : b.Append(e.member.data.data(), e.member.data.size());
            const size_t shortName = b.AppendText(e.member.shortName);
            const size_t longName = b.AppendText(e.member.longName);
            uint8_t* base = b.mBytes.data();
            DPMSG_DESTROYPLAYERORGROUP* m = At<DPMSG_DESTROYPLAYERORGROUP>(base);
            m->dwType = DPSYS_DESTROYPLAYERORGROUP;
            m->dwPlayerType = e.member.isGroup ? DPPLAYERTYPE_GROUP : DPPLAYERTYPE_PLAYER;
            m->dpId = e.member.id;
            m->lpRemoteData = e.member.data.empty() ? nullptr : base + data;
            m->dwRemoteDataSize = static_cast<DWORD>(e.member.data.size());
            FillName(m->dpnName, base, shortName, longName, !e.member.longName.empty());
            m->dwFlags = e.member.flags;
            return;
        }

        case net::Event::AddToGroup:
        case net::Event::DeleteFromGroup:
        {
            b.Struct<DPMSG_ADDPLAYERTOGROUP>();
            DPMSG_ADDPLAYERTOGROUP* m = At<DPMSG_ADDPLAYERTOGROUP>(b.mBytes.data());
            m->dwType = e.type == net::Event::AddToGroup ? DPSYS_ADDPLAYERTOGROUP : DPSYS_DELETEPLAYERFROMGROUP;
            m->dpIdGroup = e.group;
            m->dpIdPlayer = e.player;
            return;
        }

        case net::Event::SetData:
        {
            b.Struct<DPMSG_SETPLAYERORGROUPDATA>();
            const size_t data = e.member.data.empty() ? 0 : b.Append(e.member.data.data(), e.member.data.size());
            uint8_t* base = b.mBytes.data();
            DPMSG_SETPLAYERORGROUPDATA* m = At<DPMSG_SETPLAYERORGROUPDATA>(base);
            m->dwType = DPSYS_SETPLAYERORGROUPDATA;
            m->dwPlayerType = e.member.isGroup ? DPPLAYERTYPE_GROUP : DPPLAYERTYPE_PLAYER;
            m->dpId = e.member.id;
            m->lpData = e.member.data.empty() ? nullptr : base + data;
            m->dwDataSize = static_cast<DWORD>(e.member.data.size());
            return;
        }

        case net::Event::Chat:
        {
            b.Struct<DPMSG_CHAT>();
            const size_t chat = b.Append(std::vector<uint8_t>(sizeof(DPCHAT)).data(), sizeof(DPCHAT));
            const size_t text = b.AppendText(e.text);
            uint8_t* base = b.mBytes.data();
            DPMSG_CHAT* m = At<DPMSG_CHAT>(base);
            m->dwType = DPSYS_CHAT;
            m->idFromPlayer = e.from;
            m->idToPlayer = e.to;
            m->idToGroup = e.group;
            DPCHAT* c = At<DPCHAT>(base + chat);
            c->dwSize = sizeof(DPCHAT);
            c->lpszMessage = reinterpret_cast<LPWSTR>(base + text);
            m->lpChat = c;
            return;
        }

        case net::Event::SessionLost:
        default:
        {
            b.Struct<DPMSG_GENERIC>();
            At<DPMSG_GENERIC>(b.mBytes.data())->dwType = DPSYS_SESSIONLOST;
            return;
        }
    }
}

// The pointers in a laid-out message point into the vector that was built; moved into the caller's
// buffer, they have to point there instead
void Rebase(const net::Event& e, std::vector<uint8_t>& bytes, uint8_t* oldBase, uint8_t* newBase)
{
    if (e.type == net::Event::Message || bytes.empty())
    {
        return;
    }
    auto fix = [&](void* field)
    {
        uint8_t* p;
        memcpy(&p, field, sizeof(p));
        if (p >= oldBase && p < oldBase + bytes.size())
        {
            p = newBase + (p - oldBase);
            memcpy(field, &p, sizeof(p));
        }
    };
    uint8_t* base = bytes.data();
    switch (e.type)
    {
        case net::Event::CreateMember:
        {
            DPMSG_CREATEPLAYERORGROUP* m = At<DPMSG_CREATEPLAYERORGROUP>(base);
            fix(&m->lpData);
            fix(&m->dpnName.lpszShortName);
            fix(&m->dpnName.lpszLongName);
            break;
        }
        case net::Event::DestroyMember:
        {
            DPMSG_DESTROYPLAYERORGROUP* m = At<DPMSG_DESTROYPLAYERORGROUP>(base);
            fix(&m->lpRemoteData);
            fix(&m->dpnName.lpszShortName);
            fix(&m->dpnName.lpszLongName);
            break;
        }
        case net::Event::SetData:
            fix(&At<DPMSG_SETPLAYERORGROUPDATA>(base)->lpData);
            break;
        case net::Event::Chat:
        {
            DPMSG_CHAT* m = At<DPMSG_CHAT>(base);
            DPCHAT* c = reinterpret_cast<DPCHAT*>(reinterpret_cast<uint8_t*>(m->lpChat) - oldBase + base);
            fix(&c->lpszMessage);
            fix(&m->lpChat);
            break;
        }
        default:
            break;
    }
}

void ToSessionDesc(const net::SessionInfo& info, DPSESSIONDESC2& desc, LPWSTR nameBuffer)
{
    memset(&desc, 0, sizeof(desc));
    desc.dwSize = sizeof(desc);
    desc.dwFlags = info.flags;
    memcpy(&desc.guidInstance, info.instance, 16);
    memcpy(&desc.guidApplication, info.application, 16);
    desc.dwMaxPlayers = info.maxPlayers;
    desc.dwCurrentPlayers = info.currentPlayers;
    desc.dwUser1 = info.user[0];
    desc.dwUser2 = info.user[1];
    desc.dwUser3 = info.user[2];
    desc.dwUser4 = info.user[3];
    desc.lpszSessionName = nameBuffer;
}

class PortDirectPlay : public IDirectPlay3
{
  public:
    virtual ~PortDirectPlay() = default;
    // IUnknown
    HRESULT QueryInterface(REFIID iid, void** out) override
    {
        if (iid == IID_IDirectPlay2 || iid == IID_IDirectPlay3)
        {
            AddRef();
            *out = static_cast<IDirectPlay3*>(this);
            return S_OK;
        }
        *out = nullptr;
        return E_NOINTERFACE;
    }
    ULONG AddRef() override
    {
        return ++mRefs;
    }
    ULONG Release() override
    {
        const ULONG refs = --mRefs;
        if (refs == 0)
        {
            delete this;
        }
        return refs;
    }

    // IDirectPlay2
    HRESULT AddPlayerToGroup(DPID group, DPID player) override
    {
        return gClient ? ToHresult(gClient->AddToGroup(group, player)) : DPERR_NOCONNECTION;
    }

    HRESULT Close() override
    {
        if (gClient)
        {
            gClient->Close();
        }
        mHeld.reset();
        return DP_OK;
    }

    HRESULT CreateGroup(LPDPID group, LPDPNAME name, LPVOID data, DWORD size, DWORD flags) override
    {
        if (!gClient || !group)
        {
            return DPERR_NOCONNECTION;
        }
        net::Id id = 0;
        const net::Result r = gClient->CreateGroup(name ? ToU16(name->lpszShortName) : u"",
                                                   name ? ToU16(name->lpszLongName) : u"",
                                                   Bytes(data, size),
                                                   flags,
                                                   id);
        *group = id;
        return r == net::kOk ? DP_OK : DPERR_CANTCREATEGROUP;
    }

    HRESULT CreatePlayer(LPDPID player, LPDPNAME name, HANDLE, LPVOID data, DWORD size, DWORD flags) override
    {
        if (!gClient || !player)
        {
            return DPERR_NOCONNECTION;
        }
        net::Id id = 0;
        const net::Result r = gClient->CreatePlayer(name ? ToU16(name->lpszShortName) : u"",
                                                    name ? ToU16(name->lpszLongName) : u"",
                                                    Bytes(data, size),
                                                    flags,
                                                    id);
        if (r != net::kOk)
        {
            SetError(std::string("can't join the game: ") + net::ResultText(r));
            return DPERR_CANTCREATEPLAYER;
        }
        *player = id;
        if (!gLocalPlayerForMessages)
        {
            gLocalPlayerForMessages = id;
        }
        Log("player " + ToUtf8(ToU16(name ? name->lpszShortName : nullptr)) + " is " + std::to_string(id));
        return DP_OK;
    }

    HRESULT DeletePlayerFromGroup(DPID group, DPID player) override
    {
        return gClient ? ToHresult(gClient->DeleteFromGroup(group, player)) : DPERR_NOCONNECTION;
    }

    HRESULT DestroyGroup(DPID group) override
    {
        return gClient ? ToHresult(gClient->DestroyGroup(group)) : DPERR_NOCONNECTION;
    }

    HRESULT DestroyPlayer(DPID player) override
    {
        if (player == gLocalPlayerForMessages)
        {
            gLocalPlayerForMessages = 0;
        }
        return gClient ? ToHresult(gClient->DestroyPlayer(player)) : DPERR_NOCONNECTION;
    }

    HRESULT EnumGroupPlayers(DPID group, LPGUID, LPDPENUMPLAYERSCALLBACK2 cb, LPVOID ctx, DWORD) override
    {
        net::Member g;
        if (!gClient || !gClient->GetMember(group, g) || !g.isGroup)
        {
            return DPERR_INVALIDGROUP;
        }
        for (net::Id id : g.members)
        {
            net::Member p;
            if (gClient->GetMember(id, p) && !EnumOne(p, cb, ctx))
            {
                break;
            }
        }
        return DP_OK;
    }

    HRESULT EnumGroups(LPGUID, LPDPENUMPLAYERSCALLBACK2 cb, LPVOID ctx, DWORD) override
    {
        if (!gClient)
        {
            return DPERR_NOCONNECTION;
        }
        for (const net::Member& g : gClient->Groups())
        {
            if (!EnumOne(g, cb, ctx))
            {
                break;
            }
        }
        return DP_OK;
    }

    HRESULT EnumPlayers(LPGUID, LPDPENUMPLAYERSCALLBACK2 cb, LPVOID ctx, DWORD) override
    {
        if (!gClient)
        {
            return DPERR_NOCONNECTION;
        }
        for (const net::Member& p : gClient->Players())
        {
            if (!EnumOne(p, cb, ctx))
            {
                break;
            }
        }
        return DP_OK;
    }

    HRESULT EnumSessions(LPDPSESSIONDESC2 desc, DWORD timeout, LPDPENUMSESSIONSCALLBACK2 cb, LPVOID ctx, DWORD flags) override
    {
        net::Client* client = Client();
        if (!client || !desc || !cb)
        {
            return DPERR_NOCONNECTION;
        }

        // Ask: a typed address directly, otherwise the LAN
        const std::string look = LookAddress();
        net::Address address;
        if (!look.empty())
        {
            std::string error;
            if (!net::Resolve(look, PortNet().port, address, error))
            {
                SetError(error);
                return DPERR_NOCONNECTION;
            }
            if (!client->CanReach(address))
            {
                SetError(look + " is an IPv6 address, and this system has no IPv6");
                return DPERR_NOCONNECTION;
            }
        }
        uint8_t application[16];
        memcpy(application, &desc->guidApplication, 16);
        client->Discover(application, look.empty(), PortNet().port, look.empty() ? nullptr : &address);

        if (!(flags & DPENUMSESSIONS_ASYNC))
        {
            // Synchronous: wait for the answers
            const uint32_t wait = timeout ? timeout : 1000;
            const uint32_t start = net::NowMs();
            while (net::NowMs() - start < wait)
            {
                Sleep(20);
            }
        }

        gSessions.clear();
        for (const net::SessionInfo& s : client->Sessions())
        {
            if (memcmp(s.application, application, 16) != 0)
            {
                continue;
            }
            const bool full = s.maxPlayers && s.currentPlayers >= s.maxPlayers;
            if (!(flags & DPENUMSESSIONS_ALL) && ((s.flags & DPSESSION_JOINDISABLED) || full))
            {
                continue;
            }
            gSessions[net::GuidKey(s.instance)] = s;
            // A game nobody here can join still shows, saying why
            std::u16string name = s.name;
            if (s.compat != PortNetCompat())
            {
                const std::string note = " (other version: " + s.build + ")";
                name += std::u16string(note.begin(), note.end());
            }
            name.push_back(0);
            DPSESSIONDESC2 found;
            ToSessionDesc(s, found, reinterpret_cast<LPWSTR>(&name[0]));
            DWORD t = timeout;
            if (!cb(&found, &t, 0, ctx))
            {
                return DP_OK;
            }
        }
        DWORD t = timeout;
        cb(nullptr, &t, DPESC_TIMEDOUT, ctx);
        return DP_OK;
    }

    HRESULT GetCaps(LPVOID, DWORD) override
    {
        return DPERR_UNSUPPORTED;
    }

    HRESULT GetGroupData(DPID group, LPVOID data, LPDWORD size, DWORD) override
    {
        return GetData(group, data, size, true);
    }

    HRESULT GetGroupName(DPID group, LPVOID data, LPDWORD size) override
    {
        return GetName(group, data, size);
    }

    HRESULT GetMessageCount(DPID, LPDWORD count) override
    {
        *count = static_cast<DWORD>((gClient ? gClient->EventCount() : 0) + (mHeld ? 1 : 0));
        return DP_OK;
    }

    HRESULT GetPlayerAddress(DPID, LPVOID, LPDWORD) override
    {
        return DPERR_UNAVAILABLE;
    }

    HRESULT GetPlayerCaps(DPID, LPVOID, DWORD) override
    {
        return DPERR_UNSUPPORTED;
    }

    HRESULT GetPlayerData(DPID player, LPVOID data, LPDWORD size, DWORD) override
    {
        return GetData(player, data, size, false);
    }

    HRESULT GetPlayerName(DPID player, LPVOID data, LPDWORD size) override
    {
        return GetName(player, data, size);
    }

    HRESULT GetSessionDesc(LPVOID data, LPDWORD size) override
    {
        if (!gClient || !gClient->InSession())
        {
            return DPERR_NOCONNECTION;
        }
        const net::SessionInfo s = gClient->Session();
        const DWORD need = static_cast<DWORD>(sizeof(DPSESSIONDESC2) + (s.name.size() + 1) * 2);
        if (!data || *size < need)
        {
            *size = need;
            return DPERR_BUFFERTOOSMALL;
        }
        uint8_t* base = static_cast<uint8_t*>(data);
        LPWSTR name = reinterpret_cast<LPWSTR>(base + sizeof(DPSESSIONDESC2));
        memcpy(name, s.name.c_str(), (s.name.size() + 1) * 2);
        ToSessionDesc(s, *reinterpret_cast<DPSESSIONDESC2*>(base), name);
        *size = need;
        return DP_OK;
    }

    HRESULT Initialize(LPGUID) override
    {
        return DPERR_ALREADYINITIALIZED;
    }

    HRESULT Open(LPDPSESSIONDESC2 desc, DWORD flags) override
    {
        net::Client* client = Client();
        if (!client || !desc)
        {
            return DPERR_NOCONNECTION;
        }
        SetError("");
        mHeld.reset();
        gLocalPlayerForMessages = 0;
        std::string error;

        if (flags & DPOPEN_CREATE)
        {
            net::SessionInfo info;
            memcpy(info.application, &desc->guidApplication, 16);
            info.flags = desc->dwFlags;
            info.maxPlayers = desc->dwMaxPlayers;
            info.user[0] = desc->dwUser1;
            info.user[1] = desc->dwUser2;
            info.user[2] = desc->dwUser3;
            info.user[3] = desc->dwUser4;
            info.name = ToU16(desc->lpszSessionName);

            net::Address server;
            if (!PortNet().hostServer.empty())
            {
                if (!net::Resolve(PortNet().hostServer, PortNet().port, server, error))
                {
                    SetError(error);
                    return DPERR_CANTCREATESESSION;
                }
            }
            else
            {
                // A server inside this game, for the others to reach
                if (!gServer)
                {
                    gServer = new net::Server();
                    if (!gServer->Start(PortNet().port, PortNet().verbose, error))
                    {
                        delete gServer;
                        gServer = nullptr;
                        SetError("can't host a game: " + error + " (is another game hosting on this computer?)");
                        return DPERR_CANTCREATESESSION;
                    }
                }
                server = net::Address::Loopback(gServer->Port());
            }
            const net::Result r = client->Create(server, info, ToU16(desc->lpszPassword), error);
            if (r != net::kOk)
            {
                SetError("can't host a game on " + server.ToString() + ": " + error);
                return DPERR_CANTCREATESESSION;
            }
            const net::SessionInfo created = client->Session();
            memcpy(&desc->guidInstance, created.instance, 16);
            Log("hosting \"" + ToUtf8(created.name) + "\" on " + server.ToString());
            return DP_OK;
        }

        if (flags & DPOPEN_JOIN)
        {
            auto it = gSessions.find(net::GuidKey(reinterpret_cast<const uint8_t*>(&desc->guidInstance)));
            if (it == gSessions.end())
            {
                SetError("that game is no longer there");
                return DPERR_NOSESSIONS;
            }
            const net::Result r = client->Join(it->second, ToU16(desc->lpszPassword), error);
            if (r != net::kOk)
            {
                SetError("can't join \"" + ToUtf8(it->second.name) + "\": " + error);
                return ToHresult(r);
            }
            Log("joined \"" + ToUtf8(it->second.name) + "\" on " + it->second.server.ToString());
            return DP_OK;
        }
        return DPERR_INVALIDPARAMS;
    }

    HRESULT Receive(LPDPID from, LPDPID to, DWORD, LPVOID data, LPDWORD size) override
    {
        if (!gClient)
        {
            return DPERR_NOMESSAGES;
        }
        if (!mHeld)
        {
            net::Event e;
            if (!gClient->PopEvent(e))
            {
                return DPERR_NOMESSAGES;
            }
            mHeld.reset(new net::Event(std::move(e)));
        }
        DPID f;
        DPID t;
        std::vector<uint8_t> bytes;
        Encode(*mHeld, f, t, bytes);
        if (!data || *size < bytes.size())
        {
            // DirectPlay keeps the message for a call with a bigger buffer
            *size = static_cast<DWORD>(bytes.size());
            return DPERR_BUFFERTOOSMALL;
        }
        uint8_t* oldBase = bytes.data();
        Rebase(*mHeld, bytes, oldBase, static_cast<uint8_t*>(data));
        memcpy(data, bytes.data(), bytes.size());
        *size = static_cast<DWORD>(bytes.size());
        if (from)
        {
            *from = f;
        }
        if (to)
        {
            *to = t;
        }
        mHeld.reset();
        return DP_OK;
    }

    HRESULT Send(DPID from, DPID to, DWORD flags, LPVOID data, DWORD size) override
    {
        if (!gClient)
        {
            return DPERR_NOCONNECTION;
        }
        // Every message goes reliably and in order, guaranteed or not: a lost input would stall
        // everyone for the game's resend timeout
        const net::Result r = gClient->Send(from, to, flags, data, size);
        return r == net::kErrGeneric ? DPERR_SENDTOOBIG : ToHresult(r);
    }

    HRESULT SetGroupData(DPID group, LPVOID data, DWORD size, DWORD) override
    {
        return gClient ? ToHresult(gClient->SetData(group, Bytes(data, size))) : DPERR_NOCONNECTION;
    }

    HRESULT SetGroupName(DPID, LPDPNAME, DWORD) override
    {
        return DP_OK;
    }

    HRESULT SetPlayerData(DPID player, LPVOID data, DWORD size, DWORD) override
    {
        return gClient ? ToHresult(gClient->SetData(player, Bytes(data, size))) : DPERR_NOCONNECTION;
    }

    HRESULT SetPlayerName(DPID, LPDPNAME, DWORD) override
    {
        return DP_OK;
    }

    HRESULT SetSessionDesc(LPDPSESSIONDESC2 desc, DWORD) override
    {
        if (!gClient || !desc)
        {
            return DPERR_NOCONNECTION;
        }
        net::SessionInfo info = gClient->Session();
        info.flags = desc->dwFlags;
        info.maxPlayers = desc->dwMaxPlayers;
        info.user[0] = desc->dwUser1;
        info.user[1] = desc->dwUser2;
        info.user[2] = desc->dwUser3;
        info.user[3] = desc->dwUser4;
        return ToHresult(gClient->SetSessionDesc(info));
    }

    // IDirectPlay3
    HRESULT AddGroupToGroup(DPID, DPID) override
    {
        return DPERR_UNSUPPORTED;
    }
    HRESULT CreateGroupInGroup(DPID, LPDPID, LPDPNAME, LPVOID, DWORD, DWORD) override
    {
        return DPERR_UNSUPPORTED;
    }
    HRESULT DeleteGroupFromGroup(DPID, DPID) override
    {
        return DPERR_UNSUPPORTED;
    }

    HRESULT EnumConnections(LPCGUID, LPDPENUMCONNECTIONSCALLBACK cb, LPVOID ctx, DWORD) override
    {
        // One kind of connection, offered under both names the game knows (it picks IPX when no
        // protocol was chosen)
        const GUID* providers[] = {&DPSPGUID_TCPIP, &DPSPGUID_IPX};
        const wchar_t* names[] = {L"Internet TCP/IP Connection For DirectPlay", L"IPX Connection For DirectPlay"};
        for (int i = 0; i < 2; i++)
        {
            std::vector<uint8_t> address = ServiceProviderAddress(*providers[i]);
            DPNAME name;
            memset(&name, 0, sizeof(name));
            name.dwSize = sizeof(name);
            name.lpszShortName = const_cast<LPWSTR>(names[i]);
            if (!cb(providers[i], address.data(), static_cast<DWORD>(address.size()), &name, DPCONNECTION_DIRECTPLAY, ctx))
            {
                break;
            }
        }
        return DP_OK;
    }

    HRESULT EnumGroupsInGroup(DPID, LPGUID, LPDPENUMPLAYERSCALLBACK2, LPVOID, DWORD) override
    {
        return DP_OK;
    }
    HRESULT GetGroupConnectionSettings(DWORD, DPID, LPVOID, LPDWORD) override
    {
        return DPERR_UNSUPPORTED;
    }

    HRESULT InitializeConnection(LPVOID connection, DWORD) override;

    HRESULT SecureOpen(LPCDPSESSIONDESC2 desc, DWORD flags, LPVOID, LPVOID) override
    {
        return Open(const_cast<LPDPSESSIONDESC2>(desc), flags);
    }

    HRESULT SendChatMessage(DPID from, DPID to, DWORD, LPDPCHAT chat) override
    {
        if (!gClient || !chat)
        {
            return DPERR_NOCONNECTION;
        }
        return ToHresult(gClient->Chat(from, to, ToU16(chat->lpszMessage)));
    }

    HRESULT SetGroupConnectionSettings(DWORD, DPID, LPVOID) override
    {
        return DPERR_UNSUPPORTED;
    }
    HRESULT StartSession(DWORD, DPID) override
    {
        return DPERR_UNSUPPORTED;
    }
    HRESULT GetGroupFlags(DPID group, LPDWORD flags) override
    {
        net::Member m;
        if (!gClient || !gClient->GetMember(group, m))
        {
            return DPERR_INVALIDGROUP;
        }
        *flags = m.flags;
        return DP_OK;
    }
    HRESULT GetGroupParent(DPID, LPDPID parent) override
    {
        *parent = 0;
        return DP_OK;
    }
    HRESULT GetPlayerAccount(DPID, DWORD, LPVOID, LPDWORD) override
    {
        return DPERR_UNSUPPORTED;
    }
    HRESULT GetPlayerFlags(DPID player, LPDWORD flags) override
    {
        net::Member m;
        if (!gClient || !gClient->GetMember(player, m))
        {
            return DPERR_INVALIDPLAYER;
        }
        *flags = m.flags;
        return DP_OK;
    }

    static std::vector<uint8_t> ServiceProviderAddress(const GUID& sp);

  private:
    static std::vector<uint8_t> Bytes(LPVOID data, DWORD size)
    {
        const uint8_t* p = static_cast<const uint8_t*>(data);
        return p && size ? std::vector<uint8_t>(p, p + size) : std::vector<uint8_t>();
    }

    static bool EnumOne(const net::Member& m, LPDPENUMPLAYERSCALLBACK2 cb, LPVOID ctx)
    {
        std::u16string shortName = m.shortName;
        std::u16string longName = m.longName;
        shortName.push_back(0);
        longName.push_back(0);
        DPNAME name;
        memset(&name, 0, sizeof(name));
        name.dwSize = sizeof(name);
        name.lpszShortName = reinterpret_cast<LPWSTR>(&shortName[0]);
        name.lpszLongName = longName.size() > 1 ? reinterpret_cast<LPWSTR>(&longName[0]) : nullptr;
        return cb(m.id, m.isGroup ? DPPLAYERTYPE_GROUP : DPPLAYERTYPE_PLAYER, &name, 0, ctx) != FALSE;
    }

    HRESULT GetData(DPID id, LPVOID data, LPDWORD size, bool group)
    {
        net::Member m;
        if (!gClient || !gClient->GetMember(id, m) || m.isGroup != group)
        {
            return group ? DPERR_INVALIDGROUP : DPERR_INVALIDPLAYER;
        }
        const DWORD need = static_cast<DWORD>(m.data.size());
        if (need && (!data || *size < need))
        {
            *size = need;
            return DPERR_BUFFERTOOSMALL;
        }
        if (need)
        {
            memcpy(data, m.data.data(), need);
        }
        *size = need;
        return DP_OK;
    }

    HRESULT GetName(DPID id, LPVOID data, LPDWORD size)
    {
        net::Member m;
        if (!gClient || !gClient->GetMember(id, m))
        {
            return DPERR_INVALIDPLAYER;
        }
        const size_t shortBytes = (m.shortName.size() + 1) * 2;
        const size_t longBytes = (m.longName.size() + 1) * 2;
        const DWORD need = static_cast<DWORD>(sizeof(DPNAME) + shortBytes + longBytes);
        if (!data || *size < need)
        {
            *size = need;
            return DPERR_BUFFERTOOSMALL;
        }
        uint8_t* base = static_cast<uint8_t*>(data);
        DPNAME* name = reinterpret_cast<DPNAME*>(base);
        memset(name, 0, sizeof(DPNAME));
        name->dwSize = sizeof(DPNAME);
        name->lpszShortName = reinterpret_cast<LPWSTR>(base + sizeof(DPNAME));
        name->lpszLongName = reinterpret_cast<LPWSTR>(base + sizeof(DPNAME) + shortBytes);
        memcpy(name->lpszShortName, m.shortName.c_str(), shortBytes);
        memcpy(name->lpszLongName, m.longName.c_str(), longBytes);
        *size = need;
        return DP_OK;
    }

    ULONG mRefs = 1;
    std::unique_ptr<net::Event> mHeld; // a message the caller's buffer was too small for
};

// DirectPlay addresses: a list of {GUID type, DWORD size, data} elements, the first giving the total
// size, as DirectPlayLobby builds them
class AddressBuilder
{
  public:
    void Add(const GUID& type, const void* data, DWORD size)
    {
        mElements.push_back(
            Element {type, std::vector<uint8_t>(static_cast<const uint8_t*>(data), static_cast<const uint8_t*>(data) + size)});
    }
    std::vector<uint8_t> Build() const
    {
        DWORD total = sizeof(GUID) + sizeof(DWORD) + sizeof(DWORD);
        for (const Element& e : mElements)
        {
            total += sizeof(GUID) + sizeof(DWORD) + static_cast<DWORD>(e.data.size());
        }
        std::vector<uint8_t> out;
        auto put = [&](const void* p, size_t n)
        {
            const uint8_t* b = static_cast<const uint8_t*>(p);
            out.insert(out.end(), b, b + n);
        };
        const DWORD four = sizeof(DWORD);
        put(&DPAID_TotalSize, sizeof(GUID));
        put(&four, sizeof(DWORD));
        put(&total, sizeof(DWORD));
        for (const Element& e : mElements)
        {
            const DWORD size = static_cast<DWORD>(e.data.size());
            put(&e.type, sizeof(GUID));
            put(&size, sizeof(DWORD));
            put(e.data.data(), e.data.size());
        }
        return out;
    }

  private:
    struct Element
    {
        GUID type;
        std::vector<uint8_t> data;
    };
    std::vector<Element> mElements;
};

// Calls cb for every element of an address. Returns false when it is malformed.
template<typename F>
bool ForEachElement(const void* address, DWORD size, F cb)
{
    const uint8_t* p = static_cast<const uint8_t*>(address);
    DWORD at = 0;
    while (at + sizeof(GUID) + sizeof(DWORD) <= size)
    {
        GUID type;
        DWORD length;
        memcpy(&type, p + at, sizeof(GUID));
        memcpy(&length, p + at + sizeof(GUID), sizeof(DWORD));
        at += sizeof(GUID) + sizeof(DWORD);
        if (length > size - at)
        {
            return false;
        }
        if (!cb(type, p + at, length))
        {
            return true;
        }
        at += length;
    }
    return at == size;
}

std::vector<uint8_t> PortDirectPlay::ServiceProviderAddress(const GUID& sp)
{
    AddressBuilder b;
    b.Add(DPAID_ServiceProvider, &sp, sizeof(GUID));
    return b.Build();
}

HRESULT PortDirectPlay::InitializeConnection(LPVOID connection, DWORD)
{
    // The connection may carry an address to look for games at (the game's registry
    // "UseConnection", made with CreateCompoundAddress)
    std::string address;
    DWORD size = 0;
    if (connection)
    {
        // The total size element comes first
        const uint8_t* p = static_cast<const uint8_t*>(connection);
        GUID first;
        memcpy(&first, p, sizeof(GUID));
        if (first == DPAID_TotalSize)
        {
            memcpy(&size, p + sizeof(GUID) + sizeof(DWORD), sizeof(DWORD));
        }
    }
    if (size)
    {
        ForEachElement(connection,
                       size,
                       [&](const GUID& type, const uint8_t* data, DWORD length)
                       {
                           if (type == DPAID_INet)
                           {
                               address.assign(reinterpret_cast<const char*>(data), strnlen(reinterpret_cast<const char*>(data), length));
                           }
                           else if (type == DPAID_INetW)
                           {
                               const wchar_t* w = reinterpret_cast<const wchar_t*>(data);
                               address.clear();
                               for (DWORD i = 0; i < length / 2 && w[i]; i++)
                               {
                                   address += static_cast<char>(w[i] < 0x80 ? w[i] : '?');
                               }
                           }
                           return true;
                       });
    }
    gConnectionAddress = address;
    Log(address.empty() ? std::string("looking for games on the LAN") : "looking for games at " + address);
    return Client() ? DP_OK : DPERR_UNAVAILABLE;
}

// DirectPlayCreate's object (the DirectPlay 1 interface): only good for asking for IDirectPlay3
class PortDirectPlay1 : public IDirectPlay
{
  public:
    virtual ~PortDirectPlay1() = default;
    HRESULT QueryInterface(REFIID iid, void** out) override
    {
        if (iid == IID_IDirectPlay2 || iid == IID_IDirectPlay3)
        {
            *out = static_cast<IDirectPlay3*>(new PortDirectPlay());
            return S_OK;
        }
        *out = nullptr;
        return E_NOINTERFACE;
    }
    ULONG AddRef() override
    {
        return ++mRefs;
    }
    ULONG Release() override
    {
        const ULONG refs = --mRefs;
        if (!refs)
        {
            delete this;
        }
        return refs;
    }

  private:
    ULONG mRefs = 1;
};

class PortDirectPlayLobby : public IDirectPlayLobby2
{
  public:
    virtual ~PortDirectPlayLobby() = default;
    HRESULT QueryInterface(REFIID iid, void** out) override
    {
        if (iid == IID_IDirectPlayLobby || iid == IID_IDirectPlayLobby2)
        {
            AddRef();
            *out = static_cast<IDirectPlayLobby2*>(this);
            return S_OK;
        }
        *out = nullptr;
        return E_NOINTERFACE;
    }
    ULONG AddRef() override
    {
        return ++mRefs;
    }
    ULONG Release() override
    {
        const ULONG refs = --mRefs;
        if (!refs)
        {
            delete this;
        }
        return refs;
    }

    HRESULT Connect(DWORD, LPDIRECTPLAY2*, IUnknown*) override
    {
        return DPERR_UNSUPPORTED;
    }

    HRESULT CreateAddress(REFGUID sp, REFGUID type, LPCVOID data, DWORD size, LPVOID address, LPDWORD addressSize) override
    {
        AddressBuilder b;
        b.Add(DPAID_ServiceProvider, &sp, sizeof(GUID));
        b.Add(type, data, size);
        return Output(b.Build(), address, addressSize);
    }

    HRESULT EnumAddress(LPDPENUMADDRESSCALLBACK cb, LPCVOID address, DWORD size, LPVOID ctx) override
    {
        const bool ok = ForEachElement(address,
                                       size,
                                       [&](const GUID& type, const uint8_t* data, DWORD length)
                                       {
                                           return cb(type, length, data, ctx) != FALSE;
                                       });
        return ok ? DP_OK : DPERR_INVALIDPARAMS;
    }

    HRESULT EnumAddressTypes(LPVOID, REFGUID, LPVOID, DWORD) override
    {
        return DPERR_UNSUPPORTED;
    }
    HRESULT EnumLocalApplications(LPVOID, LPVOID, DWORD) override
    {
        return DPERR_UNSUPPORTED;
    }
    HRESULT GetConnectionSettings(DWORD, LPVOID, LPDWORD) override
    {
        return DPERR_UNSUPPORTED;
    }
    HRESULT ReceiveLobbyMessage(DWORD, DWORD, LPDWORD, LPVOID, LPDWORD) override
    {
        return DPERR_NOMESSAGES;
    }
    HRESULT RunApplication(DWORD, LPDWORD, LPVOID, HANDLE) override
    {
        return DPERR_UNSUPPORTED;
    }
    HRESULT SendLobbyMessage(DWORD, DWORD, LPVOID, DWORD) override
    {
        return DPERR_UNSUPPORTED;
    }
    HRESULT SetConnectionSettings(DWORD, DWORD, LPVOID) override
    {
        return DPERR_UNSUPPORTED;
    }
    HRESULT SetLobbyMessageEvent(DWORD, DWORD, HANDLE) override
    {
        return DPERR_UNSUPPORTED;
    }

    HRESULT CreateCompoundAddress(LPCDPCOMPOUNDADDRESSELEMENT elements, DWORD count, LPVOID address, LPDWORD size) override
    {
        AddressBuilder b;
        for (DWORD i = 0; i < count; i++)
        {
            b.Add(elements[i].guidDataType, elements[i].lpData, elements[i].dwDataSize);
        }
        return Output(b.Build(), address, size);
    }

  private:
    static HRESULT Output(const std::vector<uint8_t>& bytes, LPVOID address, LPDWORD size)
    {
        if (!address || *size < bytes.size())
        {
            *size = static_cast<DWORD>(bytes.size());
            return DPERR_BUFFERTOOSMALL;
        }
        memcpy(address, bytes.data(), bytes.size());
        *size = static_cast<DWORD>(bytes.size());
        return DP_OK;
    }

    ULONG mRefs = 1;
};
} // namespace

PortNetConfig& PortNet()
{
    static PortNetConfig config;
    return config;
}

const char* PortNetCompat()
{
    // The network protocol and the game code. Two builds of the same sources agree on everything the
    // simulation computes, whatever system they were built for: the game's own maths is fixed-point,
    // rand() is the port's (crt.cpp) and floating point is IEEE double everywhere (CMakeLists.txt).
    // GTA2_NET_TEST_COMPAT stands in for another build in the tests
    const char* test = getenv("GTA2_NET_TEST_COMPAT");
    static const std::string compat =
        "gta2-native/" + std::to_string(net::kProtocolVersion) + "/" + (test && *test ? std::string(test) : std::string(GTA2_GAME_HASH));
    return compat.c_str();
}

const char* PortNetBuild()
{
    static const std::string build = BuildDescription();
    return build.c_str();
}

std::string PortNetDescribe()
{
    return (gClient ? "  client " + gClient->Describe() + "\n" : std::string()) + (gServer ? gServer->Describe() : std::string());
}

bool PortNetSessionLost()
{
    return gClient && gClient->SessionLost();
}

std::string PortNetLastError()
{
    return gLastError;
}

long PortDirectPlayCreate(const GUID& iid, void** out)
{
    PortDirectPlay* dp = new PortDirectPlay();
    const HRESULT hr = dp->QueryInterface(iid, out);
    dp->Release();
    return hr;
}

extern "C" HRESULT DirectPlayCreate(LPGUID, LPDIRECTPLAY* out, IUnknown*)
{
    *out = new PortDirectPlay1();
    return DP_OK;
}

extern "C" HRESULT DirectPlayLobbyCreateW(LPGUID, LPDIRECTPLAYLOBBY* out, IUnknown*, LPVOID, DWORD)
{
    *out = new PortDirectPlayLobby();
    return DP_OK;
}

extern "C" HRESULT DirectPlayLobbyCreateA(LPGUID sp, LPDIRECTPLAYLOBBY* out, IUnknown* outer, LPVOID data, DWORD size)
{
    return DirectPlayLobbyCreateW(sp, out, outer, data, size);
}

// Ends the session cleanly when the game exits, so the others hear it at once
struct NetShutdown
{
    ~NetShutdown()
    {
        delete gClient;
        gClient = nullptr;
        delete gServer;
        gServer = nullptr;
    }
} gNetShutdown;
