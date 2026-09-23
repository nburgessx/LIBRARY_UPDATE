// aqMathRates.h

/*
 * @brief			Swig interface for the aqMathForwardRate(s) and aqMathDiscountFactor(s) low-level curve-fit primitives.
 *                  Each takes a raw (dates, values) curve fit directly - not a cached curve object.
 */

#pragma once

#include <string>
#include <vector>

/* @brief			swig interface for aqMathForwardRate - forward rate at a fixing date, from a raw (dates, values) curve fit
*  @param [in]		fixingDate						The fixing date (YYYYMMDD)
*  @param [in]		asOfDate						The curve as-of date (YYYYMMDD)
*  @param [in]		interpolation					Interpolation method, e.g. LINEAR, MONOTONE_CONVEX
*  @param [in]		stateVariable					The interpolated state variable, e.g. FORWARD_RATE, DISCOUNT_FACTOR
*  @param [in]		xValues							Column of curve pillar dates (YYYYMMDD)
*  @param [in]		yValues							Column of curve pillar values, aligned with xValues
*  @param [in]		accrualDaycount					Day count convention
*  @param [in]		curveFrequencyTenor				The curve's own tenor, e.g. 3M
*  @param [in]		fixingBusinessDayAdjustment		Business day adjustment for the fixing
*  @param [in]		fixingCalendar					Holiday centre(s) for the fixing
*  @param [in]		joinDate						Optional. Piecewise-scheme join date (YYYYMMDD). Blank for none
*  @param [in]		polynomialOrder					Optional. Default 0
*  @param [in]		forwardAdjustmentTable			Optional. A forward-adjustment override table
*  @param [in]		compoundFreq					Optional. Default SIMPLE
*  @return			The forward rate
*/
double aqMathForwardRate( const std::string& fixingDate,
                           const std::string& asOfDate,
                           const std::string& interpolation,
                           const std::string& stateVariable,
                           const std::vector<std::string>& xValues,
                           const std::vector<double>& yValues,
                           const std::string& accrualDaycount,
                           const std::string& curveFrequencyTenor,
                           const std::string& fixingBusinessDayAdjustment,
                           const std::string& fixingCalendar,
                           const std::string& joinDate = "",
                           const unsigned int polynomialOrder = 0,
                           const std::vector<std::vector<std::string> >& forwardAdjustmentTable = std::vector<std::vector<std::string> >(),
                           const std::string& compoundFreq = "SIMPLE" );

/* @brief			swig interface for aqMathForwardRates - forward rates at a column of fixing dates, from a raw (dates, values) curve fit
*  @return			The forward rates
*/
std::vector<double> aqMathForwardRates( const std::vector<std::string>& fixingDates,
                                         const std::string& asOfDate,
                                         const std::string& interpolation,
                                         const std::string& stateVariable,
                                         const std::vector<std::string>& xValues,
                                         const std::vector<double>& yValues,
                                         const std::string& accrualDaycount,
                                         const std::string& curveFrequencyTenor,
                                         const std::string& fixingBusinessDayAdjustment,
                                         const std::string& fixingCalendar,
                                         const std::string& joinDate = "",
                                         const unsigned int polynomialOrder = 0,
                                         const std::vector<std::vector<std::string> >& forwardAdjustmentTable = std::vector<std::vector<std::string> >(),
                                         const std::string& compoundFreq = "SIMPLE" );

/* @brief			swig interface for aqMathDiscountFactor - discount factor at a payment date, from a raw (dates, values) curve fit
*  @return			The discount factor
*/
double aqMathDiscountFactor( const std::string& paymentDate,
                              const std::string& asOfDate,
                              const std::string& interpolation,
                              const std::string& stateVariable,
                              const std::vector<std::string>& xValues,
                              const std::vector<double>& yValues,
                              const std::string& accrualDaycount,
                              const std::string& curveFrequencyTenor,
                              const std::string& fixingBusinessDayAdjustment,
                              const std::string& fixingCalendar,
                              const std::string& joinDate = "",
                              const unsigned int polynomialOrder = 0,
                              const std::vector<std::vector<std::string> >& forwardAdjustmentTable = std::vector<std::vector<std::string> >(),
                              const std::string& compoundFreq = "SIMPLE" );

/* @brief			swig interface for aqMathDiscountFactors - discount factors at a column of payment dates, from a raw (dates, values) curve fit
*  @return			The discount factors
*/
std::vector<double> aqMathDiscountFactors( const std::vector<std::string>& paymentDates,
                                            const std::string& asOfDate,
                                            const std::string& interpolation,
                                            const std::string& stateVariable,
                                            const std::vector<std::string>& xValues,
                                            const std::vector<double>& yValues,
                                            const std::string& accrualDaycount,
                                            const std::string& curveFrequencyTenor,
                                            const std::string& fixingBusinessDayAdjustment,
                                            const std::string& fixingCalendar,
                                            const std::string& joinDate = "",
                                            const unsigned int polynomialOrder = 0,
                                            const std::vector<std::vector<std::string> >& forwardAdjustmentTable = std::vector<std::vector<std::string> >(),
                                            const std::string& compoundFreq = "SIMPLE" );
