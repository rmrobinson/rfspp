#include <iostream>

#include "fs/ProcessFileSystem.hpp"
#include "modules/TimeProcessFile.hpp"

using namespace rfs;

int main()
{
    ProcessFileSystem fs;

    TimeProcessFile tm ( fs );

    FileHandle fh;
    RetCode rc = fs.openFile ( "/time", false, fh );

    if ( rc != Success )
    {
        std::cerr << "Unable to open file: /time due to " << rc << std::endl;
        return 1;
    }
    {
        std::cerr << "Successfully opened file /time" << std::endl;
    }

    std::vector<char> data;
    size_t processed = 0;
    rc = fs.readFile ( fh, data, 0, processed );

    if ( rc != Success )
    {
        std::cerr << "Unable to read data from file: " << rc << std::endl;
        return 1;
   }

    std::string s ( data.begin(), data.end() );
    std::cout << "Current Time: " << s << std::endl;

    Metadata md;
    rc = fs.readMetadata ( "/", md );

    if ( rc != Success )
    {
        std::cerr << "Unable to read metadata from file: " << rc << std::endl;
        return 1;
    }

    std::cout << "Owner: " << md.uid() << "; mtime: " << md.mtime()
        << "path: " << md.path() << "; type: " << md.type() << std::endl;

    return 0;
}

