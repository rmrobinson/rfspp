#include "PosixFileSystem.hpp"

extern "C"
{
#include <fcntl.h>
#include <unistd.h>

#include <sys/types.h>
#include <dirent.h>

#include <grp.h>
#include <pwd.h>
}

#include <cerrno>
#include <cstdio>

#include "PosixUtils.hpp"

using namespace rfs;

PosixFileSystem::PosixFileSystem ( const std::string& rootPath ) : rootPath_ ( rootPath )
{
    assert ( rootPath_.back() != '/' );
}

PosixFileSystem::~PosixFileSystem()
{
    for ( size_t i = 0; i < fds_.size(); ++i )
    {
        if ( fds_.at ( i ) >= 0 )
            close ( fds_.at ( i ) );
    }

    fds_.clear();
}

RetCode PosixFileSystem::createFile ( const Metadata& md, bool reqWrite, FileHandle& fh )
{
    std::string p ( rootPath_ );
    p.append ( md.path() );

    mode_t mode = 0;
    PosixUtils::PosixUtils::metadataToPosixMode ( md, mode );
    
    if ( md.type() != Metadata::File )
    {
        return InvalidMetadata;
    }

    int flags = O_CREAT | O_EXCL;

    if ( reqWrite )
        flags |= O_RDWR;
    else
        flags |= O_RDONLY;

    int fd = open ( p.c_str(), flags, mode );

    if ( fd < 0 )
        return PosixUtils::errnoToRetCode ( errno );

    RetCode rc = setOwner ( md.path(), md.uid(), md.gid() );
    if ( NotOk ( rc ) )
    {
        close ( fd );
        return rc;
    }

    fds_.push_back ( fd );

    fh.Clear();
    fh.set_fid ( fds_.size() );
    fh.set_hid ( HostId );

    return Success;
}

RetCode PosixFileSystem::openFile ( const std::string& path, bool reqWrite,
                                    FileHandle& fh )
{
    std::string p ( rootPath_ );
    p.append ( path );

    int flags = 0;

    if ( reqWrite )
        flags |= O_RDWR;
    else
        flags |= O_RDONLY;

    int fd = open ( p.c_str(), flags );

    if ( fd < 0 )
        return PosixUtils::errnoToRetCode ( errno );

    fds_.push_back ( fd );

    fh.Clear();
    fh.set_fid ( fds_.size() );
    fh.set_hid ( HostId );

    return Success;
}

RetCode PosixFileSystem::closeFile ( const FileHandle& fh )
{
    if ( fh.hid() != HostId || fh.fid() < 0 || (size_t) fh.fid() >= fds_.size()
         || fds_.at ( fh.fid() < 0 ) )
    {
        return InvalidFileHandle;
    }

    close ( fds_.at ( fh.fid() ) );
    fds_.at ( fh.fid() ) = -1;

    return Success;
}

RetCode PosixFileSystem::readFile ( const FileHandle& fh, std::vector<char>& data,
                                    off_t offset, size_t& processed ) const
{
    if ( fh.hid() != HostId || fh.fid() < 0 || (size_t) fh.fid() >= fds_.size()
         || fds_.at ( fh.fid() < 0 ) )
    {
        return InvalidFileHandle;
    }

    ssize_t ret = pread ( fds_.at ( fh.fid() ), &data[0], data.size(), offset );

    if ( ret < 0 )
        return PosixUtils::errnoToRetCode ( errno );

    processed = ret;

    return Success;
}

RetCode PosixFileSystem::writeFile ( const FileHandle& fh, const std::vector<char>& data,
                                     off_t offset, size_t& processed )
{
    if ( fh.hid() != HostId || fh.fid() < 0 || (size_t) fh.fid() >= fds_.size()
         || fds_.at ( fh.fid() < 0 ) )
    {
        return InvalidFileHandle;
    }

    ssize_t ret = pwrite ( fds_.at ( fh.fid() ), &data[0], data.size(), offset );

    if ( ret < 0 )
        return PosixUtils::errnoToRetCode ( errno );

    processed = ret;

    return Success;
}

RetCode PosixFileSystem::resizeFile ( const std::string& path, size_t size )
{
    std::string p ( rootPath_ );
    p.append ( path );

    int ret = truncate ( p.c_str(), size );

    if ( ret < 0 )
        return PosixUtils::errnoToRetCode ( errno );

    return Success;
}

RetCode PosixFileSystem::createDirectory ( const std::string& path, const Metadata& md )
{
    std::string p ( rootPath_ );
    p.append ( path );

    mode_t mode = 0;
    PosixUtils::metadataToPosixMode ( md, mode );

    if ( md.type() != Metadata::Directory )
    {
        return InvalidMetadata;
    }

    int ret = mkdir ( p.c_str(), mode );

    if ( ret < 0 )
        return PosixUtils::errnoToRetCode ( errno );

    return setOwner ( path, md.uid(), md.gid() );
}

