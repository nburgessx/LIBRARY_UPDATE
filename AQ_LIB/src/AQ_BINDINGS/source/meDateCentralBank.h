/* 
 * @brief			Swig interface to Java for meDate CentralBank and ECB related methods
 * @Created:		03 June 2016
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#pragma once

#include <string>

/* @brief			swig interface for the meDateCentralBank method
*  @param [in]		centralBankId	Central bank Id, e.g. ECB, BoE, Fed
*  @param [in]		date			A base date
*  @param [in]		strictlyAfter	True if strictly after the base date
*  @return			Date of the next central bank meeting on/after the base date
*/
std::string meDateCentralBank(const std::string& centralBankId, 
							const std::string& date,
							bool strictlyAfter);

/* @brief			swig interface for the meDateECB method
*  @param [in]		date			Base date
*  @param [in]		strictlyAfter	True if strictly after the base date
*  @return			Date of the next ECB (European Central Bank) meeting on/after the base date
*/
std::string meDateECB(const std::string& date,
					bool strictlyAfter);

/* @brief			swig interfac for the meDateECBSwapStart method
*  @param [in]		date	An input date
*  @return			The start date of an ECB(European Central Bank) Swap based on the given date
*/
std::string meDateECBSwapStart(const std::string& date);

/* @brief			swig interfac for the meDateECBSwapEnd method
*  @param [in]		date	An input date
*  @return			The end date of an ECB(European Central Bank) Swap based on the given date
*/
std::string meDateECBSwapEnd(const std::string& date);
