#pragma once

#include <mutex>

#include <boost/asio.hpp>

#include "Proxy.hpp"

namespace rfs
{

class ProxyThread
{
public:
    static ProxyThread& get();

    ProxyThread();

    void run();

    inline const std::string& getPath()
    {
        std::lock_guard<std::mutex> guard ( mtx_ );
        return proxy_.getPath();
    }

private:
    boost::asio::io_service ioService_;

    std::mutex mtx_;

    bool running_;

    Proxy proxy_;
};

}

