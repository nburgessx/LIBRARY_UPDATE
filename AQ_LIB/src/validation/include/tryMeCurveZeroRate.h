/*
 * @brief			validation interface for meCurveZeroRate method(s)
 * @Created:		27 April 2016
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#pragma once

#include "LACoreTemplateType.h"

namespace validation_api
{


    /* @brief			validation interface for meCurveZeroRateFromYearFractions
    *  @param [in]		yearFractions	A  list of given year fractions
    *  @param [in]		curveCollection	Curve collection id
    *  @param [in]		curveIndex		Curve index name. Default to STD
    *  @param [in]		frequency		Frequency. Default to SIMPLE for STD curve, ANNUAL for other curves
    *  @param [in]		dayCount		Day count convention. Default to ACT/365
    *  @return			Zero rate over the yearFractions from the curve's asof date
    */
    DoubleVector tryMeCurveZeroRatesFromYearFractions( const DoubleVector& yearFractions,
														const LAString& curveCollection,
														const LAString& curveIndex,
														const LAString& frequency,
														const LAString& dayCount,
														const std::string& fwdInter="");


    /* @brief			validation interface for meCurveZeroRateFromTenors
    *  @param [in]		tenors			A  list of tenors
    *  @param [in]		curveCollection	Curve collection id
    *  @param [in]		curveIndex		Curve index name. Default to STD
    *  @param [in]		frequency		Frequency. Default to SIMPLE for STD curve, ANNUAL for other curves
    *  @param [in]		dayCount		Day count convention. Default to ACT/365
    *  @param [in]		calendar		Calendar
    *  @param [in]		businessDayAdj	Rule that decides if end date should roll forward or backward etc. Default to NO_CHANGE
    *  @return			Zero rate over the tenors from the curve's asof date
    */
    DoubleVector tryMeCurveZeroRatesFromTenors( const LAStringVector& tenors,
												const LAString& curveCollection,
												const LAString& curveIndex,
												const LAString& frequency,
												const LAString& dayCount,
												const LAString& calendar,
												const LAString& businessDayAdj,
												const std::string& fwdInter="");

}
