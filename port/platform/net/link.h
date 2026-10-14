// A reliable, ordered stream of messages between two UDP endpoints: sequence numbers, acks and
// resends. Every message the game sends goes through one, so a lost datagram costs a resend's delay
// but never a desync.
#pragma once
#include "socket.h"
#include "wire.h"

#include <stdint.h>

#include <deque>
#include <map>
#include <string>
#include <vector>

namespace net
{
// Test hook: GTA2_NET_DROP=<percent> drops that share of outgoing datagrams at random
bool ShouldDropOutgoing();
// How long a silent peer is waited for before it counts as gone: 15 s, or GTA2_NET_TIMEOUT_MS (tests)
uint32_t PeerTimeoutMs();
// Sends a datagram through `socket`, unless the drop test hook eats it
bool SendRaw(UdpSocket& socket, const Address& to, const std::vector<uint8_t>& packet);

class Link
{
  public:
    Link(UdpSocket& socket, const Address& peer, uint32_t id);

    const Address& Peer() const
    {
        return mPeer;
    }
    uint32_t Id() const
    {
        return mId;
    }

    // Queues a message and sends it at once
    void Send(const std::vector<uint8_t>& message);

    // Handles a kPacketData or kPacketAck packet from the peer (r is past the link id)
    void OnData(Reader& r);
    void OnAck(Reader& r);
    void OnAnyPacket(uint32_t now)
    {
        mLastHeard = now;
    }

    // Resends what wasn't acked, sends the pending ack and keep-alives. Call often.
    void Update(uint32_t now);

    // Messages that arrived, in order
    bool PopMessage(std::vector<uint8_t>& out);

    // No packet from the peer for timeoutMs
    bool TimedOut(uint32_t now, uint32_t timeoutMs) const
    {
        return now - mLastHeard > timeoutMs;
    }
    // Everything sent has been acknowledged
    bool AllAcked() const
    {
        return mUnacked.empty();
    }
    uint32_t SmoothedRttMs() const
    {
        return mSrtt;
    }

    void SendDisconnect();
    std::string Describe(uint32_t now) const; // for diagnostics

  private:
    struct Outgoing
    {
        std::vector<uint8_t> packet;
        uint32_t firstSent;
        uint32_t lastSent;
        int sends;
    };

    void SendAck();

    UdpSocket& mSocket;
    Address mPeer;
    uint32_t mId;

    uint32_t mNextSendSeq = 0;
    std::map<uint32_t, Outgoing> mUnacked;

    uint32_t mNextRecvSeq = 0;
    uint32_t mNewestReceived = 0; // the sequence number of the last data packet that came in
    std::map<uint32_t, std::vector<uint8_t>> mOutOfOrder;
    std::deque<std::vector<uint8_t>> mReceived;
    bool mAckPending = false;

    uint32_t mLastHeard;
    uint32_t mLastSent;
    uint32_t mSrtt = 0; // 0 until measured
    uint32_t mRttVar = 0;
};
} // namespace net
