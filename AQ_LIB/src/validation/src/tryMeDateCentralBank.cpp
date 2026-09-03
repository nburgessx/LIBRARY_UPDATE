#include "tryMeDateCentralBank.h"
#include "LADateHelpers.h"
#include "CreateDataFile.h"
#include "StructuredExceptionHandler.h"
#include "RecordMacros.h"

namespace validation
{
    using etrading::CreateDataFile;
    using etrading::decorateFilename;

    /* @brief			validation interface for the meDateCentralBank method
    *  @param [in]		centralBankId	Central bank Id, e.g. ECB, BoE, Fed
    *  @param [in]		date			A base date
    *  @param [in]		strictlyAfter	True if strictly after the base date
    *  @return			Date of the next central bank meeting on/after the base date
    */
    AQLDate tryMeDateCentralBank( const AQLString& centralBankId,
                                 const AQLDate& date,
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

        AQLDate ret = etrading::LADateHelpers::getNextCBDate( centralBankId, date, strictlyAfter );

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
    AQLDate tryMeDateECB( const AQLDate& date,
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

        AQLDate ret = etrading::LADateHelpers::getNextECBDate( date, strictlyAfter );

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
    AQLDate tryMeDateECBSwapStart( const AQLDate& date )
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
        AQLDate ecbDate = tryMeDateECB( date, false );

        //Get the swap start date based on the ECB date
        AQLDate ret = etrading::LADateHelpers::getECBStartDate( ecbDate );

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
    AQLDate tryMeDateECBSwapEnd( const AQLDate& date )
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
        AQLDate firstEcbDate = tryMeDateECB( date, false );

        //Get the second ECB date based on the first ECB, always Thursday
        AQLDate secondEcbDate = tryMeDateECB( firstEcbDate, true );

        //Get the swap start date of the second ECB date, always Wednesday
        AQLDate ret = tryMeDateECBSwapStart( secondEcbDate );

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
	AQLDate tryMeDateNthECBMeetingDate(const AQLDate& asOfDate, const int n)
	{
		VALID_EXCEPTION_START
		
		// Record Inputs for logs, tests and playback
		RECORD_INPUTS(asOfDate, n);

		// Calculation
		const AQLDate result = etrading::LADateHelpers::getNthECBMeetingDate(asOfDate, n);

		// Record Outputs AND Return the Result for logs, tests and playback
		RECORD_OUTPUTS_AND_RETURN_RESULT(result);

		VALID_EXCEPTION_END
	}

	/* @brief			Function to calculate the nth ECB Swap Start Date
	*  @param [in]		asOfDate	The asOfDate
	*  @param [in]		n			The nth date to return
	*  @return			Date of the nth ECB Swap Start Date on/after the asOfDate
	*/
	AQLDate tryMeDateNthECBSwapStartDate(const AQLDate& asOfDate, const int n)
	{
		VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
		RECORD_INPUTS(asOfDate, n);

		// Calculation
		const AQLDate result = etrading::LADateHelpers::getNthECBSwapStartDate(asOfDate, n);

		// Record Outputs AND Return the Result for logs, tests and playback
		RECORD_OUTPUTS_AND_RETURN_RESULT(result);

		VALID_EXCEPTION_END
	}

	/* @brief			Function to calculate the nth ECB Swap End Date
	*  @param [in]		asOfDate	The asOfDate
	*  @param [in]		n			The nth date to return
	*  @return			Date of the nth ECB Swap End Date on/after the asOfDate
	*/
	AQLDate tryMeDateNthECBSwapEndDate(const AQLDate& asOfDate, const int n)
	{
		VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
		RECORD_INPUTS(asOfDate, n);

		// Calculation
		const AQLDate result = etrading::LADateHelpers::getNthECBSwapEndDate(asOfDate, n);

		// Record Outputs AND Return the Result for logs, tests and playback
		RECORD_OUTPUTS_AND_RETURN_RESULT(result);

		VALID_EXCEPTION_END
	}

	/* @brief			Function to calculate the next ECB Meeting Date
	*  @param [in]		meetingDate		The ECB meeting date
	*  @return			Next ECB Meeting Date
	*/
	AQLDate tryMeDateNextECBMeetingDate(const AQLDate& meetingDate)
	{
		VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
		RECORD_INPUTS(meetingDate);

		// Calculation
		const AQLDate result = etrading::LADateHelpers::getNextECBMeetingDate(meetingDate);

		// Record Outputs AND Return the Result for logs, tests and playback
		RECORD_OUTPUTS_AND_RETURN_RESULT(result);

		VALID_EXCEPTION_END
	}

	/* @brief			Function to calculate the next ECB Swap Start Date
	*  @param [in]		swapStartDate		The ECB swap start date
	*  @return			Next ECB Swap Start Date
	*/
	AQLDate tryMeDateNextECBSwapStartDate(const AQLDate& swapStartDate)
	{
		VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
		RECORD_INPUTS(swapStartDate);

		// Calculation
		const AQLDate result = etrading::LADateHelpers::getNextECBSwapStartDate(swapStartDate);

		// Record Outputs AND Return the Result for logs, tests and playback
		RECORD_OUTPUTS_AND_RETURN_RESULT(result);

		VALID_EXCEPTION_END
	}

	/* @brief			Function to calculate the next ECB Swap End Date
	*  @param [in]		swapEndDate		The ECB meeting date
	*  @return			Next ECB Meeting Date
	*/
	AQLDate tryMeDateNextECBSwapEndDate(const AQLDate& swapEndDate)
	{
		VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
		RECORD_INPUTS(swapEndDate);

		// Calculation
		const AQLDate result = etrading::LADateHelpers::getNextECBSwapEndDate(swapEndDate);

		// Record Outputs AND Return the Result for logs, tests and playback
		RECORD_OUTPUTS_AND_RETURN_RESULT(result);

		VALID_EXCEPTION_END
	}

}
