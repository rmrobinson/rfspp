#pragma once

#include "fs/ProcessFile.hpp"

namespace rfs
{

class TimeProcessFile : public ProcessFile
{
public:
    TimeProcessFile ( ProcessFileSystem& fs );

    virtual RetCode read ( const FileHandle& fh, std::vector<char>& data,
                           off_t offset, size_t& processed );
    virtual RetCode write ( const FileHandle& fh, const std::vector<char>& data,
                            off_t offset, size_t& processed );
    virtual size_t size() const;
};

}

