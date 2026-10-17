// The session server: see net.h
#include "net.h"

#include <stdio.h>

#include <algorithm>
#include <random>

namespace net
{
const char* ResultText(Result r)
{
    switch (r)
    {
        case kOk:
            return "ok";
        case kErrNoSession:
            return "the game is gone or has already started";
        case kErrFull:
            return "the game is full";
        case kErrIncompatible:
            return "the game was started by a different version of GTA2 (everyone needs the same build)";
        case kErrPassword:
            return "wrong password";
        case kErrInvalidPlayer:
            return "no such player";
        case kErrInvalidGroup:
            return "no such group";
        case kErrTimeout:
            return "no answer";
        case kErrNotConnected:
            return "not connected";
        default:
            return "failed";
    }
}

std::string GuidKey(const uint8_t guid[16])
{
    static const char hex[] = "0123456789abcdef";
    std::string s;
    for (int i = 0; i < 16; i++)
    {
        s += hex[guid[i] >> 4];
        s += hex[guid[i] & 15];
    }
    return s;
}

void SessionInfo::Write(Writer& w) const
{
    w.Guid(instance);
    w.Guid(application);
    w.U32(flags);
    w.U32(maxPlayers);
    w.U32(currentPlayers);
    for (uint32_t u : user)
    {
        w.U32(u);
    }
    w.Text(name);
    w.U8(hasPassword);
    w.Str(compat);
    w.Str(build);
}

void SessionInfo::Read(Reader& r)
{
    r.Guid(instance);
    r.Guid(application);
    flags = r.U32();
    maxPlayers = r.U32();
    currentPlayers = r.U32();
    for (uint32_t& u : user)
    {
        u = r.U32();
    }
    name = r.Text();
    hasPassword = r.U8() != 0;
    compat = r.Str();
    build = r.Str();
}

void WriteMember(Writer& w, const Member& m)
{
    w.U32(m.id);
    w.U8(m.isGroup);
    w.Text(m.shortName);
    w.Text(m.longName);
    w.Blob(m.data);
    w.U32(m.flags);
    w.U16(static_cast<uint16_t>(m.members.size()));
    for (Id id : m.members)
    {
        w.U32(id);
    }
}

void ReadMember(Reader& r, Member& m)
{
    m.id = r.U32();
    m.isGroup = r.U8() != 0;
    m.shortName = r.Text();
    m.longName = r.Text();
    m.data = r.Blob();
    m.flags = r.U32();
    const uint16_t count = r.U16();
    m.members.clear();
    for (uint16_t i = 0; i < count && !r.Failed(); i++)
    {
        m.members.push_back(r.U32());
    }
}

namespace
{
std::string ToUtf8(const std::u16string& s)
{
    std::string out;
    for (char16_t c : s)
    {
        if (c < 0x80)
        {
            out += static_cast<char>(c);
        }
        else if (c < 0x800)
        {
            out += static_cast<char>(0xC0 | (c >> 6));
            out += static_cast<char>(0x80 | (c & 0x3F));
        }
        else
        {
            out += static_cast<char>(0xE0 | (c >> 12));
            out += static_cast<char>(0x80 | ((c >> 6) & 0x3F));
            out += static_cast<char>(0x80 | (c & 0x3F));
        }
    }
    return out;
}
} // namespace

Server::~Server()
{
    Stop();
}

bool Server::Start(uint16_t port, bool verbose, std::string& error)
{
    mVerbose = verbose;
    if (!mSocket.Open(port, error))
    {
        return false;
    }
    std::random_device rd;
    mNextId = (rd() & 0x0FFFFFFF) | 0x10000000;
    mStop = false;
    mThread = std::thread(&Server::Run, this);
    Log("listening on UDP port " + std::to_string(mSocket.LocalPort()));
    return true;
}

void Server::Stop()
{
    if (!mThread.joinable())
    {
        return;
    }
    {
        std::lock_guard<std::mutex> lock(mMutex);
        mStop = true;
        // Tell everyone, so their games end at once instead of after a timeout
        for (auto& kv : mPeers)
        {
            kv.second->link.SendDisconnect();
        }
    }
    mThread.join();
    mPeers.clear();
    mSessions.clear();
    mSocket.Close();
}

std::string Server::Describe()
{
    std::lock_guard<std::mutex> lock(mMutex);
    std::string s;
    const uint32_t now = NowMs();
    for (auto& kv : mPeers)
    {
        s += "  server " + kv.second->link.Describe(now) + "\n";
    }
    return s;
}

void Server::Log(const std::string& text)
{
    if (mVerbose)
    {
        fprintf(stderr, "gta2-server: %s\n", text.c_str());
        fflush(stderr);
    }
}

Id Server::NewId()
{
    // Never 0 (everyone/system), nor the values the game uses as "none"
    do
    {
        mNextId++;
    } while (mNextId == 0 || mNextId == 0xEEEEEEEE || mNextId == 0xFFFFFFFF);
    return mNextId;
}

void Server::Run()
{
    std::vector<uint8_t> buffer(2048);
    for (;;)
    {
        Address from;
        const int got = mSocket.Receive(buffer.data(), buffer.size(), from, 5);
        std::lock_guard<std::mutex> lock(mMutex);
        if (mStop)
        {
            return;
        }
        const uint32_t now = NowMs();
        if (got > 0)
        {
            OnPacket(from, buffer.data(), got, now);
        }

        for (auto it = mPeers.begin(); it != mPeers.end();)
        {
            Peer& peer = *it->second;
            std::vector<uint8_t> message;
            while (peer.link.PopMessage(message))
            {
                OnMessage(peer, message);
            }
            if (peer.link.TimedOut(now, PeerTimeoutMs()))
            {
                Log(peer.link.Peer().ToString() + " timed out");
                LeaveSession(peer);
                it = mPeers.erase(it);
                continue;
            }
            peer.link.Update(now);
            ++it;
        }
    }
}

void Server::OnPacket(const Address& from, const uint8_t* data, size_t size, uint32_t now)
{
    Reader r(data, size);
    uint8_t version;
    uint8_t type;
    if (!ReadPacketHeader(r, version, type))
    {
        return;
    }
    if (version != kProtocolVersion)
    {
        if (type == kPacketConnect)
        {
            const uint32_t nonce = r.U32();
            Writer w;
            BeginPacket(w, kPacketRefused);
            w.U32(nonce);
            w.Str("the server runs a different version of the network protocol (" + std::to_string(kProtocolVersion) + ", yours is " +
                  std::to_string(version) + ")");
            SendRaw(mSocket, from, w.Data());
        }
        return;
    }

    switch (type)
    {
        case kPacketDiscover:
        {
            const uint32_t nonce = r.U32();
            uint8_t application[16];
            r.Guid(application);
            if (r.Failed())
            {
                return;
            }
            for (auto& kv : mSessions)
            {
                Session& s = *kv.second;
                if (memcmp(s.info.application, application, 16) != 0)
                {
                    continue;
                }
                // The game sets a new session up in a few calls (player, group, adding to it); a
                // player joining in between would find no group to join. Shown once that is done.
                bool hasGroup = false;
                for (auto& m : s.members)
                {
                    hasGroup = hasGroup || (m.second.isGroup && !m.second.members.empty());
                }
                if (!hasGroup && now - s.created < 2000)
                {
                    continue;
                }
                Writer w;
                BeginPacket(w, kPacketDiscoverReply);
                w.U32(nonce);
                s.info.currentPlayers = PlayerCount(s);
                s.info.Write(w);
                SendRaw(mSocket, from, w.Data());
            }
            break;
        }

        case kPacketConnect:
        {
            const uint32_t nonce = r.U32();
            const std::string compat = r.Str();
            const std::string build = r.Str();
            if (r.Failed())
            {
                return;
            }
            auto it = mPeers.find(from);
            if (it == mPeers.end())
            {
                std::random_device rd;
                uint32_t id;
                do
                {
                    id = rd();
                } while (id == 0);
                it = mPeers.emplace(from, std::unique_ptr<Peer>(new Peer(mSocket, from, id))).first;
                it->second->compat = compat;
                it->second->build = build;
                Log(from.ToString() + " connected (" + build + ")");
            }
            // Answer repeats too: our first answer may have been lost
            Writer w;
            BeginPacket(w, kPacketConnected);
            w.U32(nonce);
            w.U32(it->second->link.Id());
            SendRaw(mSocket, from, w.Data());
            it->second->link.OnAnyPacket(now);
            break;
        }

        case kPacketData:
        case kPacketAck:
        case kPacketKeepAlive:
        case kPacketDisconnect:
        {
            const uint32_t id = r.U32();
            auto it = mPeers.find(from);
            if (it == mPeers.end() || it->second->link.Id() != id || r.Failed())
            {
                return;
            }
            Peer& peer = *it->second;
            peer.link.OnAnyPacket(now);
            if (type == kPacketData)
            {
                peer.link.OnData(r);
            }
            else if (type == kPacketAck)
            {
                peer.link.OnAck(r);
            }
            else if (type == kPacketDisconnect)
            {
                Log(from.ToString() + " disconnected");
                std::vector<uint8_t> message;
                while (peer.link.PopMessage(message))
                {
                    OnMessage(peer, message);
                }
                LeaveSession(peer);
                mPeers.erase(it);
            }
            break;
        }
    }
}

void Server::Reply(Peer& peer, uint32_t request, Result result, const Writer* payload)
{
    Writer w;
    w.U8(kMsgResult);
    w.U32(request);
    w.U32(result);
    if (payload)
    {
        w.Bytes(payload->Data().data(), payload->Size());
    }
    peer.link.Send(w.Data());
}

void Server::Broadcast(Session& session, const Writer& message, Peer* except)
{
    for (auto& kv : mPeers)
    {
        Peer& p = *kv.second;
        if (p.session == &session && &p != except)
        {
            p.link.Send(message.Data());
        }
    }
}

uint32_t Server::PlayerCount(const Session& session) const
{
    uint32_t n = 0;
    for (const auto& kv : session.members)
    {
        n += !kv.second.isGroup;
    }
    return n;
}

void Server::DestroyMember(Session& session, Id id)
{
    auto it = session.members.find(id);
    if (it == session.members.end())
    {
        return;
    }
    if (!it->second.isGroup)
    {
        // DirectPlay first takes a leaving player out of its groups, telling everyone
        for (Id groupId : session.order)
        {
            Member& g = session.members[groupId];
            if (!g.isGroup)
            {
                continue;
            }
            auto m = std::find(g.members.begin(), g.members.end(), id);
            if (m != g.members.end())
            {
                g.members.erase(m);
                Writer w;
                w.U8(kEvDeleteFromGroup);
                w.U32(groupId);
                w.U32(id);
                Broadcast(session, w, nullptr);
            }
        }
    }
    Writer w;
    w.U8(kEvDestroy);
    w.U32(id);
    Broadcast(session, w, nullptr);
    session.members.erase(it);
    session.owners.erase(id);
    session.order.erase(std::remove(session.order.begin(), session.order.end(), id), session.order.end());
}

void Server::LeaveSession(Peer& peer)
{
    Session* session = peer.session;
    if (!session)
    {
        return;
    }
    peer.session = nullptr;

    if (session->creator == &peer)
    {
        // The game's host left: the game can't go on (DirectPlay without host migration does the same)
        Log("session \"" + ToUtf8(session->info.name) + "\" ended: its host left");
        Writer w;
        w.U8(kEvSessionLost);
        w.Text(u"the host left the game");
        Broadcast(*session, w, &peer);
        for (auto& kv : mPeers)
        {
            if (kv.second->session == session)
            {
                kv.second->session = nullptr;
            }
        }
        mSessions.erase(GuidKey(session->info.instance));
        return;
    }

    // Everything the peer owned goes, the way DirectPlay cleans up after a player's machine
    std::vector<Id> owned;
    for (auto& kv : session->owners)
    {
        if (kv.second == &peer)
        {
            owned.push_back(kv.first);
        }
    }
    for (Id id : owned)
    {
        DestroyMember(*session, id);
    }
}

void Server::OnMessage(Peer& peer, const std::vector<uint8_t>& message)
{
    Reader r(message.data(), message.size());
    const uint8_t type = r.U8();
    Session* session = peer.session;

    // Everything except opening needs a session, and only acts on what the peer owns
    switch (type)
    {
        case kMsgOpenCreate:
        {
            const uint32_t req = r.U32();
            std::unique_ptr<Session> s(new Session);
            s->info.Read(r);
            s->password = r.Text();
            if (r.Failed())
            {
                return;
            }
            LeaveSession(peer);
            std::random_device rd;
            for (int i = 0; i < 16; i++)
            {
                s->info.instance[i] = static_cast<uint8_t>(rd());
            }
            s->info.compat = peer.compat;
            s->info.build = peer.build;
            s->info.hasPassword = !s->password.empty();
            s->info.currentPlayers = 0;
            s->info.flags &= ~kSessionJoinDisabled;
            s->creator = &peer;
            s->created = NowMs();
            peer.session = s.get();
            Log(peer.link.Peer().ToString() + " created session \"" + ToUtf8(s->info.name) + "\"");
            Writer payload;
            s->info.Write(payload);
            mSessions[GuidKey(s->info.instance)] = std::move(s);
            Reply(peer, req, kOk, &payload);
            return;
        }

        case kMsgOpenJoin:
        {
            const uint32_t req = r.U32();
            uint8_t instance[16];
            r.Guid(instance);
            const std::u16string password = r.Text();
            if (r.Failed())
            {
                return;
            }
            auto it = mSessions.find(GuidKey(instance));
            if (it == mSessions.end() || (it->second->info.flags & kSessionJoinDisabled))
            {
                Reply(peer, req, kErrNoSession);
                return;
            }
            Session& s = *it->second;
            if (s.info.compat != peer.compat)
            {
                Log(peer.link.Peer().ToString() + " refused: " + peer.build + " can't play with " + s.info.build);
                Reply(peer, req, kErrIncompatible);
                return;
            }
            if (!s.password.empty() && s.password != password)
            {
                Reply(peer, req, kErrPassword);
                return;
            }
            if (s.info.maxPlayers && PlayerCount(s) >= s.info.maxPlayers)
            {
                Reply(peer, req, kErrFull);
                return;
            }
            LeaveSession(peer);
            peer.session = &s;
            Log(peer.link.Peer().ToString() + " joined session \"" + ToUtf8(s.info.name) + "\"");
            // The answer carries the whole session, in creation order
            Writer payload;
            s.info.currentPlayers = PlayerCount(s);
            s.info.Write(payload);
            payload.U16(static_cast<uint16_t>(s.order.size()));
            for (Id id : s.order)
            {
                WriteMember(payload, s.members[id]);
            }
            Reply(peer, req, kOk, &payload);
            return;
        }

        case kMsgClose:
            LeaveSession(peer);
            return;
    }

    if (!session)
    {
        // A request after the session ended: answer it, so the client doesn't wait
        if (type == kMsgCreatePlayer || type == kMsgCreateGroup)
        {
            Reply(peer, r.U32(), kErrNoSession);
        }
        return;
    }

    switch (type)
    {
        case kMsgCreatePlayer:
        case kMsgCreateGroup:
        {
            const uint32_t req = r.U32();
            Member m;
            m.isGroup = type == kMsgCreateGroup;
            m.shortName = r.Text();
            m.longName = r.Text();
            m.data = r.Blob();
            m.flags = r.U32();
            if (r.Failed())
            {
                return;
            }
            if (!m.isGroup && session->info.maxPlayers && PlayerCount(*session) >= session->info.maxPlayers)
            {
                Reply(peer, req, kErrFull);
                return;
            }
            m.id = NewId();
            session->members[m.id] = m;
            session->owners[m.id] = &peer;
            session->order.push_back(m.id);
            Writer payload;
            payload.U32(m.id);
            Reply(peer, req, kOk, &payload);
            // Everyone else learns of it; the creator knows from the answer
            Writer ev;
            ev.U8(kEvCreate);
            WriteMember(ev, m);
            Broadcast(*session, ev, &peer);
            Log(std::string(m.isGroup ? "group " : "player ") + ToUtf8(m.shortName) + " created");
            break;
        }

        case kMsgDestroyMember:
        {
            const Id id = r.U32();
            if (!r.Failed() && session->owners.count(id) && session->owners[id] == &peer)
            {
                DestroyMember(*session, id);
            }
            break;
        }

        case kMsgAddToGroup:
        case kMsgDeleteFromGroup:
        {
            const Id group = r.U32();
            const Id player = r.U32();
            if (r.Failed() || !session->members.count(group) || !session->members.count(player) || !session->members[group].isGroup ||
                session->members[player].isGroup)
            {
                break;
            }
            std::vector<Id>& list = session->members[group].members;
            auto at = std::find(list.begin(), list.end(), player);
            if (type == kMsgAddToGroup)
            {
                if (at != list.end())
                {
                    break;
                }
                list.push_back(player);
            }
            else
            {
                if (at == list.end())
                {
                    break;
                }
                list.erase(at);
            }
            Writer ev;
            ev.U8(type == kMsgAddToGroup ? kEvAddToGroup : kEvDeleteFromGroup);
            ev.U32(group);
            ev.U32(player);
            Broadcast(*session, ev, nullptr);
            break;
        }

        case kMsgSetData:
        {
            const Id id = r.U32();
            std::vector<uint8_t> data = r.Blob();
            if (r.Failed() || !session->members.count(id))
            {
                break;
            }
            session->members[id].data = data;
            Writer ev;
            ev.U8(kEvSetData);
            ev.U32(id);
            ev.Blob(data);
            Broadcast(*session, ev, nullptr);
            break;
        }

        case kMsgSetSessionDesc:
        {
            if (session->creator != &peer)
            {
                break;
            }
            SessionInfo info;
            info.Read(r);
            if (r.Failed())
            {
                break;
            }
            // Only what the game may change: the rest stays the server's
            session->info.flags = info.flags;
            session->info.maxPlayers = info.maxPlayers;
            memcpy(session->info.user, info.user, sizeof(info.user));
            if (!info.name.empty())
            {
                session->info.name = info.name;
            }
            session->info.currentPlayers = PlayerCount(*session);
            Writer ev;
            ev.U8(kEvSetDesc);
            session->info.Write(ev);
            Broadcast(*session, ev, nullptr);
            break;
        }

        case kMsgSend:
        case kMsgChat:
        {
            const Id from = r.U32();
            const Id to = r.U32();
            const uint32_t flags = r.U32();
            std::vector<uint8_t> data;
            std::u16string text;
            if (type == kMsgSend)
            {
                data = r.Blob();
            }
            else
            {
                text = r.Text();
            }
            (void)flags;
            if (r.Failed() || !session->owners.count(from) || session->owners[from] != &peer)
            {
                break;
            }
            // To everyone, a group or one player; never back to the sender
            std::vector<Id> targets;
            Id group = 0;
            if (to == 0)
            {
                for (Id id : session->order)
                {
                    if (!session->members[id].isGroup && id != from)
                    {
                        targets.push_back(id);
                    }
                }
            }
            else if (session->members.count(to) && session->members[to].isGroup)
            {
                group = to;
                for (Id id : session->members[to].members)
                {
                    if (id != from)
                    {
                        targets.push_back(id);
                    }
                }
            }
            else if (session->members.count(to))
            {
                targets.push_back(to);
            }
            for (Id target : targets)
            {
                Peer* owner = session->owners[target];
                if (!owner)
                {
                    continue;
                }
                Writer ev;
                if (type == kMsgSend)
                {
                    ev.U8(kEvDeliver);
                    ev.U32(from);
                    ev.U32(target);
                    ev.Blob(data);
                }
                else
                {
                    ev.U8(kEvChat);
                    ev.U32(from);
                    ev.U32(group ? 0 : target);
                    ev.U32(group);
                    ev.Text(text);
                }
                owner->link.Send(ev.Data());
            }
            break;
        }
    }
}
} // namespace net
