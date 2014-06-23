#include "Node.hpp"

#include <boost/lexical_cast.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "Message.pb.h"

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

RetCode Node::create ( const Metadata& md )
{
    RetCode rc = fs_.sendMessage ( md );

    if ( rc == Success )
    {
        rc = open();
    }

    return rc;
}

RetCode Node::open()
{
    if ( fid_ >= 0 )
    {
        return AlreadyOpen;
    }

    int32_t newFid = fs_.generateFid();

    proto::OpenMsg msg;
    msg.set_fid ( newFid );
    msg.set_name ( name_ );

    RetCode rc = fs_.sendMessage ( msg );

    if ( rc != Success )
        return rc;

    fid_ = newFid;
    fs_.setFid ( newFid, *this );

    return Success;
}

RetCode Node::close()
{
    if ( fid_ < 0 )
    {
        return Success;
    }

    proto::CloseMsg msg;
    msg.set_fid ( fid_ );

    fs_.sendMessage ( msg );

    fs_.releaseFid ( fid_ );
    fid_ = -1;

    return Success;
}

RetCode Node::stat ( Metadata& md )
{
    proto::StatMsg msg;

    if ( fid_ >= 0 )
    {
        msg.set_fid ( fid_ );
    }
    else
    {
        msg.set_name ( name_ );
    }

    return fs_.sendMessage ( msg, md );
}

RetCode Node::rename ( const std::string& )
{
    return NotImplemented;
}

RetCode Node::remove()
{
    return NotImplemented;
}

RetCode Node::setOwner ( const boost::uuids::uuid& uid, const boost::uuids::uuid& gid )
{
    Metadata meta;

    proto::StatMsg statReq;
    statReq.set_name ( name_ );

    RetCode ret = fs_.sendMessage ( statReq, meta );

    if ( ret != Success )
        return ret;

    if ( meta.name() != name_ )
    {
        assert ( false );
        return InvalidData;
    }

    meta.set_uid ( boost::lexical_cast<std::string> ( uid ) );
    meta.set_gid ( boost::lexical_cast<std::string> ( gid ) );
    return fs_.sendMessage ( meta );
}

RetCode Node::setMode ( int )
{
    return NotImplemented;
}

