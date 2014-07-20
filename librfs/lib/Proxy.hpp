#pragma once

#include <string>

#include <boost/asio.hpp>

#include "LocalListener.hpp"
#include "Log.hpp"
#include "Peer.hpp"

namespace rfs
{

class Proxy
{
public:
    Proxy ( boost::asio::io_service& svc );

    void start();

    inline const std::string& getPath() const
    {
        return path_;
    }

private:
    void onConnect ( Listener* listener, ChannelPtr channel );

    static Logger log_;

    boost::asio::io_service& svc_;

    std::string path_;

    LocalListener localListener_;

    Peer peer_;

};

}

