// A game's connection to a session server: see net.h
#include "net.h"

#include <stdio.h>

#include <algorithm>
#include <chrono>
#include <random>

namespace net
{
// The server is gone if we hear nothing from it for PeerTimeoutMs(). Its thread answers even while
// the host's game is busy loading a map, so this only fires when the machine or the network went away.
const uint32_t kRequestTimeoutMs = 5000;
const uint32_t kConnectTimeoutMs = 5000;
// A discovered session is listed while it has answered within this long
const uint32_t kSessionStaleMs = 3500;

Client::Client(const std::string& compat, const std::string& build) : mCompat(compat), mBuild(build)
{
    std::random_device rd;
    mDiscoverNonce = rd();
}

Client::~Client()
{
    if (mThread.joinable())
    {
        {
            std::unique_lock<std::mutex> lock(mMutex);
            if (mLink && mConnected)
            {
                if (mInSession && !mLost)
                {
                    // Let the server hear that we leave, rather than finding out from a timeout
                    Writer w;
                    w.U8(kMsgClose);
                    mLink->Send(w.Data());
                    // The wait lets the client's thread run, which drops the link if the server
                    // has gone meanwhile
                    const uint32_t start = NowMs();
                    while (mLink && !mLink->AllAcked() && NowMs() - start < 300)
                    {
                        mChanged.wait_for(lock, std::chrono::milliseconds(10));
                    }
                }
                if (mLink)
                {
                    mLink->SendDisconnect();
                }
            }
            mStop = true;
        }
        mThread.join();
    }
}

bool Client::Start(std::string& error)
{
    if (mThread.joinable())
    {
        return true;
    }
    if (!mSocket.Open(0, error))
    {
        return false;
    }
    mThread = std::thread(&Client::Run, this);
    return true;
}

void Client::Run()
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
        if (mLink && mConnected)
        {
            std::vector<uint8_t> message;
            while (mLink->PopMessage(message))
            {
                OnMessage(message);
            }
            if (mLink->TimedOut(now, PeerTimeoutMs()))
            {
                Lost("lost the connection to the game (no answer for " + std::to_string(PeerTimeoutMs() / 1000) + " seconds)");
                mConnected = false;
                mLink.reset();
                mChanged.notify_all();
            }
            else
            {
                mLink->Update(now);
            }
        }
    }
}

void Client::OnPacket(const Address& from, const uint8_t* data, size_t size, uint32_t now)
{
    Reader r(data, size);
    uint8_t version;
    uint8_t type;
    if (!ReadPacketHeader(r, version, type) || version != kProtocolVersion)
    {
        return;
    }
    switch (type)
    {
        case kPacketDiscoverReply:
        {
            const uint32_t nonce = r.U32();
            SessionInfo info;
            info.Read(r);
            if (r.Failed() || nonce != mDiscoverNonce)
            {
                return;
            }
            info.server = from;
            Heard& h = mHeard[GuidKey(info.instance)];
            // Keep the address it was first heard from: a session answers on the LAN and on
            // loopback alike, and switching between them gains nothing
            if (h.info.server.Valid() && h.when && now - h.when < kSessionStaleMs)
            {
                info.server = h.info.server;
            }
            h.info = info;
            h.when = now;
            break;
        }

        case kPacketConnected:
        {
            const uint32_t nonce = r.U32();
            const uint32_t id = r.U32();
            if (r.Failed() || nonce != mConnectNonce || mConnected)
            {
                return;
            }
            mLink.reset(new Link(mSocket, from, id));
            mConnected = true;
            mChanged.notify_all();
            break;
        }

        case kPacketRefused:
        {
            const uint32_t nonce = r.U32();
            const std::string reason = r.Str();
            if (r.Failed() || nonce != mConnectNonce)
            {
                return;
            }
            mRefusal = reason;
            mChanged.notify_all();
            break;
        }

        case kPacketData:
        case kPacketAck:
        case kPacketKeepAlive:
        case kPacketDisconnect:
        {
            const uint32_t id = r.U32();
            if (r.Failed() || !mLink || !mConnected || mLink->Id() != id || mLink->Peer() != from)
            {
                return;
            }
            mLink->OnAnyPacket(now);
            if (type == kPacketData)
            {
                mLink->OnData(r);
            }
            else if (type == kPacketAck)
            {
                mLink->OnAck(r);
                mChanged.notify_all();
            }
            else if (type == kPacketDisconnect)
            {
                std::vector<uint8_t> message;
                while (mLink->PopMessage(message))
                {
                    OnMessage(message);
                }
                Lost("the game's host closed it");
                mConnected = false;
                mLink.reset();
                mChanged.notify_all();
            }
            break;
        }
    }
}

