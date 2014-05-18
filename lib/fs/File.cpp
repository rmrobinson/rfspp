#include "File.hpp"

#include "Common.pb.h"
#include "Message.pb.h"

#include "FileSystem.hpp"

using namespace rfs;

File::File ( FileSystem& fs, const std::string& name ) : Node ( fs, name )
{
}

RetCode File::read ( std::vector<char>& data, size_t size, size_t offset )
{
    if ( fid_ < 0 )
    {
        return NotOpen;
    }

    proto::ReadMsg msg;
    msg.set_fid ( fid_ );
    msg.set_size ( size );
    msg.set_offset ( offset );

    proto::FileMsg resp;

    RetCode rc = fs_.sendMessage ( msg, resp );

    if ( rc != Success )
    {
        return rc;
    }

    if ( resp.fid() != fid_ || resp.offset() < 0 || (size_t) resp.offset() != offset
        || resp.data().size() > size )
    {
        return InvalidMessage;
    }

    std::copy ( resp.data().begin(), resp.data().end(), std::back_inserter ( data ) );

    return Success;
}

RetCode File::write ( const std::vector<char>& data, size_t size, size_t offset )
{
    if ( fid_ < 0 )
    {
        return NotOpen;
    }

    const size_t actualSize = ( data.size() < size ? data.size() : size );

    proto::FileMsg msg;
    msg.set_fid ( fid_ );
    msg.set_size ( actualSize );
    msg.set_offset ( offset );
    msg.set_data ( std::string ( data.begin(), data.end() ) );

    return fs_.sendMessage ( msg );
}

RetCode File::resize ( const size_t newSize )
{
    if ( fid_ < 0 )
    {
        return NotOpen;
    }

    proto::StatMsg msg;
    msg.set_fid ( fid_ );    

    Metadata meta;
    RetCode rc = fs_.sendMessage ( msg, meta );

    if ( rc != Success )
    {
        return rc;
    }

    if ( meta.name() != name_ )
    {
        return InvalidMessage;
    }

    meta.set_size ( newSize );

    return fs_.sendMessage ( meta );
}

