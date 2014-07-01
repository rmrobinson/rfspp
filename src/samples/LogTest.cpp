
#include <iostream>
#include "utils/Log.hpp"

int main()
{
    std::clog.rdbuf ( new rfs::Log ( "rfsTest", LOG_LOCAL0 ) );

    std::clog << rfs::Err << "This is a test" << std::endl;
    std::clog << rfs::Warning << "This is a warning" << std::endl;

    return 0;
}

