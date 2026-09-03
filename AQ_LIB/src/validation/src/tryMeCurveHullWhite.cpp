#include "tryMeCurveHullWhite.h"
#include "AffineModelUtilities.h"
#include "HullWhiteModel.h"
#include "ParameterValidation.h"
#include "CurveUtilities.h"
#include "CurveValidation.h"

// Includes for Structured Exception Handling and Input/Output Logging
#include "CreateDataFile.h"
#include "StructuredExceptionHandler.h"
#include "CommonConstants.h"
#include "RecordMacros.h"

// Namespaces for Input/Output Logging
using etrading::CreateDataFile;
using etrading::decorateFilename;

namespace validation_api
{

	/* @brief			Check forward rates from on the HW model
    * @param [in]		fixingDates         fixing dates of the forward rates
    * @param [in]		curveCollection		The curve collection ID
    * @param [in]		curveIndex			Curve index name
    * @param [in]		alpha				The HW 1F Mean Reversion Spead Parameter
    * @param [in]		sigma				The HW 1F Volatility Parameter
    * @param [in]		rt					short rate at time t
    * @param [in]		valuationDate		Valuation date of the forward rates, can be equal or greater than the curveAsOfDate, Default to curveAsOfDate if not provided
    * @param [out]		Returns forward rates from on the Vasicek model
    */
	DoubleVector tryMeCurveHullWhiteForwardRates(const DateVector& fixingDates, const std::string& curveCollection, const std::string& curveIndex,
											   const double& sigma, const double& alpha, const double& rt , const LADate& valuationDate)
	{
        VALID_EXCEPTION_START

        // Record Inputs for logs, tests and playback
        RECORD_INPUTS( fixingDates, curveCollection, curveIndex, sigma, alpha, rt, valuationDate);

		etrading::HullWhiteModel hwModel(alpha, sigma);

		const LADate curveAsOfDate = etrading::getCurveAsOfDate( curveCollection.c_str() );
		const bool futureValuation = etrading::isFutureValuation(curveAsOfDate, valuationDate);
		const double shortRate = etrading::getDefaultShortRate(curveAsOfDate, curveCollection, curveIndex, rt, futureValuation);

		// Calculation
		auto result = hwModel.forwardRatesAnalytical( fixingDates, curveCollection, curveIndex, shortRate, futureValuation);

        // Record Outputs AND Return the Result for logs, tests and playback
        RECORD_OUTPUTS_AND_RETURN_RESULT( result );

		VALID_EXCEPTION_END
    }


  
}