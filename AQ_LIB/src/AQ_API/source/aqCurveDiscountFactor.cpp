#include "TypeUtilities.h"
#include "AQLDate.h"
#include "AQLString.h"
#include "aqCurveDiscountFactor.h"
#include "tryAqCurveDiscountFactor.h"
#include "APISetUp.h"               // AQ_API_START and AQ_API_END Macros
#include "ParameterValidation.h"


/* @brief			validation method for aqCurveDiscountFactors
*  @param [in]		paymenbtDates		A single or an array of to-date in YYYYMMDD format
*  @param [in]		curveCollection	    Curve set ID
*  @param [in]		curveIndex		    Index of the curve set. Default to OIS
*  @return			A array of discount factors
*/
std::vector<double> aqCurveDiscountFactors( const std::vector<std::string>& paymentDates,
                                            const std::string& curveCollection,
                                            const std::string& curveIndex ) 
{
    AQ_API_START
    
    std::vector<double> result;
        
    // Input marshalling
    DateVector tempPaymentDates;
	swig::buildDateVector(tempPaymentDates, paymentDates);

    result = validation::tryAqCurveDiscountFactors( tempPaymentDates, curveCollection.c_str(), curveIndex.c_str() );
	return result;
     
	AQ_API_END
}

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
											const std::string& curveIndex)
{
    AQ_API_START
	double ret = 0.0;

    // Input marshalling
	std::vector<double> yearFractions;
	yearFractions.push_back(yearFraction);

	AQLString tmp_curveCollection(curveCollection.c_str());
	AQLString tmp_curveIndex(curveIndex.c_str());
	AQLString tmp_dayCount(dayCount.c_str());

	std::vector<double> DFs = validation::tryAqCurveDiscountFactorsFromYearFractions(yearFractions,
																						tmp_dayCount,
																						tmp_curveCollection, 
																						tmp_curveIndex);
	ret = DFs[0];
	return ret;
    AQ_API_END
}

/* @brief			swig interface for aqCurveDiscountFactorsFromYearFractions
*  @param [in]		yearFractions	An array of yearFractions by year count
*  @param [in]		curveCollection	Curve set ID
*  @param [in]		curveIndex		Index of the curve set. Default to OIS
*  @return			An array of discount factor
*/
std::vector<double> aqCurveDiscountFactorsFromYearFractions(const std::vector<double>& yearFractions, 
															const std::string& dayCount,
															const std::string& curveCollection, 
															const std::string& curveIndex)
{
	AQ_API_START
    std::vector<double> ret;
	
    // Input marshalling
	AQLString tmp_curveCollection(curveCollection.c_str());
	AQLString tmp_curveIndex(curveIndex.c_str());
	AQLString tmp_dayCount(dayCount.c_str());

	ret = validation::tryAqCurveDiscountFactorsFromYearFractions(yearFractions, 
																	tmp_dayCount,
																	tmp_curveCollection, 
																	tmp_curveIndex);
    return ret;
    AQ_API_END
}

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
					const std::string& calendar)
{
	AQ_API_START
    double ret = 0.0;

    // Input marshalling
	AQLStringVector tenors;
	tenors.push_back(tenor.c_str());

	AQLString tmp_curveCollection(curveCollection.c_str());
	AQLString tmp_curveIndex(curveIndex.c_str());
	AQLString tmp_businessDayAdj(businessDayAdj.c_str());
	AQLString tmp_calendar(calendar.c_str());

	std::vector<double> DFs = validation::tryAqCurveDiscountFactorsFromTenors(tenors,
																				tmp_curveCollection, 
																				tmp_curveIndex,
																				tmp_businessDayAdj, 
																				tmp_calendar);
	ret = DFs[0];
	return ret;
    AQ_API_END
}

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
					const std::string& calendar)
{
    AQ_API_START
	std::vector<double> ret;

    // Input marshalling
	AQLStringVector tmp_tenors;
	swig::buildStringVector(tmp_tenors, tenors);

	AQLString tmp_curveCollection(curveCollection.c_str());
	AQLString tmp_curveIndex(curveIndex.c_str());
	AQLString tmp_businessDayAdj(businessDayAdj.c_str());
	AQLString tmp_calendar(calendar.c_str());

	ret = validation::tryAqCurveDiscountFactorsFromTenors(tmp_tenors,
															tmp_curveCollection, 
															tmp_curveIndex,
															tmp_businessDayAdj, 
															tmp_calendar);
	return ret;
    AQ_API_END
}

/* @brief			swig interface for aqCurveDiscountFactorsForwardStarting
*  @param [in]		fromDate		A single from-date in YYYYMMDD formate
*  @param [in]		toDate			A single to-date in YYYYMMDD formate
*  @param [in]		curveCollection	Curve set ID
*  @param [in]		curveIndex		Index of the curve set. Default to OIS
*  @return			A discount factor
*/
double aqCurveDiscountFactorsForwardStarting(const std::string& fromDate, 
												const std::string& toDate, 
												const std::string& curveCollection, 
												const std::string& curveIndex)
{
    AQ_API_START
	double ret = 0.0;

    // Input marshalling
	DateVector fromDates;
	fromDates.push_back( etrading::stringToDate( fromDate ) );
	DateVector toDates;
	toDates.push_back( etrading::stringToDate( toDate ) );

	AQLString tmp_curveCollection(curveCollection.c_str());
	AQLString tmp_curveIndex(curveIndex.c_str());

	std::vector<double> DFs = validation::tryAqCurveDiscountFactorsForwardStarting(
										fromDates, 
										toDates,
										tmp_curveCollection, 
										tmp_curveIndex
										);

	ret = DFs[0];
	return ret;
    AQ_API_END
}


