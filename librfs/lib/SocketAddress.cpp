#include "SocketAddress.hpp"

extern "C"
{
#include <net/if_dl.h>
#include <netinet/in.h>
#include <sys/un.h>
}

#include <cassert>
#include <iomanip>
#include <sstream>
#include <string>

using namespace rfs;

SocketAddress::SocketAddress()
{
    memset ( &sa_, 0, sizeof ( sa_ ) );
}

SocketAddress::SocketAddress ( const struct sockaddr* sa, socklen_t saLen )
{
    memset ( &sa_, 0, sizeof ( sa_ ) );

    if ( sa != nullptr )
    {
        set ( sa, saLen );
    }
}

bool SocketAddress::parse ( Type t, const std::string& str )
{
    memset ( &sa_, 0, sizeof ( sa_ ) );

    switch ( t )
    {
        case Internet:
        {
            const size_t idx = str.find_last_of ( ":" );

            if ( idx == std::string::npos )
                return false;

            IpAddress addr;

            if ( ! addr.parse ( str.substr ( 0, idx ) ) || ! addr.isV4() )
                return false;

            assert ( addr.isV4() );
            const uint16_t port = std::stoi ( str.substr ( idx + 1 ) );

            struct sockaddr_in* sin = ( struct sockaddr_in* ) &sa_;
            sin->sin_family = AF_INET;
            sin->sin_port = htons ( port );
            sin->sin_addr.s_addr = addr.v4()->s_addr;

            return true;
        }

        case Internet6:
        {
            const size_t idx = str.find_last_of ( ":" );

            if ( idx == std::string::npos )
                return false;

            IpAddress addr;

            if ( ! addr.parse ( str.substr ( 0, idx ) ) || ! addr.isV6() )
                return false;

            assert ( addr.isV6() );
            const uint16_t port = std::stoi ( str.substr ( idx + 1 ) );

            struct sockaddr_in6* sin6 = ( struct sockaddr_in6* ) &sa_;
            sin6->sin6_family = AF_INET6;
            sin6->sin6_port = htons ( port );
            memcpy ( & ( sin6->sin6_addr ), addr.v6(), sizeof ( in6_addr ) );
            sin6->sin6_scope_id = 0;
            sin6->sin6_flowinfo = 0;

            return true;
        }

        case Local:
        {
            struct sockaddr_un* sun = ( struct sockaddr_un* ) &sa_;
            sun->sun_family = AF_LOCAL;
            strlcpy ( sun->sun_path, str.c_str(), sizeof ( sun->sun_path )  );

            return true;
        }

        case Datalink:
        {
            struct sockaddr_dl* sdl = ( struct sockaddr_dl* ) &sa_;
            sdl->sdl_family = AF_LINK;

            /// @todo Implement this further
        }

        case Unspecified:
        default:
            assert ( false );
            return false;
    }
}

void SocketAddress::set ( const struct sockaddr* sa, socklen_t saLen )
{
    assert ( sa != nullptr );
    assert ( saLen > 0 );
    assert ( saLen < sizeof ( sa_ ) );

    memcpy ( &sa_, sa, saLen );
}

socklen_t SocketAddress::socklen() const
{
    switch ( sa_.ss_family )
    {
        case Internet:
            return sizeof ( struct sockaddr_in );

        case Internet6:
            return sizeof ( struct sockaddr_in6 );

        case Local:
            return SUN_LEN ( ( struct sockaddr_un* ) &sa_ );

        case Datalink:
            return ( ( struct sockaddr_dl* ) &sa_ )->sdl_len;

        default:
            return 0;
    }
}

SocketAddress::Type SocketAddress::getType() const
{
    return static_cast<SocketAddress::Type> ( sa_.ss_family );
}

SocketAddress::InetAddr SocketAddress::getInetAddress() const
{
    assert ( sa_.ss_family == Internet || sa_.ss_family == Internet6 );

    InetAddr ret;

    if ( sa_.ss_family == Internet )
    {
        const struct sockaddr_in* sin = ( const struct sockaddr_in*) &sa_;
        ret.port = ntohs ( sin->sin_port );
        ret.addr = IpAddress ( sin->sin_addr );
    }
    else if ( sa_.ss_family == Internet6 )
    {
        const struct sockaddr_in6* sin6 = ( const struct sockaddr_in6* ) &sa_;
        ret.port = ntohs ( sin6->sin6_port );
        ret.addr = sin6->sin6_addr;
        ret.scopeId = sin6->sin6_scope_id;
        ret.flowInfo = sin6->sin6_flowinfo;
    }

    return ret;
}

SocketAddress::LocalAddr SocketAddress::getLocalAddress() const
{
    assert ( sa_.ss_family == Local );
    const struct sockaddr_un* sun = ( const struct sockaddr_un* ) &sa_;

    LocalAddr ret;
    ret.path = sun->sun_path;

    return ret;
}

SocketAddress::DatalinkAddr SocketAddress::getDatalinkAddress() const
{
    assert ( sa_.ss_family == Datalink );
    const struct sockaddr_dl* sdl = ( const struct sockaddr_dl* ) &sa_;

    DatalinkAddr ret;
    ret.index = sdl->sdl_index;
    ret.type = sdl->sdl_type;
    ret.name = std::string ( sdl->sdl_data, sdl->sdl_nlen );

    std::stringstream ss;

    for ( size_t i = 0; i < sdl->sdl_alen; ++i )
    {
        if ( i > 0 )
            ss << ":";

        ss << std::hex << std::setfill ( '0' ) << std::setw ( 2 )
            << static_cast<unsigned > ( ( *( LLADDR ( sdl ) + i ) & 0xff ) );
    }

    ret.addr = ss.str();

    return ret;
}

const struct sockaddr* SocketAddress::sa() const
{
    return ( const struct sockaddr* ) &sa_;
}

struct sockaddr* SocketAddress::sa()
{
    return ( struct sockaddr* ) &sa_;
}

std::ostream& operator<< ( std::ostream& out, const rfs::SocketAddress& sa )
{
    switch ( sa.getType() )
    {
        case rfs::SocketAddress::Internet:
        case rfs::SocketAddress::Internet6:
            out << sa.getInetAddress();
            break;

        case rfs::SocketAddress::Local:
            out << "Local: " << sa.getLocalAddress();
            break;

        case rfs::SocketAddress::Datalink:
            out << "Datalink: " << sa.getDatalinkAddress();
            break;

        case rfs::SocketAddress::Unspecified:
        default:
            out << "Unspecified socket address";
            break;
    }

    return out;
}

std::ostream& operator<< ( std::ostream& out, const rfs::SocketAddress::InetAddr& ia )
{
    if ( ia.addr.isV6() && ia.port > 0 )
    {
        out << "[" << ia.addr << "]";
    }
    else
    {
        out << ia.addr;
    }

    if ( ia.port > 0 )
    {
        out << ":" << std::to_string ( ia.port );
    }

    if ( ia.addr.isV6() )
    {
        out << " (Scope: " << std::to_string ( ia.scopeId ) << "; FlowInfo: "
            << std::to_string ( ia.flowInfo ) << ")";
    }

    return out;
}

std::ostream& operator<< ( std::ostream& out, const rfs::SocketAddress::LocalAddr& la )
{
    out << la.path;

    return out;
}

std::ostream& operator<< ( std::ostream& out, const rfs::SocketAddress::DatalinkAddr& da )
{
    out << da.name << " (ID: " << std::to_string ( da.index ) << "); Type: "
        << std::to_string ( da.type );

    if ( da.addr.length() > 0 )
    {
        out << "; Addr: " << da.addr;
    }

    return out;
}
