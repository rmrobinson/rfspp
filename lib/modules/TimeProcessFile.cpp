#include "TimeProcessFile.hpp"

extern "C"
{
#include <sys/time.h>
}

#include <algorithm>

using namespace rfs;

TimeProcessFile::TimeProcessFile ( ProcessFileSystem& fs ) : ProcessFile ( fs, "/time" )
{
}

RetCode TimeProcessFile::read ( const FileHandle&, std::vector<char>& data,
                                off_t, size_t& processed )
{
    data.clear();

    time_t now = time ( 0 );
    std::string s ( ctime ( &now ) );

    s.erase ( std::remove ( s.begin(), s.end(), '\n' ), s.end() );
    std::copy ( s.begin(), s.end(), std::back_inserter ( data ) );

    processed = s.length();

    return Success;
}

RetCode TimeProcessFile::write ( const FileHandle&, const std::vector<char>&,
                                 off_t, size_t& )
{
    return NotSupported;
}

size_t TimeProcessFile::size() const
{
    // ctime() returns a string 26 characters long.
    return 26;
}

