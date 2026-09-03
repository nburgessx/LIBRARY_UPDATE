/* 
 * @brief			Swig interface to Java for meCurveZeroRate methods
 * @Created:		03 June 2016
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#include "TypeUtilities.h"

#include "meCurveZeroRate.h"
#include "tryMeCurveZeroRate.h"
#include "APISetUp.h"               // MLIB_API_START and MLIB_API_END Macros


/* @brief			swig interface for meCurveZeroRatesFromYearFractions
*  @param [in]		yearFractions	Year fraction
*  @param [in]		curveCollection	Curve collection id
*  @param [in]		curveIndex		Curve index name. Default to STD
*  @param [in]		frequency		Frequency. Default to SIMPLE for STD curve, ANNUAL for other curves
*  @param [in]		dayCount		Day count convention. Default to ACT/365
*  @return			Zero rate over a given term from the curve's asof date 
*/
double meCurveZeroRatesFromYearFractions(double yearFraction, 
						const std::string& curveCollection, 
						const std::string& curveIndex, 
						const std::string& frequency, 
						const std::string& dayCount)
{
    MLIB_API_START
	double ret(0.);
	
	// Input marshalling
	DoubleVector yearFractions;
	yearFractions.push_back(yearFraction);

	LAString tmp_curveCollection(curveCollection.c_str());
	LAString tmp_curveIndex(curveIndex.c_str());
	LAString tmp_dayCount(dayCount.c_str());
	LAString tmp_frequency(frequency.c_str());

	DoubleVector results = validation_api::tryMeCurveZeroRatesFromYearFractions(yearFractions,
																				tmp_curveCollection, 
																				tmp_curveIndex,
																				tmp_frequency, 
																				tmp_dayCount);
	ret = results[0];
	return ret;
    MLIB_API_END
}


/* @brief			swig interface for meCurveZeroRatesFromYearFractions
*  @param [in]		yearFractions	A list of year fractions
*  @param [in]		curveCollection	Curve collection id
*  @param [in]		curveIndex		Curve index name. Default to STD
*  @param [in]		frequency		Frequency. Default to SIMPLE for STD curve, ANNUAL for other curves
*  @param [in]		dayCount		Day count convention. Default to ACT/365
*  @return			A list of zero rates over a list of given terms from the curve's asof date 
*/
std::vector<double> meCurveZeroRatesFromYearFractions(const std::vector<double>& yearFractions, 
						const std::string& curveCollection, 
						const std::string& curveIndex, 
						const std::string& frequency, 
						const std::string& dayCount)
{
    MLIB_API_START
	std::vector<double> ret;
	
	// Input marshalling
	LAString tmp_curveCollection(curveCollection.c_str());
	LAString tmp_curveIndex(curveIndex.c_str());
	LAString tmp_dayCount(dayCount.c_str());
	LAString tmp_frequency(frequency.c_str());

	ret = validation_api::tryMeCurveZeroRatesFromYearFractions(yearFractions,
																tmp_curveCollection, 
																tmp_curveIndex,
																tmp_frequency, 
																tmp_dayCount);

	return ret;
    MLIB_API_END
}

/* @brief			swig interface for meCurveZeroRatesFromTenors
*  @param [in]		tenor			A tenor string
*  @param [in]		curveCollection	Curve collection id
*  @param [in]		curveIndex		Curve index name. Default to STD
*  @param [in]		frequency		Frequency. Default to SIMPLE for STD curve, ANNUAL for other curves
*  @param [in]		dayCount		Day count convention. Default to ACT/365
*  @param [in]		calendar		Calendar 
*  @param [in]		businessDayAdj	Rule that decides if end date should roll forward or backward etc. Default to NO_CHANGE
*  @return			Zero rate over a given term from the curve's asof date 
*/
double meCurveZeroRatesFromTenors(const std::string& tenor, 
						const std::string& curveCollection, 
						const std::string& curveIndex, 
						const std::string& frequency, 
						const std::string& dayCount, 
						const std::string& calendar, 
						const std::string& businessDayAdj)
{
    MLIB_API_START
	double ret(0.0);
	
	LAStringVector tenors;
	tenors.push_back(tenor.c_str());

	// Input marshalling
	LAString tmp_curveCollection(curveCollection.c_str());
	LAString tmp_curveIndex(curveIndex.c_str());
	LAString tmp_dayCount(dayCount.c_str());
	LAString tmp_frequency(frequency.c_str());
	LAString tmp_calendar(calendar.c_str());
	LAString tmp_businessDayAdj(businessDayAdj.c_str());

	std::vector<double> results = validation_api::tryMeCurveZeroRatesFromTenors(tenors,
																				tmp_curveCollection, 
																				tmp_curveIndex,
																				tmp_frequency, 
																				tmp_dayCount, 
																				tmp_calendar,
																				tmp_businessDayAdj);
	ret = results[0];

	return ret;
    MLIB_API_END
}

/* @brief			swig interface for meCurveZeroRatesFromTenors
*  @param [in]		tenors			A list of tenors
*  @param [in]		curveCollection	Curve collection id
*  @param [in]		curveIndex		Curve index name. Default to STD
*  @param [in]		frequency		Frequency. Default to SIMPLE for STD curve, ANNUAL for other curves
*  @param [in]		dayCount		Day count convention. Default to ACT/365
*  @param [in]		calendar		Calendar 
*  @param [in]		businessDayAdj	Rule that decides if end date should roll forward or backward etc. Default to NO_CHANGE
*  @return			A list of zero rates over a list of given terms from the curve's asof date 
*/
std::vector<double> meCurveZeroRatesFromTenors(const std::vector<std::string>& tenors, 
						const std::string& curveCollection, 
						const std::string& curveIndex, 
						const std::string& frequency, 
						const std::string& dayCount, 
						const std::string& calendar, 
						const std::string& businessDayAdj)
{
    MLIB_API_START
	std::vector<double> ret;
	
	LAStringVector tmp_tenors;
	swig::buildStringVector(tmp_tenors, tenors);

	// Input marshalling
	LAString tmp_curveCollection(curveCollection.c_str());
	LAString tmp_curveIndex(curveIndex.c_str());
	LAString tmp_dayCount(dayCount.c_str());
	LAString tmp_frequency(frequency.c_str());
	LAString tmp_calendar(calendar.c_str());
	LAString tmp_businessDayAdj(businessDayAdj.c_str());

	ret = validation_api::tryMeCurveZeroRatesFromTenors(tmp_tenors,
														tmp_curveCollection, 
														tmp_curveIndex,
														tmp_frequency, 
														tmp_dayCount, 
														tmp_calendar,
														tmp_businessDayAdj);

	return ret;
    MLIB_API_END
}

