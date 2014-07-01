#include "Log.hpp"

using namespace rfs;

Log::Log ( const std::string& ident, int facility ) : level_ ( Log::Debug )
{
    openlog ( ident.c_str(), LOG_PID, facility ); 
}

Log::~Log()
{
    closelog();
}

int Log::sync()
{
    if ( buf_.length() )
    {
        syslog ( level_, "%s", buf_.c_str() );

        buf_.clear();
        level_ = Log::Debug;
    }

    return 0;
}

Log::int_type Log::overflow ( int_type c )
{
    if ( c != traits_type::eof() )
    {
        buf_ += static_cast<char> ( c );
    }
    else
    {
        sync();
    }

    return c;
}

std::ostream& rfs::operator<< ( std::ostream& os, const Log::Level& ll )
{
    static_cast<Log*> ( os.rdbuf() )->setLevel ( ll );

    return os;
}

