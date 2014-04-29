#include "Node.hpp"

#include <boost/lexical_cast.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "Client.pb.h"

#include "FileSystem.hpp"

using namespace rfs;

Node::Node ( FileSystem& fs, const std::string& name ) : fs_ ( fs ), name_ ( name ), fid_ ( -1 )
{
}

Node::~Node()
{
    if ( fid_ >= 0 )
    {
        close();
    }
}

proto::RetCode Node::open()
{
    if ( fid_ >= 0 )
    {
        return proto::AlreadyOpen;
    }

    int32_t newFid = fs_.generateFid();

    proto::OpenMsg msg;
    msg.set_fid ( newFid );
    msg.set_name ( name_ );

    proto::RetCode rc = fs_.sendMessage ( msg );

    if ( rc != proto::Success )
        return rc;

    fid_ = newFid;
    fs_.setFid ( newFid, *this );

    return proto::Success;
}

proto::RetCode Node::close()
{
    if ( fid_ < 0 )
    {
        return proto::Success;
    }

    proto::CloseMsg msg;
    msg.set_fid ( fid_ );

    fs_.sendMessage ( msg );

    fs_.releaseFid ( fid_ );
    fid_ = -1;

    return proto::Success;
}

proto::RetCode Node::rename ( const std::string& )
{
    return proto::NotImplemented;
}

proto::RetCode Node::remove()
{
    return proto::NotImplemented;
}

proto::RetCode Node::setOwner ( const boost::uuids::uuid& uid, const boost::uuids::uuid& gid )
{
    proto::Metadata meta;

    proto::StatMsg statReq;
    statReq.set_name ( name_ );

    proto::RetCode ret = fs_.sendMessage ( statReq, meta );

    if ( ret != proto::Success )
        return ret;

    if ( meta.name() != name_ )
    {
        assert ( false );
        return proto::InvalidData;
    }

    meta.set_uid ( boost::lexical_cast<std::string> ( uid ) );
    meta.set_gid ( boost::lexical_cast<std::string> ( gid ) );
    return fs_.sendMessage ( meta );
}

proto::RetCode Node::setMode ( int )
{
    return proto::NotImplemented;
}

