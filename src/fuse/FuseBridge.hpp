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
    FuseBridge();
    ~FuseBridge();

    RetCode run ( int argc, char* argv[] );

private:

    static int getAttr ( const char* path, struct stat* stat );
    static int listXAttr ( const char* path, char* mem, size_t memSize );
    static int getXAttr ( const char* path, const char* key, char* value, size_t valueSize );
    static int setXAttr ( const char* path, const char* key, const char* value, size_t valueSize, int flags );
    static int removeXAttr ( const char* path, const char* key );

    static int chown ( const char* path, uid_t uid, gid_t gid );
    static int chmod ( const char* path, mode_t mode );

    static int utimens ( const char* path, const struct timespec tv[2] );

    static int create ( const char* path, mode_t mode, struct fuse_file_info* fi );
    static int open ( const char* path, struct fuse_file_info* fi );
    static int read ( const char* path, char* mem, size_t memSize, off_t memOff, struct fuse_file_info* fi );
    static int write ( const char* path, const char* mem, size_t memSize, off_t memOff, struct fuse_file_info* fi );

    static int release ( const char* path, struct fuse_file_info* fi );
    static int unlink ( const char* path );
    static int rename ( const char* from, const char* to );
    static int truncate ( const char* path, off_t len );

    static int mkdir ( const char* path, mode_t mode );
    static int opendir ( const char* path, struct fuse_file_info* fi );
    static int readdir ( const char* path, void* mem, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info* fi );
    static int releasedir ( const char* path, struct fuse_file_info* fi );

    static int link ( const char* from, const char* to );
    static int symlink ( const char* from, const char* to );
    static int readlink ( const char* path, char* mem, size_t memSize );

    static int statfs ( const char* path, struct statvfs* statvfs );

    static mode_t metadataToPosixMode ( const Metadata& md );
    static void posixModeToMetadata ( mode_t mode, Metadata& md );
    static int retCodeToErrno ( const RetCode& rc );

    static FileSystem& getFs();
};

}

