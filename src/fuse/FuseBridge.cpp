#include "FuseBridge.hpp"

extern "C"
{
#include <unistd.h>

#include <sys/types.h>
#include <grp.h>
#include <pwd.h>
}

#include <cassert>
#include <ctime>

#include <sstream>

#include "fs/FileSystem.hpp"
#include "fs/PosixUtils.hpp"

using namespace rfs;

FileSystem* FuseBridge::fs_ ( 0 );

const std::string FuseBridge::RfsXAttrHid ( "user.rfs_hostid" );
const std::string FuseBridge::RfsXAttrFid ( "user.rfs_fileid" );

FuseBridge::FuseBridge ( FileSystem& fs )
{
    if ( ! fs_ )
    {
        fs_ = &fs;
    }
}

FuseBridge::~FuseBridge()
{
    fs_ = nullptr;
}

#ifdef __APPLE__
int getXAttrOSX ( const char* path, const char* key, char* value, size_t valueSize,
                      uint32_t )
{
    return FuseBridge::getXAttr ( path, key, value, valueSize );
}

int setXAttrOSX ( const char* path, const char* key, const char* value,
                      size_t valueSize, int flags, uint32_t )
{
    return FuseBridge::setXAttr ( path, key, value, valueSize, flags );
}
#endif

RetCode FuseBridge::run ( int argc, char* argv[] )
{
    struct fuse_operations rfsOper;
    memset ( &rfsOper, 0, sizeof ( rfsOper ) );

    // .mknod is not set because it has been depricated.
    // .fgetattr is not implemented because getattr() will be used instead
    // .ftruncate is not implemented because truncate() will be used instead
    // .access is not implemented because 
    // .flush is not implemented because this doesn't cache data
    // .fsync is not implemented because this doesn't cache data
    // .link is not implemented because hard links are not supported
    // .fsyncdir is not implemented because this doesn't cache data
    // .opendir and .releasedir are not implemented because directory reads are single-shot

    // The following operations deal with all files; regardless of type.
    // That is; any of the following functions can be invoked on
    // regular files, directories, and symlinks
    // The one exception to this is unlink; which only applies to files & symlinks;
    // rmdir is the equivalent of unlink for directories, however
    // the same function is used to remove files and directories (FUSE limitation).

    rfsOper.getattr = FuseBridge::getAttr; // get data for struct stat
    rfsOper.listxattr = FuseBridge::listXAttr; // get list of extended attributes
#ifdef __APPLE__
    rfsOper.getxattr = getXAttrOSX; // get specific extended attribute
    rfsOper.setxattr = setXAttrOSX; // set specific extended attribute
#else
    rfsOper.getxattr = FuseBridge::getXAttr; // get specific extended attribute
    rfsOper.setxattr = FuseBridge::setXAttr; // set specified extended attribute
#endif
    rfsOper.removexattr = FuseBridge::removeXAttr; // remove specific extended attribute

    rfsOper.chown = FuseBridge::setOwner; // set owner and group
    rfsOper.chmod = FuseBridge::setMode; // set access permissions
    rfsOper.utimens = FuseBridge::setAccessTimes; // set access/mod times; supersedes utime

    rfsOper.unlink = FuseBridge::remove; // remove a file
    rfsOper.rmdir = FuseBridge::remove; // remove a directory
    rfsOper.rename = FuseBridge::rename; // rename a file

    // The following operations deal exclusively with regular files.

    rfsOper.create = FuseBridge::createFile; // create & open file (not symlink nor directory)
    rfsOper.open = FuseBridge::openFile; // open file; truncate() may be called first
    rfsOper.read = FuseBridge::readFile; // read data from file
    rfsOper.write = FuseBridge::writeFile; // write data to file
    rfsOper.truncate = FuseBridge::resizeFile; // change file size
    rfsOper.release = FuseBridge::closeFile; // release an open file (1:1 mapping to open())

    // The following operations deal with symlinks

    rfsOper.symlink = FuseBridge::createSymlink; // create a symbolic link
    rfsOper.readlink = FuseBridge::readSymlink; // get the name of the symlink target

    // The following operations deal exclusively with directories

    rfsOper.mkdir = FuseBridge::createDirectory; // create a directory
    rfsOper.readdir = FuseBridge::readDirectory; // read directory entries from directory

    // The following operations operate on the global fs
    // Instead of using init and destroy we pass in the global context to fuse_main
    // this is done because the base path is only known at start time (not in init).

    rfsOper.statfs = FuseBridge::statfs; // get data for struct statvfs (file system stats)

    int ret = fuse_main ( argc, argv, &rfsOper, 0 );

    if ( ret == 0 )
        return Success;

    return Unknown;
}

