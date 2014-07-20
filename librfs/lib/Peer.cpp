#include "Peer.hpp"

using namespace rfs;

Logger Peer::log_ ( "rfsPeer" );

void Peer::setChannel ( ChannelPtr channel )
{
    channel_ = channel;

    channel_->setOnReceiveHandler ( std::bind ( &Peer::onRfsMsgReceived,
                                                this,
                                                std::placeholders::_1 ) );

    channel_->setOnCloseHandler ( std::bind ( &Peer::onChannelClose,
                                  this ) );
}

void Peer::onRfsMsgReceived ( const proto::RfsMsg& msg )
{
    log_ << Log::Crit << "Channel received a command " << msg.cmd() << std::endl;

    proto::RfsMsg resp;
    resp.set_cmd ( proto::RfsMsg::Response );

    RetCode rc = Success;

    switch ( msg.cmd() )
    {
    case proto::RfsMsg::Stat:
        {
        if ( ! msg.has_statreq() )
        {
            log_ << Log::Crit << "Received stat msg without statReq" << std::endl;
            rc = MalformedMessage;
            break;
        }

        log_ << Log::Crit << "Received a stat req for " << msg.statreq().path() << std::endl;

        Metadata* md = resp.mutable_metadata();
        md->set_path ( msg.statreq().path() );
        md->set_type ( Metadata::File );
        }

        break;

    default:
        rc = NotImplemented;
        break;
    }

    if ( rc != Success )
    {
        resp.Clear();
        resp.set_cmd ( proto::RfsMsg::Response );
        proto::RfsMsg::ResponseMsg* respMsg = resp.mutable_response();
        respMsg->set_ret ( rc );
    }

    channel_->send ( resp );
}

void Peer::onChannelClose()
{
    log_ << Log::Crit << "Channel for peer closed" << std::endl;
}

