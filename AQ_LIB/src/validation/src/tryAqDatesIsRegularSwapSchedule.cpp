#include "tryAqDatesIsRegularSwapSchedule.h"
#include "ScheduleValidation.h"
#include "ExceptionMacros.h"
#include "DataUtilities.h"
#include "CreateDataFile.h"
#include "StructuredExceptionHandler.h"
#include "RecordMacros.h"

namespace validation
{
    using etrading::CreateDataFile;

    /* @brief			Validate if swap has regular date schedule without stub coupons.
	* @param [in]		swapStart			    Swap start date
	* @param [in]		swapMaturity            Swap end date. Tenors are typically adjusted and end dates are not
	* @param [in]		isMaturityAdjusted      Swap end date business day adjusted. Maturities derived from Tenors are adjusted, whereas explicit maturity dates are unadjusted
	* @param [in]		frequency			    Swap floating frequency
	* @param [in]		busDayAdj			    Swap business date adjustment convention
	* @param [in]		calendar			    Swap calendar
	* @param [in]		rollDay				    Roll day
	* @param [in]		rollConvention		    Swap roll convention e.g. IMM, EOM
	* @output			Returns TRUE if the swap schedule is regular (with no stub) and FALSE otherwise
	*/
    bool tryAqDatesIsRegularSwapSchedule( const AQLDate& swapStart,
		                                 const AQLDate& swapMaturity,
		                                 bool isMaturityAdjusted,
		                                 const AQLString& frequency,
		                                 const AQLString& busDayAdj,
		                                 const AQLString& calendar,
		                                 int rollDay,
		                                 const AQLString& rollConvention )
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        AQ_RECORD_INPUTS( swapStart, swapMaturity, isMaturityAdjusted, frequency, busDayAdj, calendar, rollDay, rollConvention );

        // Validation
        AQ_REQUIRE( swapMaturity >= swapStart, "Invalid Swap Dates sepcified - Swap Maturity cannot be before the Swap Start date." );
        AQ_REQUIRE( rollDay >= 0, "Invalid RollDay sepcified - RollDay cannot be negative." );
        
        // Call the underlying function
        const bool result = etrading::isRegularSwapSchedule( swapStart, swapMaturity, isMaturityAdjusted, frequency, busDayAdj, calendar, rollDay, rollConvention );

        // Record Outputs AND Return the Result for logs, tests and playback
        AQ_RECORD_OUTPUTS_AND_RETURN_RESULT( result );

        VALID_EXCEPTION_END
    }

}



