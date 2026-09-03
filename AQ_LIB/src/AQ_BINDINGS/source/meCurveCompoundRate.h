/* 
 * @brief			Swig interface to Java for meCurveCompoundRate methods
 * @Created:		03 June 2016 
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#pragma once

#include <string>
#include <vector>

/* @brief swig interface for meCurveCompoundRate method
*  @param [in]		startDates			A vector of start dates
*  @param [in]		endDates			A vector of end dates
*  @param [in]		curveCollection		ID of the curve set
*  @param [in]		forecastCurveIndex	Forecasting curve index
*  @param [in]		frequency			Frequency
*  @param [in]		spread				Spread in basis points.
*  @param [in]		stubType			StubType: None, ShortStart (SS), LongStart (LS), ShortEnd (SE) or LongEnd (LE).
*  @param [in]		rollDayInput		Roll Day conventions, e.g. ENDDATE
*  @param [in]		calendar			Calendar
*  @param [in]		businessDayAdj		Rule that decides if end date should roll forward or backward etc, ie, MF
*  @param [in]		dayCount			Day count convention
*  @param [in]		interpolation		Interpolation method
*  @param [in]		compoundType		Compound Type: NORMAL, FLAT, SIMPLE, SIMPLE, AVERAGE
*  @param [in]		firstStubDate		End date of the front stub period
*  @param [in]		lastStubDate		Start date of the end stub period
*  @return			interest rates calculated based on the startDates, endDates, and compound type
*/
std::vector<double> meCurveCompoundRate(const std::vector<std::string>& startDates, 
									const std::vector<std::string>& endDates, 
									const std::string& curveCollection, 
									const std::string& forecastCurveIndex, 
									const std::string& frequency, 
									double spread, 
									const std::string& stubType, 
									const std::string& rollDayInput,
									const std::string& calendar,
									const std::string& businessDayAdj,
									const std::string& dayCount,
									const std::string& interpolation,
									const std::string& compoundType,
									const std::string& firstStubDate,
									const std::string& lastStubDate);


/* @brief swig interface for meCurveCompoundRate method
*  @param [in]		startDate			Start date
*  @param [in]		endDate				End date
*  @param [in]		curveCollection		ID of the curve set
*  @param [in]		forecastCurveIndex	Forecasting curve index
*  @param [in]		frequency			Frequency
*  @param [in]		spread				Spread in basis points.
*  @param [in]		stubType			StubType: None, ShortStart (SS), LongStart (LS), ShortEnd (SE) or LongEnd (LE).
*  @param [in]		rollDayInput		Roll Day conventions, e.g. ENDDATE
*  @param [in]		calendar			Calendar
*  @param [in]		businessDayAdj		Rule that decides if end date should roll forward or backward etc, ie, MF
*  @param [in]		dayCount			Day count convention
*  @param [in]		interpolation		Interpolation method
*  @param [in]		compoundType		Compound Type: NORMAL, FLAT, SIMPLE, SIMPLE, AVERAGE
*  @param [in]		firstStubDate		End date of the front stub period
*  @param [in]		lastStubDate		Start date of the end stub period
*  @return			compounding rate calculated based on the startDate, endDate, and compound type
*/
double meCurveCompoundRate(const std::string& startDate, 
							const std::string& endDate, 
							const std::string& curveCollection,
							const std::string& forecastCurveIndex,
							const std::string& frequency,
							double spread, 
							const std::string& stubType,
							const std::string& rollDayInput,
							const std::string& calendar,
							const std::string& businessDayAdj,
							const std::string& dayCount,
							const std::string& interpolation,
							const std::string& compoundType,
							const std::string& firstStubDate,
							const std::string& lastStubDate);

