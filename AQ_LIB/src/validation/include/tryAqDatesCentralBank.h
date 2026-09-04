#pragma once

#include "AQLDate.h"

namespace validation
{

    /* @brief			validation interface for the aqDatesCentralBank method
    *  @param [in]		centralBankId	Central bank Id, e.g. ECB, BoE, Fed
    *  @param [in]		date			A base date
    *  @param [in]		strictlyAfter	True if strictly after the base date
    *  @return			Date of the next central bank meeting on/after the base date
    */
    AQLDate tryAqDatesCentralBank( const AQLString& centralBankId,
                                 const AQLDate& date,
                                 bool strictlyAfter );

    /* @brief			validation interface for the aqDatesECB method
    *  @param [in]		date			A base date
    *  @param [in]		strictlyAfter	True if strictly after the input date
    *  @return			Date of the ECB (European Central Bank) meeting on/after the base date
    */
    AQLDate tryAqDatesECB( const AQLDate& date,
                         bool strictlyAfter = true );

    /* @brief			validation interface for the aqDatesECBSwapStart method
    *  @param [in]		date	An input date
    *  @return			The start date of an ECB(European Central Bank) Swap based on the given date
    */
    AQLDate tryAqDatesECBSwapStart( const AQLDate& date );

    /* @brief			validation interface for the aqDatesECBSwapEnd method
    *  @param [in]		date	An input date
    *  @return			The end date of an ECB(European Central Bank) Swap based on the given date
    */
    AQLDate tryAqDatesECBSwapEnd( const AQLDate& date );

	/* @brief			Function to calculate the nth ECB Meeting Date
	*  @param [in]		asOfDate	The asOfDate
	*  @param [in]		n			The nth date to return	
	*  @return			Date of the nth ECB (European Central Bank) meeting on/after the asOfDate
	*/
	AQLDate tryAqDatesNthECBMeetingDate( const AQLDate& asOfDate, const int n);

	/* @brief			Function to calculate the nth ECB Swap Start Date
	*  @param [in]		asOfDate	The asOfDate
	*  @param [in]		n			The nth date to return
	*  @return			Start Date of the nth ECB Swap Start Date on/after the asOfDate
	*/
	AQLDate tryAqDatesNthECBSwapStartDate(const AQLDate& asOfDate, const int n);
	
	/* @brief			Function to calculate the nth ECB Swap End Date
	*  @param [in]		asOfDate	The asOfDate
	*  @param [in]		n			The nth date to return
	*  @return			End Date of the nth ECB Swap on/after the asOfDate
	*/
	AQLDate tryAqDatesNthECBSwapEndDate(const AQLDate& asOfDate, const int n);

	/* @brief			Function to calculate the next ECB Meeting Date
	*  @param [in]		meetingDate		The ECB meeting date
	*  @return			Next ECB Meeting Date
	*/
	AQLDate tryAqDatesNextECBMeetingDate(const AQLDate& meetingDate);

	/* @brief			Function to calculate the next ECB Swap Start Date
	*  @param [in]		swapStartDate		The ECB swap start date
	*  @return			Next ECB Swap Start Date
	*/
	AQLDate tryAqDatesNextECBSwapStartDate(const AQLDate& swapStartDate);

	/* @brief			Function to calculate the next ECB Swap End Date
	*  @param [in]		swapEndDate		The ECB swap end date
	*  @return			Next ECB Swap End Date
	*/
	AQLDate tryAqDatesNextECBSwapEndDate(const AQLDate& swapEndDate);

}
