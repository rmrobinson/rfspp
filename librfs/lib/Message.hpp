#pragma once

#include <vector>

namespace rfs
{

class Message
{
public:
    struct Header
    {
        uint32_t size; ///< Size of the message; including the header.
        uint16_t type; ///< Optional message type.
        uint16_t encoding; ///< Optional message encoding.

        Header();
    };

    Message();
    Message ( uint16_t type, uint16_t encoding, const std::vector<char>& data );

    void clear();

    const char* get() const;
    size_t size() const;

    Header header() const;

    char* header();

    const std::vector<char> payload() const;

    char* payload();
    size_t payloadSize() const;

    bool decodeHeader();

private:
    std::vector<char> data_; ///< The entire message; header (may be empty if encode() not called) and payload.

};

}

