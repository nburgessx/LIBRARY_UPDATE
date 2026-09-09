#pragma once

#include <string>
#include <vector>

/* @brief			swig interface for aqCurveForwardRatesFromYearFraction
*  @param [in]		fromDates	    A vector of from dates
*  @param [in]		yearFraction	YearFraction
*  @param [in]		curveCollection	ID of the yield curve
*  @param [in]		curveIndex		Index of the curve set. Default to STD
*  @param [in]		dayCount		Day count convention. Default to ACT/365
*  @return			The forward rates based on fromDates and toDates
*/
std::vector<double> aqCurveForwardRatesFromYearFraction( const std::vector<std::string>& fromDates, 
											             double yearFraction,
											             const std::string& curveCollection, 
											             const std::string& curveIndex, 
											             const std::string& dayCount );


/* @brief			swig interface for aqCurveForwardRates
*  @param [in]		fixingDates	    A vector of fixing dates
*  @param [in]		curveCollection	ID of the yield curve
*  @param [in]		curveIndex		Index of the curve set. Default to STD
*  @return			The forward rates based on fromDates and toDates
*/
std::vector<double> aqCurveForwardRates( const std::vector<std::string>& fixingDates, 
										 const std::string& curveCollection,
										 const std::string& curveIndex );


/* @brief			swig interface for aqCurveForwardRatesFromForwardDates
*  @param [in]		startDates	        A vector of start dates
*  @param [in]		endDates	        A vector of end dates
*  @param [in]		curveCollection	    ID of the yield curve
*  @param [in]		curveIndex		    Index of the curve set. Default to STD
*  @return			The forward rates based on fromDates and toDates
*/
std::vector<double> aqCurveForwardRatesFromForwardDates( const std::vector<std::string>& fromDates,
                                                         const std::vector<std::string>& toDates,
											             const std::string& curveCollection,
											             const std::string& curveIndex );

std::vector<double> aqCurveForwardRatesFromForwardDates(const std::vector<std::string>& fromDates,
														const std::vector<std::string>& toDates,
														const std::string& curveCollection,
														const std::string& curveIndex,
														const std::string& fwdInter );