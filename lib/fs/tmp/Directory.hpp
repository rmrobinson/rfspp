#pragma once

#include "Node.hpp"

namespace rfs
{

/// @brief A directory, containing a collection of other elements.
/// It is possible to retrieve the names of the items contained in this directory.
/// These items may be other directories, files, or any type of node.
class Directory : public Node
{
public:
    /// @brief Constructor.
    /// @param [in] fs The file system that all directory operations will take place against.
    /// @param [in] name The fully qualified name of the directory to operate on.
    Directory ( FileSystem& fs, const std::string& name );

    /// @brief Retrieve the metadata of the nodes stored in this directory.
    /// @param [out] nodes The metadata of the children of this directory.
    /// If there was an error, this list will not be modified.
    /// @return Standard error code.
    RetCode getChildren ( std::vector<Metadata>& nodes );

    /// @brief Create a new child node in this directory.
    /// @param [in] name The name of this child (not the full path, just file name).
    /// @return Standard error code.
    RetCode addChild ( const std::string& name );
};

}

