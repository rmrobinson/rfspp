#pragma once

#include <string>

#include "Rfs.pb.h"

namespace rfs
{

class Client
{
public:
    Client();
    ~Client();

    RetCode start();

    RetCode open ( Handle& hd );

    RetCode close ( uint32_t hd );

    RetCode read ( uint32_t hd, std::vector<char>& data, off_t offset = 0 );

    RetCode write ( uint32_t hd, const std::vector<char>& data, off_t offset = 0 );

    RetCode readdir ( const std::string& path, std::vector<Metadata>& entries );

    RetCode stat ( const std::string& path, Metadata& md );

private:
    RetCode doOperation ( const proto::RfsMsg& cmd, proto::RfsMsg& resp );

    int fd_;
};

}

