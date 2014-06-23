#include "FuseBridge.hpp"

#include "fs/ProcessFileSystem.hpp"
#include "modules/TimeProcessFile.hpp"

using namespace rfs;

int main ( int argc, char* argv[] )
{
    ProcessFileSystem fs;

    TimeProcessFile tm ( fs );

    FuseBridge fb ( fs );
    fb.run ( argc, argv );
    return 0;

}