RetCode PosixFileSystem::readDirectory ( const std::string& path,
                                         std::vector<Metadata>& children ) const
{
    std::string p ( rootPath_ );
    p.append ( path );

    DIR* dir = opendir ( p.c_str() );

    if ( dir == nullptr )
        return PosixUtils::errnoToRetCode ( errno );

    for ( struct dirent* de = readdir ( dir ); de != nullptr; de = readdir ( dir ) )
    {
        Metadata md;
        if ( de->d_type == DT_DIR )
            md.set_type ( Metadata::Directory );
        else if ( de->d_type == DT_LNK )
            md.set_type ( Metadata::Symlink );
        else if ( de->d_type == DT_REG )
            md.set_type ( Metadata::File );
        else
            md.set_type ( Metadata::Unknown );

        md.set_size ( de->d_reclen );

        std::string name ( de->d_name );

        if ( name.find ( rootPath_ ) == 0 )
            name = name.substr ( rootPath_.length() );

        md.set_path ( name );

        children.push_back ( md );
    }

    closedir ( dir );

    return Success;
}

RetCode PosixFileSystem::createLink ( const std::string& target,
                                      const std::string& link )
{
    std::string t ( rootPath_ );
    t.append ( target );
    std::string l ( rootPath_ );
    l.append ( link );

    if ( symlink ( t.c_str(), l.c_str() ) < 0 )
        return PosixUtils::errnoToRetCode ( errno );

    return Success;
}

RetCode PosixFileSystem::readLink ( const std::string& path, std::string& link ) const
{
    std::string p ( rootPath_ );
    p.append ( path );

    link.clear();
    link.reserve ( PATH_MAX );

    int ret = readlink ( p.c_str(), &link[0], link.capacity() -1 );

    if ( ret < 0 )
        return PosixUtils::errnoToRetCode ( errno );

    // if this isn't true, the OS didn't obey the function signature
    assert ( (size_t) ret < link.capacity() );

    // readlink does not null terminate its values
    link[ret] = '\0';

    if ( link.find ( rootPath_ ) != 0 )
        return MalformedLink;

    // Trim out the root path from the returned link
    link = link.substr ( rootPath_.length() );

    return Success;
}

RetCode PosixFileSystem::remove ( const std::string& path )
{
    std::string p ( rootPath_ );
    p.append ( path );

    int ret = remove ( p.c_str() );

    if ( ret == 0 )
    {
        return Success;
    }

    assert ( ret == -1 );

    /// @todo Perform a recursive remove if necessary
    return PosixUtils::errnoToRetCode ( errno );
}

RetCode PosixFileSystem::rename ( const std::string& from, const std::string& to )
{
    std::string f ( rootPath_ );
    f.append ( from );
    std::string t ( rootPath_ );
    t.append ( to );

    if ( rename ( f.c_str(), t.c_str() ) < 0 )
        return PosixUtils::errnoToRetCode ( errno );

    return Success;
}

RetCode PosixFileSystem::readMetadata ( const std::string& path, Metadata& md ) const
{
    std::string p ( rootPath_ );
    p.append ( path );

    struct stat s;
    memset ( &s, 0, sizeof ( s ) );

    if ( stat ( p.c_str(), &s ) < 0 )
        return PosixUtils::errnoToRetCode ( errno );

    PosixUtils::statToMetadata ( &s, md );
    return Success;
}

RetCode PosixFileSystem::setOwner ( const std::string& path, const std::string& user,
                                    const std::string& group )
{
    std::string p ( rootPath_ );
    p.append ( path );

    const size_t userAtIdx = user.find ( '@' );

    std::string username;
    std::string userhost;

    if ( userAtIdx != std::string::npos )
    {
        username = user.substr ( 0, userAtIdx );
        userhost = user.substr ( userAtIdx + 1 );
    }
    else
    {
        username = user;
    }

    struct passwd* pw = getpwnam ( username.c_str() );

    if ( pw == nullptr )
        return InvalidUser;

    const size_t groupAtIdx = group.find ( '@' );

    std::string groupname;
    std::string grouphost;

    if ( groupAtIdx != std::string::npos )
    {
        groupname = group.substr ( 0, groupAtIdx );
        grouphost = group.substr ( groupAtIdx + 1 );
    }
    else
    {
        groupname = group;
    }

    struct group* gr = getgrnam ( groupname.c_str() );

    if ( gr == nullptr )
        return InvalidGroup;

    if ( chown ( p.c_str(), pw->pw_uid, gr->gr_gid ) < 0 )
        return PosixUtils::errnoToRetCode ( errno );

    return Success;
}

RetCode PosixFileSystem::setMode ( const std::string& path,
                                   const Metadata::Modes& modes )
{
    std::string p ( rootPath_ );
    p.append ( path );

    mode_t mode = 0;
    Metadata tmp;
    Metadata::Modes* tmpModes = tmp.mutable_modes();
    *tmpModes = modes;

    PosixUtils::metadataToPosixMode ( tmp, mode );

    if ( chmod ( p.c_str(), mode ) < 0 )
        return PosixUtils::errnoToRetCode ( errno );

    return Success;
}

