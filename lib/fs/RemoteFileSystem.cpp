#include "RemoteFileSystem.hpp"

#include <cassert>

#include "Message.pb.h"

using namespace rfs;

RemoteFileSystem::RemoteFileSystem() 
{
}

RemoteFileSystem::~RemoteFileSystem()
{
}

RetCode RemoteFileSystem::sendMessage ( const proto::OpenMsg& )
{
    return NotImplemented;
}

RetCode RemoteFileSystem::sendMessage ( const proto::CloseMsg& )
{
    return NotImplemented;
}

RetCode RemoteFileSystem::sendMessage ( const proto::ReadMsg&, proto::FileMsg& )
{
    return NotImplemented;
}

RetCode RemoteFileSystem::sendMessage ( const proto::ReadMsg&, proto::DirectoryMsg& )
{
    return NotImplemented;
}

RetCode RemoteFileSystem::sendMessage ( const proto::ReadMsg&, proto::SymlinkMsg& )
{
    return NotImplemented;
}

RetCode RemoteFileSystem::sendMessage ( const proto::StatMsg&, Metadata& )
{
    
    return NotImplemented;
}

RetCode RemoteFileSystem::sendMessage ( const Metadata& )
{
    return NotImplemented;
}

RetCode RemoteFileSystem::sendMessage ( const proto::RemoveMsg& )
{
    return NotImplemented;
}

RetCode RemoteFileSystem::sendMessage ( const proto::FileMsg& )
{
    return NotImplemented;
}

RetCode RemoteFileSystem::sendMessage ( const proto::SymlinkMsg&, proto::ResponseMsg& )
{
    return NotImplemented;
}

