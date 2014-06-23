#include "ProcessFile.hpp"

using namespace rfs;

ProcessFile::ProcessFile ( ProcessFileSystem& fs, const std::string& path ) : fs_ ( fs ), path_ ( path )
{
    fs_.addFile ( *this );

    md_.set_type ( Metadata::File );
    md_.set_path ( path );

    md_.set_uid ( "invalid_uid" );
    md_.set_gid ( "invalid_gid" );

    Metadata::Modes* modes = md_.mutable_modes();
    Metadata::Modes::Values* mUser = modes->mutable_user();
    mUser->set_read ( true );
    mUser->set_write ( true );
    mUser->set_execute ( false );

    Metadata::Modes::Values* mGroup = modes->mutable_group();
    mGroup->set_read ( true );
    mGroup->set_write ( true );
    mGroup->set_execute ( false );

    Metadata::Modes::Values* mOther = modes->mutable_other();
    mOther->set_read ( true );
    mOther->set_write ( true );
    mOther->set_execute ( false );

    const time_t now = time ( nullptr );

    md_.set_mtime ( now );
    md_.set_ctime ( now );
    md_.set_atime ( now );
}

ProcessFile::~ProcessFile()
{
    fs_.removePath ( getPath() );
}

RetCode ProcessFile::open ( const FileHandle& )
{
    /// @todo Log something useful
    return Success;
}

RetCode ProcessFile::close ( const FileHandle& )
{
    /// @todo Log something useful
    return Success;
}

