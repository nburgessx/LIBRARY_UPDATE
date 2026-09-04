#pragma once

#include <string>
#include <vector>

/* @brief			swig interface for aqCurvesZeroRatesFromYearFractions
*  @param [in]		yearFractions	Year fraction
*  @param [in]		curveCollection	Curve collection id
*  @param [in]		curveIndex		Curve index name. Default to STD
*  @param [in]		frequency		Frequency. Default to SIMPLE for STD curve, ANNUAL for other curves
*  @param [in]		dayCount		Day count convention. Default to ACT/365
*  @return			Zero rate over a given term from the curve's asof date 
*/
double aqCurvesZeroRatesFromYearFractions(double yearFraction, 
						const std::string& curveCollection, 
						const std::string& curveIndex, 
						const std::string& frequency, 
						const std::string& dayCount);


/* @brief			swig interface for aqCurvesZeroRatesFromYearFractions
*  @param [in]		yearFractions	A list of year fractions
*  @param [in]		curveCollection	Curve collection id
*  @param [in]		curveIndex		Curve index name. Default to STD
*  @param [in]		frequency		Frequency. Default to SIMPLE for STD curve, ANNUAL for other curves
*  @param [in]		dayCount		Day count convention. Default to ACT/365
*  @return			A list of zero rates over a list of given terms from the curve's asof date 
*/
std::vector<double> aqCurvesZeroRatesFromYearFractions(const std::vector<double>& yearFractions, 
						const std::string& curveCollection, 
						const std::string& curveIndex, 
						const std::string& frequency, 
						const std::string& dayCount);


/* @brief			swig interface for aqCurvesZeroRatesFromTenors
*  @param [in]		tenor			A tenor string
*  @param [in]		curveCollection	Curve collection id
*  @param [in]		curveIndex		Curve index name. Default to STD
*  @param [in]		frequency		Frequency. Default to SIMPLE for STD curve, ANNUAL for other curves
*  @param [in]		dayCount		Day count convention. Default to ACT/365
*  @param [in]		calendar		Calendar 
*  @param [in]		businessDayAdj	Rule that decides if end date should roll forward or backward etc. Default to NO_CHANGE
*  @return			Zero rate over a given term from the curve's asof date 
*/
double aqCurvesZeroRatesFromTenors(const std::string& tenor, 
						const std::string& curveCollection, 
						const std::string& curveIndex, 
						const std::string& frequency, 
						const std::string& dayCount, 
						const std::string& calendar, 
						const std::string& businessDayAdj);


/* @brief			swig interface for aqCurvesZeroRatesFromTenors
*  @param [in]		tenors			A list of tenors
*  @param [in]		curveCollection	Curve collection id
*  @param [in]		curveIndex		Curve index name. Default to STD
*  @param [in]		frequency		Frequency. Default to SIMPLE for STD curve, ANNUAL for other curves
*  @param [in]		dayCount		Day count convention. Default to ACT/365
*  @param [in]		calendar		Calendar 
*  @param [in]		businessDayAdj	Rule that decides if end date should roll forward or backward etc. Default to NO_CHANGE
*  @return			A list of zero rates over a list of given terms from the curve's asof date 
*/
std::vector<double> aqCurvesZeroRatesFromTenors(const std::vector<std::string>& tenors, 
						const std::string& curveCollection, 
						const std::string& curveIndex, 
						const std::string& frequency, 
						const std::string& dayCount, 
						const std::string& calendar, 
						const std::string& businessDayAdj);
