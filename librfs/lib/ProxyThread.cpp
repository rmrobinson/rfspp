#include "ProxyThread.hpp"

using namespace rfs;

ProxyThread& ProxyThread::get()
{
    static ProxyThread instance;
    return instance;
}

ProxyThread::ProxyThread() : running_ ( false ), proxy_ ( ioService_ )
{
}

void ProxyThread::run()
{
    std::lock_guard<std::mutex> guard ( mtx_ );

    if ( running_ )
        return;

    proxy_.start();

    ioService_.run();
}

