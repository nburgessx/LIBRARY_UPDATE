// tryAqFuturesTickersIMM.cpp

#include "tryAqFuturesTickersIMM.h"

#include "AQLDateScheduleHelpers.h"
#include "CreateDataFile.h"
#include "CurveValidation.h"
#include "LabelValueBlockValidation.h"
#include "ScheduleValidation.h"
#include "SwapValidation.h"
#include "ParameterValidation.h"
#include "StructuredExceptionHandler.h"
#include "RecordMacros.h"
#include "FuturesDates.h"

using etrading::CreateDataFile;

namespace validation
{
    /* @brief			validation interface for tryAqFuturesTickerCurrent
    *  @param [in]		valuationDate	The valuation date
    *  @param [in]		includeToday    Allow the IMM roll date to match the valuation date: TRUE = Don't roll IMM date if it falls on the valuation date
    *  @param [in]		showYearWithTwoDigits - Show contacts with 2 year digits i.e. Z19 instead of the traditional Z9 format
    *  @return			The current IMM futures ticker, which may start in the past yet end in the future
    */
    std::string tryAqFuturesTickerCurrent( const AQLDate & valuationDate, const bool includeToday, const bool showYearWithTwoDigits )
    {
        VALID_EXCEPTION_START

        // Record Inputs for logs, tests and playback
        AQ_RECORD_INPUTS( valuationDate, includeToday, showYearWithTwoDigits );

        // Calculation
        const std::string result = etrading::currentIMMFuturesTicker( valuationDate, includeToday, "", "NO_CHANGE", showYearWithTwoDigits );

        // Record Outputs AND Return the Result for logs, tests and playback
        AQ_RECORD_OUTPUTS_AND_RETURN_RESULT( result );

        VALID_EXCEPTION_END
    }
    
    /* @brief			validation interface for tryAqDatesIMMNext
    *  @param [in]		referenceDate	The IMM reference date
    *  @param [in]		showYearWithTwoDigits - Show contacts with 2 year digits i.e. Z19 instead of the traditional Z9 format
    *  @return			The next IMM futures ticker relative to the reference date
    */
    std::string tryAqFuturesTickerNext( const AQLDate & referenceDate, const bool showYearWithTwoDigits )
    {
        VALID_EXCEPTION_START

        // Record Inputs for logs, tests and playback
        AQ_RECORD_INPUTS( referenceDate, showYearWithTwoDigits );

        // Calculation
        const std::string result = etrading::nextIMMFuturesTicker( referenceDate, "", "NO_CHANGE", showYearWithTwoDigits );

        // Record Outputs AND Return the Result for logs, tests and playback
        AQ_RECORD_OUTPUTS_AND_RETURN_RESULT( result );

        VALID_EXCEPTION_END
    }
    
    /* @brief			validation interface for tryAqDatesIMMPrevious
    *  @param [in]		referenceDate	The IMM reference date
    *  @param [in]		showYearWithTwoDigits - Show contacts with 2 year digits i.e. Z19 instead of the traditional Z9 format
    *  @return			The previous IMM futures ticker relative to the reference date
    */
    std::string tryAqFuturesTickerPrevious( const AQLDate & referenceDate, const bool showYearWithTwoDigits )
    {
        VALID_EXCEPTION_START

        // Record Inputs for logs, tests and playback
        AQ_RECORD_INPUTS( referenceDate, showYearWithTwoDigits );

        // Calculation
        const std::string result = etrading::previousIMMFuturesTicker( referenceDate, "", "NO_CHANGE", showYearWithTwoDigits );

        // Record Outputs AND Return the Result for logs, tests and playback
        AQ_RECORD_OUTPUTS_AND_RETURN_RESULT( result );

        VALID_EXCEPTION_END
    }

    /* @brief			validation interface for tryAqDatesIMMNth
    *  @param [in]		valuationDate	The valuation date
    *  @param [in]		nthIMM	        The nth IMM futures ticker to generate
    *  @param [in]		includeToday    Allow the IMM roll date to match the valuation date: TRUE = Don't roll IMM date if it falls on the valuation date
    *  @param [in]		showYearWithTwoDigits - Show contacts with 2 year digits i.e. Z19 instead of the traditional Z9 format
    *  @return			The nth IMM futures ticker relative to the valuation date
    */
    std::string tryAqFuturesTickerNth( const AQLDate & valuationDate, const int nthIMM, const bool includeToday, const bool showYearWithTwoDigits )
    {
        VALID_EXCEPTION_START

        // Record Inputs for logs, tests and playback
        AQ_RECORD_INPUTS( valuationDate, nthIMM, includeToday, showYearWithTwoDigits );

        // Calculation
        const std::string result = etrading::nthIMMFuturesTicker( valuationDate, nthIMM, includeToday, "", "NO_CHANGE", showYearWithTwoDigits );

        // Record Outputs AND Return the Result for logs, tests and playback
        AQ_RECORD_OUTPUTS_AND_RETURN_RESULT( result );

        VALID_EXCEPTION_END
    }
}