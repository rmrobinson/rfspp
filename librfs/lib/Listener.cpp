#include "Listener.hpp"

using namespace rfs;

Logger Listener::log_ ( "rfsListener" );

void Listener::start()
{
    setupNextAccept();
}

void Listener::doAccept ( ChannelPtr channel, const boost::system::error_code& err )
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

