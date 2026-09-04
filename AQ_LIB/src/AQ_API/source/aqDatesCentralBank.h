#pragma once

#include <string>

/* @brief			swig interface for the aqDatesCentralBank method
*  @param [in]		centralBankId	Central bank Id, e.g. ECB, BoE, Fed
*  @param [in]		date			A base date
*  @param [in]		strictlyAfter	True if strictly after the base date
*  @return			Date of the next central bank meeting on/after the base date
*/
std::string aqDatesCentralBank(const std::string& centralBankId, 
							const std::string& date,
							bool strictlyAfter);

/* @brief			swig interface for the aqDatesECB method
*  @param [in]		date			Base date
*  @param [in]		strictlyAfter	True if strictly after the base date
*  @return			Date of the next ECB (European Central Bank) meeting on/after the base date
*/
std::string aqDatesECB(const std::string& date,
					bool strictlyAfter);

/* @brief			swig interfac for the aqDatesECBSwapStart method
*  @param [in]		date	An input date
*  @return			The start date of an ECB(European Central Bank) Swap based on the given date
*/
std::string aqDatesECBSwapStart(const std::string& date);

/* @brief			swig interfac for the aqDatesECBSwapEnd method
*  @param [in]		date	An input date
*  @return			The end date of an ECB(European Central Bank) Swap based on the given date
*/
std::string aqDatesECBSwapEnd(const std::string& date);
