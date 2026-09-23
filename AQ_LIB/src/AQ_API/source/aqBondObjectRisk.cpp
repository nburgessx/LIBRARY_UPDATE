// aqBondObjectRisk.cpp

/*
 * @brief			Swig interface for aqBondObject risk/sensitivity functions: DV01, Modified Duration, BPV per tick, Z-Spread
 */

#include "aqBondObjectRisk.h"
#include "AQLCoreTemplateType.h"
#include "TypeUtilities.h"
#include "tryAqBondObject.h"
#include "APISetUp.h"               // AQ_API_START and AQ_API_END Macros


/* @brief			swig interface for aqBondObjectDV01 function. Bond DV01(s), computed analytically.
*  @param [in]		bondObjectName		Bond Object Name
*  @param [in]		settlementDates		Settlement date(s)
*  @param [in]		yields				Yield(s), aligned with settlementDates
*  @param [in]		yieldCalculationType	Yield calculation type
*  @param [out]		Returns the bond DV01(s)
*/
std::vector<double> aqBondObjectDV01( const std::string& bondObjectName,
                                   const std::vector<std::string>& settlementDates,
                                   const std::vector<double>& yields,
                                   const std::string& yieldCalculationType )
{
    AQ_API_START

    // Marshall Inputs
    DateVector settlementDatesAsDateVector;
    swig::buildDateVector( settlementDatesAsDateVector, settlementDates );

    // Call validation method
    std::vector<double> results = validation::tryAqBondObjectDV01( bondObjectName, settlementDatesAsDateVector, yields, yieldCalculationType );
    return results;

    AQ_API_END
}

/* @brief			swig interface for aqBondObjectDV01Numerical function. Bond DV01(s), computed by numerically bumping the yield.
*  @param [in]		bondObjectName		Bond Object Name
*  @param [in]		settlementDates		Settlement date(s)
*  @param [in]		yields				Yield(s), aligned with settlementDates
*  @param [in]		bumpSize			Size of the yield bump in bps
*  @param [in]		bumpMode			How to calculate the delta: Up, Down or Central difference
*  @param [in]		yieldCalculationType	Yield calculation type
*  @param [out]		Returns the bond DV01(s)
*/
std::vector<double> aqBondObjectDV01Numerical( const std::string& bondObjectName,
                                            const std::vector<std::string>& settlementDates,
                                            const std::vector<double>& yields,
                                            const double& bumpSize,
                                            const std::string& bumpMode,
                                            const std::string& yieldCalculationType )
{
    AQ_API_START

    // Marshall Inputs
    DateVector settlementDatesAsDateVector;
    swig::buildDateVector( settlementDatesAsDateVector, settlementDates );
    AQLString bumpModeAsAQLString( bumpMode.c_str() );

    // Call validation method
    std::vector<double> results = validation::tryAqBondObjectDV01Numerical( bondObjectName, settlementDatesAsDateVector, yields, bumpSize, bumpModeAsAQLString, yieldCalculationType );
    return results;

    AQ_API_END
}

/* @brief			swig interface for aqBondObjectModifiedDuration function. Bond modified duration(s), computed analytically.
*  @param [in]		bondObjectName		Bond Object Name
*  @param [in]		settlementDates		Settlement date(s)
*  @param [in]		yields				Yield(s), aligned with settlementDates
*  @param [in]		yieldCalculationType	Yield calculation type
*  @param [out]		Returns the bond modified duration(s)
*/
std::vector<double> aqBondObjectModifiedDuration( const std::string& bondObjectName,
                                               const std::vector<std::string>& settlementDates,
                                               const std::vector<double>& yields,
                                               const std::string& yieldCalculationType )
{
    AQ_API_START

    // Marshall Inputs
    DateVector settlementDatesAsDateVector;
    swig::buildDateVector( settlementDatesAsDateVector, settlementDates );

    // Call validation method
    std::vector<double> results = validation::tryAqBondObjectModifiedDuration( bondObjectName, settlementDatesAsDateVector, yields, yieldCalculationType );
    return results;

    AQ_API_END
}

