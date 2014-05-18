#pragma once

#include <vector>

#include "Node.hpp"

namespace rfs
{

/// @brief A standard file, containing arbitrary data.
/// It is possible to read and write either binary or textual data from this file.
/// A typical user of the file would perform the following operations:
/// open() to indicate that future operations will take place on this file.
/// some combination of read() and/or write() operations.
/// close() to indicate that no future operations will take place.
/// Destruction of the file (going out of scope) will cause an implicit close().
class File : public Node
{
public:
    /// @brief Constructor.
    /// @param [in] fs The file system that all file operations will be invoked against.
    /// @param [in] name The fully qualified name of the file to operate on.
    File ( FileSystem& fs, const std::string& name );

    /// @brief Read data from the file.
    /// If a previous call to open() has not succeeded, this will fail.
    /// @param [out] data The contents of the file will be stored in this buffer.
    /// @param [in] size The number of bytes to read, defaults to full file size.
    /// @param [in] offset The offset of the file to read from, defaults to the beginning of the file.
    /// @return Standard error code.
    RetCode read ( std::vector<char>& data, size_t size = SIZE_MAX, size_t offset = 0 );

    /// @brief Write data to the file.
    /// If a previous call to open() has not succeeded, this will fail.
    /// @param [in] data The new contents of the file.
    /// @param [in] size The number of bytes to write to the file. Defaults to the size of data.
    /// @param [in] offset The offset within the file to start writing the data.
    /// @return Standard error code.
    RetCode write ( const std::vector<char>& data, size_t size = SIZE_MAX, size_t offset = 0 );

    /// @brief Resize the file (either larger or smaller).
    /// If a previous call to open() has not succeeded, this will fail.
    /// @param [in] size The new size of the file.
    /// @return Standard error code.
    RetCode resize ( size_t size );
};

}

