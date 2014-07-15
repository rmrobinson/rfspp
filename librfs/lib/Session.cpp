#include "Session.hpp"

#include <sstream>

using namespace rfs;

Session::Session ( boost::asio::io_service& svc, Callback& cb )
    : socket_ ( svc ), cb_ ( cb )
{
}

std::string Session::getRemote() const
{
    std::stringstream ss;
    ss << socket_.remote_endpoint();

    return ss.str();
}

void Session::write ( const Message& message )
{
    bool writeInProgress = ( writeMsgs_.size() > 0 );

    writeMsgs_.push_back ( message );

    if ( ! writeInProgress )
    {
        Message& msg = writeMsgs_.front();

        boost::asio::async_write ( socket_,
                                   boost::asio::buffer ( msg.get(), msg.size() ),
                                   boost::bind ( &Session::doNextWrite, shared_from_this(),
                                       boost::asio::placeholders::error ) );
    }
}

void Session::close()
{
    socket_.close();
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
        return;
    }

    if ( ! readMsg_.decodeHeader() )
    {
        boost::system::error_code protoErr ( boost::system::errc::protocol_error, boost::system::system_category() );
        cb_.onSessionError ( shared_from_this(), protoErr );
        return;
    }

    boost::asio::async_read ( socket_,
                              boost::asio::buffer ( readMsg_.payload(), readMsg_.payloadSize() ),
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

    readMsg_.clear();

    boost::asio::async_read ( socket_,
                              boost::asio::buffer ( readMsg_.header(), sizeof ( Message::Header ) ),
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

    if ( writeMsgs_.empty() )
        return;

    Message& msg = writeMsgs_.front();

    boost::asio::async_write ( socket_,
                               boost::asio::buffer ( msg.get(), msg.size() ),
                               boost::bind ( &Session::doNextWrite, shared_from_this(),
                                   boost::asio::placeholders::error ) );
}

