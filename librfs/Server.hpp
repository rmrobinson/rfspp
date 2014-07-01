#pragma once

#include <boost/asio.hpp>

namespace rfs
{

class Server
{
public:
    Server ( boost::asio::io_service& ioSvc );
    virtual ~Server();

    virtual bool start ( const std::string& addr ) = 0;

private:
    void doAccept ( SessionPtr session, const boost::system::error_code& err );

    boost::asio::io_service& ioSvc_;
};

}

