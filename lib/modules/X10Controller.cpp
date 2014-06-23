#include "X10Controller.hpp"

extern "C"
{
#include <sys/fcntl.h>
#include <unistd.h>
#include "bottlerocket/br_cmd.h"
}

#include <cstdlib>
#include <boost/algorithm/string.hpp>

using namespace rfs;

X10Controller::X10Controller ( ProcessFileSystem& fs, const std::string& portName )
    : Controller<ProtoProcessFile<proto::modules::Device>, proto::modules::Device> ( fs ), fd_ ( -1 )
{
    fd_ = open ( portName.c_str(), O_RDONLY | O_NONBLOCK );
}

X10Controller::~X10Controller()
{
    if ( fd_ >= 0 )
    {
        close ( fd_ );
        fd_ = -1;
    }
}

RetCode X10Controller::set ( ProtoProcessFile<proto::modules::Device>& dev, const proto::modules::Device& state )
{
    uint8_t devId = UINT8_MAX;

    const size_t lastSlashIdx = dev.getPath().find_last_of ( '/' );
    const std::string name = dev.getPath().substr ( lastSlashIdx );

    if ( fd_ < 0 || ! nameToId ( name, devId ) )
    {
        return NotPossible;
    }

    x10_br_out ( fd_, devId, ( state.ison() ? ON : OFF ) );
    return Success;
}

bool X10Controller::nameToId ( const std::string& devName, uint8_t& id )
{
    if ( devName.size() < 2 )
        return false;

    std::string address = devName.substr ( devName.size() - 2 );
    boost::to_upper ( address );

    const int house = address[0] - 'A';
    const int unit = atoi ( &address[1] );

    if ( house < 0 || house > 15
        || unit < 0 || unit > 15 )
    {
        return false;
    }

    id = ( ( house << 4 ) | unit );
    return true;
}

