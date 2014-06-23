#pragma once

extern "C"
{
#include <sys/types.h>
#include <sys/stat.h>
}

#include "Common.pb.h"
#include "RetCode.hpp"

namespace rfs
{

class PosixUtils
{
public:
    static void metadataToStat ( const Metadata& md, struct stat* s );

    static void statToMetadata ( const struct stat* s, Metadata& md );

    static void posixModeToMetadata ( mode_t mode, Metadata& md );

    static void metadataToPosixMode ( const Metadata& md, mode_t& mode );

    static int retCodeToErrno ( const RetCode& rc );

    static RetCode errnoToRetCode ( int err );
};

}

