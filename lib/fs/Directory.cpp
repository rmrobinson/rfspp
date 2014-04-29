#include "Directory.hpp"

#include "Client.pb.h"

#include "FileSystem.hpp"

using namespace rfs;

Directory::Directory ( FileSystem& fs, const std::string& name ) : Node ( fs, name )
{
}

proto::RetCode Directory::getChildren ( std::vector<std::string>& nodes )
{
    if ( fid_ < 0 )
    {
        return proto::NotOpen;
    }

    proto::ReadMsg msg;
    msg.set_fid ( fid_ );
    // we want all the entries
    msg.set_size ( INT32_MAX );
    msg.set_offset ( 0 );

    proto::DirectoryMsg resp;

    proto::RetCode rc = fs_.sendMessage ( msg, resp );

    if ( rc != proto::Success )
    {
        return rc;
    }

    if ( resp.fid() != fid_ )
    {
        return proto::InvalidMessage;
    }

    nodes.clear();
    /// @todo Iterate over the list of metadata entries

    return proto::Success;
}

