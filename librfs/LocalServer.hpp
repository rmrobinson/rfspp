#pragma once

#include "Server.hpp"

namespace rfs
{

class LocalServer : public Server
{
public:
    LocalServer ( boost::asio::io_service& ioSvc, const std::string& addr );
    ~LocalServer();

private:
    boost::asio::local::stream_protocol::acceptor acceptor_;

};

}

