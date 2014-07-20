extern "C"
{
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
}

#include "Client.hpp"
#include "ProxyThread.hpp"
#include "WireHeader.hpp"

using namespace rfs;

Client::Client() : fd_ ( -1 )
{
}

Client::~Client()
{
    disconnect();
}

RetCode Client::connect()
{
    if ( fd_ >= 0 )
    {
        return AlreadyStarted;
    }

    // This can safely be called multiple times; only the first time will cause the
    // thread to run.
    ProxyThread::get().run();

    const std::string srvPath ( ProxyThread::get().getPath() );

    fd_ = socket ( AF_LOCAL, SOCK_STREAM, 0 );

    if ( fd_ <= 0 )
    {
        return SocketError;
    }

    struct sockaddr_un sa;
    memset ( &sa, 0, sizeof ( sa ) );

    if ( srvPath.length() >= sizeof ( sa.sun_path ) - 1 )
    {
        return InvalidPath;
    }

    sa.sun_family = AF_LOCAL;
    strncpy ( sa.sun_path, srvPath.c_str(), sizeof ( sa.sun_path ) - 1 );

    int ret = ::connect ( fd_, ( struct sockaddr* ) &sa, sizeof ( sa ) );

    if ( ret < 0 )
    {
        return UnableToConnect;
    }

    return Success;
}

void Client::disconnect()
{
    if ( ! isConnected() )
        return;

    ::close ( fd_ );
    fd_ = -1;
}

RetCode Client::stat ( const std::string& path, Metadata& md )
{
    proto::RfsMsg cmd;
    cmd.set_cmd ( proto::RfsMsg::Stat );

    proto::RfsMsg::StatReq* sr = cmd.mutable_statreq();
    assert ( sr != nullptr );
    sr->set_path ( path );

    proto::RfsMsg resp;

    RetCode rc = execCmd ( cmd, resp );

    if ( NotOk ( rc ) )
    {
        return rc;
    }

    if ( ! resp.has_metadata() )
    {
        return MalformedMessage;
    }

    md = resp.metadata();
    return Success;
}

RetCode Client::execCmd ( const proto::RfsMsg& cmd, proto::RfsMsg& resp )
{
    if ( ! isConnected() )
    {
        RetCode rc = connect();

        if ( NotOk ( rc ) )
            return rc;
    }

    assert ( isConnected() );

    const size_t hdrSize = sizeof ( WireHeader );
    const size_t cmdSize = cmd.ByteSize();
    std::vector<char> tmp ( hdrSize + cmdSize );

    WireHeader hdr;
    hdr.size = cmdSize;

    if ( ! hdr.serialize ( &tmp[0], hdrSize )
          || ! cmd.SerializeToArray ( &tmp[hdrSize], cmdSize ) )
    {
        return MalformedMessage;
    }

    ssize_t ret = ::write ( fd_, &tmp[0], tmp.size() );
    if ( ret < 0 )
    {
        disconnect();
        return WriteError;
    }

    hdr.reset();
    ret = ::read ( fd_, &tmp[0], hdrSize );

    if ( ret < 0 || ! hdr.deserialize ( tmp ) )
    {
        disconnect();
        return ReadError;
    }

    tmp.resize ( hdr.size );
    size_t read = 0;

    while ( read < hdr.size )
    {
        ret = ::read ( fd_, &tmp[read], ( hdr.size - read ) );

        if ( ret < 0 )
        {
            disconnect();
            return ReadError;
        }

        read += ret;
    }

    assert ( read == hdr.size );

    resp.Clear();
    if ( ! resp.ParseFromArray ( &tmp[0], hdr.size ) )
    {
        disconnect();
        return MalformedMessage;
    }

    return Success;
}
