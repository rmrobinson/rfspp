#include "IpListener.hpp"

#include <boost/bind.hpp>

using namespace rfs;

IpListener::IpListener ( boost::asio::io_service& svc, const boost::asio::ip::tcp::endpoint& ep )
    : acceptor_ ( svc, ep )
{
}

void IpListener::setupNextAccept()
{
    boost::asio::generic::stream_protocol proto ( acceptor_.local_endpoint().protocol() );
    ChannelPtr channel ( new Channel ( acceptor_.get_io_service(), proto ) );

    acceptor_.async_accept ( channel->getSocket(),
                             boost::bind ( &Listener::doAccept, this, channel, boost::asio::placeholders::error ) );
}

std::string IpListener::getEndpoint() const
{
    const boost::asio::ip::tcp::endpoint& ep = acceptor_.local_endpoint();
    std::stringstream ss;

    if ( ep.address().is_v6() )
    {
        ss << "[" << ep.address().to_string() << "]";
    }
    else
    {
        ss << ep.address().to_string();
    }

    ss << ":" << std::to_string ( ep.port() );

    return ss.str();
}