void Client::Queue(Event&& e, bool systemMessage)
{
    // DirectPlay delivers system messages to the local players: with none yet, nobody hears them
    if (systemMessage && mLocalPlayers.empty())
    {
        return;
    }
    // The game learns of a player from its creation message, or from the group's list as it was when
    // our own player was made. A player made while ours was still being created, and only then added
    // to the group, was heard of by neither: tell the game about it before it hears of the adding.
    auto announced = [&](Id id)
    {
        return std::find(mAnnounced.begin(), mAnnounced.end(), id) != mAnnounced.end() ||
            std::find(mLocalPlayers.begin(), mLocalPlayers.end(), id) != mLocalPlayers.end();
    };
    if (e.type == Event::CreateMember)
    {
        mAnnounced.push_back(e.member.id);
    }
    else if (e.type == Event::AddToGroup && !announced(e.player) && mMembers.count(e.player))
    {
        Event create;
        create.type = Event::CreateMember;
        create.member = mMembers[e.player];
        mAnnounced.push_back(e.player);
        mEvents.push_back(std::move(create));
    }
    mEvents.push_back(std::move(e));
    mChanged.notify_all();
}

void Client::Lost(const std::string& why)
{
    if (!mInSession || mLost)
    {
        return;
    }
    mLost = true;
    Event e;
    e.type = Event::SessionLost;
    e.text = std::u16string(why.begin(), why.end());
    mEvents.push_back(std::move(e));
    for (auto& kv : mPending)
    {
        kv.second.done = true;
        kv.second.result = kErrNotConnected;
    }
    mChanged.notify_all();
}

void Client::OnMessage(const std::vector<uint8_t>& message)
{
    Reader r(message.data(), message.size());
    const uint8_t type = r.U8();
    switch (type)
    {
        case kMsgResult:
        {
            const uint32_t req = r.U32();
            const Result result = static_cast<Result>(r.U32());
            auto it = mPending.find(req);
            if (r.Failed() || it == mPending.end())
            {
                return;
            }
            it->second.done = true;
            it->second.result = result;
            it->second.payload.assign(r.Here(), r.Here() + r.Remaining());
            if (result == kOk)
            {
                ApplyResult(it->second);
            }
            mChanged.notify_all();
            return;
        }
        case kEvSessionLost:
        {
            const std::u16string why = r.Text();
            Lost(std::string(why.begin(), why.end()));
            return;
        }
    }

    if (!mInSession || mLost)
    {
        return;
    }

    switch (type)
    {
        case kEvCreate:
        {
            Event e;
            e.type = Event::CreateMember;
            ReadMember(r, e.member);
            if (r.Failed())
            {
                return;
            }
            mMembers[e.member.id] = e.member;
            mOrder.push_back(e.member.id);
            Queue(std::move(e), true);
            break;
        }
        case kEvDestroy:
        {
            const Id id = r.U32();
            auto it = mMembers.find(id);
            if (r.Failed() || it == mMembers.end())
            {
                return;
            }
            Event e;
            e.type = Event::DestroyMember;
            e.member = it->second;
            mMembers.erase(it);
            mOrder.erase(std::remove(mOrder.begin(), mOrder.end(), id), mOrder.end());
            Queue(std::move(e), true);
            mLocalPlayers.erase(std::remove(mLocalPlayers.begin(), mLocalPlayers.end(), id), mLocalPlayers.end());
            break;
        }
        case kEvAddToGroup:
        case kEvDeleteFromGroup:
        {
            Event e;
            e.type = type == kEvAddToGroup ? Event::AddToGroup : Event::DeleteFromGroup;
            e.group = r.U32();
            e.player = r.U32();
            if (r.Failed() || !mMembers.count(e.group))
            {
                return;
            }
            std::vector<Id>& list = mMembers[e.group].members;
            list.erase(std::remove(list.begin(), list.end(), e.player), list.end());
            if (type == kEvAddToGroup)
            {
                list.push_back(e.player);
            }
            Queue(std::move(e), true);
            break;
        }
        case kEvSetData:
        {
            const Id id = r.U32();
            std::vector<uint8_t> data = r.Blob();
            if (r.Failed() || !mMembers.count(id))
            {
                return;
            }
            mMembers[id].data = data;
            Event e;
            e.type = Event::SetData;
            e.member = mMembers[id];
            Queue(std::move(e), true);
            break;
        }
        case kEvSetDesc:
        {
            const Address server = mSession.server;
            mSession.Read(r);
            mSession.server = server;
            break;
        }
        case kEvChat:
        {
            Event e;
            e.type = Event::Chat;
            e.from = r.U32();
            e.to = r.U32();
            e.group = r.U32();
            e.text = r.Text();
            if (!r.Failed())
            {
                Queue(std::move(e), false);
            }
            break;
        }
        case kEvDeliver:
        {
            Event e;
            e.type = Event::Message;
            e.from = r.U32();
            e.to = r.U32();
            e.data = r.Blob();
            if (!r.Failed())
            {
                Queue(std::move(e), false);
            }
            break;
        }
    }
}

