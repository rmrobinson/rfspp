#include "LocalListener.hpp"

#include <boost/bind.hpp>

using namespace rfs;

LocalListener::LocalListener ( boost::asio::io_service& svc, const boost::asio::local::stream_protocol::endpoint& ep )
    : acceptor_ ( svc, ep )
{
}

void LocalListener::setupNextAccept()
{
    //boost::asio::generic::stream_protocol proto ( acceptor_.local_endpoint().protocol() );
    boost::asio::generic::stream_protocol proto ( AF_UNIX, 0 );
    ChannelPtr channel ( new Channel ( acceptor_.get_io_service(), proto ) );

    acceptor_.async_accept ( channel->getSocket(),
                             boost::bind ( &LocalListener::doAccept, this, channel, boost::asio::placeholders::error ) );
}

std::string LocalListener::getEndpoint() const
{
    const boost::asio::local::stream_protocol::endpoint& ep = acceptor_.local_endpoint();

    return ep.path();
}

void LocalListener::doAccept ( ChannelPtr channel, const boost::system::error_code& err )
{
    log_ << Log::Crit << "Listener on " << getEndpoint() << " received connection" << std::endl;

    if ( err )
    {
        log_ << Log::Crit << "Error accepting on channel " << getEndpoint() << ": " << err.message() << std::endl;

        abort();
    }
    else if ( onConnectHandler_ )
    {
        onConnectHandler_ ( this, channel );
    }

    setupNextAccept();
}

