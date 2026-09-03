#include "tryMeDateCentralBank.h"
#include "LADateHelpers.h"
#include "CreateDataFile.h"
#include "StructuredExceptionHandler.h"
#include "RecordMacros.h"

namespace validation_api
{
    using etrading::CreateDataFile;
    using etrading::decorateFilename;

    /* @brief			validation interface for the meDateCentralBank method
    *  @param [in]		centralBankId	Central bank Id, e.g. ECB, BoE, Fed
    *  @param [in]		date			A base date
    *  @param [in]		strictlyAfter	True if strictly after the base date
    *  @return			Date of the next central bank meeting on/after the base date
    */
    LADate tryMeDateCentralBank( const LAString& centralBankId,
                                 const LADate& date,
                                 bool strictlyAfter )
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( "tryMeDateCentralBank_inputs" );
            file.write( "generatorFunction", "tryMeDateCentralBank" );
            file.write( "centralBankId", centralBankId );
            file.write( "date", date );
            file.write( "strictlyAfter", strictlyAfter );
        }

        LADate ret = etrading::LADateHelpers::getNextCBDate( centralBankId, date, strictlyAfter );

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( "tryMeDateCentralBank_outputs" );
            file.write( "output", ret );
        }

        return ret;

        VALID_EXCEPTION_END
    }

    /* @brief			validation interface for the meDateECB method
    *  @param [in]		date			A base date
    *  @param [in]		strictlyAfter	True if strictly after the input date
    *  @return			Date of the ECB (European Central Bank) meeting on/after the base date
    */
    LADate tryMeDateECB( const LADate& date,
                         bool strictlyAfter )
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( "tryMeDateECB_inputs" );
            file.write( "generatorFunction", "tryMeDateECB" );
            file.write( "date", date );
            file.write( "strictlyAfter", strictlyAfter );
        }

        LADate ret = etrading::LADateHelpers::getNextECBDate( date, strictlyAfter );

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( "tryMeDateECB_outputs" );
            file.write( "output", ret );
        }

        return ret;

        VALID_EXCEPTION_END
    }

    /* @brief			validation interface for the meDateECBSwapStart method
    *  @param [in]		date	An input date
    *  @return			The start date of an ECB(European Central Bank) Swap based on the given date
    */
    LADate tryMeDateECBSwapStart( const LADate& date )
    {
        VALID_EXCEPTION_START_WITH_NO_THREAD_GUARD

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( "tryMeDateECBSwapStart_inputs" );
            file.write( "generatorFunction", "tryMeDateECBSwapStart" );
            file.write( "date", date );
        }

        //Get the ECB date
        LADate ecbDate = tryMeDateECB( date, false );

        //Get the swap start date based on the ECB date
        LADate ret = etrading::LADateHelpers::getECBStartDate( ecbDate );

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( "tryMeDateECBSwapStart_outputs" );
            file.write( "output", ret );
        }

        return ret;

        VALID_EXCEPTION_END
    }


    /* @brief			validation interface for the meDateECBSwapEnd method
    *  @param [in]		date	An input date
    *  @return			The end date of an ECB(European Central Bank) Swap based on the given date
    */
    LADate tryMeDateECBSwapEnd( const LADate& date )
    {
        VALID_EXCEPTION_START_WITH_NO_THREAD_GUARD

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( "tryMeDateECBSwapEnd_inputs" );
            file.write( "generatorFunction", "tryMeDateECBSwapEnd" );
            file.write( "date", date );
        }

        //Get the first ECB date based on the input date, always Thursday
        LADate firstEcbDate = tryMeDateECB( date, false );

        //Get the second ECB date based on the first ECB, always Thursday
        LADate secondEcbDate = tryMeDateECB( firstEcbDate, true );

        //Get the swap start date of the second ECB date, always Wednesday
        LADate ret = tryMeDateECBSwapStart( secondEcbDate );

        //Always Tuesday
        ret.addDays( -1 );

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( "tryMeDateECBSwapEnd_outputs" );
            file.write( "output", ret );
        }

        return ret;

        VALID_EXCEPTION_END
    }

	/* @brief			Function to calculate the nth ECB Meeting Date
	*  @param [in]		asOfDate	The asOfDate
	*  @param [in]		n			The nth date to return
	*  @return			Date of the nth ECB (European Central Bank) meeting on/after the asOfDate
	*/
	LADate tryMeDateNthECBMeetingDate(const LADate& asOfDate, const int n)
	{
		VALID_EXCEPTION_START
		
		// Record Inputs for logs, tests and playback
		RECORD_INPUTS(asOfDate, n);

		// Calculation
		const LADate result = etrading::LADateHelpers::getNthECBMeetingDate(asOfDate, n);

		// Record Outputs AND Return the Result for logs, tests and playback
		RECORD_OUTPUTS_AND_RETURN_RESULT(result);

		VALID_EXCEPTION_END
	}

	/* @brief			Function to calculate the nth ECB Swap Start Date
	*  @param [in]		asOfDate	The asOfDate
	*  @param [in]		n			The nth date to return
	*  @return			Date of the nth ECB Swap Start Date on/after the asOfDate
	*/
	LADate tryMeDateNthECBSwapStartDate(const LADate& asOfDate, const int n)
	{
		VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
		RECORD_INPUTS(asOfDate, n);

		// Calculation
		const LADate result = etrading::LADateHelpers::getNthECBSwapStartDate(asOfDate, n);

		// Record Outputs AND Return the Result for logs, tests and playback
		RECORD_OUTPUTS_AND_RETURN_RESULT(result);

		VALID_EXCEPTION_END
	}

	/* @brief			Function to calculate the nth ECB Swap End Date
	*  @param [in]		asOfDate	The asOfDate
	*  @param [in]		n			The nth date to return
	*  @return			Date of the nth ECB Swap End Date on/after the asOfDate
	*/
	LADate tryMeDateNthECBSwapEndDate(const LADate& asOfDate, const int n)
	{
		VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
		RECORD_INPUTS(asOfDate, n);

		// Calculation
		const LADate result = etrading::LADateHelpers::getNthECBSwapEndDate(asOfDate, n);

		// Record Outputs AND Return the Result for logs, tests and playback
		RECORD_OUTPUTS_AND_RETURN_RESULT(result);

		VALID_EXCEPTION_END
	}

	/* @brief			Function to calculate the next ECB Meeting Date
	*  @param [in]		meetingDate		The ECB meeting date
	*  @return			Next ECB Meeting Date
	*/
	LADate tryMeDateNextECBMeetingDate(const LADate& meetingDate)
	{
		VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
		RECORD_INPUTS(meetingDate);

		// Calculation
		const LADate result = etrading::LADateHelpers::getNextECBMeetingDate(meetingDate);

		// Record Outputs AND Return the Result for logs, tests and playback
		RECORD_OUTPUTS_AND_RETURN_RESULT(result);

		VALID_EXCEPTION_END
	}

	/* @brief			Function to calculate the next ECB Swap Start Date
	*  @param [in]		swapStartDate		The ECB swap start date
	*  @return			Next ECB Swap Start Date
	*/
	LADate tryMeDateNextECBSwapStartDate(const LADate& swapStartDate)
	{
		VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
		RECORD_INPUTS(swapStartDate);

		// Calculation
		const LADate result = etrading::LADateHelpers::getNextECBSwapStartDate(swapStartDate);

		// Record Outputs AND Return the Result for logs, tests and playback
		RECORD_OUTPUTS_AND_RETURN_RESULT(result);

		VALID_EXCEPTION_END
	}

	/* @brief			Function to calculate the next ECB Swap End Date
	*  @param [in]		swapEndDate		The ECB meeting date
	*  @return			Next ECB Meeting Date
	*/
	LADate tryMeDateNextECBSwapEndDate(const LADate& swapEndDate)
	{
		VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
		RECORD_INPUTS(swapEndDate);

		// Calculation
		const LADate result = etrading::LADateHelpers::getNextECBSwapEndDate(swapEndDate);

		// Record Outputs AND Return the Result for logs, tests and playback
		RECORD_OUTPUTS_AND_RETURN_RESULT(result);

		VALID_EXCEPTION_END
	}

}
