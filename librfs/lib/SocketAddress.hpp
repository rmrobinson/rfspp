#pragma once

extern "C"
{
#include <sys/types.h>
#include <sys/socket.h>
}

#include <cstdint>
#include <ostream>
#include <string>

#include "IpAddress.hpp"

namespace rfs
{

class SocketAddress
{
public:
    enum Type
    {
        Unspecified = AF_UNSPEC,
        Internet = AF_INET,
        Internet6 = AF_INET6,
        Local = AF_LOCAL,
        Datalink = AF_LINK
    };

    struct InetAddr
    {
        IpAddress addr;
        uint16_t port;

        /// @brief The IPv6 scope ID; only set for IPv6 addresses.
        uint32_t scopeId;

        uint32_t flowInfo;
    };

    struct LocalAddr
    {
        std::string path;
    };

    struct DatalinkAddr
    {
        uint16_t index;
        uint8_t type;
        std::string name;
        std::string addr;
    };

    SocketAddress();

    SocketAddress ( const struct sockaddr* sa, socklen_t saLen );

    bool parse ( Type t, const std::string& str );

    Type getType() const;

    InetAddr getInetAddress() const;
    LocalAddr getLocalAddress() const;
    DatalinkAddr getDatalinkAddress() const;

    socklen_t socklen() const;

    const struct sockaddr* sa() const;

    struct sockaddr* sa();

    void set ( const struct sockaddr* sa, socklen_t saLen );

private:
    struct sockaddr_storage sa_;
};

}

std::ostream& operator<< ( std::ostream& out, const rfs::SocketAddress& sa );

std::ostream& operator<< ( std::ostream& out, const rfs::SocketAddress::InetAddr& ia );

std::ostream& operator<< ( std::ostream& out, const rfs::SocketAddress::LocalAddr& la );

std::ostream& operator<< ( std::ostream& out, const rfs::SocketAddress::DatalinkAddr& da );

