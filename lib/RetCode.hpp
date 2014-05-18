#pragma once

#include "RetCode.pb.h"

namespace rfs
{

inline bool IsOk ( const RetCode& rc )
{
    return ( rc == Success );
}

inline bool NotOk ( const RetCode& rc )
{
    return ( ! IsOk ( rc ) );
}

}

