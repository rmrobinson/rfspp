#pragma once

#include <string>
#include <vector>

#include "ProcessFileSystem.hpp"

namespace rfs
{

class ProcessDirectory
{
public:
    ProcessDirectory ( ProcessFileSystem& fs, const std::string& path );
    virtual ~ProcessDirectory();

    /// @brief Create a file with the specified name (not path!)
    RetCode createFile ( const std::string& name );

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

