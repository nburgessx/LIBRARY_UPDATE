#pragma once

#include <string>

/* @brief			validation interface for the aqToolVersion function
*  @param [in]		enable		True to enable the recording of tests
*  @param [in]		folder		Output folder
*  @param [in]		repeat		Allowed repeated tests
*  @param [in]		startIndex	Start index for repeated tests
*  @param [in]		maxIndex	Max index for repeated tests
*  @return			A notification string
*/
std::string aqToolRecord( bool enable,
                             const std::string& folder,
                             bool repeat,
                             int startIndex,
                             int maxIndex );

/* @brief			validation interface for the aqToolReplay function. Replays a recorded input file
*                   back through the validation layer.
*  @param [in]		filepath		Full file path to the recorded test csv file
*  @return			The result string
*/
std::string aqToolReplay( const std::string& filepath );
