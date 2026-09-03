/* 
 * @brief			Swig interface to Java for function meCurveDiscountFactors functions
 * @Created:		03 June 2016 
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#include "TypeUtilities.h"
#include "LADate.h"
#include "LAString.h"
#include "meCurveDiscountFactor.h"
#include "tryMeCurveDiscountFactor.h"
#include "APISetUp.h"               // MLIB_API_START and MLIB_API_END Macros
#include "ParameterValidation.h"


/* @brief			validation method for meCurveDiscountFactors
*  @param [in]		paymenbtDates		A single or an array of to-date in YYYYMMDD format
*  @param [in]		curveCollection	    Curve set ID
*  @param [in]		curveIndex		    Index of the curve set. Default to OIS
*  @return			A array of discount factors
*/
std::vector<double> meCurveDiscountFactors( const std::vector<std::string>& paymentDates,
                                            const std::string& curveCollection,
                                            const std::string& curveIndex ) 
{
    MLIB_API_START
    
    std::vector<double> result;
        
    // Input marshalling
    DateVector tempPaymentDates;
	swig::buildDateVector(tempPaymentDates, paymentDates);

    result = validation_api::tryMeCurveDiscountFactors( tempPaymentDates, curveCollection.c_str(), curveIndex.c_str() );
	return result;
     
	MLIB_API_END
}

/* @brief			swig interface for meCurveDiscountFactorsFromYearFractions
*  @param [in]		yearFraction	Year fraction
*  @param [in]		dayCount		Day count
*  @param [in]		curveCollection	Curve set ID
*  @param [in]		curveIndex		Index of the curve set. Default to OIS
*  @return			A discount factor to the curve's asof date
*/
double meCurveDiscountFactorsFromYearFractions(double yearFraction, 
											const std::string& dayCount,
											const std::string& curveCollection, 
											const std::string& curveIndex)
{
    MLIB_API_START
	double ret = 0.0;

    // Input marshalling
	std::vector<double> yearFractions;
	yearFractions.push_back(yearFraction);

	LAString tmp_curveCollection(curveCollection.c_str());
	LAString tmp_curveIndex(curveIndex.c_str());
	LAString tmp_dayCount(dayCount.c_str());

	std::vector<double> DFs = validation_api::tryMeCurveDiscountFactorsFromYearFractions(yearFractions,
																						tmp_dayCount,
																						tmp_curveCollection, 
																						tmp_curveIndex);
	ret = DFs[0];
	return ret;
    MLIB_API_END
}

/* @brief			swig interface for meCurveDiscountFactorsFromYearFractions
*  @param [in]		yearFractions	An array of yearFractions by year count
*  @param [in]		curveCollection	Curve set ID
*  @param [in]		curveIndex		Index of the curve set. Default to OIS
*  @return			An array of discount factor
*/
std::vector<double> meCurveDiscountFactorsFromYearFractions(const std::vector<double>& yearFractions, 
															const std::string& dayCount,
															const std::string& curveCollection, 
															const std::string& curveIndex)
{
	MLIB_API_START
    std::vector<double> ret;
	
    // Input marshalling
	LAString tmp_curveCollection(curveCollection.c_str());
	LAString tmp_curveIndex(curveIndex.c_str());
	LAString tmp_dayCount(dayCount.c_str());

	ret = validation_api::tryMeCurveDiscountFactorsFromYearFractions(yearFractions, 
																	tmp_dayCount,
																	tmp_curveCollection, 
																	tmp_curveIndex);
    return ret;
    MLIB_API_END
}

/* @brief			swig interface for meCurveDiscountFactorsFromTenors
*  @param [in]		tenor			Tenor
*  @param [in]		curveCollection	Curve set ID
*  @param [in]		curveIndex		Index of the curve set. Default to OIS
*  @param [in]		businessDayAdj	Rule that decides if end date should roll forward or backward etc, ie, MF. Default to 'NO_CHANGE'.
*  @param [in]		calendar		Calendar 
*  @return			A discount factor to the curve's asof date
*/
double meCurveDiscountFactorsFromTenors(const std::string& tenor, 
					const std::string& curveCollection, 
					const std::string& curveIndex, 
					const std::string& businessDayAdj, 
					const std::string& calendar)
{
	MLIB_API_START
    double ret = 0.0;

    // Input marshalling
	LAStringVector tenors;
	tenors.push_back(tenor.c_str());

	LAString tmp_curveCollection(curveCollection.c_str());
	LAString tmp_curveIndex(curveIndex.c_str());
	LAString tmp_businessDayAdj(businessDayAdj.c_str());
	LAString tmp_calendar(calendar.c_str());

	std::vector<double> DFs = validation_api::tryMeCurveDiscountFactorsFromTenors(tenors,
																				tmp_curveCollection, 
																				tmp_curveIndex,
																				tmp_businessDayAdj, 
																				tmp_calendar);
	ret = DFs[0];
	return ret;
    MLIB_API_END
}

