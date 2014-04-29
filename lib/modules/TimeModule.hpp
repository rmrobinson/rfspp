#pragma once

#include "fs/Module.hpp"

namespace rfs
{

class TimeModule : public Module
{
public:
    TimeModule ( FileSystem& fs );

    virtual proto::RetCode read ( const FileHandle& fh, std::vector<char>& data,
                                  size_t size, size_t offset );
    virtual proto::RetCode write ( const FileHandle& fh, const std::vector<char>& data,
                                   size_t size, size_t offset );
    virtual size_t size();
};

}

