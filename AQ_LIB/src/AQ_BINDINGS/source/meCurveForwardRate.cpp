#include "TypeUtilities.h"

#include "meCurveForwardRate.h"
#include "tryMeCurveForwardRate.h"
#include "APISetUp.h"               // AQ_API_START and AQ_API_END Macros


/* @brief			swig interface for meCurveForwardRatesFromYearFraction
*  @param [in]		fromDates	    A vector of from dates
*  @param [in]		yearFraction	YearFraction
*  @param [in]		curveCollection	ID of the yield curve
*  @param [in]		curveIndex		Index of the curve set. Default to STD
*  @param [in]		dayCount		Day count convention. Default to ACT/365
*  @return			The forward rates based on fromDates and toDates
*/
std::vector<double> meCurveForwardRatesFromYearFraction( const std::vector<std::string>& fromDates, 
											             double yearFraction,
											             const std::string& curveCollection, 
											             const std::string& curveIndex, 
											             const std::string& dayCount)
{
    AQ_API_START
	std::vector<double> results;
	
	// Input marshalling
	DateVector tmp_fromDates;
	swig::buildDateVector(tmp_fromDates, fromDates);

	LAString tmp_curveCollection(curveCollection.c_str());
	LAString tmp_curveIndex(curveIndex.c_str());
	LAString tmp_dayCount(dayCount.c_str());

	results = validation_api::tryMeCurveForwardRatesFromYearFraction( tmp_fromDates, 
																	    yearFraction,
																	    tmp_dayCount,
																	    tmp_curveCollection, 
																	    tmp_curveIndex );
	return results;
    AQ_API_END
}


/* @brief			swig interface for meCurveForwardRates
*  @param [in]		fixingDates	    A vector of fixing dates
*  @param [in]		curveCollection	ID of the yield curve
*  @param [in]		curveIndex		Index of the curve set. Default to STD
*  @return			The forward rates based on fromDates and toDates
*/
std::vector<double> meCurveForwardRates( const std::vector<std::string>& fixingDates, 
										 const std::string& curveCollection,
										 const std::string& curveIndex)
{
	AQ_API_START
    std::vector<double> results;
	
	// Input marshalling
	DateVector tmp_fixingDates;
	swig::buildDateVector(tmp_fixingDates, fixingDates);

	LAString tmp_curveCollection(curveCollection.c_str());
	LAString tmp_curveIndex(curveIndex.c_str());

	results = validation_api::tryMeCurveForwardRates( tmp_fixingDates, 
														tmp_curveCollection, 
														tmp_curveIndex );
	return results;
    AQ_API_END
}


/* @brief			swig interface for meCurveForwardRatesFromForwardDates
*  @param [in]		startDates	        A vector of start dates
*  @param [in]		endDates	        A vector of end dates
*  @param [in]		curveCollection	    ID of the yield curve
*  @param [in]		curveIndex		    Index of the curve set. Default to STD
*  @return			The forward rates based on fromDates and toDates
*/
std::vector<double> meCurveForwardRatesFromForwardDates( const std::vector<std::string>& fromDates,
                                                         const std::vector<std::string>& toDates,
											             const std::string& curveCollection,
											             const std::string& curveIndex )
{
	AQ_API_START
    std::vector<double> results;
		
    // Input marshalling
	DateVector tmp_fromDates;
    swig::buildDateVector(tmp_fromDates, fromDates);

    DateVector tmp_toDates;
    swig::buildDateVector(tmp_toDates, toDates);

	LAString tmp_curveCollection(curveCollection.c_str());
	LAString tmp_curveIndex(curveIndex.c_str());

	results = validation_api::tryMeCurveForwardRatesFromForwardDates( tmp_fromDates,
                                                                        tmp_toDates,
														                tmp_curveCollection, 
														                tmp_curveIndex );
	return results;
    AQ_API_END
}

std::vector<double> meCurveForwardRatesFromForwardDates(const std::vector<std::string>& fromDates,
														const std::vector<std::string>& toDates,
														const std::string& curveCollection,
														const std::string& curveIndex,
														const std::string& fwdInter)
{
	AQ_API_START
	
	std::vector<double> results;

	// Input marshalling
	DateVector tmp_fromDates;
	swig::buildDateVector(tmp_fromDates, fromDates);

	DateVector tmp_toDates;
	swig::buildDateVector(tmp_toDates, toDates);

	LAString tmp_curveCollection(curveCollection.c_str());
	LAString tmp_curveIndex(curveIndex.c_str());

	results = validation_api::tryMeCurveForwardRatesFromForwardDates(tmp_fromDates,
																	 tmp_toDates,
																	 tmp_curveCollection,
																	 tmp_curveIndex,
																	 fwdInter);
	return results;
	AQ_API_END
}
