#pragma once

#include <string>
#include <boost/uuid/uuid.hpp>

#include "Common.pb.h"

namespace rfs
{
class FileSystem;

/// @brief A class with functionality common across all filesystem items.
/// Examples include:
///  - renaming a (file/directory/symlink)
///  - removing a (file/directory/symlink)
/// Classes wishing to expose file system behaviour (for example, files or directories)
/// should inherit from this class.
/// This allows the FileSystem object to manage all items in the file system identically.
/// The FileSystem 'owns' all items inheriting from Node; if the FileSystem is destroyed,
/// no Node can perform any further operations.
class Node
{
public:
    friend class FileSystem;

    /// @brief Constructor.
    /// @param [in] fs The file system owning this node.
    Node ( FileSystem& fs, const std::string& name );
    /// @brief Destructor
    /// This will cause the item to be closed, if it is currently open.
    virtual ~Node();

    /// @brief Open the node for subsequent operations.
    /// If the node does not exist in the file system, this operation will fail.
    /// @return Standard error code.
    proto::RetCode open();

    /// @brief Close the node.
    /// This won't do anything if the node is not currently open.
    /// @return Standard error code.
    proto::RetCode close();

    /// @brief Change the name of this item.
    /// If a particular implementation wishes to not allow this, then this function
    /// can be overriden to simply return a NotSupported error.
    /// @param [in] name The new name for this item.
    /// @return Standard error code.
    virtual proto::RetCode rename ( const std::string& name );

    /// @brief Remove this item from the file system
    /// If this file has already been removed, future operations will fail.
    /// @return Standard error code.
    virtual proto::RetCode remove();

    /// @brief Retrieve the name of this item.
    /// @return Item name.
    inline const std::string& getName() const
    {
        return name_;
    }

    /// @brief Set the owner of this particular node.
    /// This can be overriden, if desired, i.e for classes which wish to do additional
    /// verification before changing the owner.
    /// @param [in] user The ID of the new owning user.
    /// @param [in] group The ID of the new owning group.
    /// @return Standard error code.
    virtual proto::RetCode setOwner ( const boost::uuids::uuid& user, const boost::uuids::uuid& group );

    /// @brief Set the mode of the node (UNIX mode).
    /// @param [in] mode Bitmask of mode information.
    /// @return Standard error code.
    virtual proto::RetCode setMode ( int mode );

protected:
    FileSystem& fs_; ///< The FileSystem instance this node belongs to.
    const std::string name_; ///< The name of this node, fully qualified.

    int32_t fid_; ///< The file identifier used for file system operations.
};

}

