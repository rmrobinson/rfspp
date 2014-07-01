#include "Session.hpp"

#include <sstream>

using namespace rfs;

Session::Session ( Callback& cb ) : cb_ ( cb )
{
}

std::string Session::getRemote() const
{
    std::stringstream ss;
    ss << socket.remote_endpoint();

    return ss.str();
}

void Session::write ( const Message& message )
{
    writeMsgs_.push_back ( message );
}

void Session::doHeaderRead ( const boost::system::error_code& err )
{
    if ( err == boost::asio::error::eof || err == boost::asio::error::connection_reset )
    {
        cb_.onSessionClose ( shared_from_this() );
        return;
    }
    else if ( err )
    {
        cb_.onSessionError ( shared_from_this(), err );
    }

    readMsg_.decodeHeader();

    boost::asio::async_read ( socket_,
                              boost::asio::buffer ( readMsg_.data(), readMsg_.size() ),
                              boost::bind ( &Session::doPayloadRead, shared_from_this(),
                                  boost::asio::placeholders::error ) );
}

void Session::doPayloadRead ( const boost::system::error_code& err )
{
    if ( err == boost::asio::error::eof || err == boost::asio::error::connection_reset )
    {
        cb_.onSessionClose ( shared_from_this() );
        return;
    }
    else if ( err )
    {
        cb_.onSessionError ( shared_from_this(), err );
        return;
    }

    cb_.onMessageReceived ( shared_from_this(), readMsg_ );

    readMsg_.reset();

    boost::asio::async_read ( socket_,
                              boost::asio::buffer ( readMsg_.data(), Message::HeaderLen ),
                              boost::bind ( &Session::doHeaderRead, shared_from_this(),
                                  boost::asio::placeholders::error ) );
}

void Session::doNextWrite ( const boost::system::error_code& err )
{
    if ( err == boost::asio::error::eof || err == boost::asio::error::connection_reset )
    {
        cb_.onSessionClose ( shared_from_this() );
        return;
    }
    else if ( err )
    {
        cb_.onSessionError ( shared_from_this(), err );
        return;
    }

    writeMsgs_.pop_front();

    if ( writeMsgs_.isEmpty() )
        return;

    Message& msg = writeMsgs_.front();
    msg.encodeHeader();

    boost::asio::async_write ( socket_,
                               boost::asio::buffer ( msg.data(), msg.size() ),
                               boost::bind ( &Session::doNextWrite, shared_from_this(),
                                   boost::asio::placeholders::error ) );
}