/* @brief			swig interface for aqBondObjectBPVPerTick function. Bond basis-point value per price tick.
*  @param [in]		bondObjectName			Bond Object Name
*  @param [in]		valuationSettingsLVB	Key/value block; set SettlementDate (and the FRN fields for a floater)
*  @param [in]		price					Bond price
*  @param [in]		yieldCalculationType	Yield calculation type
*  @param [in]		tickSize				Bond tick size, defaults to 1/32 for US Treasuries
*  @param [out]		Returns the bond basis-point value per tick
*/
double aqBondObjectBPVPerTick( const std::string& bondObjectName,
                            const SWIG_STRINGMATRIX& valuationSettingsLVB,
                            const double& price,
                            const std::string& yieldCalculationType,
                            const double& tickSize )
{
    AQ_API_START

    // Marshall Inputs
    AQLStringMatrix valuationSettingsAsStringMatrix;
    swig::buildStringMatrix( valuationSettingsAsStringMatrix, valuationSettingsLVB );
    LabelValueBlock valuationSettingsAsLabelValueBlock( valuationSettingsAsStringMatrix );

    // Call validation method
    double result = validation::tryAqBondObjectBPVPerTick( bondObjectName, valuationSettingsAsLabelValueBlock, price, yieldCalculationType, tickSize );
    return result;

    AQ_API_END
}

/* @brief			swig interface for aqBondObjectZSpread function (single bond). Bond Z-Spread against a curve collection.
*  @param [in]		bondObjectName				Bond Object Name
*  @param [in]		settlementDate				Bond's settlement date (valuation date)
*  @param [in]		bondPrice					Bond price
*  @param [in]		curveCollection				Curve collection name
*  @param [in]		forecastCurve				Forecast curve name
*  @param [in]		continuouslyCompounding	True to calculate the z-spread using continuous compounding
*  @param [out]		Returns the Z-Spread
*/
double aqBondObjectZSpread( const std::string& bondObjectName,
                         const std::string& settlementDate,
                         const double& bondPrice,
                         const std::string& curveCollection,
                         const std::string& forecastCurve,
                         const bool& continuouslyCompounding )
{
    AQ_API_START

    // Marshall Inputs
    AQLDate settlementDate_( etrading::stringToDate( settlementDate ) );

    // Call validation method
    double result = validation::tryAqBondObjectZSpread( bondObjectName, settlementDate_, bondPrice, curveCollection, forecastCurve, continuouslyCompounding );
    return result;

    AQ_API_END
}

/* @brief			swig interface for aqBondObjectZSpreads function (multiple bonds). Bond Z-Spread(s) against curve collection(s).
*  @param [in]		bondObjectName				Bond Object Name
*  @param [in]		settlementDates				Bond's settlement date(s) (valuation date(s))
*  @param [in]		bondPrices					Bond price(s)
*  @param [in]		curveCollections			Curve collection name(s)
*  @param [in]		forecastCurves				Forecast curve name(s)
*  @param [in]		continuouslyCompounding	True to calculate the z-spread(s) using continuous compounding
*  @param [out]		Returns the Z-Spread(s)
*/
std::vector<double> aqBondObjectZSpreads( const std::string& bondObjectName,
                                       const std::vector<std::string>& settlementDates,
                                       const std::vector<double>& bondPrices,
                                       const std::vector<std::string>& curveCollections,
                                       const std::vector<std::string>& forecastCurves,
                                       const bool& continuouslyCompounding )
{
    AQ_API_START

    // Marshall Inputs
    DateVector settlementDatesAsDateVector;
    swig::buildDateVector( settlementDatesAsDateVector, settlementDates );

    // Call validation method
    std::vector<double> results = validation::tryAqBondObjectZSpreads( bondObjectName, settlementDatesAsDateVector, bondPrices, curveCollections, forecastCurves, continuouslyCompounding );
    return results;

    AQ_API_END
}

/* @brief			swig interface for aqBondObjectZSpreadFromRates function. Bond Z-Spread against an external set of zero rates.
*  @param [in]		bondObjectName				Bond Object Name
*  @param [in]		settlementDate				Bond's settlement date (valuation date)
*  @param [in]		bondPrice					Bond price
*  @param [in]		zeroRates					The external curve's zero rates
*  @param [in]		continuouslyCompounding	True to calculate the z-spread using continuous compounding
*  @param [out]		Returns the Z-Spread
*/
double aqBondObjectZSpreadFromRates( const std::string& bondObjectName,
                                  const std::string& settlementDate,
                                  const double& bondPrice,
                                  const std::vector<double>& zeroRates,
                                  const bool& continuouslyCompounding )
{
    AQ_API_START

    // Marshall Inputs
    AQLDate settlementDate_( etrading::stringToDate( settlementDate ) );

    // Call validation method
    double result = validation::tryAqBondObjectZSpreadFromRates( bondObjectName, settlementDate_, bondPrice, zeroRates, continuouslyCompounding );
    return result;

    AQ_API_END
}
