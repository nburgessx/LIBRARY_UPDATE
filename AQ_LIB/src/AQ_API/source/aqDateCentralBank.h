#pragma once

#include <string>

/* @brief			swig interface for the aqDateCentralBank method
*  @param [in]		centralBankId	Central bank Id, e.g. ECB, BoE, Fed
*  @param [in]		date			A base date
*  @param [in]		strictlyAfter	True if strictly after the base date
*  @return			Date of the next central bank meeting on/after the base date
*/
std::string aqDateCentralBank(const std::string& centralBankId, 
							const std::string& date,
							bool strictlyAfter);

/* @brief			swig interface for the aqDateECB method
*  @param [in]		date			Base date
*  @param [in]		strictlyAfter	True if strictly after the base date
*  @return			Date of the next ECB (European Central Bank) meeting on/after the base date
*/
std::string aqDateECB(const std::string& date,
					bool strictlyAfter);

/* @brief			swig interfac for the aqDateECBSwapStart method
*  @param [in]		date	An input date
*  @return			The start date of an ECB(European Central Bank) Swap based on the given date
*/
std::string aqDateECBSwapStart(const std::string& date);

/* @brief			swig interfac for the aqDateECBSwapEnd method
*  @param [in]		date	An input date
*  @return			The end date of an ECB(European Central Bank) Swap based on the given date
*/
std::string aqDateECBSwapEnd(const std::string& date);
