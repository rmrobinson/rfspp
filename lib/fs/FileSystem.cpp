#include "FileSystem.hpp"

#include <cassert>

#include "Message.pb.h"

#include "Module.hpp"
#include "Node.hpp"

using namespace rfs;

/// @todo initialize the boost uuid to something useful
FileSystem::FileSystem() : nextFid_ ( 0 )
{
}

FileSystem::~FileSystem()
{
}

void FileSystem::addModule ( Module& m )
{
    modules_.insert ( std::pair<std::string, Module*> ( m.getName(), &m ) );
}

void FileSystem::removeModule ( Module& m )
{
    modules_.erase ( m.getName() );
}

int32_t FileSystem::generateFid()
{
    if ( ++nextFid_ > INT32_MAX )
    {
        nextFid_ = 0;
    }

    return nextFid_;
}

void FileSystem::setFid ( int32_t fid, Node& n )
{
    if ( fid < 0 )
    {
        return;
    }

    if ( (size_t) fid >= nodes_.size() )
    {
        nodes_.resize ( fid + 1 );
    }

    nodes_[fid] = &n;
}

void FileSystem::releaseFid ( int32_t fid )
{
    // if the fid is greater than the current size, we never stored this fid, so
    // there is nothing to release at the moment.
    /// @todo Store freed fids for reuse
    if ( fid < 0 || (size_t) fid >= nodes_.size() )
        return;

    nodes_[fid] = 0;
}

RetCode FileSystem::sendMessage ( const proto::OpenMsg& msg )
{
    std::unordered_map<std::string, Module*>::const_iterator it = modules_.find ( msg.name() );

    if ( it == modules_.end() )
    {
        return NotImplemented;
    }

    if ( ! it->second )
    {
        assert ( false );
        return NoSuchPath;
    }

    Module::FileHandle fh;
    fh.fsid = fsid_;
    fh.fd = msg.fid();

    return it->second->open ( fh );
}

RetCode FileSystem::sendMessage ( const proto::CloseMsg& msg )
{
    if ( msg.fid() < 0 || (size_t) msg.fid() >= nodes_.size() || ! nodes_[msg.fid()] )
    {
        return InvalidFid;
    }

    std::unordered_map<std::string, Module*>::const_iterator it = modules_.find ( nodes_[msg.fid()]->getName() );

    if ( it == modules_.end() )
    {
        return NotImplemented;
    }

    if ( ! it->second )
    {
        assert ( false );
        return NoSuchPath;
    }

    Module::FileHandle fh;
    fh.fsid = fsid_;
    fh.fd = msg.fid();

    return it->second->close ( fh );
}

RetCode FileSystem::sendMessage ( const proto::ReadMsg& msg, proto::FileMsg& file )
{
    if ( msg.fid() < 0 || (size_t) msg.fid() >= nodes_.size() || ! nodes_[msg.fid()] )
    {
        return InvalidFid;
    }

    std::unordered_map<std::string, Module*>::const_iterator it = modules_.find ( nodes_[msg.fid()]->getName() );

    if ( it == modules_.end() )
    {
        return NotImplemented;
    }

    if ( ! it->second )
    {
        assert ( false );
        return NoSuchPath;
    }

    Module::FileHandle fh;
    fh.fsid = fsid_;
    fh.fd = msg.fid();

    std::vector<char> data;
    RetCode rc = it->second->read ( fh, data, msg.size(), msg.offset() );

    if ( rc != Success )
    {
        return rc;
    }

    file.set_fid ( msg.fid() );
    file.set_size ( msg.size() );
    file.set_offset ( msg.offset() );
    file.set_data ( std::string ( data.begin(), data.end() ) );

    return Success;
}

RetCode FileSystem::sendMessage ( const proto::ReadMsg&, proto::DirectoryMsg& )
{
    return NotImplemented;
}

RetCode FileSystem::sendMessage ( const proto::ReadMsg&, proto::SymlinkMsg& )
{
    return NotImplemented;
}

RetCode FileSystem::sendMessage ( const proto::StatMsg&, Metadata& )
{
    return NotImplemented;
}

RetCode FileSystem::sendMessage ( const Metadata& )
{
    return NotImplemented;
}

RetCode FileSystem::sendMessage ( const proto::RemoveMsg& )
{
    return NotImplemented;
}

RetCode FileSystem::sendMessage ( const proto::FileMsg& )
{
    return NotImplemented;
}

RetCode FileSystem::sendMessage ( const proto::SymlinkMsg&, proto::ResponseMsg& )
{
    return NotImplemented;
}

