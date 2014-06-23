#pragma once

#include <string>
#include <vector>

#include "FileSystem.hpp"

namespace rfs
{
class ProcessFile;
class ProcessDirectory;

class ProcessFileSystem : public FileSystem
{
public:
    friend class ProcessFile;
    friend class ProcessDirectory;

    ProcessFileSystem();

    virtual RetCode openFile ( const std::string& path, bool reqWrite, FileHandle& fh );
    virtual RetCode closeFile ( const FileHandle& fh );

    virtual RetCode readFile ( const FileHandle& fh, std::vector<char>& data,
                               off_t offset, size_t& processed ) const;
    virtual RetCode writeFile ( const FileHandle& fh, const std::vector<char>& data,
                                off_t offset, size_t& processed );

    virtual RetCode readDirectory ( const std::string& path,
                                    std::vector<Metadata>& children ) const;

    virtual RetCode readMetadata ( const std::string& path, Metadata& md ) const;

protected:
    bool addFile ( ProcessFile& file );
    bool addDirectory ( ProcessDirectory& dir );
    bool removePath ( const std::string& path, bool recurse = false );

private:
    struct Entry
    {
        Entry ( ProcessFileSystem& pfs, Entry& parent, const std::string& name );
        ~Entry();

        const std::string name;
        Metadata md;

        ProcessFile* file;
        ProcessDirectory* dir;

        Entry& parent;
        std::vector<Entry*> children;

        ProcessFileSystem& pfs;
        std::vector<int32_t> handles;
    };

    static void splitPath ( const std::string& path, std::vector<std::string>& sPath );
    static void getPath ( const Entry& entry, std::string& path );

    const Entry* getEntry ( const std::string& path ) const; 
    Entry* getEntry ( const std::string& path, bool force = false );

    int32_t genHandle ( Entry* e );
    void releaseHandle ( int32_t handle );

    Entry root_;

    std::vector<Entry*> handles_;
};

}

