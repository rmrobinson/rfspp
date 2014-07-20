#pragma once

#include "RetCode.pb.h"

namespace rfs
{

/// @brief Check if the return code indicates a successful state.
/// @param [in] rc The return code to check.
/// @return true if the ret code indicates success; false otherwise.
inline bool IsOk ( const RetCode& rc )
{
    return ( rc == Success );
}

/// @brief Check if the return code indicates a failure state.
/// @param [in] rc The return code to check.
/// @return true if the ret code does not indicate success; false otherwise.
inline bool NotOk ( const RetCode& rc )
{
    return ( ! IsOk ( rc ) );
}

}