/* @brief			swig interface for aqCurveDiscountFactorsForwardStarting
*  @param [in]		fromDates		An array of from-date in YYYYMMDD formate
*  @param [in]		toDates			An array of to-date in YYYYMMDD formate
*  @param [in]		curveCollection	Curve set ID
*  @param [in]		curveIndex		Index of the curve set. Default to OIS
*  @return			An array of discount factors
*/
std::vector<double> aqCurveDiscountFactorsForwardStarting(const std::vector<std::string>& fromDates, 
															const std::vector<std::string>& toDates, 
															const std::string& curveCollection, 
															const std::string& curveIndex)
{
    AQ_API_START
	std::vector<double> ret;

    // Input marshalling
	DateVector tmp_fromDates;
	swig::buildDateVector(tmp_fromDates, fromDates);
	DateVector tmp_toDates;
	swig::buildDateVector(tmp_toDates, toDates);

	AQLString tmp_curveCollection(curveCollection.c_str());
	AQLString tmp_curveIndex(curveIndex.c_str());

	ret = validation::tryAqCurveDiscountFactorsForwardStarting(
										tmp_fromDates, 
										tmp_toDates,
										tmp_curveCollection, 
										tmp_curveIndex
										);
	return ret;
    AQ_API_END
}


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
																const std::string& curveIndex)
{
    AQ_API_START
	double ret = 0.0;

    // Input marshalling
	DateVector fromDates;
	fromDates.push_back( etrading::stringToDate( fromDate ) );
	DoubleVector yearFractions;
	yearFractions.push_back(yearFraction);

	AQLString tmp_curveCollection(curveCollection.c_str());
	AQLString tmp_curveIndex(curveIndex.c_str());
	AQLString tmp_dayCount(dayCount.c_str());

	std::vector<double> DFs = validation::tryAqCurveDiscountFactorsForwardStartingFromYearFractions( fromDates, 
														                                                 yearFractions,
														                                                 tmp_dayCount,
														                                                 tmp_curveCollection, 
														                                                 tmp_curveIndex);
	ret = DFs[0];
	return ret;
    AQ_API_END
}


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
																			const std::string& curveIndex)
{
    AQ_API_START
	std::vector<double> ret;
	
	// Input marshalling
	DateVector tmp_fromDates;
	swig::buildDateVector(tmp_fromDates, fromDates);

	AQLString tmp_curveCollection(curveCollection.c_str());
	AQLString tmp_curveIndex(curveIndex.c_str());
	AQLString tmp_dayCount(dayCount.c_str());

	ret = validation::tryAqCurveDiscountFactorsForwardStartingFromYearFractions( tmp_fromDates, 
														                             yearFractions,
                                                                                     tmp_dayCount,
														                             tmp_curveCollection, 
														                             tmp_curveIndex);
	return ret;
    AQ_API_END
}


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
														const std::string& calendar)
{
    AQ_API_START
	double ret = 0.0;

    // Input marshalling
	DateVector fromDates;
	fromDates.push_back( etrading::stringToDate( fromDate ) );

	AQLString tmp_tenor(tenor.c_str());
	AQLString tmp_curveCollection(curveCollection.c_str());
	AQLString tmp_curveIndex(curveIndex.c_str());
	AQLString tmp_businessDayAdj(businessDayAdj.c_str());
	AQLString tmp_calendar(calendar.c_str());

	std::vector<double> DFs = validation::tryAqCurveDiscountFactorsForwardStartingFromTenor(
												fromDates, 
												tmp_tenor,
												tmp_curveCollection, 
												tmp_curveIndex,
												tmp_businessDayAdj, 
												tmp_calendar);
	ret = DFs[0];
	return ret;
    AQ_API_END
}


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
														const std::string& calendar)
{
    AQ_API_START
	std::vector<double> ret;

    // Input marshalling
	DateVector tmp_fromDates;
	swig::buildDateVector(tmp_fromDates, fromDates);

	AQLString tmp_tenor(tenor.c_str());
	AQLString tmp_curveCollection(curveCollection.c_str());
	AQLString tmp_curveIndex(curveIndex.c_str());
	AQLString tmp_businessDayAdj(businessDayAdj.c_str());
	AQLString tmp_calendar(calendar.c_str());

	std::vector<double> DFs = validation::tryAqCurveDiscountFactorsForwardStartingFromTenor(
												tmp_fromDates, 
												tmp_tenor,
												tmp_curveCollection, 
												tmp_curveIndex,
												tmp_businessDayAdj, 
												tmp_calendar);
	return ret;
    AQ_API_END
}