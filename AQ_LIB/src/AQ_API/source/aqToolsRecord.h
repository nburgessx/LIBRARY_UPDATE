#pragma once

#include <string>

/* @brief			validation interface for the aqToolsVersion function
*  @param [in]		enable		True to enable the recording of tests
*  @param [in]		folder		Output folder
*  @param [in]		repeat		Allowed repeated tests
*  @param [in]		startIndex	Start index for repeated tests
*  @param [in]		maxIndex	Max index for repeated tests
*  @return			A notification string
*/
std::string aqToolsRecord( bool enable,
                             const std::string& folder,
                             bool repeat,
                             int startIndex,
                             int maxIndex );
