#pragma once

#include <string>
#include <vector>

#include "ProcessFileSystem.hpp"

namespace rfs
{

/// @brief A base class used by other classes wishing to expose data via the file system.
/// A process file is a single file within the file system of the local process.
/// ProcessFiles can operate in one of two modes:
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
/// All ProcessFile implementations need to implement the read(), write() and size()
/// functions, however if some behaviour isn't required (i.e. read-only files),
/// simply return NotImplemented for the unused operation.
/// Mode 1 is default; the provided implementation of open() and close() does nothing.
/// Classes wishing to support Mode 2 simply need to override the open() and close()
/// functions, which is where file-specific context would be created or destroyed. 
/// The FileHandle object is only really relevant for Mode 2 operation; though it
/// may provide for useful debugging info in Mode 1 scenarios.
class ProcessFile
{
public:
    /// @brief Constructor.
    /// @param [in] fs The file system this file will register with.
    /// @param [in] path The fully qualified path of this file in the fs.
    /// If a file with the path already exists, this instance will replace it.
    ProcessFile ( ProcessFileSystem& fs, const std::string& path );

    /// @brief Destructor.
    /// This will unregister the module from the file system.
    virtual ~ProcessFile();

    /// @brief Open the module for subsequent read/write operations.
    /// @param [in] fd The file handle of the requester.
    /// @return Standard error code.
    virtual RetCode open ( const FileHandle& fh );

    /// @brief Close the module once read/write operations are complete.
    /// @param [in] fh The file handle of the requester.
    /// @return Standard error code.
    virtual RetCode close ( const FileHandle& fh );

    /// @brief Read data from this module.
    /// @param [in] fh The file handle of the requester.
    /// @param [out] data The buffer to store the data in + amount of data to read.
    /// @param [in] offset The offset to start reading the data from.
    /// @param [out] processed The number of bytes filled into the buffer.
    /// This is guaranteed to be less than or equal to data.size().
    /// @return Standard error code.
    virtual RetCode read ( const FileHandle& fh, std::vector<char>& data,
                                  off_t offset, size_t& processed ) = 0;

    /// @brief Write data to this module.
    /// @param [in] fh The file handle of the requester.
    /// @param [in] data The data to write to the file.
    /// @param [in] size The number of bytes to write.
    /// The smaller of size and data.size() will be written.
    /// @param [in] offset The offset from the start of the file to start writing data.
    /// @return Standard error code.
    virtual RetCode write ( const FileHandle& fh, const std::vector<char>& data,
                                   off_t offset, size_t& processed ) = 0;

    /// @brief Exposes the current size of the data which can be read.
    /// @return Amount of data available to be read (in bytes).
    virtual size_t size() const = 0;

    /// @brief The fully qualified path of the file.
    /// @return ProcessFile path.
    const std::string& getPath() const
    {
        return path_;
    }

    /// @brief The metadata associated with this entity.
    /// @return File metadata.
    const Metadata& getMetadata() const
    {
        return md_;
    }
        
protected:
    ProcessFileSystem& fs_; ///< The file system which is managing this module.

private:
    const std::string path_; ///< The name of the module as it exists in the file system.

    Metadata md_; ///< The metadata of this file.
};

}

