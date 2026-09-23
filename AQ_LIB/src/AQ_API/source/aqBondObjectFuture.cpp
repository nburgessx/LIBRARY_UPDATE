// aqBondObjectFuture.cpp

/*
 * @brief			Swig interface for aqBondObject bond-future analytics: conversion factor, future price, gross/net basis,
 *					cheapest-to-deliver and repo rate implied from a future price
 */

#include "aqBondObjectFuture.h"
#include "AQLCoreTemplateType.h"
#include "TypeUtilities.h"
#include "tryAqBondObject.h"
#include "APISetUp.h"               // AQ_API_START and AQ_API_END Macros


/* @brief			swig interface for aqBondObjectConversionFactor function. Bond conversionFactor against a futures contract.
*/
double aqBondObjectConversionFactor( const std::string& bondObjectName,
                                  const std::string& firstFutureSettleDate,
                                  const double& notionalBondCouponRate )
{
    AQ_API_START

    // Marshall Inputs
    AQLDate firstFutureSettleDate_( etrading::stringToDate( firstFutureSettleDate ) );

    // Call validation method
    double result = validation::tryAqBondObjectConversionFactor( bondObjectName, firstFutureSettleDate_, notionalBondCouponRate );
    return result;

    AQ_API_END
}

/* @brief			swig interface for aqBondObjectFuturePrice function. Bond's future price at future settle date, implied from a repo rate.
*/
double aqBondObjectFuturePrice( const std::string& bondObjectName,
                             const std::string& settleDate,
                             const std::string& deliveryDate,
                             const double& bondPrice,
                             const double& repoRate,
                             const std::string& repoDayCount,
                             const double& conversionFactor )
{
    AQ_API_START

    // Marshall Inputs
    AQLDate settleDate_( etrading::stringToDate( settleDate ) );
    AQLDate deliveryDate_( etrading::stringToDate( deliveryDate ) );

    // Call validation method
    double result = validation::tryAqBondObjectFuturePrice( bondObjectName, settleDate_, deliveryDate_, bondPrice, repoRate, repoDayCount, conversionFactor );
    return result;

    AQ_API_END
}

/* @brief			swig interface for aqBondObjectGrossBasis function. Bond gross basis against a futures contract.
*/
double aqBondObjectGrossBasis( const std::string& bondObjectName,
                            const std::string& settleDate,
                            const double& bondPrice,
                            const double& futurePrice,
                            const double& conversionFactor )
{
    AQ_API_START

    // Marshall Inputs
    AQLDate settleDate_( etrading::stringToDate( settleDate ) );

    // Call validation method
    double result = validation::tryAqBondObjectGrossBasis( bondObjectName, settleDate_, bondPrice, futurePrice, conversionFactor );
    return result;

    AQ_API_END
}

/* @brief			swig interface for aqBondObjectNetBasis function. Bond net basis against a futures contract.
*/
double aqBondObjectNetBasis( const std::string& bondObjectName,
                          const std::string& settleDate,
                          const std::string& deliveryDate,
                          const double& bondPrice,
                          const double& actualRepoRate,
                          const std::string& repoDayCount,
                          const double& futurePrice,
                          const double& conversionFactor )
{
    AQ_API_START

    // Marshall Inputs
    AQLDate settleDate_( etrading::stringToDate( settleDate ) );
    AQLDate deliveryDate_( etrading::stringToDate( deliveryDate ) );

    // Call validation method
    double result = validation::tryAqBondObjectNetBasis( bondObjectName, settleDate_, deliveryDate_, bondPrice, actualRepoRate, repoDayCount, futurePrice, conversionFactor );
    return result;

    AQ_API_END
}

/* @brief			swig interface for aqBondObjectCheapestToDeliver function. Calculates the cheapest-to-deliver (CTD) bond via Implied Repo Rate.
*/
std::string aqBondObjectCheapestToDeliver( const double& futurePrice,
                                        const std::string& settleDate,
                                        const std::string& deliveryDate,
                                        const std::string& repoDayCount,
                                        const std::vector<std::string>& bondObjectNames,
                                        const std::vector<double>& bondPrices,
                                        const std::vector<double>& conversionFactors )
{
    AQ_API_START

    // Marshall Inputs
    AQLDate settleDate_( etrading::stringToDate( settleDate ) );
    AQLDate deliveryDate_( etrading::stringToDate( deliveryDate ) );

    // Call validation method
    std::string result = validation::tryAqBondObjectCheapestToDeliver( futurePrice, settleDate_, deliveryDate_, repoDayCount, bondObjectNames, bondPrices, conversionFactors );
    return result;

    AQ_API_END
}

/* @brief			swig interface for aqBondObjectCheapestToDeliverByNetBasis function. Calculates the cheapest-to-deliver (CTD) bond via Net Basis.
*/
std::string aqBondObjectCheapestToDeliverByNetBasis( const double& futurePrice,
                                                  const std::string& settleDate,
                                                  const std::string& deliveryDate,
                                                  const std::string& repoDayCount,
                                                  const std::vector<std::string>& bondObjectNames,
                                                  const std::vector<double>& bondPrices,
                                                  const std::vector<double>& conversionFactors,
                                                  const std::vector<double>& actualRepoRates )
{
    AQ_API_START

    // Marshall Inputs
    AQLDate settleDate_( etrading::stringToDate( settleDate ) );
    AQLDate deliveryDate_( etrading::stringToDate( deliveryDate ) );

    // Call validation method
    std::string result = validation::tryAqBondObjectCheapestToDeliverByNetBasis( futurePrice, settleDate_, deliveryDate_, repoDayCount, bondObjectNames, bondPrices, conversionFactors, actualRepoRates );
    return result;

    AQ_API_END
}

/* @brief			swig interface for aqBondObjectRepoRateFromFuture function. Bond's implied repo rate from its future price at settle date.
*/
double aqBondObjectRepoRateFromFuture( const std::string& bondObjectName,
                                    const std::string& settlementDate,
                                    const std::string& deliveryDate,
                                    const double& bondPrice,
                                    const double& futurePrice,
                                    const double& conversionFactor,
                                    const std::string& repoDayCount )
{
    AQ_API_START

    // Marshall Inputs
    AQLDate settlementDate_( etrading::stringToDate( settlementDate ) );
    AQLDate deliveryDate_( etrading::stringToDate( deliveryDate ) );

    // Call validation method
    double result = validation::tryAqBondObjectRepoRateFromFuture( bondObjectName, settlementDate_, deliveryDate_, bondPrice, futurePrice, conversionFactor, repoDayCount );
    return result;

    AQ_API_END
}
