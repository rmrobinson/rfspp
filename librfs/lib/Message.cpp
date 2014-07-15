#include "Message.hpp"

#include <cassert>

using namespace rfs;

Message::Header::Header() : size ( 0 ), type ( 0 ), encoding ( 0 )
{
}

Message::Message() 
{
    data_.resize ( sizeof ( Header ) );
}

Message::Message ( uint16_t type, uint16_t encoding, const std::vector<char>& data )
{
    data_.resize ( sizeof ( Header ) );

    Header* hdrPtr = reinterpret_cast<Header*> ( &data_[0] );
    hdrPtr->size = htonl ( data.size() + sizeof ( Header ) );
    hdrPtr->type = htons ( type );
    hdrPtr->encoding = htons ( encoding );

    data_.insert ( data_.end(), data.begin(), data.end() );
}

const char* Message::get() const
{
    return &data_[0];
}

size_t Message::size() const
{
    return data_.size();
}

Message::Header Message::header() const
{
    const Header* hdrPtr = reinterpret_cast<const Header*> ( &data_[0] );

    Header hdr;
    hdr.size = ntohl ( hdrPtr->size );
    hdr.type = ntohs ( hdrPtr->type );
    hdr.encoding = ntohs ( hdrPtr->encoding );

    return hdr;
}

char* Message::header()
{
    return &data_[0];
}

const std::vector<char> Message::payload() const
{
    return std::vector<char> ( data_.begin() + sizeof ( Header ), data_.end() );
}

char* Message::payload()
{
    return &data_[sizeof ( Header )];
}

size_t Message::payloadSize() const
{
    return ( data_.size() - sizeof ( Header ) );
}

bool Message::decodeHeader()
{
    if ( data_.size() < sizeof ( Header ) )
    {
        return false;
    }

    const Header* hdrPtr = reinterpret_cast<const Header*> ( &data_[0] );

    data_.resize ( ntohl ( hdrPtr->size ) + sizeof ( Header ) );

    return true;
}

