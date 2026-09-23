// aqBondSchedule.h

/*
 * @brief			Swig interface for the stateless aqBondSchedule function - builds a bond's cashflow schedule from a label/value block
 */

#pragma once

#include "SwigTypes.h"

#include <string>
#include <vector>

/* @brief			swig interface for aqBondScheduleLVBKeys function
*  @param [out]		Returns the expected keys for a bond leg schedule label/value block
*/
std::vector<std::string> aqBondScheduleLVBKeys();

/* @brief			swig interface for aqBondSchedule function
*  @param [in]		bondScheduleLVB		A label value block defining the bond
*  @param [in]		validateKeys		True to validate the keys provided are valid
*  @param [in]		showColumnHeaders	True to include the column headers in the output
*  @param [out]		Returns a matrix of the bond's cashflow schedule
*/
SWIG_STRINGMATRIX aqBondSchedule( const SWIG_STRINGMATRIX& bondScheduleLVB, const bool validateKeys, const bool showColumnHeaders );
