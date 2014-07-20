extern "C"
{
#include <sys/socket.h>`
#include <sys/un.h>
}

#include "Client.hpp"

using namespace rfs;

Client::Client() : fd_ ( -1 )
{
}

RetCode Client::start()
{
    if ( fd_ >= 0 )
    {
        return RetCode::AlreadyStarted;
    }

    /// @todo find string to connect to
    std::string srvPath ( "/tmp/rfs" );

    fd_ = socket ( AF_LOCAL, SOCK_STREAM, 0 );

    if ( fd_ <= 0 )
    {
        return RetCode::SocketError;
    }

    struct sockaddr_un sa;
    memset ( &sa, 0, sizeof ( sa ) );

    if ( srvPath.length() >= sizeof ( sa.sun_path ) - 1 )
    {
        return RetCode::PathTooLong;
    }

    sa.sun_family = AF_LOCAL;
    strncpy ( sa.sun_path, srvPath.c_str(), sizeof ( sa.sun_path ) - 1 );

    int ret = connect ( fd_, ( struct sockaddr* ) &sa, sizeof ( sa ) );

    if ( ret < 0 )
    {
        return RetCode::UnableToConnect;
    }

    return RetCode::Success;
}

