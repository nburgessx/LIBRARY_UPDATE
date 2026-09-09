// aqBondObjectPrice.cpp

/* 
 * @brief			Swig interface for aqBondObjectPrice... functions
 */

#include "aqBondObjectPrice.h"
#include "AQLCoreTemplateType.h"
#include "TypeUtilities.h"
#include "tryAqBondObject.h"
#include "APISetUp.h"               // AQ_API_START and AQ_API_END Macros


/* @brief			swig interface for aqBondObjectPrice function
*  @param [in]		bondObjectName		    Bond Object Name
*  @param [in]		settlementDates	        Bond Settlement Date(s)
*  @param [in]		yields     	            Bond Yield(s)
*  @param [in]		yieldCalculationType	Yield Calculation Type
*  @param [out]		Returns a vector of bond prices for each settlement date, quoted clean/dirty based on market and Bond definition
*/
const std::vector<double> aqBondObjectPrice( const std::string& bondObjectName,
                                          const std::vector<std::string>& settlementDates,
                                          const std::vector<double>& yields,
                                          const std::string& yieldCalculationType )
{
    AQ_API_START

    // Marshall Inputs
	DateVector settlementDatesAsDateVector;
	swig::buildDateVector( settlementDatesAsDateVector, settlementDates );

    // Call validation method
    const std::vector<double> results = validation::tryAqBondObjectPrice( bondObjectName, settlementDatesAsDateVector, yields, yieldCalculationType );
    return results;
    
    AQ_API_END
}

/* @brief			swig interface for aqBondObjectCleanPrice function
*  @param [in]		bondObjectName		    Bond Object Name
*  @param [in]		settlementDates	        Bond Settlement Date(s)
*  @param [in]		yields     	            Bond Yield(s)
*  @param [in]		yieldCalculationType	Yield Calculation Type
*  @param [out]		Returns a vector of clean bond prices for each settlement date
*/
const std::vector<double> aqBondObjectCleanPrice( const std::string& bondObjectName,
                                               const std::vector<std::string>& settlementDates,
                                               const std::vector<double>& yields,
                                               const std::string& yieldCalculationType )
{
    AQ_API_START

    // Marshall Inputs
	DateVector settlementDatesAsDateVector;
	swig::buildDateVector( settlementDatesAsDateVector, settlementDates );

    // Call validation method
    const std::vector<double> results = validation::tryAqBondObjectCleanPrice( bondObjectName, settlementDatesAsDateVector, yields, yieldCalculationType );
    return results;

    AQ_API_END
}

/* @brief			swig interface for aqBondObjectDirtyPrice function
*  @param [in]		bondObjectName		    Bond Object Name
*  @param [in]		settlementDates	        Bond Settlement Date(s)
*  @param [in]		yields     	            Bond Yield(s)
*  @param [in]		yieldCalculationType	Yield Calculation Type
*  @param [out]		Returns a vector of clean bond prices for each settlement date
*/
const std::vector<double> aqBondObjectDirtyPrice( const std::string& bondObjectName,
                                               const std::vector<std::string>& settlementDates,
                                               const std::vector<double>& yields,
                                               const std::string& yieldCalculationType )
{
    AQ_API_START

    // Marshall Inputs
	DateVector settlementDatesAsDateVector;
	swig::buildDateVector( settlementDatesAsDateVector, settlementDates );

    // Call validation method
    const std::vector<double> results = validation::tryAqBondObjectDirtyPrice( bondObjectName, settlementDatesAsDateVector, yields, yieldCalculationType );
    return results;
    
    AQ_API_END
}


/* @brief			swig interface for aqBondObjectYield function
*  @param [in]		bondObjectName		    Bond Object Name
*  @param [in]		settlementDates	        Bond Settlement Date(s)
*  @param [in]		quotedPrices            Bond Quoted Price(s), must input consistently as clean or dirty as quoted in the market and specified in the Bond Definition
*  @param [in]		yieldCalculationType	Yield Calculation Type
*  @param [in]		optimizePerformance	    Optimize for performance
*  @param [out]		Returns a vector of clean bond prices for each settlement date
*/
const std::vector<double> aqBondObjectYield( const std::string& bondObjectName,
                                          const std::vector<std::string>& settlementDates,
                                          const std::vector<double>& quotedPrices,
                                          const std::string& yieldCalculationType,
                                          const bool& optimizePerformance )
{
    AQ_API_START

    // Marshall Inputs
	DateVector settlementDatesAsDateVector;
	swig::buildDateVector( settlementDatesAsDateVector, settlementDates );

    // Call validation method
    const std::vector<double> results = validation::tryAqBondObjectYield( bondObjectName, settlementDatesAsDateVector, quotedPrices, yieldCalculationType, optimizePerformance );
    return results;
    
    AQ_API_END
}


/* @brief			Validation interface for the aqBondObjectForwardPrice method. Calculate Bond's forward price at forward settle date, from repo rate.
*  @param [in]		bondObjectName		Bond object name
*  @param [in]		settleDate			Bond current settle date
*  @param [in]		forwardSettleDate   Bond forward settle date
*  @param [in]		price				Bond price at settle date, can be dirty or clean depends on Bond's isCleanPrice flag
*  @param [in]		repoRate			Bond repo rate
*  @param [in]		repoDayCount		Day count for repo year fraction
*  @return			Bond forward price
*/
double aqBondObjectForwardPrice( const std::string& bondObjectName,
                              const std::string& settleDate,
                              const std::string& forwardSettleDate,
                              const double& price,
                              const double& repoRate,
                              const std::string& repoDayCount )
{
	AQ_API_START
        
    // Marshall Inputs
    AQLDate settleDate_          = swig::fromStringToAQLDate( settleDate );
    AQLDate forwardSettleDate_   = swig::fromStringToAQLDate( forwardSettleDate );
        
    // Call validation method
    const double result = validation::tryAqBondObjectForwardPrice( bondObjectName, settleDate_, forwardSettleDate_, price, repoRate, repoDayCount );
    return result;
    
    AQ_API_END
}

