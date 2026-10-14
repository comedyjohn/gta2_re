// The session layer the native port's DirectPlay stands on (port/platform/dplay.cpp). Plain C++, no
// game or Windows types, so the dedicated server (gta2-server) and the tests build from it alone.
//
// A Server holds sessions: their players, groups, data, and relays every message between the
// players' machines (a star). A game that hosts runs one inside itself; anyone can also run
// gta2-server on a machine everybody can reach, so nobody else needs an open port. A Client is one
// game's connection: it finds sessions (LAN broadcast or a named address), joins or creates one,
// keeps a copy of the session's state and queues what arrives for DirectPlay's Receive.
#pragma once
#include "link.h"
#include "socket.h"

#include <stdint.h>

#include <condition_variable>
#include <deque>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace net
{
// Ids as DirectPlay hands them out (DPID). 0 is "everyone"/"system".
typedef uint32_t Id;

// Error codes the server returns; dplay.cpp maps them to DPERR_*
enum Result : uint32_t
{
    kOk = 0,
    kErrGeneric = 1,
    kErrNoSession = 2, // no such session, or it no longer takes players
    kErrFull = 3,
    kErrIncompatible = 4, // another build of the game: it could not stay in step
    kErrPassword = 5,
    kErrInvalidPlayer = 6,
    kErrInvalidGroup = 7,
    kErrTimeout = 8, // the server didn't answer
    kErrNotConnected = 9,
};
const char* ResultText(Result r);

struct SessionInfo
{
    uint8_t instance[16] = {};
    uint8_t application[16] = {};
    uint32_t flags = 0;
    uint32_t maxPlayers = 0;
    uint32_t currentPlayers = 0;
    uint32_t user[4] = {};
    std::u16string name;
    bool hasPassword = false;
    // Filled in by discovery
    std::string compat; // the creator's build compatibility id
    std::string build; // the creator's build, for messages ("linux x86-64")
    Address server; // where the session lives

    void Write(Writer& w) const;
    void Read(Reader& r);
};

// A session flag that matters to the server (DirectPlay's DPSESSION_JOINDISABLED)
const uint32_t kSessionJoinDisabled = 0x20;

struct Member // a player or a group
{
    Id id = 0;
    bool isGroup = false;
    std::u16string shortName;
    std::u16string longName;
    std::vector<uint8_t> data;
    uint32_t flags = 0;
    std::vector<Id> members; // groups: players in the order they were added
};

// What the client hands to DirectPlay's Receive: one system message or one player's message
struct Event
{
    enum Type
    {
        CreateMember, // member
        DestroyMember, // member (as it was)
        AddToGroup, // group, player
        DeleteFromGroup, // group, player
        SetData, // member (with the new data)
        SetSessionDesc,
        Chat, // from, to (player or 0), group (or 0), text
        Message, // from, to, data
        SessionLost, // text: why
    };
    Type type;
    Member member;
    Id group = 0;
    Id player = 0;
    Id from = 0;
    Id to = 0;
    std::vector<uint8_t> data;
    std::u16string text;
};

class Server
{
  public:
    Server() = default;
    ~Server();

    // Listens on `port` (both IPv4 and IPv6 where available) on a thread of its own
    bool Start(uint16_t port, bool verbose, std::string& error);
    void Stop();
    std::string Describe();
    uint16_t Port() const
    {
        return mSocket.LocalPort();
    }

  private:
    struct Session;
    struct Peer
    {
        explicit Peer(UdpSocket& socket, const Address& addr, uint32_t id) : link(socket, addr, id)
        {
        }
        Link link;
        std::string compat;
        std::string build;
        Session* session = nullptr;
    };
    struct Session
    {
        SessionInfo info;
        std::u16string password;
        Peer* creator = nullptr;
        uint32_t created = 0; // NowMs()
        std::map<Id, Member> members;
        std::map<Id, Peer*> owners;
        std::vector<Id> order; // members by creation
    };

    void Run();
    void OnPacket(const Address& from, const uint8_t* data, size_t size, uint32_t now);
    void OnMessage(Peer& peer, const std::vector<uint8_t>& message);
    void Reply(Peer& peer, uint32_t request, Result result, const Writer* payload = nullptr);
    void Broadcast(Session& session, const Writer& message, Peer* except);
    void LeaveSession(Peer& peer);
    void DestroyMember(Session& session, Id id);
    uint32_t PlayerCount(const Session& session) const;
    Id NewId();
    void Log(const std::string& text);

    UdpSocket mSocket;
    std::thread mThread;
    std::mutex mMutex;
    bool mStop = false;
    bool mVerbose = false;
    std::map<Address, std::unique_ptr<Peer>> mPeers;
    std::map<std::string, std::unique_ptr<Session>> mSessions; // by instance guid
    Id mNextId;
};

class Client
{
  public:
    // compat: who this client can play with (the same game code); build: shown in messages
    Client(const std::string& compat, const std::string& build);
    ~Client();

    bool Start(std::string& error);

    // Session discovery. Sends a query to the LAN (broadcast, at `port`) and/or to `address`; the
    // answers come in on the client's thread. Sessions() lists those heard from in the last few
    // seconds.
    void Discover(const uint8_t application[16], bool lan, uint16_t port, const Address* address);
    bool CanReach(const Address& address) const
    {
        return mSocket.CanReach(address);
    }
    std::vector<SessionInfo> Sessions();

    // Opening a session. Both block (up to a few seconds) for the server's answer.
    Result Create(const Address& server, const SessionInfo& desc, const std::u16string& password, std::string& error);
    Result Join(const SessionInfo& session, const std::u16string& password, std::string& error);
    void Close();
    bool InSession();
    // The session went away under us (the host left, or the connection died)
    bool SessionLost();

    Result CreatePlayer(const std::u16string& shortName,
                        const std::u16string& longName,
                        const std::vector<uint8_t>& data,
                        uint32_t flags,
                        Id& out);
    Result CreateGroup(const std::u16string& shortName,
                       const std::u16string& longName,
                       const std::vector<uint8_t>& data,
                       uint32_t flags,
                       Id& out);
    Result DestroyPlayer(Id id);
    Result DestroyGroup(Id id);
    Result AddToGroup(Id group, Id player);
    Result DeleteFromGroup(Id group, Id player);
    Result SetData(Id id, const std::vector<uint8_t>& data);
    Result SetSessionDesc(const SessionInfo& desc);
    Result Send(Id from, Id to, uint32_t flags, const void* data, size_t size);
    Result Chat(Id from, Id to, const std::u16string& text);

    // The session as this client knows it
    bool GetMember(Id id, Member& out);
    std::vector<Member> Groups();
    std::vector<Member> Players();
    SessionInfo Session();
    bool IsLocalPlayer(Id id);

    std::string Describe();
    // The next queued event. Messages for local players and system messages arrive in the order the
    // server sent them.
    bool PopEvent(Event& out);
    size_t EventCount();
    // Waits up to timeoutMs for an event to be queued
    void WaitForEvent(int timeoutMs);

  private:
    // A request waiting for its answer. What a successful answer changes is applied on the client's
    // thread as it arrives (ApplyResult), before anything the server sent after it: applied later, by
    // the thread that asked, the events in between would find a session or a local player missing.
    struct Pending
    {
        uint8_t kind = 0; // the request's message type
        Address server; // opening: where the session lives
        Member member; // creating: what is being created
        bool done = false;
        Result result = kErrTimeout;
        std::vector<uint8_t> payload;
    };
    void ApplyResult(Pending& p);

    void Run();
    void OnPacket(const Address& from, const uint8_t* data, size_t size, uint32_t now);
    void OnMessage(const std::vector<uint8_t>& message);
    Result Connect(const Address& server, std::string& error, std::unique_lock<std::mutex>& lock);
    Result Request(Writer& message, uint32_t request, Pending pending, Pending* out, std::unique_lock<std::mutex>& lock);
    void Queue(Event&& e, bool systemMessage);
    void Lost(const std::string& why);
    void ResetSession();

    std::string mCompat;
    std::string mBuild;
    UdpSocket mSocket;
    std::thread mThread;
    std::mutex mMutex;
    std::condition_variable mChanged;
    bool mStop = false;

    // Discovery
    struct Heard
    {
        SessionInfo info;
        uint32_t when;
    };
    std::map<std::string, Heard> mHeard; // by instance guid
    uint32_t mDiscoverNonce;

    // The connection to the server
    std::unique_ptr<Link> mLink;
    uint32_t mConnectNonce = 0;
    bool mConnected = false;
    std::string mRefusal;
    uint32_t mNextRequest = 1;
    std::map<uint32_t, Pending> mPending;

    // The session
    bool mInSession = false;
    bool mLost = false;
    SessionInfo mSession;
    std::map<Id, Member> mMembers;
    std::vector<Id> mOrder;
    std::vector<Id> mLocalPlayers;
    // Players the game has been told about (see Queue)
    std::vector<Id> mAnnounced;
    std::deque<Event> mEvents;
};

// Message types inside kPacketData. Shared by client and server.
enum MessageType : uint8_t
{
    // Client to server
    kMsgOpenCreate = 1,
    kMsgOpenJoin = 2,
    kMsgCreatePlayer = 3,
    kMsgDestroyMember = 4,
    kMsgCreateGroup = 5,
    kMsgAddToGroup = 7,
    kMsgDeleteFromGroup = 8,
    kMsgSetData = 9,
    kMsgSetSessionDesc = 10,
    kMsgSend = 11,
    kMsgChat = 12,
    kMsgClose = 13,
    // Server to client
    kMsgResult = 50,
    kEvCreate = 60,
    kEvDestroy = 61,
    kEvAddToGroup = 62,
    kEvDeleteFromGroup = 63,
    kEvSetData = 64,
    kEvSetDesc = 65,
    kEvChat = 66,
    kEvDeliver = 67,
    kEvSessionLost = 68,
};

void WriteMember(Writer& w, const Member& m);
void ReadMember(Reader& r, Member& m);
std::string GuidKey(const uint8_t guid[16]);
} // namespace net
