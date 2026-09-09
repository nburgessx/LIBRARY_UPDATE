// tryAqCurveEuroDollarConvexityAdjustment.h

/*
 * @brief			validation interface for the aqRatesConvexityAdjustment method
 */

#pragma once
#include "AQLDate.h"

namespace validation
{
    /* @brief			Calculate the EuroDollar Futures Convexity Adjustment, validation API for aqCurveEuroDollarConvexityAdjustment
    * @param [in]		curveAsOfDate       The yield curve as of or valuation date
    * @param [in]		futuresStartDate    The futures start date
    * @param [in]		futuresEndDate      The futures end date
    * @param [in]		meanReversion       The Hull-White 1F Mean Reversion Parameter
    * @param [in]		volatility          The Hull-White 1F Volatility Parameter
    * @param [out]		Returns the EuroDollar Futures Convexity Adjustment
    */
    double tryAqCurveEuroDollarConvexityAdjustment( const AQLDate& curveAsOfDate, const AQLDate& futuresStartDate, const AQLDate& futuresEndDate, const double& meanReversion, const double& volatility );

}