/* @brief			swig interface for meCurveDiscountFactorsFromTenors
*  @param [in]		tenors			An array of tenor strings
*  @param [in]		curveCollection	Curve set ID
*  @param [in]		curveIndex		Index of the curve set. Default to OIS
*  @param [in]		businessDayAdj	Rule that decides if end date should roll forward or backward etc, ie, MF. Default to 'NO_CHANGE'.
*  @param [in]		calendar		Calendar 
*  @return			An array of discount factor
*/
std::vector<double> meCurveDiscountFactorsFromTenors(const std::vector<std::string>& tenors, 
					const std::string& curveCollection, 
					const std::string& curveIndex, 
					const std::string& businessDayAdj, 
					const std::string& calendar)
{
    MLIB_API_START
	std::vector<double> ret;

    // Input marshalling
	LAStringVector tmp_tenors;
	swig::buildStringVector(tmp_tenors, tenors);

	LAString tmp_curveCollection(curveCollection.c_str());
	LAString tmp_curveIndex(curveIndex.c_str());
	LAString tmp_businessDayAdj(businessDayAdj.c_str());
	LAString tmp_calendar(calendar.c_str());

	ret = validation_api::tryMeCurveDiscountFactorsFromTenors(tmp_tenors,
															tmp_curveCollection, 
															tmp_curveIndex,
															tmp_businessDayAdj, 
															tmp_calendar);
	return ret;
    MLIB_API_END
}

/* @brief			swig interface for meCurveDiscountFactorsForwardStarting
*  @param [in]		fromDate		A single from-date in YYYYMMDD formate
*  @param [in]		toDate			A single to-date in YYYYMMDD formate
*  @param [in]		curveCollection	Curve set ID
*  @param [in]		curveIndex		Index of the curve set. Default to OIS
*  @return			A discount factor
*/
double meCurveDiscountFactorsForwardStarting(const std::string& fromDate, 
												const std::string& toDate, 
												const std::string& curveCollection, 
												const std::string& curveIndex)
{
    MLIB_API_START
	double ret = 0.0;

    // Input marshalling
	DateVector fromDates;
	fromDates.push_back( etrading::stringToDate( fromDate ) );
	DateVector toDates;
	toDates.push_back( etrading::stringToDate( toDate ) );

	LAString tmp_curveCollection(curveCollection.c_str());
	LAString tmp_curveIndex(curveIndex.c_str());

	std::vector<double> DFs = validation_api::tryMeCurveDiscountFactorsForwardStarting(
										fromDates, 
										toDates,
										tmp_curveCollection, 
										tmp_curveIndex
										);

	ret = DFs[0];
	return ret;
    MLIB_API_END
}


/* @brief			swig interface for meCurveDiscountFactorsForwardStarting
*  @param [in]		fromDates		An array of from-date in YYYYMMDD formate
*  @param [in]		toDates			An array of to-date in YYYYMMDD formate
*  @param [in]		curveCollection	Curve set ID
*  @param [in]		curveIndex		Index of the curve set. Default to OIS
*  @return			An array of discount factors
*/
std::vector<double> meCurveDiscountFactorsForwardStarting(const std::vector<std::string>& fromDates, 
															const std::vector<std::string>& toDates, 
															const std::string& curveCollection, 
															const std::string& curveIndex)
{
    MLIB_API_START
	std::vector<double> ret;

    // Input marshalling
	DateVector tmp_fromDates;
	swig::buildDateVector(tmp_fromDates, fromDates);
	DateVector tmp_toDates;
	swig::buildDateVector(tmp_toDates, toDates);

	LAString tmp_curveCollection(curveCollection.c_str());
	LAString tmp_curveIndex(curveIndex.c_str());

	ret = validation_api::tryMeCurveDiscountFactorsForwardStarting(
										tmp_fromDates, 
										tmp_toDates,
										tmp_curveCollection, 
										tmp_curveIndex
										);
	return ret;
    MLIB_API_END
}


/* @brief			swig interface for meCurveDiscountFactorsForwardStartingFromYearFractions
*  @param [in]		fromDate			A single fromDate in YYYYMMDD formate
*  @param [in]		yearFraction		A single year fraction
*  @param [in]		dayCount	        The daycount fraction used to generate the yearFraction
*  @param [in]		curveCollection		Curve set ID
*  @param [in]		curveIndex			Index of the curve set. Default to OIS
*  @return			A discount factor
*/
double meCurveDiscountFactorsForwardStartingFromYearFractions(const std::string& fromDate, 
																double yearFraction, 
																const std::string& dayCount,
																const std::string& curveCollection, 
																const std::string& curveIndex)
{
    MLIB_API_START
	double ret = 0.0;

    // Input marshalling
	DateVector fromDates;
	fromDates.push_back( etrading::stringToDate( fromDate ) );
	DoubleVector yearFractions;
	yearFractions.push_back(yearFraction);

	LAString tmp_curveCollection(curveCollection.c_str());
	LAString tmp_curveIndex(curveIndex.c_str());
	LAString tmp_dayCount(dayCount.c_str());

	std::vector<double> DFs = validation_api::tryMeCurveDiscountFactorsForwardStartingFromYearFractions( fromDates, 
														                                                 yearFractions,
														                                                 tmp_dayCount,
														                                                 tmp_curveCollection, 
														                                                 tmp_curveIndex);
	ret = DFs[0];
	return ret;
    MLIB_API_END
}