void Client::Discover(const uint8_t application[16], bool lan, uint16_t port, const Address* address)
{
    Writer w;
    BeginPacket(w, kPacketDiscover);
    w.U32(mDiscoverNonce);
    w.Guid(application);
    if (lan)
    {
        for (const Address& a : BroadcastAddresses(port))
        {
            SendRaw(mSocket, a, w.Data());
        }
    }
    if (address)
    {
        SendRaw(mSocket, *address, w.Data());
    }
}

std::vector<SessionInfo> Client::Sessions()
{
    std::lock_guard<std::mutex> lock(mMutex);
    std::vector<SessionInfo> out;
    const uint32_t now = NowMs();
    for (auto it = mHeard.begin(); it != mHeard.end();)
    {
        if (now - it->second.when > kSessionStaleMs)
        {
            it = mHeard.erase(it);
            continue;
        }
        out.push_back(it->second.info);
        ++it;
    }
    return out;
}

Result Client::Connect(const Address& server, std::string& error, std::unique_lock<std::mutex>& lock)
{
    if (mLink && mConnected && mLink->Peer() == server)
    {
        return kOk;
    }
    if (mLink && mConnected)
    {
        mLink->SendDisconnect();
    }
    mLink.reset();
    mConnected = false;
    mRefusal.clear();
    std::random_device rd;
    mConnectNonce = rd() | 1;

    Writer w;
    BeginPacket(w, kPacketConnect);
    w.U32(mConnectNonce);
    w.Str(mCompat);
    w.Str(mBuild);

    const uint32_t start = NowMs();
    uint32_t lastSent = 0;
    while (!mConnected && mRefusal.empty())
    {
        const uint32_t now = NowMs();
        if (now - start > kConnectTimeoutMs)
        {
            error = "no answer from " + server.ToString() + " (is the game still open there, and is UDP port " +
                std::to_string(server.Port()) + " reachable?)";
            return kErrTimeout;
        }
        if (!lastSent || now - lastSent >= 250)
        {
            SendRaw(mSocket, server, w.Data());
            lastSent = now;
        }
        mChanged.wait_for(lock, std::chrono::milliseconds(20));
    }
    if (!mRefusal.empty())
    {
        error = mRefusal;
        return kErrIncompatible;
    }
    return kOk;
}

Result Client::Request(Writer& message, uint32_t request, Pending pending, Pending* out, std::unique_lock<std::mutex>& lock)
{
    if (!mLink || !mConnected)
    {
        return kErrNotConnected;
    }
    mPending[request] = pending;
    mLink->Send(message.Data());
    const uint32_t start = NowMs();
    while (!mPending[request].done && NowMs() - start < kRequestTimeoutMs)
    {
        mChanged.wait_for(lock, std::chrono::milliseconds(20));
    }
    Pending p = mPending[request];
    mPending.erase(request);
    if (out)
    {
        *out = p;
    }
    return p.done ? p.result : kErrTimeout;
}

