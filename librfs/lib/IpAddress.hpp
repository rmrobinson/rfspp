#pragma once

extern "C"
{
#include <netinet/in.h>
}

#include <ostream>
#include <string>

namespace rfs
{

class IpAddress
{
public:
    IpAddress();
    IpAddress ( struct in_addr ia );
    IpAddress ( const struct in6_addr& i6a );

    bool parse ( const std::string& str );

    bool operator== ( const IpAddress& rhs ) const;
    inline bool operator!= ( const IpAddress& rhs ) const
    {
        return ! operator== ( rhs );
    }

    bool isV4() const;
    bool isV6() const;

    bool isValid() const;
    bool isLoopback() const;
    bool isMulticast() const;

    // for IPv6
    bool isLinkLocal() const;

    const struct in_addr* v4() const;
    const struct in6_addr* v6() const;

private:
    enum
    {
        Empty = 1,
        V4 = 2,
        V6 = 3
    } t_;

    union
    {
        struct in_addr v4;
        struct in6_addr v6;
    } addr_;
};

}

std::ostream& operator<< ( std::ostream& out, const rfs::IpAddress& ip );
