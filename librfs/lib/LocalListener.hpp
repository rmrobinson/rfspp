#pragma once

#include <memory>

#include "Listener.hpp"

namespace rfs
{

class LocalListener : public Listener
{
public:
    LocalListener ( boost::asio::io_service& svc, const boost::asio::local::stream_protocol::endpoint& ep );

private:
    virtual void setupNextAccept();

    virtual std::string getEndpoint() const;

    void doAccept ( ChannelPtr channel, const boost::system::error_code& err );

    boost::asio::local::stream_protocol::acceptor acceptor_;
    
};

}

