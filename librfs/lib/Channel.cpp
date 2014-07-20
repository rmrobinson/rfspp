#include "Channel.hpp"

#include <sstream>

#include <boost/bind.hpp>

using namespace rfs;

uint32_t Channel::MaxMessageSize ( 1024 * 128 );

void Channel::Header::reset()
{
    size = 0;
}

bool Channel::Header::serialize ( char* data, size_t dataSize ) const
{
    if ( dataSize < sizeof ( Header ) )
        return false;

    memcpy ( data, this, sizeof ( Header ) );
    return true;
}

bool Channel::Header::deserialize ( const std::vector<char>& data )
{
    if ( data.size() < sizeof ( Header ) )
        return false;

    const Header* tmp = reinterpret_cast<const Header*> ( &data[0] );
    size = ntohl ( tmp->size );

    return true;
}

Channel::Channel ( boost::asio::io_service& svc, const boost::asio::generic::stream_protocol& proto )
    : socket_ ( svc, proto )
{
    readMsg_.resize ( MaxMessageSize );
}

void Channel::close()
{
    socket_.close();
}

void Channel::send ( const proto::RfsMsg& msg )
{
    bool writeInProgress = ( writeMsgs_.size() > 0 );

    const size_t hdrSize = sizeof ( Header );
    const size_t msgSize = msg.ByteSize();
    std::vector<char> tmp ( hdrSize + msgSize );

    Header hdr;
    hdr.size = msgSize;

    if ( ! hdr.serialize ( &tmp[0], hdrSize ) || ! msg.SerializeToArray ( &tmp[hdrSize], msgSize ) )
    {
        log_ << Log::Crit << "Unable to serialize RFS message to array, size " << msgSize << std::endl;
        return;
    }

    writeMsgs_.push_back ( tmp );

    if ( ! writeInProgress )
    {
        const std::vector<char>& toSend = writeMsgs_.front();

        boost::asio::async_write ( socket_,
                                   boost::asio::buffer ( toSend ),
                                   boost::bind ( &Channel::doNextWrite, shared_from_this(),
                                       boost::asio::placeholders::error ) );
    }
}

void Channel::doHeaderRead ( const boost::system::error_code& err, size_t readSize )
{
    if ( err )
    {
        if ( err != boost::asio::error::eof && err != boost::asio::error::connection_reset )
        {
            log_ << Log::Crit << "Error occurred when writing to socket: " << err.message()
                << ", closing socket" << std::endl;
        }

        if ( closeCb_ )
            closeCb_();

        return;
    }

    assert ( readSize == sizeof ( Header ) );

    if ( ! readHdr_.deserialize ( readMsg_ ) )
    {
        log_ << Log::Crit << "Unable to parse header message of size " << readSize << std::endl;

        assert ( false );
        return;
    }
    else if ( readHdr_.size > MaxMessageSize )
    {
        log_ << Log::Crit << "Received a request for a message of size " << readHdr_.size
            << " but the max allowed size is " << MaxMessageSize << "; resetting channel" << std::endl;

        if ( closeCb_ )
            closeCb_();

        close();
        return;
    }

    boost::asio::async_read ( socket_,
                              boost::asio::buffer ( &readMsg_[0], readHdr_.size ),
                              boost::bind ( &Channel::doPayloadRead, shared_from_this(),
                                  boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred ) );
}

void Channel::doPayloadRead ( const boost::system::error_code& err, size_t readSize )
{
    if ( err )
    {
        if ( err != boost::asio::error::eof && err != boost::asio::error::connection_reset )
        {
            log_ << Log::Crit << "Error occurred when writing to socket: " << err.message()
                << ", closing socket" << std::endl;
        }

        if ( closeCb_ )
            closeCb_();

        return;
    }

    assert ( readSize == readHdr_.size );

    proto::RfsMsg msg;
    if ( msg.ParseFromArray ( &readMsg_[0], readSize ) && recvCb_ )
    {
        recvCb_ ( msg );
    }

    readHdr_.reset();
    readMsg_.clear();

    boost::asio::async_read ( socket_,
                              boost::asio::buffer ( &readMsg_[0], sizeof ( Header ) ),
                              boost::bind ( &Channel::doHeaderRead, shared_from_this(),
                                  boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred ) );
}

void Channel::doNextWrite ( const boost::system::error_code& err )
{
    if ( err )
    {
        if ( err != boost::asio::error::eof && err != boost::asio::error::connection_reset )
        {
            log_ << Log::Crit << "Error occurred when writing to socket: " << err.message()
                << ", closing socket" << std::endl;
        }

        if ( closeCb_ )
            closeCb_();

        return;
    }

    writeMsgs_.pop_front();

    if ( writeMsgs_.empty() )
        return;

    const std::vector<char>& msg = writeMsgs_.front();

    boost::asio::async_write ( socket_,
                               boost::asio::buffer ( msg ),
                               boost::bind ( &Channel::doNextWrite, shared_from_this(),
                                   boost::asio::placeholders::error ) );
}

