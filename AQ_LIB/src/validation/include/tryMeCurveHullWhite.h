/*
 * @brief			validation interface for Hull White Curve Fitting method(s)
 * @Created:		17 Oct 2017
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#pragma once

#include "LACoreTemplateType.h"

namespace validation_api
{

	/* @brief			Check forward rates from on the HW model
    * @param [in]		fixingDates         fixing dates of the forward rates
    * @param [in]		curveCollection		The curve collection ID
    * @param [in]		curveIndex			Curve index name
    * @param [in]		sigma				The HW 1F Volatility Parameter
    * @param [in]		alpha				The HW 1F Mean Reversion Spead Parameter
    * @param [in]		rt					short rate at time t
    * @param [in]		valuationDate		Valuation date of the forward rates, can be equal or greater than the curveAsOfDate, Default to curveAsOfDate if not provided
    * @param [out]		Returns forward rates from on the Vasicek model
    */
	DoubleVector tryMeCurveHullWhiteForwardRates(const DateVector& fixingDates, const std::string& curveCollection, const std::string& curveIndex,
											   const double& sigma, const double& alpha = 0.03, const double& rt = std::numeric_limits<double>::quiet_NaN() , const LADate& valuationDate = LADate());
}