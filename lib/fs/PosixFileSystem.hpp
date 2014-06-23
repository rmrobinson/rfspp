#pragma once

#include "FileSystem.hpp"

namespace rfs
{

class PosixFileSystem : public FileSystem
{
public:
    PosixFileSystem ( const std::string& rootPath = "" );
    virtual ~PosixFileSystem();

    virtual RetCode createFile ( const Metadata& md, bool reqWrite, FileHandle& fh );
    virtual RetCode openFile ( const std::string& path, bool reqWrite, FileHandle& fh );
    virtual RetCode closeFile ( const FileHandle& fh );

    virtual RetCode readFile ( const FileHandle& fh, std::vector<char>& data,
                               off_t offset, size_t& processed ) const;
    virtual RetCode writeFile ( const FileHandle& fh, const std::vector<char>& data,
                                off_t offset, size_t& processed );
    virtual RetCode resizeFile ( const std::string& path, size_t size );

    virtual RetCode createDirectory ( const std::string& path, const Metadata& md );
    virtual RetCode readDirectory ( const std::string& path,
                                    std::vector<Metadata>& children ) const;


    virtual RetCode createLink ( const std::string& target, const std::string& link );
    virtual RetCode readLink ( const std::string& target, std::string& link ) const;

    virtual RetCode remove ( const std::string& path );
    virtual RetCode rename ( const std::string& from, const std::string& to );

    virtual RetCode readMetadata ( const std::string& path, Metadata& md ) const;

    virtual RetCode setOwner ( const std::string& path, const std::string& user,
                               const std::string& group );

    virtual RetCode setMode ( const std::string& path, const Metadata::Modes& mode );

private:
    const std::string rootPath_;

    std::vector<int> fds_;

};

}