void Client::ApplyResult(Pending& p)
{
    Reader r(p.payload.data(), p.payload.size());
    switch (p.kind)
    {
        case kMsgOpenCreate:
        case kMsgOpenJoin:
        {
            // The session as it is now; the events that follow change it from here
            mSession.Read(r);
            mSession.server = p.server;
            if (p.kind == kMsgOpenJoin)
            {
                const uint16_t count = r.U16();
                for (uint16_t i = 0; i < count && !r.Failed(); i++)
                {
                    Member m;
                    ReadMember(r, m);
                    mMembers[m.id] = m;
                    mOrder.push_back(m.id);
                }
            }
            mInSession = true;
            break;
        }

        case kMsgCreatePlayer:
        case kMsgCreateGroup:
        {
            Member m = p.member;
            m.id = r.U32();
            p.member.id = m.id;
            if (r.Failed() || !mInSession || mLost)
            {
                p.result = kErrNoSession;
                break;
            }
            if (!m.isGroup)
            {
                if (mLocalPlayers.empty())
                {
                    // What the game will see listing the groups' players
                    for (auto& kv : mMembers)
                    {
                        if (kv.second.isGroup)
                        {
                            mAnnounced.insert(mAnnounced.end(), kv.second.members.begin(), kv.second.members.end());
                        }
                    }
                }
                mLocalPlayers.push_back(m.id);
            }
            mMembers[m.id] = m;
            mOrder.push_back(m.id);
            break;
        }
    }
}

void Client::ResetSession()
{
    mInSession = false;
    mLost = false;
    mSession = SessionInfo();
    mMembers.clear();
    mOrder.clear();
    mLocalPlayers.clear();
    mAnnounced.clear();
    mEvents.clear();
}

Result Client::Create(const Address& server, const SessionInfo& desc, const std::u16string& password, std::string& error)
{
    std::unique_lock<std::mutex> lock(mMutex);
    if (mInSession && !mLost && mLink)
    {
        Writer w;
        w.U8(kMsgClose);
        mLink->Send(w.Data());
    }
    ResetSession();
    Result result = Connect(server, error, lock);
    if (result != kOk)
    {
        return result;
    }
    const uint32_t req = mNextRequest++;
    Writer w;
    w.U8(kMsgOpenCreate);
    w.U32(req);
    desc.Write(w);
    w.Text(password);
    Pending p;
    p.kind = kMsgOpenCreate;
    p.server = server;
    result = Request(w, req, p, nullptr, lock);
    if (result != kOk)
    {
        error = ResultText(result);
    }
    return result;
}

Result Client::Join(const SessionInfo& session, const std::u16string& password, std::string& error)
{
    std::unique_lock<std::mutex> lock(mMutex);
    if (mInSession && !mLost && mLink)
    {
        Writer w;
        w.U8(kMsgClose);
        mLink->Send(w.Data());
    }
    ResetSession();
    Result result = Connect(session.server, error, lock);
    if (result != kOk)
    {
        return result;
    }
    const uint32_t req = mNextRequest++;
    Writer w;
    w.U8(kMsgOpenJoin);
    w.U32(req);
    w.Guid(session.instance);
    w.Text(password);
    Pending p;
    p.kind = kMsgOpenJoin;
    p.server = session.server;
    result = Request(w, req, p, nullptr, lock);
    if (result != kOk)
    {
        error = ResultText(result);
        if (result == kErrIncompatible)
        {
            error += ": the host has " + session.build;
        }
    }
    return result;
}

void Client::Close()
{
    std::lock_guard<std::mutex> lock(mMutex);
    if (mInSession && !mLost && mLink && mConnected)
    {
        Writer w;
        w.U8(kMsgClose);
        mLink->Send(w.Data());
    }
    ResetSession();
}

bool Client::SessionLost()
{
    std::lock_guard<std::mutex> lock(mMutex);
    return mInSession && mLost;
}

