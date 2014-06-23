#pragma once

#undef FUSE_USE_VERSION
#define FUSE_USE_VERSION 30

extern "C"
{
#include <fuse.h>
}

#include <cstdlib>

#include "Common.pb.h"
#include "RetCode.hpp"

namespace rfs
{
class FileSystem;

class FuseBridge
{
public:
    FuseBridge ( FileSystem& fs );
    ~FuseBridge();

    RetCode run ( int argc, char* argv[] );

    /// @brief Implements the FUSE function getattr
    static int getAttr ( const char* path, struct stat* stat );
    /// @brief Implements the FUSE function listxattr
    static int listXAttr ( const char* path, char* mem, size_t memSize );
    /// @brief Implements the FUSE function getxattr
    static int getXAttr ( const char* path, const char* key, char* value,
                          size_t valueSize );
    /// @brief Implements the FUSE function setxattr
    static int setXAttr ( const char* path, const char* key, const char* value,
                          size_t valueSize, int flags );
    /// @brief Implements the FUSE function removexattr
    static int removeXAttr ( const char* path, const char* key );

    /// @brief Implements the FUSE function chown
    static int setOwner ( const char* path, uid_t uid, gid_t gid );
    /// @brief Implements the FUSE function chmod
    static int setMode ( const char* path, mode_t mode );

    /// @brief Implements the FUSE function utimens
    /// @param [in] tv The new time values; tv[0] == atime, tv[1] == mtime.
    /// ctime is always set to 'now'.
    static int setAccessTimes ( const char* path, const struct timespec tv[2] );
    /// @brief Implements the FUSE function rename
    static int rename ( const char* path, const char* newPath );
    /// @brief Implements the FUSE function unlink and removedir
    static int remove ( const char* path );

    /// @brief Implements the FUSE function create
    static int createFile ( const char* path, mode_t mode, struct fuse_file_info* fi );
    /// @brief Implements the FUSE function open
    static int openFile ( const char* path, struct fuse_file_info* fi );
    /// @brief Implements the FUSE function read
    static int readFile ( const char* path, char* mem, size_t memSize, off_t offset,
                          struct fuse_file_info* fi );
    /// @brief Implements the FUSE function write
    static int writeFile ( const char* path, const char* mem, size_t memSize,
                           off_t offset, struct fuse_file_info* fi );
    /// @brief Implements the FUSE function release
    static int closeFile ( const char* path, struct fuse_file_info* fi );
    /// @brief Implements the FUSE function truncate
    static int resizeFile ( const char* path, off_t len );

    /// @brief Implements the FUSE function mkdir
    static int createDirectory ( const char* path, mode_t mode );
    /// @brief Implements the FUSE function readdir
    static int readDirectory ( const char* path, void* mem, fuse_fill_dir_t filler,
                         off_t offset, struct fuse_file_info* fi );

    /// @brief Implements the FUSE function symlink
    static int createSymlink ( const char* from, const char* to );
    /// @brief Implements the FUSE function readlink
    static int readSymlink ( const char* path, char* mem, size_t memSize );

    /// @brief Implements the FUSE function statfs
    static int statfs ( const char* path, struct statvfs* statvfs );

private:
    static void genUserInfo ( uid_t uid, gid_t gid,
                              std::string& username, std::string& groupname );

    static FileSystem* fs_;

    static const std::string RfsXAttrHid;
    static const std::string RfsXAttrFid;
};

}