void FuseBridge::genUserInfo ( uid_t uid, gid_t gid, std::string& username, std::string& groupname )
{
    username.clear();
    groupname.clear();

    std::string hostname;

    hostname.reserve ( _POSIX_HOST_NAME_MAX + 1 );

    struct passwd* user = getpwuid ( uid );
    struct group* group = getgrgid ( gid );

    int ret = gethostname ( &hostname[0], hostname.size() );

    if ( ret < 0 )
    {
        // If we don't have a hostname, it'll just be empty for now.
        hostname.clear();
    }

    if ( user != nullptr )
    {
        username.append ( user->pw_name );
    }
    else
    {
        username.append ( std::to_string ( uid ) );
    }

    username.append ( "@" ).append ( hostname );

    if ( group != nullptr )
    {
        groupname.append ( group->gr_name );
    }
    else
    {
        groupname.append ( std::to_string ( gid ) );
    }

    groupname.append ( "@" ).append ( hostname );
}

int FuseBridge::getAttr ( const char* path, struct stat* stat )
{
    assert ( path != nullptr );
    assert ( stat != nullptr );
    assert ( fs_ != nullptr );

    Metadata md;
    RetCode rc = fs_->readMetadata ( path, md );

    if ( NotOk ( rc ) )
        return -PosixUtils::retCodeToErrno ( rc );

    PosixUtils::metadataToStat ( md, stat );
    return 0;
}

int FuseBridge::listXAttr ( const char* path, char* mem, size_t memSize )
{
    assert ( path != nullptr );
    assert ( mem != nullptr );

    if ( memSize < ( RfsXAttrHid.length() + RfsXAttrFid.length() + 2 ) )
        return -ERANGE;

    size_t size = 0;

    strncpy ( mem + size, RfsXAttrHid.c_str(), RfsXAttrHid.length() );
    size += RfsXAttrHid.length();
    mem[size] = '\0';
    ++size;

    strncpy ( mem + size, RfsXAttrFid.c_str(), RfsXAttrFid.length() );
    size += RfsXAttrHid.length();
    mem[size] = '\0';
    ++size;

    return size;
    
}

int FuseBridge::getXAttr ( const char* path, const char* key, char* value, size_t valueSize )
{
    assert ( path != nullptr );
    assert ( key != nullptr );
    assert ( value != nullptr );
    assert ( fs_ != nullptr );

    Metadata md;
    RetCode rc = fs_->readMetadata ( path, md );

    if ( NotOk ( rc ) )
        return -PosixUtils::retCodeToErrno ( rc );

    const std::string keyS ( key );

    if ( keyS == RfsXAttrHid )
    {
        const size_t size = md.hid().length();

        if ( valueSize == 0 )
            return size;
        else if ( valueSize < size )
            return -ERANGE;

        strncpy ( value, md.hid().c_str(), size );
        return size;
    }
    else if ( keyS == RfsXAttrFid )
    {
        const size_t size = md.fid().length();

        if ( valueSize == 0 )
            return size;
        else if ( valueSize < size )
            return -ERANGE;

        strncpy ( value, md.fid().c_str(), size );
        return size;
    }

    return -ENOATTR;
}

int FuseBridge::setXAttr ( const char* path, const char* key, const char* value,
                           size_t valueSize, int flags )
{
    assert ( path != nullptr );
    assert ( key != nullptr );
    assert ( value != nullptr );
    assert ( fs_ != nullptr );

    return -ENOTSUP;
}

int FuseBridge::removeXAttr ( const char* path, const char* key )
{
    assert ( path != nullptr );
    assert ( key != nullptr );
    assert ( fs_ != nullptr );

    return -ENOTSUP;
}

int FuseBridge::setOwner ( const char* path, uid_t uid, gid_t gid )
{
    assert ( path != nullptr );
    assert ( fs_ != nullptr );

    std::string username;
    std::string groupname;

    genUserInfo ( uid, gid, username, groupname );

    RetCode rc = fs_->setOwner ( path, username, groupname );

    if ( NotOk ( rc ) )
        return -PosixUtils::retCodeToErrno ( rc );

    return 0;
}

