#include "tryMeCurveVasicek.h"
#include "AffineModelUtilities.h"
#include "VasicekFitting.h"
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

namespace validation
{

    /* @brief			Check initial volatility when Vasicek model forward rates match curve's forward rates
    * @param [in]		fixingDates         fixing dates of the forward rates
    * @param [in]		targetForwardRates  target forward rates the Vasicek model will fit to 
    * @param [in]		curveCollection		The curve collection ID
    * @param [in]		curveIndex			Curve index name
    * @param [in]		initialTheta		The Vasicek 1F Theta Parameter
    * @param [in]		initialSigma		The Vasicek 1F Volatility Parameter
    * @param [in]		alpha				The Vasicek 1F Mean Reversion Spead Parameter
    * @param [in]		rt					short rate at time t
    * @param [in]		valuationDate		Valuation date of the forward rates, can be equal or greater than the curveAsOfDate, Default to curveAsOfDate if not provided
 	* @param [in]		showColumnHeaders	showColumnHeaders
    * @param [out]		Returns initial volatility, should be close to zero
    */
    AnyTypeMatrix tryMeCurveVasicekChecking(const DateVector& fixingDates, const DoubleVector& targetForwardRates, const std::string& curveCollection, const std::string& curveIndex, 
											const double& initialTheta, const double& initialSigma, const double& alpha, const double& rt, const AQLDate& valuationDate, const bool& showColumnHeaders)
    {
        VALID_EXCEPTION_START

        // Record Inputs for logs, tests and playback
        RECORD_INPUTS( valuationDate, fixingDates, targetForwardRates, curveCollection, curveIndex, initialTheta, initialSigma, alpha, rt, valuationDate, showColumnHeaders);

		const AQLDate curveAsOfDate = etrading::getCurveAsOfDate( curveCollection.c_str() );

		const bool futureValuation = etrading::isFutureValuation(curveAsOfDate, valuationDate);
		const double shortRate = etrading::getDefaultShortRate(curveAsOfDate, curveCollection, curveIndex, rt, futureValuation);

		// Check if vol is close to zero when Vasicek curve match forwardCurve
		auto fittingResult = etrading::calibrateVasicekToForwardCurve( alpha, initialTheta, initialSigma, curveCollection, curveIndex, fixingDates, targetForwardRates, shortRate, futureValuation );

        AnyTypeVector body;
		body.push_back(fittingResult.theta);
		body.push_back(fittingResult.sigma);
		body.push_back(fittingResult.lse);
		body.push_back(fittingResult.iteration);

		AnyTypeMatrix result;

		if( showColumnHeaders )
		{
            AnyTypeVector headers;
			headers.push_back((std::string)"Theta");
			headers.push_back((std::string)"Sigma");
			headers.push_back((std::string)"LSE");
			headers.push_back((std::string)"Iteration");

			result.push_back(headers);
		}

		//merge headers and body
	    result.push_back(body);

        // Record Outputs AND Return the Result for logs, tests and playback
        RECORD_OUTPUTS_AND_RETURN_RESULT( result );

		VALID_EXCEPTION_END
    }

    /* @brief			Check forward rates from on the Vasicek model
    * @param [in]		fixingDates         fixing dates of the forward rates
    * @param [in]		curveCollection		The curve collection ID
    * @param [in]		curveIndex			Curve index name
    * @param [in]		theta				The Vasicek 1F Theta Parameter
    * @param [in]		alpha				The Vasicek 1F Mean Reversion Spead Parameter
    * @param [in]		sigma				The Vasicek 1F Volatility Parameter
    * @param [in]		rt					short rate at time t
    * @param [in]		valuationDate		Valuation date of the forward rates, can be equal or greater than the curveAsOfDate, Default to curveAsOfDate if not provided
    * @param [out]		Returns forward rates from on the Vasicek model
    */
	DoubleVector tryMeCurveVasicekForwardRates(const DateVector& fixingDates, const std::string& curveCollection, const std::string& curveIndex,
											   const double& theta, const double& sigma, const double& alpha, const double& rt, const AQLDate& valuationDate)
    {
        VALID_EXCEPTION_START

        // Record Inputs for logs, tests and playback
        RECORD_INPUTS( fixingDates, curveCollection, curveIndex, theta, sigma, alpha, rt, valuationDate);

		etrading::VasicekModel vkModel(alpha, theta, sigma);

		const AQLDate curveAsOfDate = etrading::getCurveAsOfDate( curveCollection.c_str() );
		const bool futureValuation = etrading::isFutureValuation(curveAsOfDate, valuationDate);
		const double shortRate = etrading::getDefaultShortRate(curveAsOfDate, curveCollection, curveIndex, rt, futureValuation);

        // Calculation
		auto result = vkModel.forwardRatesAnalytical( fixingDates, curveCollection, curveIndex, shortRate, futureValuation);

        // Record Outputs AND Return the Result for logs, tests and playback
        RECORD_OUTPUTS_AND_RETURN_RESULT( result );

		VALID_EXCEPTION_END
    }

}