bool Client::InSession()
{
    std::lock_guard<std::mutex> lock(mMutex);
    return mInSession && !mLost;
}

Result Client::CreatePlayer(const std::u16string& shortName,
                            const std::u16string& longName,
                            const std::vector<uint8_t>& data,
                            uint32_t flags,
                            Id& out)
{
    std::unique_lock<std::mutex> lock(mMutex);
    if (!mInSession || mLost)
    {
        return kErrNoSession;
    }
    const uint32_t req = mNextRequest++;
    Writer w;
    w.U8(kMsgCreatePlayer);
    w.U32(req);
    w.Text(shortName);
    w.Text(longName);
    w.Blob(data);
    w.U32(flags);
    Pending p;
    p.kind = kMsgCreatePlayer;
    p.member.shortName = shortName;
    p.member.longName = longName;
    p.member.data = data;
    p.member.flags = flags;
    Pending answer;
    const Result result = Request(w, req, p, &answer, lock);
    out = answer.member.id;
    return result;
}

Result Client::CreateGroup(const std::u16string& shortName,
                           const std::u16string& longName,
                           const std::vector<uint8_t>& data,
                           uint32_t flags,
                           Id& out)
{
    std::unique_lock<std::mutex> lock(mMutex);
    if (!mInSession || mLost)
    {
        return kErrNoSession;
    }
    const uint32_t req = mNextRequest++;
    Writer w;
    w.U8(kMsgCreateGroup);
    w.U32(req);
    w.Text(shortName);
    w.Text(longName);
    w.Blob(data);
    w.U32(flags);
    Pending p;
    p.kind = kMsgCreateGroup;
    p.member.isGroup = true;
    p.member.shortName = shortName;
    p.member.longName = longName;
    p.member.data = data;
    p.member.flags = flags;
    Pending answer;
    const Result result = Request(w, req, p, &answer, lock);
    out = answer.member.id;
    return result;
}

// The asynchronous requests: checked against what we know, then sent without waiting. Their system
// messages come back in order with everything else.
#define CLIENT_SEND_PRELUDE                            \
    std::lock_guard<std::mutex> lock(mMutex);          \
    if (!mInSession || mLost || !mLink || !mConnected) \
    {                                                  \
        return kErrNoSession;                          \
    }

Result Client::DestroyPlayer(Id id)
{
    CLIENT_SEND_PRELUDE
    if (std::find(mLocalPlayers.begin(), mLocalPlayers.end(), id) == mLocalPlayers.end())
    {
        return kErrInvalidPlayer;
    }
    Writer w;
    w.U8(kMsgDestroyMember);
    w.U32(id);
    mLink->Send(w.Data());
    return kOk;
}

Result Client::DestroyGroup(Id id)
{
    CLIENT_SEND_PRELUDE
    if (!mMembers.count(id) || !mMembers[id].isGroup)
    {
        return kErrInvalidGroup;
    }
    Writer w;
    w.U8(kMsgDestroyMember);
    w.U32(id);
    mLink->Send(w.Data());
    return kOk;
}

Result Client::AddToGroup(Id group, Id player)
{
    CLIENT_SEND_PRELUDE
    if (!mMembers.count(group) || !mMembers[group].isGroup)
    {
        return kErrInvalidGroup;
    }
    if (!mMembers.count(player) || mMembers[player].isGroup)
    {
        return kErrInvalidPlayer;
    }
    Writer w;
    w.U8(kMsgAddToGroup);
    w.U32(group);
    w.U32(player);
    mLink->Send(w.Data());
    return kOk;
}

Result Client::DeleteFromGroup(Id group, Id player)
{
    CLIENT_SEND_PRELUDE
    if (!mMembers.count(group) || !mMembers[group].isGroup)
    {
        return kErrInvalidGroup;
    }
    if (!mMembers.count(player) || mMembers[player].isGroup)
    {
        return kErrInvalidPlayer;
    }
    Writer w;
    w.U8(kMsgDeleteFromGroup);
    w.U32(group);
    w.U32(player);
    mLink->Send(w.Data());
    return kOk;
}

