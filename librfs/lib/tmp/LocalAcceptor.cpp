#include "LocalAcceptor.hpp"

using namespace rfs;

LocalAcceptor::LocalAcceptor ( boost::asio::io_service& ioSvc, Callback& cb, const std::string& path )
    : cb_ ( cb ), acceptor_ ( ioSvc, boost::asio::local::stream_protocol::endpoint ( path ) )
{
}

void LocalAcceptor::doAccept ( SessionPtr session, const boost::system::error_code& err )
{
    if ( ! err )
    {
        cb_.onSessionAccepted ( *this, session );
    }

    session.reset ( new Session ( acceptor_.get_io_service() );
}

