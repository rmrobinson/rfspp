#pragma once

extern "C"
{
#include <syslog.h>
}

#include <ostream>
#include <string>

namespace rfs
{
    enum Level
    {
        Emerg = LOG_EMERG,
        Alert = LOG_ALERT,
        Crit = LOG_CRIT,
        Err = LOG_ERR,
        Warning = LOG_WARNING,
        Notice = LOG_NOTICE,
        Info = LOG_INFO,
        Debug = LOG_DEBUG
    };

std::ostream& operator<< ( std::ostream& os, const rfs::Level& ll );

class Log : public std::basic_streambuf<char, std::char_traits<char> >
{
public:
    explicit Log ( const std::string& ident, int facility );
    ~Log();

    inline void setLevel ( const Level& level )
    {
        level_ = level;
    }

protected:
    int sync();

    int overflow ( int c );

private:
    friend std::ostream& operator<< ( std::ostream& os, const rfs::Level& ll );

    std::string buf_;
    Level level_;
};

}

