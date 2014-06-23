#include "ProcessFileSystem.hpp"

#include <boost/algorithm/string.hpp>

#include "ProcessFile.hpp"
#include "ProcessDirectory.hpp"

using namespace rfs;

ProcessFileSystem::Entry::Entry ( ProcessFileSystem& _pfs, Entry& _parent,
                                  const std::string& _name )
    : name ( _name ), file ( nullptr ), dir ( nullptr ), parent ( _parent ), pfs ( _pfs )
{
    if ( &parent == this )
        return;

    parent.children.push_back ( this );
}

ProcessFileSystem::Entry::~Entry()
{
    /// @todo: inform file or dir that they are being removed

    for ( size_t i = 0; i < handles.size(); ++i )
    {
        if ( handles.at ( i ) >= 0 )
        {
            pfs.releaseHandle ( handles.at ( i ) );
        }
    }

    bool found = false;
    for ( size_t i = 0; i < parent.children.size(); ++i )
    {
        if ( parent.children.at ( i ) == this )
        {
            parent.children[i] = nullptr;
            found = true;
            break;
        }
    }

    // we are the root node
    if ( this == &parent )
        found = true;

    assert ( found == true );

    for ( size_t i = 0; i < children.size(); ++i )
    {
        if ( children.at ( i ) != 0 )
        {
            delete children.at ( i );
        }
    }
}

ProcessFileSystem::ProcessFileSystem()
    : root_ ( Entry ( *this, root_, "" ) )
{
}

RetCode ProcessFileSystem::openFile ( const std::string& path, bool, FileHandle& fh )
{
    Entry* e = getEntry ( path );

    if ( e == nullptr )
        return NoSuchPath;

    if ( e->file == nullptr )
        return InvalidFileType;

    assert ( e->file != nullptr );
    assert ( e->dir == nullptr );

    fh.Clear();
    fh.set_hid ( HostId );
    fh.set_fid ( genHandle ( e ) );

    assert ( fh.fid() >= 0 );

    RetCode rc = e->file->open ( fh );

    if ( NotOk ( rc ) )
    {
        releaseHandle ( fh.fid() );
        fh.Clear();
    }

    return rc;
}

RetCode ProcessFileSystem::closeFile ( const FileHandle& fh )
{
    if ( fh.hid() != HostId )
        return InvalidFileHandle;

    Entry* e = handles_[fh.fid()];

    if ( e == nullptr )
        return InvalidFileHandle;

    assert ( e->file != nullptr );
    assert ( e->dir == nullptr );

    e->file->close ( fh );

    // we release the file handle regardless of errors (can't really be any)
    releaseHandle ( fh.fid() );

    return Success;
}

RetCode ProcessFileSystem::readFile ( const FileHandle& fh, std::vector<char>& data,
                                      off_t offset, size_t& processed ) const
{
    Entry* e = handles_.at ( fh.fid() );

    if ( e == nullptr || e->file == nullptr )
        return InvalidFileHandle;

    assert ( e->dir == nullptr );

    return e->file->read ( fh, data, offset, processed );
}

RetCode ProcessFileSystem::writeFile ( const FileHandle& fh,
                                       const std::vector<char>& data, off_t offset,
                                       size_t& processed )
{
    Entry* e = handles_.at ( fh.fid() );

    if ( e == nullptr || e->file == nullptr )
        return InvalidFileHandle;

    assert ( e->dir == nullptr );

    return e->file->write ( fh, data, offset, processed );
}

RetCode ProcessFileSystem::readDirectory ( const std::string& path,
                                           std::vector<Metadata>& children ) const
{
    const Entry* e = getEntry ( path );

    if ( e == nullptr )
        return InvalidPath;

    if ( e->file != nullptr )
        return InvalidFileType;

    // directory entries aren't guaranteed to have 'owning' process directory objects;
    // we only need to guarantee it isn't a file (it can't be a file & directory).

    assert ( e->file == nullptr );

    for ( size_t i = 0; i < e->children.size(); ++i )
    {
        const Entry* c = e->children.at ( i );

        if ( c != nullptr )
        {
            std::string p;
            getPath ( *c, p );

            Metadata md;
            md.set_path ( p );

            if ( c->file != nullptr )
                md.set_type ( Metadata::File );
            else if ( c->dir != nullptr )
                md.set_type ( Metadata::Directory );
            else
                md.set_type ( Metadata::Unknown );

            children.push_back ( md );
        }
    }

    return Success;
}

RetCode ProcessFileSystem::readMetadata ( const std::string& path, Metadata& md ) const
{
    const Entry* e = getEntry ( path );

    if ( e == nullptr )
        return InvalidPath;

    if ( e->file != nullptr )
    {
        md = e->file->getMetadata();
        return Success;
    }

    return NotImplemented;
}

