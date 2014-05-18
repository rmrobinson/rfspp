#include "Directory.hpp"

#include "Message.pb.h"

#include "FileSystem.hpp"

using namespace rfs;

Directory::Directory ( FileSystem& fs, const std::string& name ) : Node ( fs, name )
{
}

RetCode Directory::getChildren ( std::vector<std::string>& nodes )
{
    if ( fid_ < 0 )
    {
        return NotOpen;
    }

    proto::ReadMsg msg;
    msg.set_fid ( fid_ );
    // we want all the entries
    msg.set_size ( INT32_MAX );
    msg.set_offset ( 0 );

    proto::DirectoryMsg resp;

    RetCode rc = fs_.sendMessage ( msg, resp );

    if ( rc != Success )
    {
        return rc;
    }

    if ( resp.fid() != fid_ )
    {
        return InvalidMessage;
    }

    nodes.clear();
    /// @todo Iterate over the list of metadata entries

    return Success;
}

RetCode Directory::addChild ( const std::string& name )
{
    if ( fid_ < 0 )
    {
        return NotOpen;
    }

    Metadata msg;

    Metadata::Modes* modes = msg.mutable_modes();
    assert ( modes != 0 );

    Metadata::Modes::Values* user = modes->mutable_user();
    assert ( user != 0 );
    user->set_read ( true );
    user->set_write ( true );
    user->set_execute ( false );

    Metadata::Modes::Values* group = modes->mutable_group();
    assert ( group != 0 );
    group->set_read ( true );
    group->set_write ( true );
    group->set_execute ( false );

    Metadata::Modes::Values* other = modes->mutable_other();
    assert ( other != 0 );
    other->set_read ( true );
    other->set_write ( true );
    other->set_execute ( false );

    std::string fullName = getName();
    fullName.append ( "/" );
    fullName.append ( name );

    msg.set_size ( 0 );
    msg.set_links ( 0 );

    return fs_.sendMessage ( msg );
}

