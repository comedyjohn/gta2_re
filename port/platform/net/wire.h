// The network protocol's byte layout. Every field is written explicitly, little-endian and fixed-size,
// so machines of any architecture or pointer size read each other's packets the same way. Nothing in
// here knows about the game or DirectPlay: see net.h for what travels inside.
#pragma once
#include <stdint.h>
#include <string.h>

#include <string>
#include <vector>

namespace net
{
// Bumped whenever a packet or message changes shape. Machines with different versions refuse each
// other with a message saying so, instead of misreading packets.
const uint8_t kProtocolVersion = 1;
const uint16_t kDefaultPort = 2302;
// Payload of one datagram. Kept under the IPv6 minimum MTU (1280) so nothing is fragmented
const size_t kMaxPayload = 1150;

// The first byte after the 'G' '2' magic and the version
enum PacketType : uint8_t
{
    kPacketDiscover = 1, // "which sessions do you have?" (broadcast or direct)
    kPacketDiscoverReply = 2,
    kPacketConnect = 3, // open a link: carries the build's compatibility id
    kPacketConnected = 4,
    kPacketRefused = 5, // carries the reason as text
    kPacketData = 6, // reliable, ordered: link id, sequence number, one message
    kPacketAck = 7,
    kPacketKeepAlive = 8,
    kPacketDisconnect = 9,
};

class Writer
{
  public:
    void U8(uint8_t v)
    {
        mBytes.push_back(v);
    }
    void U16(uint16_t v)
    {
        U8(v & 0xFF);
        U8(v >> 8);
    }
    void U32(uint32_t v)
    {
        U16(v & 0xFFFF);
        U16(v >> 16);
    }
    void U64(uint64_t v)
    {
        U32(static_cast<uint32_t>(v));
        U32(static_cast<uint32_t>(v >> 32));
    }
    void Bytes(const void* data, size_t size)
    {
        const uint8_t* p = static_cast<const uint8_t*>(data);
        mBytes.insert(mBytes.end(), p, p + size);
    }
    // A length-prefixed block (u16 length)
    void Blob(const void* data, size_t size)
    {
        U16(static_cast<uint16_t>(size));
        Bytes(data, size);
    }
    void Blob(const std::vector<uint8_t>& v)
    {
        Blob(v.data(), v.size());
    }
    // UTF-16 text, length-prefixed in code units
    void Text(const std::u16string& s)
    {
        U16(static_cast<uint16_t>(s.size()));
        for (char16_t c : s)
        {
            U16(c);
        }
    }
    void Str(const std::string& s)
    {
        Blob(s.data(), s.size());
    }
    void Guid(const uint8_t guid[16])
    {
        Bytes(guid, 16);
    }

    const std::vector<uint8_t>& Data() const
    {
        return mBytes;
    }
    std::vector<uint8_t>& Data()
    {
        return mBytes;
    }
    size_t Size() const
    {
        return mBytes.size();
    }

  private:
    std::vector<uint8_t> mBytes;
};

// Reads what Writer wrote. Running past the end sets Failed() and returns zeros, so a short or
// malformed packet is simply dropped by whoever checks Failed() at the end.
class Reader
{
  public:
    Reader(const uint8_t* data, size_t size) : mData(data), mSize(size)
    {
    }

    uint8_t U8()
    {
        if (!Need(1))
        {
            return 0;
        }
        return mData[mPos++];
    }
    uint16_t U16()
    {
        uint16_t lo = U8();
        return static_cast<uint16_t>(lo | (U8() << 8));
    }
    uint32_t U32()
    {
        uint32_t lo = U16();
        return lo | (static_cast<uint32_t>(U16()) << 16);
    }
    uint64_t U64()
    {
        uint64_t lo = U32();
        return lo | (static_cast<uint64_t>(U32()) << 32);
    }
    bool Bytes(void* out, size_t size)
    {
        if (!Need(size))
        {
            memset(out, 0, size);
            return false;
        }
        memcpy(out, mData + mPos, size);
        mPos += size;
        return true;
    }
    std::vector<uint8_t> Blob()
    {
        const size_t size = U16();
        std::vector<uint8_t> v;
        if (Need(size))
        {
            v.assign(mData + mPos, mData + mPos + size);
            mPos += size;
        }
        return v;
    }
    std::u16string Text()
    {
        const size_t size = U16();
        std::u16string s;
        if (!Need(size * 2))
        {
            return s;
        }
        s.reserve(size);
        for (size_t i = 0; i < size; i++)
        {
            s.push_back(static_cast<char16_t>(U16()));
        }
        return s;
    }
    std::string Str()
    {
        std::vector<uint8_t> v = Blob();
        return std::string(v.begin(), v.end());
    }
    void Guid(uint8_t out[16])
    {
        Bytes(out, 16);
    }

    bool Failed() const
    {
        return mFailed;
    }
    size_t Remaining() const
    {
        return mFailed ? 0 : mSize - mPos;
    }
    const uint8_t* Here() const
    {
        return mData + mPos;
    }

  private:
    bool Need(size_t n)
    {
        if (mFailed || mSize - mPos < n)
        {
            mFailed = true;
            return false;
        }
        return true;
    }

    const uint8_t* mData;
    size_t mSize;
    size_t mPos = 0;
    bool mFailed = false;
};

// Starts a packet: magic, version and type
inline void BeginPacket(Writer& w, PacketType type)
{
    w.U8('G');
    w.U8('2');
    w.U8(kProtocolVersion);
    w.U8(type);
}

// Checks the magic. Returns false for anything that isn't ours. The version and type are returned
// for the caller to judge: a packet of another version is answered (with a refusal) rather than read.
inline bool ReadPacketHeader(Reader& r, uint8_t& version, uint8_t& type)
{
    const uint8_t g = r.U8();
    const uint8_t two = r.U8();
    version = r.U8();
    type = r.U8();
    return !r.Failed() && g == 'G' && two == '2';
}
} // namespace net
