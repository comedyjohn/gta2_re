// UDP over POSIX sockets, IPv4 and IPv6, for the network layer. No game or SDL types.
#pragma once
#include <stdint.h>

#include <string>
#include <vector>

#include <sys/socket.h>

namespace net
{
uint32_t NowMs(); // a monotonic clock

// Where a datagram came from or goes to
struct Address
{
    sockaddr_storage storage {};
    socklen_t length = 0;

    bool Valid() const
    {
        return length != 0;
    }
    bool IsV6() const;
    uint16_t Port() const;
    void SetPort(uint16_t port);
    std::string ToString() const; // "1.2.3.4:2302" or "[::1]:2302"
    bool operator==(const Address& other) const;
    bool operator!=(const Address& other) const
    {
        return !(*this == other);
    }
    bool operator<(const Address& other) const; // for maps

    static Address Loopback(uint16_t port);
};

// "host", "host:port", "1.2.3.4", "[::1]:2302", "2001:db8::1": a name or an address, with an
// optional port (defaultPort otherwise). Returns false with a message when it doesn't resolve.
bool Resolve(const std::string& text, uint16_t defaultPort, Address& out, std::string& error);

// The IPv4 broadcast addresses of this machine's interfaces, plus 255.255.255.255, at `port`
std::vector<Address> BroadcastAddresses(uint16_t port);

// One UDP endpoint with an IPv4 and, where the system has it, an IPv6 socket on the same port
class UdpSocket
{
  public:
    UdpSocket() = default;
    ~UdpSocket();
    UdpSocket(const UdpSocket&) = delete;
    UdpSocket& operator=(const UdpSocket&) = delete;

    // port 0 picks a free one. Fails (with a message) when the port is taken.
    bool Open(uint16_t port, std::string& error);
    void Close();
    bool IsOpen() const
    {
        return mFd4 >= 0 || mFd6 >= 0;
    }
    uint16_t LocalPort() const
    {
        return mPort;
    }

    // This system can send there (an IPv6 address needs IPv6)
    bool CanReach(const Address& to) const
    {
        return to.IsV6() ? mFd6 >= 0 : mFd4 >= 0;
    }
    bool SendTo(const Address& to, const void* data, size_t size);
    // Waits up to timeoutMs for a datagram. Returns its size, or -1 when nothing came.
    int Receive(void* buffer, size_t size, Address& from, int timeoutMs);

  private:
    int mFd4 = -1;
    int mFd6 = -1;
    uint16_t mPort = 0;
};
} // namespace net
