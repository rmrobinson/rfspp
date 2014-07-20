#pragma once

#include "Channel.hpp"
#include "Log.hpp"

namespace rfs
{

class Peer
{
public:
    void setChannel ( ChannelPtr channel );

private:
    void onRfsMsgReceived ( const proto::RfsMsg& msg );
    void onChannelClose();

    static Logger log_;

    ChannelPtr channel_;

};

}

