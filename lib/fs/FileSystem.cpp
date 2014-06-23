#include "FileSystem.hpp"

using namespace rfs;

FileSystem::~FileSystem()
{
}

RetCode FileSystem::createFile ( const Metadata&, bool, FileHandle& )
{
    return NotImplemented;
}

RetCode FileSystem::openFile ( const std::string&, bool, FileHandle& )
{
    return NotImplemented;
}

RetCode FileSystem::closeFile ( const FileHandle& )
{
    return NotImplemented;
}

RetCode FileSystem::readFile ( const FileHandle&, std::vector<char>&, off_t,
                               size_t& ) const
{
    return NotImplemented;
}

RetCode FileSystem::writeFile ( const FileHandle&, const std::vector<char>&, off_t,
                                size_t& )
{
    return NotImplemented;
}

RetCode FileSystem::resizeFile ( const std::string&, size_t )
{
    return NotImplemented;
}

RetCode FileSystem::createDirectory ( const std::string&, const Metadata& )
{
    return NotImplemented;
}

RetCode FileSystem::readDirectory ( const std::string&, std::vector<Metadata>& ) const
{
    return NotImplemented;
}

RetCode FileSystem::createLink ( const std::string&, const std::string& )
{
    return NotImplemented;
}

RetCode FileSystem::readLink ( const std::string&, std::string& ) const
{
    return NotImplemented;
}

RetCode FileSystem::remove ( const std::string& )
{
    return NotImplemented;
}

RetCode FileSystem::rename ( const std::string&, const std::string& )
{
    return NotImplemented;
}

RetCode FileSystem::readMetadata ( const std::string&, Metadata& ) const
{
    return NotImplemented;
}

RetCode FileSystem::setOwner ( const std::string&, const std::string&,
                               const std::string& )
{
    return NotImplemented;
}

RetCode FileSystem::setMode ( const std::string&, const Metadata::Modes& )
{
    return NotImplemented;
}

