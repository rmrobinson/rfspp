extern "C"
{
#include <arpa/inet.h>
}

#include "WireHeader.hpp"

using namespace rfs;

void WireHeader::reset()
{
    size = 0;
}

bool WireHeader::serialize ( char* data, size_t dataSize ) const
{
    if ( dataSize < sizeof ( WireHeader ) )
        return false;

    memcpy ( data, this, sizeof ( WireHeader ) );
    return true;
}

bool WireHeader::deserialize ( const std::vector<char>& data )
{
    if ( data.size() < sizeof ( WireHeader ) )
        return false;

    const WireHeader* tmp = reinterpret_cast<const WireHeader*> ( &data[0] );
    size = ntohl ( tmp->size );

    return true;
}

