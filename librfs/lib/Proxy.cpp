extern "C"
{
#include <sys/types.h>
#include <unistd.h>
}

#include "Proxy.hpp"

#define RFS_PATH "/tmp/rfs_"

using namespace rfs;

Logger Proxy::log_ ( "rfsProxy" );

Proxy::Proxy ( boost::asio::io_service& svc )
    : svc_ ( svc ),
      path_ ( std::string ( RFS_PATH ).append ( std::to_string ( getpid() ) ) ),
      localListener_ ( svc_, boost::asio::local::stream_protocol::endpoint ( path_ ) )
{
    localListener_.setOnConnectHandler ( std::bind ( &Proxy::onConnect, this,
                                                     std::placeholders::_1,
                                                     std::placeholders::_2 ) );

}

void Proxy::start()
{
    log_ << Log::Crit << "Starting proxy on " << getPath() << std::endl;

    localListener_.start();
}

void Proxy::onConnect ( Listener* listener, ChannelPtr channel )
{
    assert ( listener == &localListener_ );
    log_ << Log::Crit << "Channel received, passing to peer" << std::endl;

    peer_.setChannel ( channel );
}

