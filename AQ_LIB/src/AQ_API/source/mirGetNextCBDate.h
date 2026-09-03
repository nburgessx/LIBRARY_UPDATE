#pragma once

#include <string>

/* @brief			swig interface for the mirGetNextCBDate method
*  @param [in]		centralBankId	Central bank Id, e.g. ECB, BoE, Fed
*  @param [in]		baseDate		Base date
*  @param [in]		strictlyAfter	True if strictly after the base date
*  @return			Date of the next central bank meeting on/after the base date
*/
std::string mirGetNextCBDate(const std::string& centralBankId, 
							const std::string& baseDate,
							bool strictlyAfter);