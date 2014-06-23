#include "ProcessDirectory.hpp"

using namespace rfs;

ProcessDirectory::ProcessDirectory ( ProcessFileSystem& fs,
                                     const std::string& path )
    : fs_ ( fs ), path_ ( path )
{
    fs_.addDirectory ( *this );
}

ProcessDirectory::~ProcessDirectory()
{
    fs_.removePath ( getPath() );
}

RetCode ProcessDirectory::createFile ( const std::string& )
{
    /// @todo Log something useful
    return NotImplemented;
}

