/*
* @brief			validation interface for the future/fra product methods
* @Created:		08 Dec 2017
* @Author:			Yongyan Zheng
* @Department:		ISD Front Office Development
*
* The copyright to the computer program(s) herein is the property of Mizuho International.
*/

#include "tryMeCurveEuroDollarConvexityAdjustment.h"
#include "StructuredExceptionHandler.h"
#include "CreateDataFile.h"
#include "CurveUtilities.h"
#include "RecordMacros.h"

using etrading::CreateDataFile;

namespace validation_api
{

	/* @brief			Calculate the FRA rate from Future price, validation API for meProductFuturePriceToFraRate
	* @param [in]		futurePrice			The given future price
	* @param [in]		curveAsOfDate       The yield curve as of or valuation date
	* @param [in]		futuresStartDate    The futures start date
	* @param [in]		futuresEndDate      The futures end date
	* @param [in]		meanReversion       The Hull-White 1F Mean Reversion Parameter
	* @param [in]		volatility          The Hull-White 1F Volatility Parameter
	* @param [out]		Returns Fra rate
	*/
	double tryMeProductFuturePriceToFraRate(const double& futurePrice, const LADate& curveAsOfDate, const LADate& futuresStartDate, const LADate& futuresEndDate, const double& meanReversion, const double& volatility)
	{
		VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
		RECORD_INPUTS(futurePrice, curveAsOfDate, futuresStartDate, futuresEndDate, meanReversion, volatility);

		double convexityAdjustment = etrading::getCurveEuroDollarConvexityAdjustment(curveAsOfDate, futuresStartDate, futuresEndDate, meanReversion, volatility);

		double fraRate = etrading::fromFuturePriceToFraRate(futurePrice, convexityAdjustment);

		// Record Outputs AND Return the Result for logs, tests and playback
		RECORD_OUTPUTS_AND_RETURN_RESULT(fraRate);

		VALID_EXCEPTION_END

	}

	/* @brief			Calculate the FRA rate from Future price, validation API for meProductFuturePriceToFraRateFromConvAdj
	* @param [in]		futurePrice			The given future price
	* @param [in]		convexityAdjustment The given convexity adjustment between Future rate and Fra rate
	* @param [out]		Returns Fra rate
	*/
	double tryMeProductFuturePriceToFraRateFromConvAdj(const double& futurePrice, const double& convexityAdjustment)
	{

		VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
		RECORD_INPUTS(futurePrice, convexityAdjustment);
		 
		double fraRate = etrading::fromFuturePriceToFraRate(futurePrice, convexityAdjustment);

		// Record Outputs AND Return the Result for logs, tests and playback
		RECORD_OUTPUTS_AND_RETURN_RESULT(fraRate);

		VALID_EXCEPTION_END

	}


}