#include "Listener.hpp"

using namespace rfs;

void Listener::start()
{
    setupNextAccept();
}

void Listener::doAccept ( ChannelPtr channel, const boost::system::error_code& err )
{
    if ( ! err && onConnectHandler_ )
    {
        onConnectHandler_ ( this, channel );
    }

    setupNextAccept();
}

