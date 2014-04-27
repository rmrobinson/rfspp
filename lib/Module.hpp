#pragma once

#include <string>
#include <vector>
#include <boost/uuid/uuid.hpp>

#include "Common.pb.h"

namespace rfs
{
class FileSystem;

/// @brief A base class used by other classes wishing to expose data via the file system.
/// A module is a single file within the file system.
/// Modules can operate in one of two modes:
/// 1) Each time the file is opened, read/write operations take place on the same object
/// 2) Each time the file is opened, read/write operations take place on independent objects.
/// Mode 1 is the behaviour exposed by a typical OS file today, where if two different
/// callers open a file, they see the same content.
/// Mode 2 is behaviour which is based on the file system capabilities of Plan 9,
/// whereby RPC-style operations can take place across a file, with each user of a file
/// having its own 'view' of the file contents.
///
/// One example of Mode 2 is a file which performs DNS resolution.
/// Each time the resolution file is opened, it creates a new resolution context,
/// which subsequent write & read operations will operate on. Each write() to a file
/// would start a new resolution request, with read() operations returning the result
/// of the resolution.
/// This allows objects to perform resolution on different hosts without requring
/// that they synchronize access to a single file.
///
/// All Module implementations need to implement the read(), write() and size()
/// functions, however if some behaviour isn't required (i.e. read-only files),
/// simply return NotImplemented for the unused operation.
/// Mode 1 is default; the provided implementation of open() and close() does nothing.
/// Classes wishing to support Mode 2 simply need to override the open() and close()
/// functions, which is where file-specific context would be created or destroyed. 
/// The FileHandle object is only really relevant for Mode 2 operation; though it
/// may provide for useful debugging info in Mode 1 scenarios.
class Module
{
public:
    /// @brief The unique identifier of a file instance.
    struct FileHandle
    {
        boost::uuids::uuid fsid; ///< The file system ID of this handle.
        int32_t fd; ///< The file descriptor.
    };

    /// @brief Constructor.
    /// @param [in] fs The file system this module will register with.
    /// @param [in] name The fully qualified name of this module in the fs.
    /// If a file with the name already exists, this instance will replace it.
    /// @param [in] mode The file access mode.
    Module ( FileSystem& fs, const std::string& name, int mode );

    /// @brief Destructor.
    /// This will unregister the module from the file system.
    virtual ~Module();

    /// @brief Open the module for subsequent read/write operations.
    /// @param [in] fd The file handle of the requester.
    /// @return Standard error code.
    virtual proto::RetCode open ( const FileHandle& fh );

    /// @brief Close the module once read/write operations are complete.
    /// @param [in] fh The file handle of the requester.
    /// @return Standard error code.
    virtual proto::RetCode close ( const FileHandle& fh );

    /// @brief Read data from this module.
    /// @param [in] fh The file handle of the requester.
    /// @param [out] data The buffer to store the data in.
    /// @param [in] size The number of bytes to read from the module.
    /// @param [in] offset The offset to start reading the data from.
    /// @return Standard error code.
    virtual proto::RetCode read ( const FileHandle& fh, std::vector<char>& data,
                                  size_t size, size_t offset ) = 0;

    /// @brief Write data to this module.
    /// @param [in] fh The file handle of the requester.
    /// @param [in] data The data to write to the file.
    /// @param [in] size The number of bytes to write.
    /// The smaller of size and data.size() will be written.
    /// @param [in] offset The offset from the start of the file to start writing data.
    /// @return Standard error code.
    virtual proto::RetCode write ( const FileHandle& fh, const std::vector<char>& data,
                                   size_t size, size_t offset ) = 0;

    /// @brief Exposes the current size of the data which can be read.
    /// @return Amount of data available to be read (in bytes).
    virtual size_t size() = 0;

    /// @brief The fully qualified name of the module.
    /// @return Module name.
    const std::string& getName() const
    {
        return name_;
    }

    /// @brief The access mode of the module.
    /// @return File access mode.
    int getMode() const
    {
        return mode_;
    }
        
private:
    FileSystem& fs_; ///< The file system which is managing this module.

    const std::string name_; ///< The name of the module as it exists in the file system.

    const int mode_; ///< The mode of this file.
};

}

