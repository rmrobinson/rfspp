#include "TimeModule.hpp"

extern "C"
{
#include <sys/time.h>
}

#include <algorithm>

#include "File.hpp"

using namespace rfs;

TimeModule::TimeModule ( FileSystem& fs ) : Module ( fs, "/time", 0 )
{
}

proto::RetCode TimeModule::read ( const FileHandle&, std::vector<char>& data,
                                  size_t, size_t )
{
    data.clear();

    time_t now = time ( 0 );
    std::string s ( ctime ( &now ) );

    s.erase ( std::remove ( s.begin(), s.end(), '\n' ), s.end() );
    std::copy ( s.begin(), s.end(), std::back_inserter ( data ) );
    return proto::Success;
}

proto::RetCode TimeModule::write ( const FileHandle&, const std::vector<char>&,
                                   size_t, size_t )
{
    return proto::NotSupported;
}

size_t TimeModule::size()
{
    // ctime() returns a string 26 characters long.
    return 26;
}

