/*
 * @brief			validation interface for Hull White Curve Fitting method(s)
 * @Created:		17 Oct 2017
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#pragma once

#include "LACoreTemplateType.h"

namespace validation_api
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
											const double& initialTheta, const double& initialSigma, const double& alpha = 0.03, const double& rt = std::numeric_limits<double>::quiet_NaN() , const LADate& valuationDate = LADate(), const bool& showColumnHeaders = true);

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
											   const double& theta, const double& sigma, const double& alpha = 0.03, const double& rt = std::numeric_limits<double>::quiet_NaN() , const LADate& valuationDate = LADate());


}