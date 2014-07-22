#include "IpAddress.hpp"

extern "C"
{
#include <arpa/inet.h>
}

#include <algorithm>
#include <cassert>

using namespace rfs;

IpAddress::IpAddress() : t_ ( Empty )
{
    memset ( &addr_, 0, sizeof ( addr_ ) );
}

IpAddress::IpAddress ( struct in_addr ia ) : t_ ( V4 )
{
    addr_.v4.s_addr = ia.s_addr;
}

IpAddress::IpAddress ( const struct in6_addr& i6a ) : t_ ( V6 )
{
    memcpy ( &addr_.v6, &i6a, sizeof ( addr_.v6 ) );
}

bool IpAddress::parse ( const std::string& str )
{
    // IPv6 addresses must have at least one : in them
    if ( str.find ( ":" ) != std::string::npos )
    {
        std::string addr ( str );

        // IPv6 addresses may be enclosed within square braces to distinguish
        // the address separating colons from the port separating colon
        // remove them since inet_pton doesn't handle them
        if ( addr.front() == '[' && addr.back() == ']' )
        {
            addr.pop_back();
            addr = addr.substr ( 1 );
        }

        const size_t modIdx = addr.find ( "%" );
        if ( modIdx != std::string::npos )
        {
            // remove the embedded scope ID from any IPv6 address with it specified
            addr = addr.substr ( 0, modIdx );
        }

        return ( inet_pton ( AF_INET6, addr.c_str(), & ( addr_.v6 ) ) == 1 );
    }

    return ( inet_pton ( AF_INET, str.c_str(), & ( addr_.v4 ) ) == 1 );
}

bool IpAddress::operator== ( const IpAddress& rhs ) const
{
    if ( t_ != rhs.t_ )
        return false;

    if ( t_ == V4 )
        return ( addr_.v4.s_addr == rhs.addr_.v4.s_addr );
    else if ( t_ == V6 )
        return IN6_ARE_ADDR_EQUAL ( & ( addr_.v6 ), & ( rhs.addr_.v6 ) );
    else
        return true; // both are empty, so equal.
}

bool IpAddress::isV4() const
{
    return ( t_ == V4 );
}

bool IpAddress::isV6() const
{
    return ( t_ == V6 );
}

bool IpAddress::isValid() const
{
    return ( t_ != Empty );
}

bool IpAddress::isLoopback() const
{
    if ( t_ == V4 )
        return IN_LOOPBACK ( addr_.v4.s_addr );
    else if ( t_ == V6 )
        return IN6_IS_ADDR_LOOPBACK ( & ( addr_.v6 ) );
    else
        return false;
}

bool IpAddress::isMulticast() const
{
    if ( t_ == V4 )
        return IN_MULTICAST ( addr_.v4.s_addr );
    else if ( t_ == V6 )
        return IN6_IS_ADDR_MULTICAST ( & ( addr_.v6 ) );
    else
        return false;
}

bool IpAddress::isLinkLocal() const
{
    if ( t_ == V6 )
        return IN6_IS_ADDR_LINKLOCAL ( &( addr_.v6 ) );
    else
        return false;
}

const struct in_addr* IpAddress::v4() const
{
    assert ( t_ == V4 );

    return & ( addr_.v4 );
}

const struct in6_addr* IpAddress::v6() const
{
    assert ( t_ == V6 );

    return & ( addr_.v6 );
}

std::ostream& operator<< ( std::ostream& out, const rfs::IpAddress& ip )
{
    char tmp[INET6_ADDRSTRLEN];

    if ( ip.isV4() )
    {
        inet_ntop ( AF_INET, ip.v4(), tmp, INET_ADDRSTRLEN );
        out << tmp;
    }
    else if ( ip.isV6() )
    {
        inet_ntop ( AF_INET6, ip.v6(), tmp, INET6_ADDRSTRLEN );
        out << tmp;
    }
    else
    {
        out << "Empty";
    }

    return out;
}

