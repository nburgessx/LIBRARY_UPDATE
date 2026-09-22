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

/* @brief			swig interface for the aqDateNthECBMeetingDate method
*  @param [in]		asOfDate	The asOfDate
*  @param [in]		n			Which meeting (1 = the next one)
*  @return			Date of the nth ECB (European Central Bank) meeting on/after the asOfDate
*/
std::string aqDateNthECBMeetingDate(const std::string& asOfDate, int n);

/* @brief			swig interface for the aqDateNthECBSwapStartDate method
*  @param [in]		asOfDate	The asOfDate
*  @param [in]		n			Which one (1 = the next)
*  @return			Start date of the nth ECB-dated swap on/after the asOfDate
*/
std::string aqDateNthECBSwapStartDate(const std::string& asOfDate, int n);

/* @brief			swig interface for the aqDateNthECBSwapEndDate method
*  @param [in]		asOfDate	The asOfDate
*  @param [in]		n			Which one (1 = the next)
*  @return			End date of the nth ECB-dated swap on/after the asOfDate
*/
std::string aqDateNthECBSwapEndDate(const std::string& asOfDate, int n);

/* @brief			swig interface for the aqDateNextECBMeetingDate method
*  @param [in]		meetingDate		An ECB meeting date
*  @return			The ECB meeting date immediately after the given meeting date
*/
std::string aqDateNextECBMeetingDate(const std::string& meetingDate);

/* @brief			swig interface for the aqDateNextECBSwapStartDate method
*  @param [in]		swapStartDate	An ECB-dated swap start date
*  @return			The ECB-dated swap start date immediately after the given one
*/
std::string aqDateNextECBSwapStartDate(const std::string& swapStartDate);

/* @brief			swig interface for the aqDateNextECBSwapEndDate method
*  @param [in]		swapEndDate		An ECB-dated swap end date
*  @return			The ECB-dated swap end date immediately after the given one
*/
std::string aqDateNextECBSwapEndDate(const std::string& swapEndDate);
