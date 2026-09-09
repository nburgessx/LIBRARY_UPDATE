#include "tryAqCurveEuroDollarConvexityAdjustment.h"
#include "StructuredExceptionHandler.h"
#include "CreateDataFile.h"
#include "CurveUtilities.h"
#include "RecordMacros.h"

using etrading::CreateDataFile;

namespace validation
{
    /* @brief			Calculate the EuroDollar Futures Convexity Adjustment
    * @param [in]		curveAsOfDate       The yield curve as of or valuation date
    * @param [in]		futuresStartDate    The futures start date
    * @param [in]		futuresEndDate      The futures end date
    * @param [in]		meanReversion       The Hull-White 1F Mean Reversion Parameter
    * @param [in]		volatility          The Hull-White 1F Volatility Parameter
    * @param [out]		Returns the EuroDollar Futures Convexity Adjustment
    */
    double tryAqCurveEuroDollarConvexityAdjustment( const AQLDate& curveAsOfDate, const AQLDate& futuresStartDate, const AQLDate& futuresEndDate, const double& meanReversion, const double& volatility )
    {
        VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
		AQ_RECORD_INPUTS(curveAsOfDate, futuresStartDate, futuresEndDate, meanReversion, volatility);
     
        double convexityAdjustment = etrading::getCurveEuroDollarConvexityAdjustment(curveAsOfDate, futuresStartDate, futuresEndDate, meanReversion, volatility);

		// Record Outputs AND Return the Result for logs, tests and playback
		AQ_RECORD_OUTPUTS_AND_RETURN_RESULT(convexityAdjustment);

        VALID_EXCEPTION_END
    }

}