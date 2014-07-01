#pragma once

#include <deque>

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

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
                                      boost::system::error_code& err ) = 0;
    };

    Session ( Callback& cb );

    std::string getRemote() const;

    void write ( const Message& message );

private:
    void doHeaderRead ( const boost::system::error_code& err );
    void doPayloadRead ( const boost::system::error_code& err );
    void doNextWrite ( const boost::system::error_code& err );

    boost::asio::basic_stream_socket socket_;

    Callback& cb_;

    Message readMsg_;

    std::deque<Message> writeMsgs_;

};

}

