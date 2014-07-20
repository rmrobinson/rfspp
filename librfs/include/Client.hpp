#pragma once

#include <string>

#include "Rfs.pb.h"

#include "RetCode.hpp"

namespace rfs
{

class Client
{
public:
    Client();
    ~Client();

    RetCode open ( uint32_t& hd );

    RetCode close ( uint32_t hd );

    RetCode read ( uint32_t hd, std::vector<char>& data, off_t offset = 0 );

    RetCode write ( uint32_t hd, const std::vector<char>& data, off_t offset = 0 );

    RetCode readdir ( const std::string& path, std::vector<Metadata>& entries );

    RetCode stat ( const std::string& path, Metadata& md );

private:
    RetCode connect();
    void disconnect();
    RetCode execCmd ( const proto::RfsMsg& cmd, proto::RfsMsg& resp );

    inline bool isConnected() const
    {
        return ( fd_ >= 0 );
    }

    int fd_;
};

}

