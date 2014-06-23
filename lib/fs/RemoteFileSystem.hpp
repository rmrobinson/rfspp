#pragma once

#include <unordered_map>
#include <vector>
#include <boost/uuid/uuid.hpp>

#include "Common.pb.h"
#include "RetCode.hpp"

namespace rfs
{

namespace proto
{
class OpenMsg;
class CloseMsg;
class ReadMsg;
class FileMsg;
class DirectoryMsg;
class SymlinkMsg;
class StatMsg;
class RemoveMsg;
class ResponseMsg;
}

/// @brief The foundation of the rfs system.
/// All file system items; including Files, Modules, etc. operate on an instance of a
/// file system, i.e. this class. Each file system has a collection of local files,
/// i.e. all of the modules which have been registered with this file system, and
/// access to a collection of remote files.
///
/// Each FileSystem object is not thread safe, if the file system is going to be used
/// in a multi-threaded program each thread should have its own instance of the file
/// system.
///
/// This file system is blocking; operations on items which are not hosted locally
/// could block for potentially an extended period of time.
///
/// It may be desirable to create different instances of a file system if the file
/// system will be configured to connect to different remote servers, for example.
class RemoteFileSystem
{
public:

    /// @brief Constructor.
    /// This does not connect to remote servers; that is done later.
    RemoteFileSystem();
    /// @brief Destructor.
    /// If the file system is connected to any remote instances, they will be disconnected.
    ~RemoteFileSystem();

protected:

    /// @brief Request that the specified file be opened.
    /// @return Either sucess (if the operation succeeded), or the relevant error code from the Response message.
    RetCode sendMessage ( const proto::OpenMsg& req );

    /// @brief Request that the specified file be closed.
    /// @return Either sucess (if the operation succeeded), or the relevant error code from the Response message.
    RetCode sendMessage ( const proto::CloseMsg& req );

    /// @brief Request that the specified file be read.
    /// @param [out] resp The specified file contents; only valid if the return code is success.
    /// @return Either sucess (if the operation succeeded), or the relevant error code from the Response message.
    RetCode sendMessage ( const proto::ReadMsg& req, proto::FileMsg& resp );

    /// @brief Request that the specified directory be read.
    /// @param [out] resp The specified directory contents; only valid if the return code is success.
    /// @return Either sucess (if the operation succeeded), or the relevant error code from the Response message.
    RetCode sendMessage ( const proto::ReadMsg& req, proto::DirectoryMsg& resp );

    /// @brief Request that the specified symbolic link be read.
    /// @param [out] resp The specified symlink contents; only valid if the return code is success.
    /// @return Either sucess (if the operation succeeded), or the relevant error code from the Response message.
    RetCode sendMessage ( const proto::ReadMsg& req, proto::SymlinkMsg& resp );

    /// @brief Request that the specified file metadata be read.
    /// @param [out] resp The metadata of the specified file; only valid if the return code is success.
    /// @return Either sucess (if the operation succeeded), or the relevant error code from the Response message.
    RetCode sendMessage ( const proto::StatMsg& req, Metadata& resp );

    /// @brief Create a new file or directory, resize an existing file or directory, or change ownership.
    /// @return Either success (if the operation succeeded), or the relevant error code from the Response message.
    RetCode sendMessage ( const Metadata& req );

    /// @brief Remove an existing file or directory.
    /// @return Either success (if the operation succeeded), or the relevant error code from the Response message.
    RetCode sendMessage ( const proto::RemoveMsg& req );

    /// @brief Write new contents to a file.
    /// @return Either success (if the operation succeeded), or the relevant error code from the Response message.
    RetCode sendMessage ( const proto::FileMsg& req );

    /// @brief Create a new symlink, or update an existing symlink.
    /// @return Either success (if the operation succeeded), or the relevant error code from the Response message.
    RetCode sendMessage ( const proto::SymlinkMsg& req, proto::ResponseMsg& resp );
        
private:
    // int fd_;
};

}

