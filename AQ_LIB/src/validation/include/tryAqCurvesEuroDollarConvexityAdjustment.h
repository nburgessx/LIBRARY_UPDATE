// tryAqCurvesEuroDollarConvexityAdjustment.h

/*
 * @brief			validation interface for the aqRatesConvexityAdjustment method
 * @Created:		18th October 2016
 * @Author:			Nicholas Burgess
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#pragma once
#include "AQLDate.h"

namespace validation
{
    /* @brief			Calculate the EuroDollar Futures Convexity Adjustment, validation API for aqCurvesEuroDollarConvexityAdjustment
    * @param [in]		curveAsOfDate       The yield curve as of or valuation date
    * @param [in]		futuresStartDate    The futures start date
    * @param [in]		futuresEndDate      The futures end date
    * @param [in]		meanReversion       The Hull-White 1F Mean Reversion Parameter
    * @param [in]		volatility          The Hull-White 1F Volatility Parameter
    * @param [out]		Returns the EuroDollar Futures Convexity Adjustment
    */
    double tryAqCurvesEuroDollarConvexityAdjustment( const AQLDate& curveAsOfDate, const AQLDate& futuresStartDate, const AQLDate& futuresEndDate, const double& meanReversion, const double& volatility );

}