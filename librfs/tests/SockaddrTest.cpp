
extern "C"
{
#include <ifaddrs.h>
}

#include <iostream>

#include "SocketAddress.hpp"

using namespace rfs;

int main()
{
    struct ifaddrs *ifaddrs = nullptr;

    getifaddrs ( &ifaddrs );

    for ( struct ifaddrs* ifaddr = ifaddrs; ifaddr != nullptr; ifaddr = ifaddr->ifa_next )
    {
        std::cout << "-- Interface (" << ifaddr->ifa_name << ") --" << std::endl;

        SocketAddress addr ( ifaddr->ifa_addr, ifaddr->ifa_addr->sa_len );
        SocketAddress mask ( ifaddr->ifa_netmask, ifaddr->ifa_addr->sa_len );
        SocketAddress bcast ( ifaddr->ifa_broadaddr, ifaddr->ifa_addr->sa_len );
        SocketAddress dst ( ifaddr->ifa_dstaddr, ifaddr->ifa_addr->sa_len );

        if ( addr.getType() != SocketAddress::Unspecified )
            std::cout << "  Address: " << addr << std::endl;
        if ( mask.getType() != SocketAddress::Unspecified )
            std::cout << "  Netmask: " << mask << std::endl;
        if ( bcast.getType() != SocketAddress::Unspecified )
            std::cout << "  Broadcast: " << bcast << std::endl;
        if ( dst.getType() != SocketAddress::Unspecified )
            std::cout << "  Destination: " << dst << std::endl;
    }

    freeifaddrs ( ifaddrs );

    return EXIT_SUCCESS;
}

