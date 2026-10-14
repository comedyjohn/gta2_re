#include "link.h"

#include <stdlib.h>

#include <algorithm>
#include <random>

namespace net
{
static int DropPercent()
{
    static int percent = -1;
    if (percent < 0)
    {
        const char* env = getenv("GTA2_NET_DROP");
        percent = env ? std::max(0, std::min(100, atoi(env))) : 0;
    }
    return percent;
}

bool ShouldDropOutgoing()
{
    const int percent = DropPercent();
    if (percent == 0)
    {
        return false;
    }
    static thread_local std::minstd_rand rng(12345);
    return static_cast<int>(rng() % 100) < percent;
}

uint32_t PeerTimeoutMs()
{
    static uint32_t ms = 0;
    if (!ms)
    {
        const char* env = getenv("GTA2_NET_TIMEOUT_MS");
        ms = env && atoi(env) > 0 ? static_cast<uint32_t>(atoi(env)) : 15000;
    }
    return ms;
}

bool SendRaw(UdpSocket& socket, const Address& to, const std::vector<uint8_t>& packet)
{
    if (ShouldDropOutgoing())
    {
        return true;
    }
    return socket.SendTo(to, packet.data(), packet.size());
}

// Out-of-order messages kept while waiting for an earlier one
const uint32_t kReceiveWindow = 4096;
const uint32_t kKeepAliveMs = 500;

Link::Link(UdpSocket& socket, const Address& peer, uint32_t id) : mSocket(socket), mPeer(peer), mId(id)
{
    mLastHeard = mLastSent = NowMs();
}

void Link::Send(const std::vector<uint8_t>& message)
{
    Writer w;
    BeginPacket(w, kPacketData);
    w.U32(mId);
    w.U32(mNextSendSeq);
    w.Bytes(message.data(), message.size());

    const uint32_t now = NowMs();
    Outgoing& out = mUnacked[mNextSendSeq++];
    out.packet = w.Data();
    out.firstSent = out.lastSent = now;
    out.sends = 1;
    SendRaw(mSocket, mPeer, out.packet);
    mLastSent = now;
}

void Link::OnData(Reader& r)
{
    const uint32_t seq = r.U32();
    if (r.Failed())
    {
        return;
    }
    mAckPending = true; // acknowledge duplicates too: our earlier ack may have been lost
    mNewestReceived = seq;
    const int32_t ahead = static_cast<int32_t>(seq - mNextRecvSeq);
    if (ahead < 0 || static_cast<uint32_t>(ahead) >= kReceiveWindow)
    {
        return;
    }
    std::vector<uint8_t> message(r.Here(), r.Here() + r.Remaining());
    if (ahead > 0)
    {
        mOutOfOrder[seq] = std::move(message);
        return;
    }
    mReceived.push_back(std::move(message));
    mNextRecvSeq++;
    for (auto it = mOutOfOrder.find(mNextRecvSeq); it != mOutOfOrder.end(); it = mOutOfOrder.find(mNextRecvSeq))
    {
        mReceived.push_back(std::move(it->second));
        mOutOfOrder.erase(it);
        mNextRecvSeq++;
    }
}

void Link::OnAck(Reader& r)
{
    const uint32_t next = r.U32(); // everything before this arrived
    const uint64_t bits = r.U64(); // bit i: next + 1 + i arrived
    const uint32_t newest = r.U32(); // the packet that made the peer send this ack
    if (r.Failed())
    {
        return;
    }
    const uint32_t now = NowMs();

    // A round-trip sample, from the packet this ack answers, if it was sent only once (Karn). Other
    // packets it acknowledges may have waited for it, so they would measure the gap between packets.
    auto sampled = mUnacked.find(newest);
    if (sampled != mUnacked.end() && sampled->second.sends == 1)
    {
        const uint32_t sample = now - sampled->second.firstSent;
        if (mSrtt == 0)
        {
            mSrtt = std::max<uint32_t>(sample, 1);
            mRttVar = sample / 2;
        }
        else
        {
            const uint32_t diff = sample > mSrtt ? sample - mSrtt : mSrtt - sample;
            mRttVar = (3 * mRttVar + diff) / 4;
            mSrtt = std::max<uint32_t>((7 * mSrtt + sample) / 8, 1);
        }
    }

    uint32_t highestSacked = next;
    for (auto it = mUnacked.begin(); it != mUnacked.end();)
    {
        const int32_t before = static_cast<int32_t>(it->first - next);
        if (before < 0 || (before >= 1 && before <= 64 && ((bits >> (before - 1)) & 1)))
        {
            if (before > 0)
            {
                highestSacked = it->first;
            }
            it = mUnacked.erase(it);
        }
        else
        {
            ++it;
        }
    }
    if (bits)
    {
        highestSacked = std::max(highestSacked, next + 64 - static_cast<uint32_t>(__builtin_clzll(bits)));
    }

    // Fast retransmit: the peer has later packets but not these, so they were lost. Resend them now
    // instead of waiting for the timeout, but not again within half a round trip.
    const uint32_t again = std::max<uint32_t>(mSrtt / 2, 5);
    for (auto& kv : mUnacked)
    {
        if (static_cast<int32_t>(kv.first - highestSacked) >= 0)
        {
            break;
        }
        Outgoing& out = kv.second;
        if (now - out.lastSent >= again)
        {
            SendRaw(mSocket, mPeer, out.packet);
            out.lastSent = now;
            out.sends++;
            mLastSent = now;
        }
    }
}

void Link::SendAck()
{
    uint64_t bits = 0;
    for (const auto& kv : mOutOfOrder)
    {
        const uint32_t ahead = kv.first - mNextRecvSeq;
        if (ahead >= 1 && ahead <= 64)
        {
            bits |= 1ull << (ahead - 1);
        }
    }
    Writer w;
    BeginPacket(w, kPacketAck);
    w.U32(mId);
    w.U32(mNextRecvSeq);
    w.U64(bits);
    w.U32(mNewestReceived);
    SendRaw(mSocket, mPeer, w.Data());
    mAckPending = false;
}

void Link::Update(uint32_t now)
{
    if (mAckPending)
    {
        SendAck();
        mLastSent = now;
    }

    // Resend after a retransmission timeout from the measured round trip, backing off per resend.
    // Kept short: every machine waits for every input of a frame, so a lost one stalls the game for
    // as long as this, and a needless resend costs only a few bytes.
    uint32_t rto = mSrtt ? mSrtt + std::max<uint32_t>(4 * mRttVar, 10) : 200;
    rto = std::max<uint32_t>(10, std::min<uint32_t>(rto, 1000));
    int sentNow = 0;
    for (auto& kv : mUnacked)
    {
        Outgoing& out = kv.second;
        const uint32_t wait = std::min<uint32_t>(rto << std::min(out.sends - 1, 4), 2000);
        if (now - out.lastSent >= wait)
        {
            SendRaw(mSocket, mPeer, out.packet);
            out.lastSent = now;
            out.sends++;
            mLastSent = now;
            if (++sentNow >= 64) // don't flood after a long stall; the rest go on the next round
            {
                break;
            }
        }
    }

    if (now - mLastSent >= kKeepAliveMs)
    {
        Writer w;
        BeginPacket(w, kPacketKeepAlive);
        w.U32(mId);
        SendRaw(mSocket, mPeer, w.Data());
        mLastSent = now;
    }
}

bool Link::PopMessage(std::vector<uint8_t>& out)
{
    if (mReceived.empty())
    {
        return false;
    }
    out = std::move(mReceived.front());
    mReceived.pop_front();
    return true;
}

void Link::SendDisconnect()
{
    Writer w;
    BeginPacket(w, kPacketDisconnect);
    w.U32(mId);
    // Best effort: a few copies, since nobody acknowledges this one
    for (int i = 0; i < 3; i++)
    {
        mSocket.SendTo(mPeer, w.Data().data(), w.Size());
    }
}
} // namespace net

namespace net
{
std::string Link::Describe(uint32_t now) const
{
    std::string s =
        "link " + mPeer.ToString() + ": sent up to " + std::to_string(mNextSendSeq) + ", " + std::to_string(mUnacked.size()) + " unacked";
    if (!mUnacked.empty())
    {
        const Outgoing& o = mUnacked.begin()->second;
        s += " (oldest " + std::to_string(mUnacked.begin()->first) + ", sent " + std::to_string(o.sends) + "x, last " +
            std::to_string(now - o.lastSent) + " ms ago)";
    }
    s += "; received up to " + std::to_string(mNextRecvSeq) + ", " + std::to_string(mOutOfOrder.size()) + " waiting, " +
        std::to_string(mReceived.size()) + " unread; heard " + std::to_string(now - mLastHeard) + " ms ago, rtt " + std::to_string(mSrtt);
    return s;
}
} // namespace net
