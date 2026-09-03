/* 
 * @brief			Swig interface to Java for mirGetNextCBDate methods
 * @Created:		04 April 2016 
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

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