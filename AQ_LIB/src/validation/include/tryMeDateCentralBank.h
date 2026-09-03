#pragma once

#include "LADate.h"

namespace validation
{

    /* @brief			validation interface for the meDateCentralBank method
    *  @param [in]		centralBankId	Central bank Id, e.g. ECB, BoE, Fed
    *  @param [in]		date			A base date
    *  @param [in]		strictlyAfter	True if strictly after the base date
    *  @return			Date of the next central bank meeting on/after the base date
    */
    LADate tryMeDateCentralBank( const LAString& centralBankId,
                                 const LADate& date,
                                 bool strictlyAfter );

    /* @brief			validation interface for the meDateECB method
    *  @param [in]		date			A base date
    *  @param [in]		strictlyAfter	True if strictly after the input date
    *  @return			Date of the ECB (European Central Bank) meeting on/after the base date
    */
    LADate tryMeDateECB( const LADate& date,
                         bool strictlyAfter = true );

    /* @brief			validation interface for the meDateECBSwapStart method
    *  @param [in]		date	An input date
    *  @return			The start date of an ECB(European Central Bank) Swap based on the given date
    */
    LADate tryMeDateECBSwapStart( const LADate& date );

    /* @brief			validation interface for the meDateECBSwapEnd method
    *  @param [in]		date	An input date
    *  @return			The end date of an ECB(European Central Bank) Swap based on the given date
    */
    LADate tryMeDateECBSwapEnd( const LADate& date );

	/* @brief			Function to calculate the nth ECB Meeting Date
	*  @param [in]		asOfDate	The asOfDate
	*  @param [in]		n			The nth date to return	
	*  @return			Date of the nth ECB (European Central Bank) meeting on/after the asOfDate
	*/
	LADate tryMeDateNthECBMeetingDate( const LADate& asOfDate, const int n);

	/* @brief			Function to calculate the nth ECB Swap Start Date
	*  @param [in]		asOfDate	The asOfDate
	*  @param [in]		n			The nth date to return
	*  @return			Start Date of the nth ECB Swap Start Date on/after the asOfDate
	*/
	LADate tryMeDateNthECBSwapStartDate(const LADate& asOfDate, const int n);
	
	/* @brief			Function to calculate the nth ECB Swap End Date
	*  @param [in]		asOfDate	The asOfDate
	*  @param [in]		n			The nth date to return
	*  @return			End Date of the nth ECB Swap on/after the asOfDate
	*/
	LADate tryMeDateNthECBSwapEndDate(const LADate& asOfDate, const int n);

	/* @brief			Function to calculate the next ECB Meeting Date
	*  @param [in]		meetingDate		The ECB meeting date
	*  @return			Next ECB Meeting Date
	*/
	LADate tryMeDateNextECBMeetingDate(const LADate& meetingDate);

	/* @brief			Function to calculate the next ECB Swap Start Date
	*  @param [in]		swapStartDate		The ECB swap start date
	*  @return			Next ECB Swap Start Date
	*/
	LADate tryMeDateNextECBSwapStartDate(const LADate& swapStartDate);

	/* @brief			Function to calculate the next ECB Swap End Date
	*  @param [in]		swapEndDate		The ECB swap end date
	*  @return			Next ECB Swap End Date
	*/
	LADate tryMeDateNextECBSwapEndDate(const LADate& swapEndDate);

}
