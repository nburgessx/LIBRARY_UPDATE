// Replay.h
#pragma once

#include "Singleton.h"
#include "AQLString.h"

namespace etrading
{
    /* @brief			Function to load and replay a test file
    *  @param [in]		filepath		Full file path to the test csv file
    */
    const AQLString replay( const AQLString& filepath );

}
