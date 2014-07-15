#pragma once

#include <vector>

#include <boost/asio.hpp>

#include "Session.hpp"

namespace rfs
{

class LocalAcceptor
{
public:
    class Callback
    {
    public:
        virtual ~Callback() {}

        virtual void onSessionAccepted ( LocalAcceptor& server, SessionPtr session ) = 0;
    };

    LocalAcceptor ( boost::asio::io_service& ioSvc, Callback& cb, const std::string& path );

private:
    void doAccept ( SessionPtr session, const boost::system::error_code& err );

    Callback& cb_;

    boost::asio::local::stream_protocol::acceptor acceptor_;
};

}