/* @brief			swig interface for meCurveDiscountFactorsForwardStartingFromYearFractions
*  @param [in]		fromDates			An array of from-dates in YYYYMMDD formate
*  @param [in]		yearFractions		An array of year fraction yearFractions
*  @param [in]		dayCount	        The daycount fraction used to generate the yearFraction
*  @param [in]		curveCollection		Curve set ID
*  @param [in]		curveIndex			Index of the curve set. Default to OIS
*  @return			An array of discount factors
*/
std::vector<double> meCurveDiscountFactorsForwardStartingFromYearFractions(const std::vector<std::string>& fromDates, 
																			const std::vector<double>& yearFractions, 
																			const std::string& dayCount,
																			const std::string& curveCollection, 
																			const std::string& curveIndex)
{
    MLIB_API_START
	std::vector<double> ret;
	
	// Input marshalling
	DateVector tmp_fromDates;
	swig::buildDateVector(tmp_fromDates, fromDates);

	LAString tmp_curveCollection(curveCollection.c_str());
	LAString tmp_curveIndex(curveIndex.c_str());
	LAString tmp_dayCount(dayCount.c_str());

	ret = validation_api::tryMeCurveDiscountFactorsForwardStartingFromYearFractions( tmp_fromDates, 
														                             yearFractions,
                                                                                     tmp_dayCount,
														                             tmp_curveCollection, 
														                             tmp_curveIndex);
	return ret;
    MLIB_API_END
}


/* @brief			swig interface for meCurveDiscountFactorsForwardStartingFromTenor
*  @param [in]		fromDate		A single from-dates in YYYYMMDD formate
*  @param [in]		tenor			A tenor string 
*  @param [in]		curveCollection	Curve set ID
*  @param [in]		curveIndex		Index of the curve set. Default to OIS
*  @param [in]		businessDayAdj	Rule that decides if end date should roll forward or backward etc, ie, MF. Default to 'NO_CHANGE'.
*  @param [in]		calendar		Calendar 
*  @return			A discount factor
*/
double meCurveDiscountFactorsForwardStartingFromTenor(const std::string& fromDate, 
														const std::string& tenor, 
														const std::string& curveCollection, 
														const std::string& curveIndex, 
														const std::string& businessDayAdj, 
														const std::string& calendar)
{
    MLIB_API_START
	double ret = 0.0;

    // Input marshalling
	DateVector fromDates;
	fromDates.push_back( etrading::stringToDate( fromDate ) );

	LAString tmp_tenor(tenor.c_str());
	LAString tmp_curveCollection(curveCollection.c_str());
	LAString tmp_curveIndex(curveIndex.c_str());
	LAString tmp_businessDayAdj(businessDayAdj.c_str());
	LAString tmp_calendar(calendar.c_str());

	std::vector<double> DFs = validation_api::tryMeCurveDiscountFactorsForwardStartingFromTenor(
												fromDates, 
												tmp_tenor,
												tmp_curveCollection, 
												tmp_curveIndex,
												tmp_businessDayAdj, 
												tmp_calendar);
	ret = DFs[0];
	return ret;
    MLIB_API_END
}


/* @brief			swig interface for meCurveDiscountFactorsForwardStartingFromTenor
*  @param [in]		fromDates		An array of from-dates in YYYYMMDD formate
*  @param [in]		tenor			A tenor string 
*  @param [in]		curveCollection	Curve set ID
*  @param [in]		curveIndex		Index of the curve set. Default to OIS
*  @param [in]		businessDayAdj	Rule that decides if end date should roll forward or backward etc, ie, MF. Default to 'NO_CHANGE'.
*  @param [in]		calendar		Calendar 
*  @return			An array of discount factors
*/
std::vector<double> meCurveDiscountFactorsForwardStartingFromTenor(const std::vector<std::string>& fromDates, 
														const std::string& tenor, 
														const std::string& curveCollection, 
														const std::string& curveIndex, 
														const std::string& businessDayAdj, 
														const std::string& calendar)
{
    MLIB_API_START
	std::vector<double> ret;

    // Input marshalling
	DateVector tmp_fromDates;
	swig::buildDateVector(tmp_fromDates, fromDates);

	LAString tmp_tenor(tenor.c_str());
	LAString tmp_curveCollection(curveCollection.c_str());
	LAString tmp_curveIndex(curveIndex.c_str());
	LAString tmp_businessDayAdj(businessDayAdj.c_str());
	LAString tmp_calendar(calendar.c_str());

	std::vector<double> DFs = validation_api::tryMeCurveDiscountFactorsForwardStartingFromTenor(
												tmp_fromDates, 
												tmp_tenor,
												tmp_curveCollection, 
												tmp_curveIndex,
												tmp_businessDayAdj, 
												tmp_calendar);
	return ret;
    MLIB_API_END
}