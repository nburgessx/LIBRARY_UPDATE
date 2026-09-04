#pragma once

#include <string>
#include <vector>


/* @brief			validation method for aqCurvesDiscountFactors
*  @param [in]		paymenbtDates		A single or an array of to-date in YYYYMMDD format
*  @param [in]		curveCollection	    Curve set ID
*  @param [in]		curveIndex		    Index of the curve set. Default to OIS
*  @return			A array of discount factors
*/
std::vector<double> aqCurvesDiscountFactors( const std::vector<std::string>& paymentDates,
                                            const std::string& curveCollection,
                                            const std::string& curveIndex );

/* @brief			swig interface for aqCurvesDiscountFactorsFromYearFractions
*  @param [in]		yearFraction	Year fraction
*  @param [in]		dayCount		Day count
*  @param [in]		curveCollection	Curve set ID
*  @param [in]		curveIndex		Index of the curve set. Default to OIS
*  @return			A discount factor to the curve's asof date
*/
double aqCurvesDiscountFactorsFromYearFractions(double yearFraction, 
											const std::string& dayCount,
											const std::string& curveCollection, 
											const std::string& curveIndex);

/* @brief			swig interface for aqCurvesDiscountFactorsFromYearFractions
*  @param [in]		yearFractions	An array of yearFractions by year count
*  @param [in]		dayCount		Day count
*  @param [in]		curveCollection	Curve set ID
*  @param [in]		curveIndex		Index of the curve set. Default to OIS
*  @return			An array of discount factor
*/
std::vector<double> aqCurvesDiscountFactorsFromYearFractions(const std::vector<double>& yearFractions, 
															const std::string& dayCount,
															const std::string& curveCollection, 
															const std::string& curveIndex);

/* @brief			swig interface for aqCurvesDiscountFactorsFromTenors
*  @param [in]		tenor			Tenor
*  @param [in]		curveCollection	Curve set ID
*  @param [in]		curveIndex		Index of the curve set. Default to OIS
*  @param [in]		businessDayAdj	Rule that decides if end date should roll forward or backward etc, ie, MF. Default to 'NO_CHANGE'.
*  @param [in]		calendar		Calendar 
*  @return			A discount factor to the curve's asof date
*/
double aqCurvesDiscountFactorsFromTenors(const std::string& tenor, 
					const std::string& curveCollection, 
					const std::string& curveIndex, 
					const std::string& businessDayAdj, 
					const std::string& calendar);


/* @brief			swig interface for aqCurvesDiscountFactorsFromTenors
*  @param [in]		tenors			An array of tenor strings
*  @param [in]		curveCollection	Curve set ID
*  @param [in]		curveIndex		Index of the curve set. Default to OIS
*  @param [in]		businessDayAdj	Rule that decides if end date should roll forward or backward etc, ie, MF. Default to 'NO_CHANGE'.
*  @param [in]		calendar		Calendar 
*  @return			An array of discount factor
*/
std::vector<double> aqCurvesDiscountFactorsFromTenors(const std::vector<std::string>& tenors, 
					const std::string& curveCollection, 
					const std::string& curveIndex, 
					const std::string& businessDayAdj, 
					const std::string& calendar);


/* @brief			swig interface for aqCurvesDiscountFactorsForwardStarting
*  @param [in]		fromDate		A single from-date in YYYYMMDD formate
*  @param [in]		toDate			A single to-date in YYYYMMDD formate
*  @param [in]		curveCollection	Curve set ID
*  @param [in]		curveIndex		Index of the curve set. Default to OIS
*  @param [in]		businessDayAdj	Rule that decides if end date should roll forward or backward etc, ie, MF. Default to 'NO_CHANGE'.
*  @param [in]		calendar		Calendar 
*  @return			A discount factor
*/
double aqCurvesDiscountFactorsForwardStarting(const std::string& fromDate, 
												const std::string& toDate, 
												const std::string& curveCollection, 
												const std::string& curveIndex);


