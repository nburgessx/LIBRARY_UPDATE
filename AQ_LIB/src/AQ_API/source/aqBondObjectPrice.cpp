// aqBondObjectPrice.cpp

/* 
 * @brief			Swig interface for aqBondObjectPrice... functions
 */

#include "aqBondObjectPrice.h"
#include "AQLCoreTemplateType.h"
#include "TypeUtilities.h"
#include "tryAqBondObject.h"
#include "AQObjUtilities.h"         // etrading::getBond - tryAqBondObjectYieldFromObject / YieldOptimized
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

/* @brief			swig interface for aqBondObjectPriceFromCleanToDirty function
*  @param [in]		bondObjectName			Bond Object Name
*  @param [in]		valuationSettingsLVB	Key/value block; set SettlementDate (and the FRN fields for a floater)
*  @param [in]		cleanPrice				Bond clean price
*  @param [out]		Returns the bond dirty price
*/
double aqBondObjectPriceFromCleanToDirty( const std::string& bondObjectName,
                                       const SWIG_STRINGMATRIX& valuationSettingsLVB,
                                       const double& cleanPrice )
{
    AQ_API_START

    // Marshall Inputs
    AQLStringMatrix valuationSettingsAsStringMatrix;
    swig::buildStringMatrix( valuationSettingsAsStringMatrix, valuationSettingsLVB );
    LabelValueBlock valuationSettingsAsLabelValueBlock( valuationSettingsAsStringMatrix );

    // Call validation method
    double result = validation::tryAqBondObjectPriceFromCleanToDirty( bondObjectName, valuationSettingsAsLabelValueBlock, cleanPrice );
    return result;

    AQ_API_END
}

/* @brief			swig interface for aqBondObjectPriceFromDirtyToClean function
*  @param [in]		bondObjectName			Bond Object Name
*  @param [in]		valuationSettingsLVB	Key/value block; set SettlementDate (and the FRN fields for a floater)
*  @param [in]		dirtyPrice				Bond dirty price
*  @param [out]		Returns the bond clean price
*/
double aqBondObjectPriceFromDirtyToClean( const std::string& bondObjectName,
                                       const SWIG_STRINGMATRIX& valuationSettingsLVB,
                                       const double& dirtyPrice )
{
    AQ_API_START

    // Marshall Inputs
    AQLStringMatrix valuationSettingsAsStringMatrix;
    swig::buildStringMatrix( valuationSettingsAsStringMatrix, valuationSettingsLVB );
    LabelValueBlock valuationSettingsAsLabelValueBlock( valuationSettingsAsStringMatrix );

    // Call validation method
    double result = validation::tryAqBondObjectPriceFromDirtyToClean( bondObjectName, valuationSettingsAsLabelValueBlock, dirtyPrice );
    return result;

    AQ_API_END
}

/* @brief			swig interface for aqBondObjectYieldFromObject function. Yield (price -> yield) for a cached bond at a settlement date.
*  @param [in]		bondObjectName		Bond Object Name
*  @param [in]		settlementDate		Settlement date
*  @param [in]		price				Bond price to solve the yield from
*  @param [in]		yieldCalculationType	Yield calculation type
*  @param [out]		Returns the bond yield
*/
double aqBondObjectYieldFromObject( const std::string& bondObjectName,
                                 const std::string& settlementDate,
                                 const double& price,
                                 const std::string& yieldCalculationType )
{
    AQ_API_START

    // Marshall Inputs
    AQLDate settlementDate_( etrading::stringToDate( settlementDate ) );

    // Call validation method
    double result = validation::tryAqBondObjectYieldFromObject( etrading::getBond( bondObjectName ), settlementDate_, price, yieldCalculationType );
    return result;

    AQ_API_END
}

/* @brief			swig interface for aqBondObjectYieldOptimized function. Optimised yield solve (price -> yield) at multiple settlement dates.
*  @param [in]		bondObjectName		Bond Object Name
*  @param [in]		settlementDates		Settlement date(s)
*  @param [in]		prices				Bond price(s), internally checked for clean or dirty
*  @param [in]		yieldCalculationType	Yield calculation type
*  @param [out]		Returns the bond yield(s)
*/
std::vector<double> aqBondObjectYieldOptimized( const std::string& bondObjectName,
                                             const std::vector<std::string>& settlementDates,
                                             const std::vector<double>& prices,
                                             const std::string& yieldCalculationType )
{
    AQ_API_START

    // Marshall Inputs
    DateVector settlementDatesAsDateVector;
    swig::buildDateVector( settlementDatesAsDateVector, settlementDates );

    // Call validation method
    std::vector<double> results = validation::tryAqBondObjectYieldOptimized( etrading::getBond( bondObjectName ), settlementDatesAsDateVector, prices, yieldCalculationType );
    return results;

    AQ_API_END
}