int FuseBridge::setMode ( const char* path, mode_t mode )
{
    assert ( path != nullptr );
    assert ( fs_ != nullptr );

    Metadata tmp;
    PosixUtils::posixModeToMetadata ( mode, tmp );

    RetCode rc = fs_->setMode ( path, tmp.modes() );

    if ( NotOk ( rc ) )
        return -PosixUtils::retCodeToErrno ( rc );

    return 0;
}

int FuseBridge::setAccessTimes ( const char* path, const struct timespec tv[2] )
{
    assert ( path != nullptr );
    assert ( fs_ != nullptr );

    // tv[0] is the atime, tv[1] is the mtime

    return -ENOTSUP;
}

int FuseBridge::rename ( const char* path, const char* newPath )
{
    assert ( path != nullptr );
    assert ( newPath != nullptr );
    assert ( fs_ != nullptr );

    RetCode rc = fs_->rename ( path, newPath );

    if ( NotOk ( rc ) )
        return -PosixUtils::retCodeToErrno ( rc );

    return 0;
}

int FuseBridge::remove ( const char* path )
{
    assert ( path != nullptr );
    assert ( fs_ != nullptr );

    RetCode rc = fs_->remove ( path );

    if ( NotOk ( rc ) )
        return -PosixUtils::retCodeToErrno ( rc );

    return 0;
}

int FuseBridge::createFile ( const char* path, mode_t mode, struct fuse_file_info* fi )
{
    assert ( path != nullptr );
    assert ( fi != nullptr );
    assert ( fs_ != nullptr );

    Metadata md;
    md.set_path ( path );
    PosixUtils::posixModeToMetadata ( mode, md );

    std::string username;
    std::string groupname;

    genUserInfo ( fuse_get_context()->uid, fuse_get_context()->gid,
                  username, groupname );

    md.set_uid ( username );
    md.set_gid ( groupname );
    md.set_size ( 0 );

    const time_t now = time ( 0 );

    md.set_atime ( now );
    md.set_mtime ( now );
    md.set_ctime ( now );

    bool reqWrite = false;

    if ( ( ( fi->flags & O_WRONLY ) == O_WRONLY )
         || ( ( fi->flags & O_RDWR ) == O_RDWR ) )
        reqWrite = true;

    FileHandle* fh = new FileHandle();

    RetCode rc = fs_->createFile ( md, reqWrite, *fh );

    if ( NotOk ( rc ) )
    {
        delete fh;

        return -PosixUtils::retCodeToErrno ( rc );
    }

    fi->fh = reinterpret_cast<uint64_t> ( fh );
    return 0;
}

int FuseBridge::openFile ( const char* path, struct fuse_file_info* fi )
{
    assert ( path != nullptr );
    assert ( fi != nullptr );
    assert ( fs_ != nullptr );

    bool reqWrite = false;

    if ( ( ( fi->flags & O_WRONLY ) == O_WRONLY )
         || ( ( fi->flags & O_RDWR ) == O_RDWR ) )
        reqWrite = true;

    FileHandle* fh = new FileHandle();

    RetCode rc = fs_->openFile ( path, reqWrite, *fh );

    if ( NotOk ( rc ) )
    {
        delete fh;

        return -PosixUtils::retCodeToErrno ( rc );
    }

    fi->fh = reinterpret_cast<uint64_t> ( fh );
    return 0;
}

int FuseBridge::readFile ( const char*, char* mem, size_t memSize, off_t offset,
                           struct fuse_file_info* fi )
{
    assert ( mem != nullptr );
    assert ( fi != nullptr );

    FileHandle* fh = reinterpret_cast<FileHandle*> ( fi->fh );
    assert ( fh != nullptr );

    std::vector<char> output;
    output.reserve ( memSize );

    size_t processed = 0;

    RetCode rc = fs_->readFile ( *fh, output, offset, processed );

    if ( NotOk ( rc ) )
        return -PosixUtils::retCodeToErrno ( rc );

    size_t outputSize = output.size();

    if ( processed != outputSize )
        outputSize = processed;

    if ( outputSize > memSize )
        outputSize = memSize;

    if ( output.size() != outputSize )
        output.resize ( outputSize );

    assert ( output.size() <= memSize );

    memcpy ( mem, &output[0], output.size() );
    return output.size();
}

int FuseBridge::writeFile ( const char*, const char* mem, size_t memSize, off_t offset,
                            struct fuse_file_info* fi )
{
    assert ( mem != nullptr );
    assert ( fi != nullptr );
    assert ( fs_ != nullptr );

