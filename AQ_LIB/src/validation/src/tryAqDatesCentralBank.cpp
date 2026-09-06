#include "tryAqDatesCentralBank.h"
#include "AQLDateHelpers.h"
#include "CreateDataFile.h"
#include "StructuredExceptionHandler.h"
#include "RecordMacros.h"

namespace validation
{
    using etrading::CreateDataFile;
    using etrading::decorateFilename;

    /* @brief			validation interface for the aqDatesCentralBank method
    *  @param [in]		centralBankId	Central bank Id, e.g. ECB, BoE, Fed
    *  @param [in]		date			A base date
    *  @param [in]		strictlyAfter	True if strictly after the base date
    *  @return			Date of the next central bank meeting on/after the base date
    */
    AQLDate tryAqDatesCentralBank( const AQLString& centralBankId,
                                 const AQLDate& date,
                                 bool strictlyAfter )
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        AQ_RECORD_INPUTS( centralBankId, date, strictlyAfter );

        AQLDate ret = etrading::AQLDateHelpers::getNextCBDate( centralBankId, date, strictlyAfter );

        AQ_RECORD_OUTPUTS( ret );

        return ret;

        VALID_EXCEPTION_END
    }

    /* @brief			validation interface for the aqDatesECB method
    *  @param [in]		date			A base date
    *  @param [in]		strictlyAfter	True if strictly after the input date
    *  @return			Date of the ECB (European Central Bank) meeting on/after the base date
    */
    AQLDate tryAqDatesECB( const AQLDate& date,
                         bool strictlyAfter )
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        AQ_RECORD_INPUTS( date, strictlyAfter );

        AQLDate ret = etrading::AQLDateHelpers::getNextECBDate( date, strictlyAfter );

        AQ_RECORD_OUTPUTS( ret );

        return ret;

        VALID_EXCEPTION_END
    }

    /* @brief			validation interface for the aqDatesECBSwapStart method
    *  @param [in]		date	An input date
    *  @return			The start date of an ECB(European Central Bank) Swap based on the given date
    */
    AQLDate tryAqDatesECBSwapStart( const AQLDate& date )
    {
        VALID_EXCEPTION_START_WITH_NO_THREAD_GUARD

        // Recording of inputs for playback
        AQ_RECORD_INPUTS( date );

        //Get the ECB date
        AQLDate ecbDate = tryAqDatesECB( date, false );

        //Get the swap start date based on the ECB date
        AQLDate ret = etrading::AQLDateHelpers::getECBStartDate( ecbDate );

        AQ_RECORD_OUTPUTS( ret );

        return ret;

        VALID_EXCEPTION_END
    }


    /* @brief			validation interface for the aqDatesECBSwapEnd method
    *  @param [in]		date	An input date
    *  @return			The end date of an ECB(European Central Bank) Swap based on the given date
    */
    AQLDate tryAqDatesECBSwapEnd( const AQLDate& date )
    {
        VALID_EXCEPTION_START_WITH_NO_THREAD_GUARD

        // Recording of inputs for playback
        AQ_RECORD_INPUTS( date );

        //Get the first ECB date based on the input date, always Thursday
        AQLDate firstEcbDate = tryAqDatesECB( date, false );

        //Get the second ECB date based on the first ECB, always Thursday
        AQLDate secondEcbDate = tryAqDatesECB( firstEcbDate, true );

        //Get the swap start date of the second ECB date, always Wednesday
        AQLDate ret = tryAqDatesECBSwapStart( secondEcbDate );

        //Always Tuesday
        ret.addDays( -1 );

        AQ_RECORD_OUTPUTS( ret );

        return ret;

        VALID_EXCEPTION_END
    }

	/* @brief			Function to calculate the nth ECB Meeting Date
	*  @param [in]		asOfDate	The asOfDate
	*  @param [in]		n			The nth date to return
	*  @return			Date of the nth ECB (European Central Bank) meeting on/after the asOfDate
	*/
	AQLDate tryAqDatesNthECBMeetingDate(const AQLDate& asOfDate, const int n)
	{
		VALID_EXCEPTION_START
		
		// Record Inputs for logs, tests and playback
		AQ_RECORD_INPUTS(asOfDate, n);

		// Calculation
		const AQLDate result = etrading::AQLDateHelpers::getNthECBMeetingDate(asOfDate, n);

		// Record Outputs AND Return the Result for logs, tests and playback
		AQ_RECORD_OUTPUTS_AND_RETURN_RESULT(result);

		VALID_EXCEPTION_END
	}

	/* @brief			Function to calculate the nth ECB Swap Start Date
	*  @param [in]		asOfDate	The asOfDate
	*  @param [in]		n			The nth date to return
	*  @return			Date of the nth ECB Swap Start Date on/after the asOfDate
	*/
	AQLDate tryAqDatesNthECBSwapStartDate(const AQLDate& asOfDate, const int n)
	{
		VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
		AQ_RECORD_INPUTS(asOfDate, n);

		// Calculation
		const AQLDate result = etrading::AQLDateHelpers::getNthECBSwapStartDate(asOfDate, n);

		// Record Outputs AND Return the Result for logs, tests and playback
		AQ_RECORD_OUTPUTS_AND_RETURN_RESULT(result);

		VALID_EXCEPTION_END
	}

	/* @brief			Function to calculate the nth ECB Swap End Date
	*  @param [in]		asOfDate	The asOfDate
	*  @param [in]		n			The nth date to return
	*  @return			Date of the nth ECB Swap End Date on/after the asOfDate
	*/
	AQLDate tryAqDatesNthECBSwapEndDate(const AQLDate& asOfDate, const int n)
	{
		VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
		AQ_RECORD_INPUTS(asOfDate, n);

		// Calculation
		const AQLDate result = etrading::AQLDateHelpers::getNthECBSwapEndDate(asOfDate, n);

		// Record Outputs AND Return the Result for logs, tests and playback
		AQ_RECORD_OUTPUTS_AND_RETURN_RESULT(result);

		VALID_EXCEPTION_END
	}

	/* @brief			Function to calculate the next ECB Meeting Date
	*  @param [in]		meetingDate		The ECB meeting date
	*  @return			Next ECB Meeting Date
	*/
	AQLDate tryAqDatesNextECBMeetingDate(const AQLDate& meetingDate)
	{
		VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
		AQ_RECORD_INPUTS(meetingDate);

		// Calculation
		const AQLDate result = etrading::AQLDateHelpers::getNextECBMeetingDate(meetingDate);

		// Record Outputs AND Return the Result for logs, tests and playback
		AQ_RECORD_OUTPUTS_AND_RETURN_RESULT(result);

		VALID_EXCEPTION_END
	}

	/* @brief			Function to calculate the next ECB Swap Start Date
	*  @param [in]		swapStartDate		The ECB swap start date
	*  @return			Next ECB Swap Start Date
	*/
	AQLDate tryAqDatesNextECBSwapStartDate(const AQLDate& swapStartDate)
	{
		VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
		AQ_RECORD_INPUTS(swapStartDate);

		// Calculation
		const AQLDate result = etrading::AQLDateHelpers::getNextECBSwapStartDate(swapStartDate);

		// Record Outputs AND Return the Result for logs, tests and playback
		AQ_RECORD_OUTPUTS_AND_RETURN_RESULT(result);

		VALID_EXCEPTION_END
	}

	/* @brief			Function to calculate the next ECB Swap End Date
	*  @param [in]		swapEndDate		The ECB meeting date
	*  @return			Next ECB Meeting Date
	*/
	AQLDate tryAqDatesNextECBSwapEndDate(const AQLDate& swapEndDate)
	{
		VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
		AQ_RECORD_INPUTS(swapEndDate);

		// Calculation
		const AQLDate result = etrading::AQLDateHelpers::getNextECBSwapEndDate(swapEndDate);

		// Record Outputs AND Return the Result for logs, tests and playback
		AQ_RECORD_OUTPUTS_AND_RETURN_RESULT(result);

		VALID_EXCEPTION_END
	}

}