Result Client::SetData(Id id, const std::vector<uint8_t>& data)
{
    CLIENT_SEND_PRELUDE
    if (!mMembers.count(id))
    {
        return kErrInvalidPlayer;
    }
    mMembers[id].data = data;
    Writer w;
    w.U8(kMsgSetData);
    w.U32(id);
    w.Blob(data);
    mLink->Send(w.Data());
    return kOk;
}

Result Client::SetSessionDesc(const SessionInfo& desc)
{
    CLIENT_SEND_PRELUDE
    mSession.flags = desc.flags;
    mSession.maxPlayers = desc.maxPlayers;
    memcpy(mSession.user, desc.user, sizeof(desc.user));
    Writer w;
    w.U8(kMsgSetSessionDesc);
    desc.Write(w);
    mLink->Send(w.Data());
    return kOk;
}

Result Client::Send(Id from, Id to, uint32_t flags, const void* data, size_t size)
{
    CLIENT_SEND_PRELUDE
    if (std::find(mLocalPlayers.begin(), mLocalPlayers.end(), from) == mLocalPlayers.end())
    {
        return kErrInvalidPlayer;
    }
    if (size > kMaxPayload - 32)
    {
        return kErrGeneric;
    }
    Writer w;
    w.U8(kMsgSend);
    w.U32(from);
    w.U32(to);
    w.U32(flags);
    w.Blob(data, size);
    mLink->Send(w.Data());
    return kOk;
}

Result Client::Chat(Id from, Id to, const std::u16string& text)
{
    CLIENT_SEND_PRELUDE
    if (std::find(mLocalPlayers.begin(), mLocalPlayers.end(), from) == mLocalPlayers.end())
    {
        return kErrInvalidPlayer;
    }
    Writer w;
    w.U8(kMsgChat);
    w.U32(from);
    w.U32(to);
    w.U32(0);
    w.Text(text.substr(0, 400));
    mLink->Send(w.Data());
    return kOk;
}

bool Client::GetMember(Id id, Member& out)
{
    std::lock_guard<std::mutex> lock(mMutex);
    auto it = mMembers.find(id);
    if (it == mMembers.end())
    {
        return false;
    }
    out = it->second;
    return true;
}

std::vector<Member> Client::Groups()
{
    std::lock_guard<std::mutex> lock(mMutex);
    std::vector<Member> out;
    for (Id id : mOrder)
    {
        if (mMembers[id].isGroup)
        {
            out.push_back(mMembers[id]);
        }
    }
    return out;
}

std::vector<Member> Client::Players()
{
    std::lock_guard<std::mutex> lock(mMutex);
    std::vector<Member> out;
    for (Id id : mOrder)
    {
        if (!mMembers[id].isGroup)
        {
            out.push_back(mMembers[id]);
        }
    }
    return out;
}

SessionInfo Client::Session()
{
    std::lock_guard<std::mutex> lock(mMutex);
    SessionInfo s = mSession;
    s.currentPlayers = 0;
    for (auto& kv : mMembers)
    {
        s.currentPlayers += !kv.second.isGroup;
    }
    return s;
}

bool Client::IsLocalPlayer(Id id)
{
    std::lock_guard<std::mutex> lock(mMutex);
    return std::find(mLocalPlayers.begin(), mLocalPlayers.end(), id) != mLocalPlayers.end();
}

bool Client::PopEvent(Event& out)
{
    std::lock_guard<std::mutex> lock(mMutex);
    if (mEvents.empty())
    {
        return false;
    }
    out = std::move(mEvents.front());
    mEvents.pop_front();
    return true;
}

std::string Client::Describe()
{
    std::lock_guard<std::mutex> lock(mMutex);
    std::string s = mLink ? mLink->Describe(NowMs()) : std::string("no link");
    return s + "; " + std::to_string(mEvents.size()) + " events queued";
}

size_t Client::EventCount()
{
    std::lock_guard<std::mutex> lock(mMutex);
    return mEvents.size();
}

void Client::WaitForEvent(int timeoutMs)
{
    std::unique_lock<std::mutex> lock(mMutex);
    if (mEvents.empty())
    {
        mChanged.wait_for(lock, std::chrono::milliseconds(timeoutMs));
    }
}
} // namespace net
