// tryMirReplay.h
#pragma once

#include "LAString.h"

namespace validation_api
{
    /* @brief			Function to load and replay a test file
    *  @param [in]		filepath		Full file path to the test csv file
    */
    const LAString tryMirReplay( const LAString& filepath );
}