    FileHandle* fh = reinterpret_cast<FileHandle*> ( fi->fh );
    assert ( fh != nullptr );

    const std::vector<char> data ( mem, mem + memSize );
    assert ( memSize == data.size() );

    size_t processed = 0;

    RetCode rc = fs_->writeFile ( *fh, data, offset, processed );

    if ( NotOk ( rc ) )
        return -PosixUtils::retCodeToErrno ( rc );

    return processed;
}

int FuseBridge::closeFile ( const char*, struct fuse_file_info* fi )
{
    assert ( fi != nullptr );
    assert ( fs_ != nullptr );

    FileHandle* fh = reinterpret_cast<FileHandle*> ( fi->fh );
    assert ( fh != nullptr );

    RetCode rc = fs_->closeFile ( *fh );

    delete fh;
    fh = nullptr;
    fi->fh = 0;

    if ( NotOk ( rc ) )
        return -PosixUtils::retCodeToErrno ( rc );

    return 0;
}

int FuseBridge::resizeFile ( const char* path, off_t len )
{
    assert ( path != nullptr );
    assert ( fs_ != nullptr );

    RetCode rc = fs_->resizeFile ( path, len );

    if ( NotOk ( rc ) )
        return -PosixUtils::retCodeToErrno ( rc );

    return 0;
}

int FuseBridge::createDirectory ( const char* path, mode_t mode )
{
    assert ( path != nullptr );
    assert ( fs_ != nullptr );

    Metadata md;
    md.set_path ( path );
    PosixUtils::posixModeToMetadata ( mode, md );

    std::string username;
    std::string groupname;

    genUserInfo ( fuse_get_context()->uid, fuse_get_context()->gid,
                  username, groupname );

    md.set_uid ( username );
    md.set_gid ( groupname );
    md.set_size ( 0 );

    const time_t now = time ( 0 );

    md.set_atime ( now );
    md.set_mtime ( now );
    md.set_ctime ( now );

    RetCode rc = fs_->createDirectory ( md.path(), md );

    if ( NotOk ( rc ) )
        return -PosixUtils::retCodeToErrno ( rc );

    return 0;
}

int FuseBridge::readDirectory ( const char* path, void* mem, fuse_fill_dir_t filler,
                                off_t, struct fuse_file_info* )
{
    assert ( path != nullptr );
    assert ( mem != nullptr );
    assert ( fs_ != nullptr );

    std::vector<Metadata> children;

    RetCode rc = fs_->readDirectory ( path, children );

    if ( NotOk ( rc ) )
        return -PosixUtils::retCodeToErrno ( rc );

    filler ( mem, ".", 0, 0 );
    filler ( mem, "..", 0, 0 );

    for ( size_t i = 0; i < children.size(); ++i )
    {
        const Metadata& md = children.at ( i );

        std::string name;

        std::string tmp ( md.path() );

        if ( tmp.length() > 0 )
        {
            // remove any possible trailing slashes
            if ( tmp.at ( tmp.length() ) == '/' )
                tmp = tmp.substr ( 0, tmp.length() - 1 );

            size_t lastSlash = tmp.find_last_of ( '/' );

            if ( lastSlash == std::string::npos )
                name = tmp;
            else
                name = tmp.substr ( lastSlash + 1 );
        }

        struct stat s;
        PosixUtils::metadataToStat ( md, &s );
        filler ( mem, name.c_str(), &s, 0 );
    }

    return 0;
}

int FuseBridge::createSymlink ( const char* from, const char* to )
{
    assert ( from != nullptr );
    assert ( to != nullptr );

    RetCode rc = fs_->createLink ( from, to );

    if ( NotOk ( rc ) )
        return -PosixUtils::retCodeToErrno ( rc );

    return 0;
}

int FuseBridge::readSymlink ( const char* path, char* mem, size_t memSize )
{
    assert ( path != nullptr );
    assert ( mem != nullptr );
    assert ( fs_ != nullptr );

    std::string name;
    name.reserve ( memSize );

    RetCode rc = fs_->readLink ( path, name );

    if ( name.length() >= memSize )
        return -ENAMETOOLONG;

    memcpy ( mem, name.c_str(), name.length() );

    return 0;
}

int FuseBridge::statfs ( const char*, struct statvfs* statvfs )
{
    assert ( statvfs != nullptr );
    assert ( fs_ != nullptr );

    return 0;
}