/* @brief			swig interface for aqCurvesDiscountFactorsForwardStarting
*  @param [in]		fromDates		An array of from-date in YYYYMMDD formate
*  @param [in]		toDates			An array of to-date in YYYYMMDD formate
*  @param [in]		curveCollection	Curve set ID
*  @param [in]		curveIndex		Index of the curve set. Default to OIS
*  @param [in]		businessDayAdj	Rule that decides if end date should roll forward or backward etc, ie, MF. Default to 'NO_CHANGE'.
*  @param [in]		calendar		Calendar 
*  @return			An array of discount factors
*/
std::vector<double> aqCurvesDiscountFactorsForwardStarting(const std::vector<std::string>& fromDates, 
															const std::vector<std::string>& toDates, 
															const std::string& curveCollection, 
															const std::string& curveIndex);


/* @brief			swig interface for aqCurvesDiscountFactorsForwardStartingFromYearFractions
*  @param [in]		fromDate			A single fromDate in YYYYMMDD formate
*  @param [in]		yearFraction		A single year fraction
*  @param [in]		dayCount	        The daycount fraction used to generate the yearFraction
*  @param [in]		curveCollection		Curve set ID
*  @param [in]		curveIndex			Index of the curve set. Default to OIS
*  @return			A discount factor
*/
double aqCurvesDiscountFactorsForwardStartingFromYearFractions(const std::string& fromDate, 
																double yearFraction, 
																const std::string& dayCount,
																const std::string& curveCollection, 
																const std::string& curveIndex);


/* @brief			swig interface for aqCurvesDiscountFactorsForwardStartingFromYearFractions
*  @param [in]		fromDates			An array of from-dates in YYYYMMDD formate
*  @param [in]		yearFractions		An array of year fraction yearFractions
*  @param [in]		dayCount	        The daycount fraction used to generate the yearFraction
*  @param [in]		curveCollection		Curve set ID
*  @param [in]		curveIndex			Index of the curve set. Default to OIS
*  @return			An array of discount factors
*/
std::vector<double> aqCurvesDiscountFactorsForwardStartingFromYearFractions(const std::vector<std::string>& fromDates, 
																			const std::vector<double>& yearFractions, 
																			const std::string& dayCount,
																			const std::string& curveCollection, 
																			const std::string& curveIndex);


/* @brief			swig interface for aqCurvesDiscountFactorsForwardStartingFromTenor
*  @param [in]		fromDate		A single from-dates in YYYYMMDD formate
*  @param [in]		tenor			A tenor string 
*  @param [in]		curveCollection	Curve set ID
*  @param [in]		curveIndex		Index of the curve set. Default to OIS
*  @param [in]		businessDayAdj	Rule that decides if end date should roll forward or backward etc, ie, MF. Default to 'NO_CHANGE'.
*  @param [in]		calendar		Calendar 
*  @return			A discount factor
*/
double aqCurvesDiscountFactorsForwardStartingFromTenor(const std::string& fromDate, 
														const std::string& tenor, 
														const std::string& curveCollection, 
														const std::string& curveIndex, 
														const std::string& businessDayAdj, 
														const std::string& calendar);


/* @brief			swig interface for aqCurvesDiscountFactorsForwardStartingFromTenor
*  @param [in]		fromDates		An array of from-dates in YYYYMMDD formate
*  @param [in]		tenor			A tenor string 
*  @param [in]		curveCollection	Curve set ID
*  @param [in]		curveIndex		Index of the curve set. Default to OIS
*  @param [in]		businessDayAdj	Rule that decides if end date should roll forward or backward etc, ie, MF. Default to 'NO_CHANGE'.
*  @param [in]		calendar		Calendar 
*  @return			An array of discount factors
*/
std::vector<double> aqCurvesDiscountFactorsForwardStartingFromTenor(const std::vector<std::string>& fromDates, 
														const std::string& tenor, 
														const std::string& curveCollection, 
														const std::string& curveIndex, 
														const std::string& businessDayAdj, 
														const std::string& calendar);