/* @brief			swig interface for aqBondObjectCompoundYield function
*  @param [in]		bondObjectName		Bond Object Name
*  @param [in]		settlementDates		Settlement date(s)
*  @param [in]		prices				User input price(s)
*  @param [in]		yieldCalculationType	Yield calculation type
*  @param [out]		Returns the compound yield(s), used as the bond's discount factors
*/
std::vector<double> aqBondObjectCompoundYield( const std::string& bondObjectName,
                                            const std::vector<std::string>& settlementDates,
                                            const std::vector<double>& prices,
                                            const std::string& yieldCalculationType )
{
    AQ_API_START

    // Marshall Inputs
    DateVector settlementDatesAsDateVector;
    swig::buildDateVector( settlementDatesAsDateVector, settlementDates );

    // Call validation method
    std::vector<double> results = validation::tryAqBondObjectCompoundYield( bondObjectName, settlementDatesAsDateVector, prices, yieldCalculationType );
    return results;

    AQ_API_END
}

/* @brief			swig interface for aqBondObjectAccruedInterest function
*  @param [in]		bondObjectName		Bond Object Name
*  @param [in]		settlementDates		Settlement date(s)
*  @param [out]		Returns the bond accrued interest, one value per settlement date
*/
std::vector<double> aqBondObjectAccruedInterest( const std::string& bondObjectName,
                                              const std::vector<std::string>& settlementDates )
{
    AQ_API_START

    // Marshall Inputs
    DateVector settlementDatesAsDateVector;
    swig::buildDateVector( settlementDatesAsDateVector, settlementDates );

    // Call validation method
    std::vector<double> results = validation::tryAqBondObjectAccruedInterest( bondObjectName, settlementDatesAsDateVector );
    return results;

    AQ_API_END
}

/* @brief			swig interface for aqBondObjectAccruedInterest function (single settlement, via a valuation-settings block - needed for FRNs)
*  @param [in]		bondObjectName			Bond Object Name
*  @param [in]		valuationSettingsLVB	Key/value block; set SettlementDate (and the FRN fields for a floater)
*  @param [out]		Returns the bond accrued interest
*/
double aqBondObjectAccruedInterestLVB( const std::string& bondObjectName,
                                 const SWIG_STRINGMATRIX& valuationSettingsLVB )
{
    AQ_API_START

    // Marshall Inputs
    AQLStringMatrix valuationSettingsAsStringMatrix;
    swig::buildStringMatrix( valuationSettingsAsStringMatrix, valuationSettingsLVB );
    LabelValueBlock valuationSettingsAsLabelValueBlock( valuationSettingsAsStringMatrix );

    // Call validation method
    double result = validation::tryAqBondObjectAccruedInterest( bondObjectName, valuationSettingsAsLabelValueBlock );
    return result;

    AQ_API_END
}

/* @brief			swig interface for aqBondObjectAccruedInterestDays function
*  @param [in]		bondObjectName		Bond Object Name
*  @param [in]		settlementDates		Settlement date(s)
*  @param [out]		Returns the bond accrued day count(s), one value per settlement date
*/
std::vector<int> aqBondObjectAccruedInterestDays( const std::string& bondObjectName,
                                               const std::vector<std::string>& settlementDates )
{
    AQ_API_START

    // Marshall Inputs
    DateVector settlementDatesAsDateVector;
    swig::buildDateVector( settlementDatesAsDateVector, settlementDates );

    // Call validation method
    std::vector<int> results = validation::tryAqBondObjectAccruedInterestDays( bondObjectName, settlementDatesAsDateVector );
    return results;

    AQ_API_END
}

/* @brief			swig interface for aqBondObjectQuote function. Quote (yield -> price) for a bond at one or more settlement dates.
*  @param [in]		bondObjectName		Bond Object Name
*  @param [in]		settlementDates		Settlement date(s)
*  @param [in]		yields				Yield(s), aligned with settlementDates
*  @param [in]		yieldCalculationType	Yield calculation type
*  @param [out]		Returns the bond price(s)
*/
std::vector<double> aqBondObjectQuote( const std::string& bondObjectName,
                                    const std::vector<std::string>& settlementDates,
                                    const std::vector<double>& yields,
                                    const std::string& yieldCalculationType )
{
    AQ_API_START

    // Marshall Inputs
    DateVector settlementDatesAsDateVector;
    swig::buildDateVector( settlementDatesAsDateVector, settlementDates );

    // Call validation method
    std::vector<double> results = validation::tryAqBondObjectQuote( bondObjectName, settlementDatesAsDateVector, yields, yieldCalculationType );
    return results;

    AQ_API_END
}

/* @brief			swig interface for aqBondObjectPriceFromCreditModel function. Price of a defaultable bond, priced off a credit model.
*  @param [in]		bondObjectName		Bond Object Name
*  @param [in]		settlementDate		Settlement date
*  @param [in]		creditModelName		Credit model object name
*  @param [out]		Returns the bond price
*/
double aqBondObjectPriceFromCreditModel( const std::string& bondObjectName,
                                      const std::string& settlementDate,
                                      const std::string& creditModelName )
{
    AQ_API_START

    // Marshall Inputs
    AQLDate settlementDate_( etrading::stringToDate( settlementDate ) );

    // Call validation method
    double result = validation::tryAqBondObjectPriceFromCreditModel( bondObjectName, settlementDate_, creditModelName );
    return result;

    AQ_API_END
}

