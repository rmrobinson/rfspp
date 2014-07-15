#pragma once

#include <deque>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "Message.hpp"

namespace rfs
{

typedef boost::shared_ptr<class Session> SessionPtr;

class Session : public boost::enable_shared_from_this<Session>
{
public:
    class Callback
    {
    public:
        virtual ~Callback() {};

        virtual void onMessageReceived ( SessionPtr session,
                                         const Message& message ) = 0;
        virtual void onSessionClose ( SessionPtr session ) = 0;
        virtual void onSessionError ( SessionPtr session,
                                      const boost::system::error_code& err ) = 0;
    };

    Session ( boost::asio::io_service& svc, Callback& cb );

    std::string getRemote() const;

    void write ( const Message& message );

    void close();

    inline boost::asio::local::stream_protocol::socket& socket()
    {
        return socket_;
    }

protected:
    boost::asio::local::stream_protocol::socket socket_;

private:
    void doHeaderRead ( const boost::system::error_code& err );
    void doPayloadRead ( const boost::system::error_code& err );
    void doNextWrite ( const boost::system::error_code& err );

    Callback& cb_;

    Message readMsg_;

    std::deque<Message> writeMsgs_;

};

}

