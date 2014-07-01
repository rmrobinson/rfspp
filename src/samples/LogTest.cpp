
#include <iostream>
#include "utils/Log.hpp"

int main()
{
    std::clog.rdbuf ( new rfs::Log ( "rfsTest2", LOG_LOCAL0 ) );

    std::clog << rfs::Log::Err << "This is a test" << std::endl;
    std::clog << rfs::Log::Warning << "This is a warning" << std::endl;

    return 0;
}

