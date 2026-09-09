#pragma once

#include <string>
#include <vector>


/* @brief			validation method for aqCurveDiscountFactors
*  @param [in]		paymenbtDates		A single or an array of to-date in YYYYMMDD format
*  @param [in]		curveCollection	    Curve set ID
*  @param [in]		curveIndex		    Index of the curve set. Default to OIS
*  @return			A array of discount factors
*/
std::vector<double> aqCurveDiscountFactors( const std::vector<std::string>& paymentDates,
                                            const std::string& curveCollection,
                                            const std::string& curveIndex );

/* @brief			swig interface for aqCurveDiscountFactorsFromYearFractions
*  @param [in]		yearFraction	Year fraction
*  @param [in]		dayCount		Day count
*  @param [in]		curveCollection	Curve set ID
*  @param [in]		curveIndex		Index of the curve set. Default to OIS
*  @return			A discount factor to the curve's asof date
*/
double aqCurveDiscountFactorsFromYearFractions(double yearFraction, 
											const std::string& dayCount,
											const std::string& curveCollection, 
											const std::string& curveIndex);

/* @brief			swig interface for aqCurveDiscountFactorsFromYearFractions
*  @param [in]		yearFractions	An array of yearFractions by year count
*  @param [in]		dayCount		Day count
*  @param [in]		curveCollection	Curve set ID
*  @param [in]		curveIndex		Index of the curve set. Default to OIS
*  @return			An array of discount factor
*/
std::vector<double> aqCurveDiscountFactorsFromYearFractions(const std::vector<double>& yearFractions, 
															const std::string& dayCount,
															const std::string& curveCollection, 
															const std::string& curveIndex);

/* @brief			swig interface for aqCurveDiscountFactorsFromTenors
*  @param [in]		tenor			Tenor
*  @param [in]		curveCollection	Curve set ID
*  @param [in]		curveIndex		Index of the curve set. Default to OIS
*  @param [in]		businessDayAdj	Rule that decides if end date should roll forward or backward etc, ie, MF. Default to 'NO_CHANGE'.
*  @param [in]		calendar		Calendar 
*  @return			A discount factor to the curve's asof date
*/
double aqCurveDiscountFactorsFromTenors(const std::string& tenor, 
					const std::string& curveCollection, 
					const std::string& curveIndex, 
					const std::string& businessDayAdj, 
					const std::string& calendar);


/* @brief			swig interface for aqCurveDiscountFactorsFromTenors
*  @param [in]		tenors			An array of tenor strings
*  @param [in]		curveCollection	Curve set ID
*  @param [in]		curveIndex		Index of the curve set. Default to OIS
*  @param [in]		businessDayAdj	Rule that decides if end date should roll forward or backward etc, ie, MF. Default to 'NO_CHANGE'.
*  @param [in]		calendar		Calendar 
*  @return			An array of discount factor
*/
std::vector<double> aqCurveDiscountFactorsFromTenors(const std::vector<std::string>& tenors, 
					const std::string& curveCollection, 
					const std::string& curveIndex, 
					const std::string& businessDayAdj, 
					const std::string& calendar);


/* @brief			swig interface for aqCurveDiscountFactorsForwardStarting
*  @param [in]		fromDate		A single from-date in YYYYMMDD formate
*  @param [in]		toDate			A single to-date in YYYYMMDD formate
*  @param [in]		curveCollection	Curve set ID
*  @param [in]		curveIndex		Index of the curve set. Default to OIS
*  @param [in]		businessDayAdj	Rule that decides if end date should roll forward or backward etc, ie, MF. Default to 'NO_CHANGE'.
*  @param [in]		calendar		Calendar 
*  @return			A discount factor
*/
double aqCurveDiscountFactorsForwardStarting(const std::string& fromDate, 
												const std::string& toDate, 
												const std::string& curveCollection, 
												const std::string& curveIndex);


