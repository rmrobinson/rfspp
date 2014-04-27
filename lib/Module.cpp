#include "Module.hpp"

#include "FileSystem.hpp"

using namespace rfs;

Module::Module ( FileSystem& fs, const std::string& name, int mode ) : fs_ ( fs ), name_ ( name ), mode_ ( mode )
{
    fs_.addModule ( *this );
}

Module::~Module()
{
    fs_.removeModule ( *this );
}

proto::RetCode Module::open ( const FileHandle& )
{
    /// @todo Log something useful
    return proto::Success;
}

proto::RetCode Module::close ( const FileHandle& )
{
    /// @todo Log something useful
    return proto::Success;
}

