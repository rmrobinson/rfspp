#pragma once

#include "RetCode.hpp"

namespace rfs
{
class ProcessFileSystem;

template<typename M, typename S>
class Controller
{
public:
    Controller ( ProcessFileSystem& fs ) : fs_ ( fs ) {}
    virtual ~Controller();

    virtual RetCode set ( M& pfs, const S& state ) = 0;

    inline ProcessFileSystem& getFS() const
    {
        return fs_;
    }

private:
    ProcessFileSystem& fs_;

};

}

