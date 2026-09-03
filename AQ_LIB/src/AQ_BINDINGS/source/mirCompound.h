/* 
 * @brief			Swig interface to Java for mirCompound3 methods
 * @Created:		04 April 2016 
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#pragma once

#include <string>
#include <vector>

/* @brief swig interface for mirCompound3 method
*  @param [in]		startDates		A vector of start dates
*  @param [in]		endDates		A vector of end dates
*  @param [in]		curveId			ID of the curve set
*  @param [in]		forecastCurve	Forecasting curve
*  @param [in]		frequency		Frequency
*  @param [in]		spread			Spread in basis points.
*  @param [in]		isStartRoll		Roll dates: True if roll dates are calculated from start date, false from end date
*  @param [in]		rollConvention	Roll conventions
*  @param [in]		calendar		Calendar
*  @param [in]		slidingRule		Rule that decides if end date should roll forward or backward etc, ie, MF
*  @param [in]		dayCount		Day count convention
*  @param [in]		interpolation	Interpolation method
*  @param [in]		compoundType	Compound Type: NORMAL, FLAT, SIMPLE, SIMPLE, AVERAGE
*  @param [in]		firstOddDate	End date of the front stub period
*  @param [in]		lastOddDate		Start date of the end stub period
*  @return			compounding rates calculated based on the startDates, endDates, and compound type
*/
std::vector<double> mirCompound3(const std::vector<std::string>& startDates, 
									const std::vector<std::string>& endDates, 
									const std::string& curveId, 
									const std::string& forecastCurve, 
									const std::string& frequency, 
									double spread, 
									bool isStartRoll,
									const std::string& rollConvention, 
									const std::string& calendar,
									const std::string& slidingRule, 
									const std::string& dayCount, 
									const std::string& interpolation, 
									const std::string& compoundType, 
									const std::string& firstOddDate, 
									const std::string& lastOddDate);


/* @brief swig interface for mirCompound3 method
*  @param [in]		startDate		Start date
*  @param [in]		endDate			End date
*  @param [in]		curveId			ID of the curve set
*  @param [in]		forecastCurve	Forecasting curve
*  @param [in]		frequency		Frequency
*  @param [in]		spread			Spread in basis points.
*  @param [in]		isStartRoll		Roll dates: True if roll dates are calculated from start date, false from end date
*  @param [in]		rollConvention	Roll conventions
*  @param [in]		calendar		Calendar
*  @param [in]		slidingRule		Rule that decides if end date should roll forward or backward etc, ie, MF
*  @param [in]		dayCount		Day count convention
*  @param [in]		interpolation	Interpolation method
*  @param [in]		compoundType	Compound Type: NORMAL, FLAT, SIMPLE, SIMPLE, AVERAGE
*  @param [in]		firstOddDate	End date of the front stub period
*  @param [in]		lastOddDate		Start date of the end stub period
*  @return			compounding rate calculated based on the startDate, endDate, and compound type
*/
double mirCompound3(const std::string& startDate, 
									const std::string& endDate, 
									const std::string& curveId, 
									const std::string& forecastCurve, 
									const std::string& frequency, 
									double spread, 
									bool isStartRoll,
									const std::string& rollConvention, 
									const std::string& calendar,
									const std::string& slidingRule, 
									const std::string& dayCount, 
									const std::string& interpolation, 
									const std::string& compoundType, 
									const std::string& firstOddDate, 
									const std::string& lastOddDate);

