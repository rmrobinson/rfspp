#pragma once

extern "C"
{
#include <syslog.h>
}

#include <ostream>
#include <string>

namespace rfs
{

/// @brief A class which provides a streambuf interface to the POSIX syslog facility.
/// This class can be used to redirect the standard cout/cerr/clog streams to syslog.
class Log : public std::streambuf
{
public:
    /// @brief The possible levels used by syslog to mark data.
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

    /// @brief Constructor.
    /// @param [in] ident The string used to identify this output in syslog.
    /// @param [in] facility The syslog facility to log this data as.
    explicit Log ( const std::string& ident, int facility );

    /// @brief Destructor.
    ~Log();

    /// @brief Set the current logging level of the stream.
    /// @param [in] level The new level.
    inline void setLevel ( const Level& level )
    {
        level_ = level;
    }

protected:
    /// @brief Overrides the default streambuf function to output data to syslog
    /// @return Always yields 0; there isn't an error case.
    int sync();

    /// @brief Overrides the default streambuf function to save a character to this stream's buffer.
    /// @param [in] c The character to process.
    /// @return The character which was processed.
    int_type overflow ( int_type c );

private:
    std::string buf_; ///< The buffer of seen characters, not yet written to syslog.
    Level level_; ///< The log level which the buf_ contents will be logged to.
};

/// @brief Streaming operator; allows log levels to be set on a Log stream
/// @param [in] os The output stream being provided the log level
/// @param [in] ll The new log level to set on the stream.
/// @return The stream being used; should equal os
std::ostream& operator<< ( std::ostream& os, const Log::Level& ll );

}

