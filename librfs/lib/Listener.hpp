#pragma once

#include <boost/asio.hpp>

#include "Channel.hpp"
#include "Log.hpp"

namespace rfs
{

class Listener
{
public:
    void start();

    inline void setOnConnectHandler ( std::function<void ( Listener* listener, ChannelPtr channel )> cb )
    {
        onConnectHandler_ = cb;
    }

    virtual std::string getEndpoint() const = 0;

    virtual void setupNextAccept() = 0;

    void doAccept ( ChannelPtr channel, const boost::system::error_code& err );

private:
    std::function<void ( Listener* listener, ChannelPtr channel )> onConnectHandler_;
};

}

