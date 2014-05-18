#include "FuseBridge.hpp"

#include <cassert>
#include <ctime>

#include <sstream>

#include "fs/FileSystem.hpp"
#include "fs/File.hpp"
#include "fs/Directory.hpp"

using namespace rfs;

FileSystem& FuseBridge::getFs()
{
    static __thread FileSystem* fs;

    if ( fs == 0 )
    {
        fs = new FileSystem();
    }

    return *fs;
}

FuseBridge::FuseBridge()
{
}

FuseBridge::~FuseBridge()
{
}

RetCode FuseBridge::run ( int argc, char* argv[] )
{
    struct fuse_operations oper;
    memset ( &oper, 0, sizeof ( oper ) );

    oper.getattr = getAttr;

    int ret = fuse_main ( argc, argv, &oper, 0 );

    if ( ret != 0 )
        return Success;

    return Unknown;
}

int FuseBridge::getAttr ( const char* path, struct stat* stat )
{
    assert ( path != 0 );
    assert ( stat != 0 );

    RetCode rc = Success;

    Node n ( getFs(), path );
    Metadata md;
    rc = n.stat ( md );

    if ( NotOk ( rc ) )
        return -retCodeToErrno ( rc );

    memset ( stat, 0, sizeof ( struct stat ) );
    stat->st_mode = metadataToPosixMode ( md );
    stat->st_size = md.size();
    stat->st_nlink = md.links();

    stat->st_atime = md.atime();
    stat->st_mtime = md.mtime();
    stat->st_ctime = md.ctime();

    return 0;
}

int FuseBridge::create ( const char* path, mode_t mode, struct fuse_file_info* fi )
{
    assert ( path != nullptr );
    assert ( fi != nullptr );

    File f ( getFs(), path );
    
    Metadata md;
    md.set_name ( path );
    posixModeToMetadata ( mode, md );

    std::stringstream uid;
    uid << fuse_get_context()->uid;
    std::stringstream gid;
    gid << fuse_get_context()->gid;

    md.set_uid ( uid.str() );
    md.set_gid ( gid.str() );

    md.set_size ( 0 );
    md.set_links ( 1 );

    const time_t now = time ( 0 );

    md.set_atime ( now );
    md.set_mtime ( now );
    md.set_ctime ( now );

    return retCodeToErrno ( f.create ( md ) );
}

int FuseBridge::open ( const char* path, struct fuse_file_info* fi )
{
    File f ( getFs(), path );
    return retCodeToErrno ( f.open() );
}

int FuseBridge::read ( const char* path, char* mem, size_t memSize, off_t memOff, struct fuse_file_info* fi )
{
    File f ( getFs(), path );
    std::vector<char> data;
    RetCode rc = f.read ( data, memSize, memOff );

    if ( rc != Success )
    {
        return retCodeToErrno ( rc );
    }

    if ( data.size() > memSize )
    {
        data.resize ( memSize );
    }

    assert ( data.size() <= memSize );

    memcpy ( mem, &data[0], data.size() );
    return data.size();
}

int FuseBridge::write ( const char* path, const char* mem, size_t memSize, off_t memOff, struct fuse_file_info* fi )
{
    File f ( getFs(), path );

    const std::vector<char> data ( mem, mem + memSize );

    assert ( memSize == data.size() );

    RetCode rc = f.write ( data, memSize, memOff );

    if ( rc != Success )
    {
        return retCodeToErrno ( rc );
    }

    return memSize;
}

int FuseBridge::release ( const char* path, struct fuse_file_info* fi )
{
    File f ( getFs(), path );
    return retCodeToErrno ( f.close() );
}

mode_t FuseBridge::metadataToPosixMode ( const Metadata& md )
{
    mode_t ret = 0;
    if ( md.type() == Metadata::File || md.type() == Metadata::KeyValueFile )
        ret |= S_IFREG;
    else if ( md.type() == Metadata::Directory )
        ret |= S_IFDIR;
    else if ( md.type() == Metadata::Symlink )
        ret |= S_IFLNK;

    if ( md.modes().user().read() )
        ret |= S_IRUSR;
    if ( md.modes().user().write() )
        ret |= S_IWUSR;
    if ( md.modes().user().execute() )
        ret |= S_IXUSR;

    if ( md.modes().group().read() )
        ret |= S_IRGRP;
    if ( md.modes().group().write() )
        ret |= S_IWGRP;
    if ( md.modes().group().execute() )
        ret |= S_IXGRP;

    if ( md.modes().other().read() )
        ret |= S_IROTH;
    if ( md.modes().other().write() )
        ret |= S_IWOTH;
    if ( md.modes().other().execute() )
        ret |= S_IXOTH;

    return ret;
}

void FuseBridge::posixModeToMetadata ( mode_t mode, Metadata& md )
{
    if ( mode & S_IFREG )
        md.set_type ( Metadata::File );
    else if ( mode & S_IFDIR )
        md.set_type ( Metadata::Directory );
    else if ( mode & S_IFLNK )
        md.set_type ( Metadata::Symlink );

    Metadata::Modes* modes = md.mutable_modes();
    assert ( modes != 0 );

    Metadata::Modes::Values* user = modes->mutable_user();
    Metadata::Modes::Values* group = modes->mutable_group();
    Metadata::Modes::Values* other = modes->mutable_other();
    assert ( user != 0 );
    assert ( group != 0 );
    assert ( other != 0 );

    user->set_read ( ( mode & S_IRUSR ) != 0 );
    user->set_write ( ( mode & S_IWUSR ) != 0 );
    user->set_execute ( ( mode & S_IXUSR ) != 0 );

    group->set_read ( ( mode & S_IRGRP ) != 0 );
    group->set_write ( ( mode & S_IWGRP ) != 0 );
    group->set_execute ( ( mode & S_IXGRP ) != 0 );

    other->set_read ( ( mode & S_IROTH ) != 0 );
    other->set_write ( ( mode & S_IWOTH ) != 0 );
    other->set_execute ( ( mode & S_IXOTH ) != 0 );
}

int FuseBridge::retCodeToErrno ( const RetCode& rc )
{
    if ( rc == NoData )
        return ENODATA;
    else if ( rc == NoSuchPath )
        return ENOENT;
    else if ( rc == InvalidPath || rc == InvalidData )
        return EINVAL;
    else if ( rc == OutOfRange )
        return ERANGE;
    else if ( rc == MemoryError )
        return EFAULT;

    return EPERM;
}

