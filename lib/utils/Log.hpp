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

/// @brief An output log stream which will redirect all output to syslog
/// This can be used in lieu of redirecting one of cout/cerr/clog since it can
/// be created locally in a thread, whereas cout/cerr/clog will be per process.
class Logger : public std::ostream
{
public:
    /// @brief Constructor.
    /// @param [in] name The syslog ident name.
    explicit Logger ( const std::string& name );

    /// @brief Destructor.
    virtual ~Logger() {};

    /// @brief Streaming output operator.
    /// @param [in] level The level this line should be logged at.
    Logger& operator<< ( const Log::Level& level );

protected:
    Log l_; ///< The log stream to output to
};

/// @brief A class which will handle redirecting an existing output stream to syslog
/// This can be used on things like cout or cerr; and when destructed will point the
/// log stream to its original target.
class LogRedirector
{
public:
    /// @brief Constructor.
    /// @param [in] name The syslog ident tag to give this stream.
    /// @param [in] stream The output stream to redirect.
    explicit LogRedirector ( const std::string& name, std::ostream& stream );

    /// @brief Destructor. Will re-point the provided output stream to its original output buffer.
    ~LogRedirector();

private:
    Log l_; ///< The syslog entry to send all output to.

    std::ostream& stream_; ///< The original stream buffer this is wrapping.
    std::streambuf* const streamBuf_; ///< The original streambuffer used by stream_.
};

}