/* @brief			swig interface for aqCurveDiscountFactorsForwardStarting
*  @param [in]		fromDates		An array of from-date in YYYYMMDD formate
*  @param [in]		toDates			An array of to-date in YYYYMMDD formate
*  @param [in]		curveCollection	Curve set ID
*  @param [in]		curveIndex		Index of the curve set. Default to OIS
*  @param [in]		businessDayAdj	Rule that decides if end date should roll forward or backward etc, ie, MF. Default to 'NO_CHANGE'.
*  @param [in]		calendar		Calendar 
*  @return			An array of discount factors
*/
std::vector<double> aqCurveDiscountFactorsForwardStarting(const std::vector<std::string>& fromDates, 
															const std::vector<std::string>& toDates, 
															const std::string& curveCollection, 
															const std::string& curveIndex);


/* @brief			swig interface for aqCurveDiscountFactorsForwardStartingFromYearFractions
*  @param [in]		fromDate			A single fromDate in YYYYMMDD formate
*  @param [in]		yearFraction		A single year fraction
*  @param [in]		dayCount	        The daycount fraction used to generate the yearFraction
*  @param [in]		curveCollection		Curve set ID
*  @param [in]		curveIndex			Index of the curve set. Default to OIS
*  @return			A discount factor
*/
double aqCurveDiscountFactorsForwardStartingFromYearFractions(const std::string& fromDate, 
																double yearFraction, 
																const std::string& dayCount,
																const std::string& curveCollection, 
																const std::string& curveIndex);


/* @brief			swig interface for aqCurveDiscountFactorsForwardStartingFromYearFractions
*  @param [in]		fromDates			An array of from-dates in YYYYMMDD formate
*  @param [in]		yearFractions		An array of year fraction yearFractions
*  @param [in]		dayCount	        The daycount fraction used to generate the yearFraction
*  @param [in]		curveCollection		Curve set ID
*  @param [in]		curveIndex			Index of the curve set. Default to OIS
*  @return			An array of discount factors
*/
std::vector<double> aqCurveDiscountFactorsForwardStartingFromYearFractions(const std::vector<std::string>& fromDates, 
																			const std::vector<double>& yearFractions, 
																			const std::string& dayCount,
																			const std::string& curveCollection, 
																			const std::string& curveIndex);


/* @brief			swig interface for aqCurveDiscountFactorsForwardStartingFromTenor
*  @param [in]		fromDate		A single from-dates in YYYYMMDD formate
*  @param [in]		tenor			A tenor string 
*  @param [in]		curveCollection	Curve set ID
*  @param [in]		curveIndex		Index of the curve set. Default to OIS
*  @param [in]		businessDayAdj	Rule that decides if end date should roll forward or backward etc, ie, MF. Default to 'NO_CHANGE'.
*  @param [in]		calendar		Calendar 
*  @return			A discount factor
*/
double aqCurveDiscountFactorsForwardStartingFromTenor(const std::string& fromDate, 
														const std::string& tenor, 
														const std::string& curveCollection, 
														const std::string& curveIndex, 
														const std::string& businessDayAdj, 
														const std::string& calendar);


/* @brief			swig interface for aqCurveDiscountFactorsForwardStartingFromTenor
*  @param [in]		fromDates		An array of from-dates in YYYYMMDD formate
*  @param [in]		tenor			A tenor string 
*  @param [in]		curveCollection	Curve set ID
*  @param [in]		curveIndex		Index of the curve set. Default to OIS
*  @param [in]		businessDayAdj	Rule that decides if end date should roll forward or backward etc, ie, MF. Default to 'NO_CHANGE'.
*  @param [in]		calendar		Calendar 
*  @return			An array of discount factors
*/
std::vector<double> aqCurveDiscountFactorsForwardStartingFromTenor(const std::vector<std::string>& fromDates, 
														const std::string& tenor, 
														const std::string& curveCollection, 
														const std::string& curveIndex, 
														const std::string& businessDayAdj, 
														const std::string& calendar);


