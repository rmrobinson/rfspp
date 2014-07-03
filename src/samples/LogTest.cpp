
#include <iostream>
#include "utils/Log.hpp"

int main()
{
    rfs::Logger logger ( "rfsTest" );
    logger << rfs::Log::Err << "This is a test of a custom ostream" << std::endl;

    {
        rfs::LogRedirector coutLog ( "rfsTestCout", std::cout );

        std::cout << "This is a test" << std::endl << " with a second line " << std::endl;
        std::cout << "This is another test" << std::endl;
        std::cout << "This is a third test" << std::endl;
    }

    std::cout << "This should be on the console" << std::endl;

    return 0;
}

