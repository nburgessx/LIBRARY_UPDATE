#pragma once

#include "AQLCoreTemplateType.h"

namespace validation
{
    /* @brief			validation interface for the aqToolVersion function
    *  @param [in]		enable		True to enable the recording of tests
    *  @param [in]		folder		Output folder
    *  @param [in]		repeat		Allowed repeated tests
    *  @param [in]		startIndex	Start index for repeated tests
    *  @param [in]		maxIndex	Max index for repeated tests
    *  @return			A notification string
    */
    AQLString tryAqToolRecord( bool enable, const AQLString& folder, bool repeat = false, int startIndex = 0, int maxIndex = 0 );
}