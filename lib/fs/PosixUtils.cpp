#include "PosixUtils.hpp"

#include <cassert>
#include <cerrno>

using namespace rfs;

void PosixUtils::metadataToStat ( const Metadata& md, struct stat* s )
{
    assert ( s != nullptr );

    memset ( s, 0, sizeof ( struct stat ) );

    s->st_size = md.size();
    s->st_nlink = 1;

    s->st_atime = md.atime();
    s->st_mtime = md.mtime();
    s->st_ctime = md.ctime();

    metadataToPosixMode ( md, s->st_mode );
}

void PosixUtils::statToMetadata ( const struct stat* s, Metadata& md )
{
    assert ( s != nullptr );

    md.Clear();

    md.set_size ( s->st_size );

    md.set_atime ( s->st_atime );
    md.set_mtime ( s->st_mtime );
    md.set_ctime ( s->st_ctime );

    posixModeToMetadata ( s->st_mode, md );
}

void PosixUtils::posixModeToMetadata ( mode_t mode, Metadata& md )
{
    if ( ( mode & S_IFREG ) == S_IFREG )
        md.set_type ( Metadata::File );
    else if ( ( mode & S_IFDIR ) == S_IFDIR )
        md.set_type ( Metadata::Directory );
    else if ( ( mode & S_IFLNK ) == S_IFLNK )
        md.set_type ( Metadata::Symlink );

    Metadata::Modes* modes = md.mutable_modes();
    assert ( modes != 0 );

    Metadata::Modes::Values* user = modes->mutable_user();
    Metadata::Modes::Values* group = modes->mutable_group();
    Metadata::Modes::Values* other = modes->mutable_other();
    assert ( user != 0 );
    assert ( group != 0 );
    assert ( other != 0 );

    user->set_read ( ( mode & S_IRUSR ) == S_IRUSR );
    user->set_write ( ( mode & S_IWUSR ) == S_IWUSR );
    user->set_execute ( ( mode & S_IXUSR ) == S_IXUSR );

    group->set_read ( ( mode & S_IRGRP ) == S_IRGRP );
    group->set_write ( ( mode & S_IWGRP ) == S_IWGRP );
    group->set_execute ( ( mode & S_IXGRP ) == S_IXGRP );

    other->set_read ( ( mode & S_IROTH ) == S_IROTH );
    other->set_write ( ( mode & S_IWOTH ) == S_IWOTH );
    other->set_execute ( ( mode & S_IXOTH ) == S_IXOTH );
}

void PosixUtils::metadataToPosixMode ( const Metadata& md, mode_t& mode )
{
    mode = 0;

    if ( md.type() == Metadata::File || md.type() == Metadata::KeyValueFile )
        mode |= S_IFREG;
    else if ( md.type() == Metadata::Directory )
        mode |= S_IFDIR;
    else if ( md.type() == Metadata::Symlink )
        mode |= S_IFLNK;

    if ( md.modes().user().read() )
        mode |= S_IRUSR;
    if ( md.modes().user().write() )
        mode |= S_IWUSR;
    if ( md.modes().user().execute() )
        mode |= S_IXUSR;

    if ( md.modes().group().read() )
        mode |= S_IRGRP;
    if ( md.modes().group().write() )
        mode |= S_IWGRP;
    if ( md.modes().group().execute() )
        mode |= S_IXGRP;

    if ( md.modes().other().read() )
        mode |= S_IROTH;
    if ( md.modes().other().write() )
        mode |= S_IWOTH;
    if ( md.modes().other().execute() )
        mode |= S_IXOTH;
}

int PosixUtils::retCodeToErrno ( const RetCode& rc )
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

RetCode PosixUtils::errnoToRetCode ( int err )
{
    if ( err == ENODATA )
        return NoData;
    else if ( err == ENOENT )
        return NoSuchPath;
    else if ( err == EINVAL )
        return InvalidPath;
    else if ( err == ERANGE )
        return OutOfRange;
    else if ( err == EFAULT )
        return MemoryError;

    return InvalidPermissions;
}

