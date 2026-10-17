#include "socket.h"

#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <netdb.h>
#include <netinet/in.h>
#include <poll.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <algorithm>

namespace net
{
uint32_t NowMs()
{
    timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return static_cast<uint32_t>(ts.tv_sec * 1000ULL + ts.tv_nsec / 1000000);
}

bool Address::IsV6() const
{
    return storage.ss_family == AF_INET6;
}

uint16_t Address::Port() const
{
    if (IsV6())
    {
        return ntohs(reinterpret_cast<const sockaddr_in6*>(&storage)->sin6_port);
    }
    return ntohs(reinterpret_cast<const sockaddr_in*>(&storage)->sin_port);
}

void Address::SetPort(uint16_t port)
{
    if (IsV6())
    {
        reinterpret_cast<sockaddr_in6*>(&storage)->sin6_port = htons(port);
    }
    else
    {
        reinterpret_cast<sockaddr_in*>(&storage)->sin_port = htons(port);
    }
}

std::string Address::ToString() const
{
    char host[INET6_ADDRSTRLEN] = "?";
    if (IsV6())
    {
        inet_ntop(AF_INET6, &reinterpret_cast<const sockaddr_in6*>(&storage)->sin6_addr, host, sizeof(host));
        return std::string("[") + host + "]:" + std::to_string(Port());
    }
    inet_ntop(AF_INET, &reinterpret_cast<const sockaddr_in*>(&storage)->sin_addr, host, sizeof(host));
    return std::string(host) + ":" + std::to_string(Port());
}

bool Address::operator==(const Address& other) const
{
    if (storage.ss_family != other.storage.ss_family || Port() != other.Port())
    {
        return false;
    }
    if (IsV6())
    {
        return memcmp(&reinterpret_cast<const sockaddr_in6*>(&storage)->sin6_addr,
                      &reinterpret_cast<const sockaddr_in6*>(&other.storage)->sin6_addr,
                      sizeof(in6_addr)) == 0;
    }
    return reinterpret_cast<const sockaddr_in*>(&storage)->sin_addr.s_addr ==
        reinterpret_cast<const sockaddr_in*>(&other.storage)->sin_addr.s_addr;
}

bool Address::operator<(const Address& other) const
{
    return ToString() < other.ToString();
}

Address Address::Loopback(uint16_t port)
{
    Address a;
    sockaddr_in* in = reinterpret_cast<sockaddr_in*>(&a.storage);
    in->sin_family = AF_INET;
    in->sin_port = htons(port);
    in->sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    a.length = sizeof(sockaddr_in);
    return a;
}

bool Resolve(const std::string& textIn, uint16_t defaultPort, Address& out, std::string& error)
{
    std::string host = textIn;
    std::string port = std::to_string(defaultPort);

    // Trim spaces
    while (!host.empty() && isspace(static_cast<unsigned char>(host.back())))
    {
        host.pop_back();
    }
    while (!host.empty() && isspace(static_cast<unsigned char>(host.front())))
    {
        host.erase(0, 1);
    }

    if (!host.empty() && host[0] == '[')
    {
        // [v6]:port
        const size_t close = host.find(']');
        if (close == std::string::npos)
        {
            error = "missing ] in " + textIn;
            return false;
        }
        if (close + 1 < host.size() && host[close + 1] == ':')
        {
            port = host.substr(close + 2);
        }
        host = host.substr(1, close - 1);
    }
    else if (std::count(host.begin(), host.end(), ':') == 1)
    {
        // host:port (more than one colon is a bare IPv6 address)
        const size_t colon = host.find(':');
        port = host.substr(colon + 1);
        host = host.substr(0, colon);
    }
    if (host.empty())
    {
        error = "no address given";
        return false;
    }

    addrinfo hints {};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    addrinfo* result = nullptr;
    const int rc = getaddrinfo(host.c_str(), port.c_str(), &hints, &result);
    if (rc != 0 || !result)
    {
        error = "can't find " + textIn + " (" + gai_strerror(rc) + ")";
        return false;
    }
    // Prefer IPv4: it is what a LAN game and a forwarded port most likely use
    const addrinfo* pick = result;
    for (const addrinfo* ai = result; ai; ai = ai->ai_next)
    {
        if (ai->ai_family == AF_INET)
        {
            pick = ai;
            break;
        }
    }
    out = Address();
    memcpy(&out.storage, pick->ai_addr, pick->ai_addrlen);
    out.length = pick->ai_addrlen;
    freeaddrinfo(result);
    return true;
}

std::vector<Address> BroadcastAddresses(uint16_t port)
{
    std::vector<Address> out;
    Address all;
    sockaddr_in* in = reinterpret_cast<sockaddr_in*>(&all.storage);
    in->sin_family = AF_INET;
    in->sin_port = htons(port);
    in->sin_addr.s_addr = htonl(INADDR_BROADCAST);
    all.length = sizeof(sockaddr_in);
    out.push_back(all);

    ifaddrs* list = nullptr;
    if (getifaddrs(&list) == 0)
    {
        for (ifaddrs* ifa = list; ifa; ifa = ifa->ifa_next)
        {
            if (!ifa->ifa_addr || ifa->ifa_addr->sa_family != AF_INET || !(ifa->ifa_flags & IFF_BROADCAST) || !(ifa->ifa_flags & IFF_UP) ||
                !ifa->ifa_broadaddr)
            {
                continue;
            }
            Address a;
            memcpy(&a.storage, ifa->ifa_broadaddr, sizeof(sockaddr_in));
            a.length = sizeof(sockaddr_in);
            a.SetPort(port);
            if (std::find(out.begin(), out.end(), a) == out.end())
            {
                out.push_back(a);
            }
        }
        freeifaddrs(list);
    }
    // And this machine itself, for a second copy of the game on the same computer
    Address local = Address::Loopback(port);
    out.push_back(local);
    return out;
}

UdpSocket::~UdpSocket()
{
    Close();
}

static int OpenOne(int family, uint16_t port, std::string& error)
{
    const int fd = socket(family, SOCK_DGRAM, 0);
    if (fd < 0)
    {
        error = strerror(errno);
        return -1;
    }
    fcntl(fd, F_SETFD, FD_CLOEXEC);
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
    int one = 1;
    if (family == AF_INET)
    {
        setsockopt(fd, SOL_SOCKET, SO_BROADCAST, &one, sizeof(one));
        sockaddr_in addr {};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
        if (bind(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0)
        {
            error = "UDP port " + std::to_string(port) + ": " + strerror(errno);
            close(fd);
            return -1;
        }
    }
    else
    {
        setsockopt(fd, IPPROTO_IPV6, IPV6_V6ONLY, &one, sizeof(one));
        sockaddr_in6 addr {};
        addr.sin6_family = AF_INET6;
        addr.sin6_port = htons(port);
        addr.sin6_addr = in6addr_any;
        if (bind(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0)
        {
            error = "UDP port " + std::to_string(port) + " (IPv6): " + strerror(errno);
            close(fd);
            return -1;
        }
    }
    return fd;
}

static uint16_t BoundPort(int fd)
{
    sockaddr_storage ss {};
    socklen_t len = sizeof(ss);
    getsockname(fd, reinterpret_cast<sockaddr*>(&ss), &len);
    if (ss.ss_family == AF_INET6)
    {
        return ntohs(reinterpret_cast<sockaddr_in6*>(&ss)->sin6_port);
    }
    return ntohs(reinterpret_cast<sockaddr_in*>(&ss)->sin_port);
}

bool UdpSocket::Open(uint16_t port, std::string& error)
{
    Close();
    mFd4 = OpenOne(AF_INET, port, error);
    if (mFd4 < 0)
    {
        return false;
    }
    mPort = BoundPort(mFd4);
    // IPv6 on the same port, if the system has it; a game without it still works over IPv4
    std::string ignored;
    mFd6 = OpenOne(AF_INET6, mPort, ignored);
    return true;
}

void UdpSocket::Close()
{
    if (mFd4 >= 0)
    {
        close(mFd4);
    }
    if (mFd6 >= 0)
    {
        close(mFd6);
    }
    mFd4 = mFd6 = -1;
    mPort = 0;
}

bool UdpSocket::SendTo(const Address& to, const void* data, size_t size)
{
    const int fd = to.IsV6() ? mFd6 : mFd4;
    if (fd < 0)
    {
        return false;
    }
    const ssize_t sent = sendto(fd, data, size, MSG_NOSIGNAL, reinterpret_cast<const sockaddr*>(&to.storage), to.length);
    return sent == static_cast<ssize_t>(size);
}

int UdpSocket::Receive(void* buffer, size_t size, Address& from, int timeoutMs)
{
    pollfd fds[2];
    int count = 0;
    if (mFd4 >= 0)
    {
        fds[count].fd = mFd4;
        fds[count].events = POLLIN;
        fds[count].revents = 0;
        count++;
    }
    if (mFd6 >= 0)
    {
        fds[count].fd = mFd6;
        fds[count].events = POLLIN;
        fds[count].revents = 0;
        count++;
    }
    if (count == 0)
    {
        return -1;
    }
    for (int attempt = 0; attempt < 2; attempt++)
    {
        for (int i = 0; i < count; i++)
        {
            from.length = sizeof(from.storage);
            const ssize_t got = recvfrom(fds[i].fd, buffer, size, 0, reinterpret_cast<sockaddr*>(&from.storage), &from.length);
            if (got >= 0)
            {
                return static_cast<int>(got);
            }
        }
        if (attempt == 0 && poll(fds, count, timeoutMs) <= 0)
        {
            break;
        }
    }
    from.length = 0;
    return -1;
}
} // namespace net
