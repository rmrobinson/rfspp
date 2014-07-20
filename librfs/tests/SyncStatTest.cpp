#include <iostream>

#include "Client.hpp"

using namespace rfs;

int main ( int argc, char* argv[] )
{
    if ( argc != 2 )
    {
        std::cerr << "Must provide 1 argument: the path to stat()" << std::endl;
        return EXIT_FAILURE;
    }

    Client c;
    Metadata result;

    RetCode rc = c.stat ( argv[1], result );

    if ( NotOk ( rc ) )
    {
        std::cerr << "Unable to stat() " << argv[1] << "; " << rc << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "Successfully called stat() on " << argv[1] << " (" << result.path()
        << "), type: " << result.type() << std::endl;

    return EXIT_SUCCESS;
}