bool ProcessFileSystem::addFile ( ProcessFile& file )
{
    Entry* e = getEntry ( file.getPath(), true );
    assert ( e != nullptr );

    // if the path already has children, or if dir is set, it can't be file
    if ( e->children.size() > 0 || e->dir != nullptr )
    {
        return false;
    }

    e->file = &file;

    return true;
}

bool ProcessFileSystem::addDirectory ( ProcessDirectory& dir )
{
    Entry* e = getEntry ( dir.getPath(), true );
    assert ( e != nullptr );

    e->dir = &dir;

    return true;
}

bool ProcessFileSystem::removePath ( const std::string& path, bool recurse )
{
    Entry* entry = getEntry ( path );

    if ( entry == nullptr 
        || entry == &root_
        || ( ! recurse && entry->children.size() > 0 )
        || entry->handles.size() > 0 )
    {
        return false;
    }

    // the destructor for entry() will ensure that:
    // a) children are cleaned up
    // b) the entry is removed from it's parent's set of children
    delete entry;
    return true;
}

ProcessFileSystem::Entry* ProcessFileSystem::getEntry ( const std::string& path,
                                                        bool force )
{
    std::vector<std::string> sPath;
    splitPath ( path, sPath );

    Entry* curr = &root_;

    // When we split '/' we have one entry of size 1, the empty root path
    // so we start indexing at 1
    for ( size_t i = 1; i < sPath.size(); ++i )
    {
        assert ( curr != nullptr );
        // splitPath should remove any empty-length names
        assert ( sPath.at ( i ).size() > 0 );

        bool found = false;

        for ( size_t j = 0; j < curr->children.size(); ++j )
        {
            const Entry* const c = curr->children.at ( j );

            if ( c == nullptr )
                continue;

            if ( sPath.at ( i ) == c->name )
            {
                curr = curr->children.at ( j );
                found = true;
                break;
            }
        }

        if ( ! found )
        {
            if ( ! force )
                return nullptr;

            curr = new Entry ( *this, *curr, sPath.at ( i ) );
        }
    }

    return curr;
}

const ProcessFileSystem::Entry* ProcessFileSystem::getEntry (
    const std::string& path ) const
{
    std::vector<std::string> sPath;
    splitPath ( path, sPath );

    const Entry* curr = &root_;

    // When we split '/' we have one entry of size 1, the empty root path
    // so we start indexing at 1
    for ( size_t i = 1; i < sPath.size(); ++i )
    {
        assert ( curr != nullptr );
        // splitPath should remove any empty-length names
        assert ( sPath.at ( i ).size() > 0 );

        bool found = false;

        for ( size_t j = 0; j < curr->children.size(); ++j )
        {
            const Entry* const c = curr->children.at ( j );

            if ( c == nullptr )
                continue;

            if ( sPath.at ( i ) == c->name )
            {
                curr = curr->children.at ( j );
                found = true;
                break;
            }
        }

        if ( ! found )
            return nullptr;
    }

    return curr;
}

int32_t ProcessFileSystem::genHandle ( Entry* e )
{
    assert ( e != nullptr );

    for ( size_t i = 0; i < handles_.size(); ++i )
    {
        if ( handles_.at ( i ) != nullptr )
        {
            continue;
        }

        handles_[i] = e;
        e->handles.push_back ( i );
        return i;
    }

    handles_.push_back ( e );
    e->handles.push_back ( handles_.size() - 1 );
    return ( handles_.size() - 1 );
}

void ProcessFileSystem::releaseHandle ( int32_t handle )
{
    if ( handle < 0 || (size_t) handle >= handles_.size() )
        return;

    Entry* e = handles_[handle];

    // somebody called close() twice, which isn't fatal
    if ( e == nullptr )
        return;

    for ( size_t i = 0; i < e->handles.size(); ++i )
    {
        if ( e->handles.at ( i ) == handle )
        {
            e->handles[i] = -1;
        }
    }

    handles_[handle] = e = nullptr;
}

void ProcessFileSystem::splitPath ( const std::string& path, std::vector<std::string>& sPath )
{
    std::string tPath ( path );
    boost::trim_if ( tPath, boost::is_any_of ( "/" ) );
    boost::split ( sPath, tPath, boost::is_any_of ( "/" ), boost::token_compress_on );
}

void ProcessFileSystem::getPath ( const Entry& entry, std::string& path )
{
    // if we aren't at the root, get the path of the entry's parent
    if ( &entry != &entry.parent )
    {
        getPath ( entry.parent, path );
    }

    path.append ( entry.name ).append ( "/" );
}

