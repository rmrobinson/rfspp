#pragma once

#include "Common.pb.h"

namespace rfs
{
class FileSystem;

template<typename M, typename S>
class Controller
{
public:
    Controller ( FileSystem& fs ) : fs_ ( fs ) {}
    virtual ~Controller();

    virtual proto::RetCode set ( M& module, const S& state ) = 0;

    inline FileSystem& getFS() const
    {
        return fs_;
    }

private:
    FileSystem& fs_;

};

}

