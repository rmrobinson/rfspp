#include <iostream>

#include "fs/FileSystem.hpp"
#include "fs/File.hpp"
#include "modules/TimeModule.hpp"

using namespace rfs;

int main()
{
    FileSystem fs;

    TimeModule tm ( fs );

    File f ( fs, "/time" );
    RetCode rc = f.open();

    if ( rc != Success )
    {
        std::cerr << "Unable to open file: " << f.getName() << " due to " << rc << std::endl;
        return 1;
    }
    {
        std::cerr << "Successfully opened file " << f.getName() << std::endl;
    }

    std::vector<char> data;
    rc = f.read ( data );

    if ( rc != Success )
    {
        std::cerr << "Unable to read data from file: " << rc << std::endl;
        return 1;
   }

    std::string s ( data.begin(), data.end() );
    std::cout << "Current Time: " << s << std::endl;

    return 0;
}

