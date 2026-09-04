#pragma once

#include "AQLString.h"

namespace validation
{
    /* @brief			Function to load and replay a test file
    *  @param [in]		filepath		Full file path to the test csv file
    */
    const AQLString tryAqToolsReplay( const AQLString& filepath );
}