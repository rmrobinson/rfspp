#include "Log.hpp"

#include <iostream>

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

Logger::Logger ( const std::string& name ) : std::ostream ( &l_ ), l_ ( name, LOG_LOCAL0 )
{
}

Logger& Logger::operator<< ( const Log::Level& level )
{
    l_.setLevel ( level );
    return *this;
}

LogRedirector::LogRedirector ( const std::string& name, std::ostream& stream )
    : l_ ( name, LOG_LOCAL0 ), stream_ ( stream ), streamBuf_ ( stream.rdbuf() )
{
    stream_.rdbuf ( &l_ );

    if ( &stream == &std::cout )
    {
        l_.setLevel ( Log::Warning );
    }
    else if ( &stream == &std::cerr )
    {
        l_.setLevel ( Log::Err );
    }
}

LogRedirector::~LogRedirector()
{
    stream_.flush();

    stream_.rdbuf ( streamBuf_ );
}
