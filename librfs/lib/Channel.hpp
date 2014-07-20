#pragma once

#include <deque>
#include <memory>

#include <boost/asio.hpp>

#include "Rfs.pb.h"

#include "Log.hpp"

namespace rfs
{

typedef std::shared_ptr<class Channel> ChannelPtr;

class Channel : public std::enable_shared_from_this<Channel>
{
public:
    Channel ( boost::asio::io_service& svc, const boost::asio::generic::stream_protocol& proto );

    void close();

    void setOnReceiveHandler ( std::function<void( const proto::RfsMsg& msg )> cb );
    void setOnCloseHandler ( std::function<void()> cb );

    void send ( const proto::RfsMsg& msg );

    inline boost::asio::generic::stream_protocol::socket& getSocket()
    {
        return socket_;
    }

private:

#pragma pack(push,1)
    struct Header
    {
        uint32_t size;

        Header() : size ( 0 ) {}

        void reset();

        bool serialize ( char* data, size_t dataSize ) const;
        bool deserialize ( const std::vector<char>& data );
    };
#pragma pack(pop)

    void doHeaderRead ( const boost::system::error_code& err, size_t readSize );
    void doPayloadRead ( const boost::system::error_code& err, size_t readSize );
    void doNextWrite ( const boost::system::error_code& err );

    static uint32_t MaxMessageSize;

    static Logger log_;

    boost::asio::generic::stream_protocol::socket socket_;

    std::function<void( const proto::RfsMsg& msg )> recvCb_;
    std::function<void()> closeCb_;

    Header readHdr_;
    std::vector<char> readMsg_;

    std::deque< std::vector<char> > writeMsgs_;

};

}