/* @brief			swig interface for aqBondObjectLastCouponDate function
*  @param [in]		bondObjectName		Bond Object Name
*  @param [in]		settlementDates		Settlement date(s)
*  @param [out]		Returns the bond's last coupon date(s), as Julian day numbers
*/
std::vector<double> aqBondObjectLastCouponDate( const std::string& bondObjectName,
                                             const std::vector<std::string>& settlementDates )
{
    AQ_API_START

    // Marshall Inputs
    DateVector settlementDatesAsDateVector;
    swig::buildDateVector( settlementDatesAsDateVector, settlementDates );

    // Call validation method
    std::vector<double> results = validation::tryAqBondObjectLastCouponDate( bondObjectName, settlementDatesAsDateVector );
    return results;

    AQ_API_END
}

/* @brief			swig interface for aqBondObjectOisSpread function (multiple settlement dates). Bond yield to OIS par rate spread.
*  @param [in]		bondObjectName		Bond Object Name
*  @param [in]		marketObjectName	OIS market-data object name
*  @param [in]		settlementDates		Settlement date(s)
*  @param [in]		yields				Bond yield(s), aligned with settlementDates
*  @param [out]		Returns the O-spread(s)
*/
std::vector<double> aqBondObjectOisSpread( const std::string& bondObjectName,
                                        const std::string& marketObjectName,
                                        const std::vector<std::string>& settlementDates,
                                        const std::vector<double>& yields )
{
    AQ_API_START

    // Marshall Inputs
    DateVector settlementDatesAsDateVector;
    swig::buildDateVector( settlementDatesAsDateVector, settlementDates );

    // Call validation method
    std::vector<double> results = validation::tryAqBondObjectOisSpread( bondObjectName, marketObjectName, settlementDatesAsDateVector, yields );
    return results;

    AQ_API_END
}

/* @brief			swig interface for aqBondObjectOisSpread function (single settlement date). Bond yield to OIS par rate spread.
*  @param [in]		bondObjectName		Bond Object Name
*  @param [in]		marketObjectName	OIS market-data object name
*  @param [in]		settleDate			Settlement date
*  @param [in]		bondYield			Bond yield
*  @param [out]		Returns the O-spread
*/
double aqBondObjectOisSpread( const std::string& bondObjectName,
                           const std::string& marketObjectName,
                           const std::string& settleDate,
                           const double& bondYield )
{
    AQ_API_START

    // Marshall Inputs
    AQLDate settleDate_( etrading::stringToDate( settleDate ) );

    // Call validation method
    double result = validation::tryAqBondObjectOisSpread( bondObjectName, marketObjectName, settleDate_, bondYield );
    return result;

    AQ_API_END
}

/* @brief			swig interface for aqBondObjectRepoRate function. Bond's implied repo rate from its forward price at settle date.
*  @param [in]		bondObjectName		Bond Object Name
*  @param [in]		settleDate			Bond current settle date
*  @param [in]		forwardSettleDate	Bond forward settle date
*  @param [in]		price				Bond price at settle date
*  @param [in]		forwardPrice		Bond's forward price at forward settle date
*  @param [in]		repoDayCount		Day count for the repo year fraction
*  @param [out]		Returns the bond's implied repo rate
*/
double aqBondObjectRepoRate( const std::string& bondObjectName,
                          const std::string& settleDate,
                          const std::string& forwardSettleDate,
                          const double& price,
                          const double& forwardPrice,
                          const std::string& repoDayCount )
{
    AQ_API_START

    // Marshall Inputs
    AQLDate settleDate_( etrading::stringToDate( settleDate ) );
    AQLDate forwardSettleDate_( etrading::stringToDate( forwardSettleDate ) );

    // Call validation method
    double result = validation::tryAqBondObjectRepoRate( bondObjectName, settleDate_, forwardSettleDate_, price, forwardPrice, repoDayCount );
    return result;

    AQ_API_END
}

/* @brief			swig interface for aqBondObjectForwardReinvestedCoupon function. Bond's reinvested coupon value at forward settle date.
*  @param [in]		bondObjectName		Bond Object Name
*  @param [in]		settleDate			Bond current settle date
*  @param [in]		forwardSettleDate	Bond forward settle date
*  @param [in]		price				Bond price at settle date
*  @param [in]		repoRate			Bond repo rate
*  @param [in]		repoDayCount		Day count for the repo year fraction
*  @param [out]		Returns the bond's received coupon value at forward settle date
*/
double aqBondObjectForwardReinvestedCoupon( const std::string& bondObjectName,
                                         const std::string& settleDate,
                                         const std::string& forwardSettleDate,
                                         const double& price,
                                         const double& repoRate,
                                         const std::string& repoDayCount )
{
    AQ_API_START

    // Marshall Inputs
    AQLDate settleDate_( etrading::stringToDate( settleDate ) );
    AQLDate forwardSettleDate_( etrading::stringToDate( forwardSettleDate ) );

    // Call validation method
    double result = validation::tryAqBondObjectForwardReinvestedCoupon( bondObjectName, settleDate_, forwardSettleDate_, price, repoRate, repoDayCount );
    return result;

    AQ_API_END
}

