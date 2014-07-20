#pragma once

#include <memory>

#include "Listener.hpp"

namespace rfs
{

class IpListener : public Listener
{
public:
    IpListener ( boost::asio::io_service& svc, const boost::asio::ip::tcp::endpoint& ep );

private:
    virtual void setupNextAccept();

    virtual std::string getEndpoint() const;

    boost::asio::ip::tcp::acceptor acceptor_;
    
};

}

