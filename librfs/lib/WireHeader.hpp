#pragma once

#include <vector>

namespace rfs
{

#pragma pack(push,1)
struct WireHeader
{
    uint32_t size;

    WireHeader() : size ( 0 ) {}

    void reset();

    bool serialize ( char* data, size_t dataSize ) const;
    bool deserialize ( const std::vector<char>& data );
};
#pragma pack(pop)

}

