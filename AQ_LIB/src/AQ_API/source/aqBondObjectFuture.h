// aqBondObjectFuture.h

/*
 * @brief			Swig interface for aqBondObject bond-future analytics: conversion factor, future price, gross/net basis,
 *					cheapest-to-deliver and repo rate implied from a future price
 */

#pragma once

#include <string>
#include <vector>

/* @brief			swig interface for aqBondObjectConversionFactor function. Bond conversionFactor against a futures contract.
*  @param [in]		bondObjectName			Bond Object Name
*  @param [in]		firstFutureSettleDate	The future's first settlement date
*  @param [in]		notionalBondCouponRate	The future's notional coupon rate
*  @param [out]		Returns the bond's conversion factor
*/
double aqBondObjectConversionFactor( const std::string& bondObjectName,
                                  const std::string& firstFutureSettleDate,
                                  const double& notionalBondCouponRate );

/* @brief			swig interface for aqBondObjectFuturePrice function. Bond's future price at future settle date, implied from a repo rate.
*  @param [in]		bondObjectName		Bond Object Name
*  @param [in]		settleDate			Bond current settle date
*  @param [in]		deliveryDate		Bond future settle date
*  @param [in]		bondPrice			Bond price at settle date, can be dirty or clean depending on the bond's isCleanPrice flag
*  @param [in]		repoRate			Bond repo rate
*  @param [in]		repoDayCount		Day count for the repo year fraction
*  @param [in]		conversionFactor	Bond's conversion factor
*  @param [out]		Returns the bond future price
*/
double aqBondObjectFuturePrice( const std::string& bondObjectName,
                             const std::string& settleDate,
                             const std::string& deliveryDate,
                             const double& bondPrice,
                             const double& repoRate,
                             const std::string& repoDayCount,
                             const double& conversionFactor );

/* @brief			swig interface for aqBondObjectGrossBasis function. Bond gross basis against a futures contract.
*  @param [in]		bondObjectName		Bond Object Name
*  @param [in]		settleDate			Bond current settle date
*  @param [in]		bondPrice			Bond price at settle date, can be dirty or clean depending on the bond's isCleanPrice flag
*  @param [in]		futurePrice			Bond future price at future settle date, always clean
*  @param [in]		conversionFactor	Conversion factor
*  @param [out]		Returns the bond's gross basis
*/
double aqBondObjectGrossBasis( const std::string& bondObjectName,
                            const std::string& settleDate,
                            const double& bondPrice,
                            const double& futurePrice,
                            const double& conversionFactor );

/* @brief			swig interface for aqBondObjectNetBasis function. Bond net basis against a futures contract.
*  @param [in]		bondObjectName		Bond Object Name
*  @param [in]		settleDate			Bond current settle date
*  @param [in]		deliveryDate		Bond future's delivery date
*  @param [in]		bondPrice			Bond price at settle date, can be dirty or clean depending on the bond's isCleanPrice flag
*  @param [in]		actualRepoRate		Bond's actual repo rate
*  @param [in]		repoDayCount		Repo rate day count
*  @param [in]		futurePrice			Bond future price at delivery date, always clean
*  @param [in]		conversionFactor	Bond's conversion factor against the future
*  @param [out]		Returns the bond's net basis
*/
double aqBondObjectNetBasis( const std::string& bondObjectName,
                          const std::string& settleDate,
                          const std::string& deliveryDate,
                          const double& bondPrice,
                          const double& actualRepoRate,
                          const std::string& repoDayCount,
                          const double& futurePrice,
                          const double& conversionFactor );

/* @brief			swig interface for aqBondObjectCheapestToDeliver function. Calculates the cheapest-to-deliver (CTD) bond via Implied Repo Rate.
*  @param [in]		futurePrice			Future price
*  @param [in]		settleDate			Bond's settle date
*  @param [in]		deliveryDate		Bond's future settle date/delivery date
*  @param [in]		repoDayCount		Day count for the repo year fraction
*  @param [in]		bondObjectNames		A list of bond object names
*  @param [in]		bondPrices			A list of bond prices at settle date, can be dirty or clean depending on the bond's isCleanPrice flag
*  @param [in]		conversionFactors	A list of conversion factors
*  @param [out]		Returns the cheapest-to-deliver (CTD) bond's name
*/
std::string aqBondObjectCheapestToDeliver( const double& futurePrice,
                                        const std::string& settleDate,
                                        const std::string& deliveryDate,
                                        const std::string& repoDayCount,
                                        const std::vector<std::string>& bondObjectNames,
                                        const std::vector<double>& bondPrices,
                                        const std::vector<double>& conversionFactors );

/* @brief			swig interface for aqBondObjectCheapestToDeliverByNetBasis function. Calculates the cheapest-to-deliver (CTD) bond via Net Basis.
*  @param [in]		futurePrice			Future price
*  @param [in]		settleDate			Bond's settle date
*  @param [in]		deliveryDate		Bond's future settle date/delivery date
*  @param [in]		repoDayCount		Day count for the repo year fraction
*  @param [in]		bondObjectNames		A list of bond object names
*  @param [in]		bondPrices			A list of bond prices at settle date, can be dirty or clean depending on the bond's isCleanPrice flag
*  @param [in]		conversionFactors	A list of conversion factors
*  @param [in]		actualRepoRates		Actual repo rates
*  @param [out]		Returns the cheapest-to-deliver (CTD) bond's name
*/
std::string aqBondObjectCheapestToDeliverByNetBasis( const double& futurePrice,
                                                  const std::string& settleDate,
                                                  const std::string& deliveryDate,
                                                  const std::string& repoDayCount,
                                                  const std::vector<std::string>& bondObjectNames,
                                                  const std::vector<double>& bondPrices,
                                                  const std::vector<double>& conversionFactors,
                                                  const std::vector<double>& actualRepoRates );

/* @brief			swig interface for aqBondObjectRepoRateFromFuture function. Bond's implied repo rate from its future price at settle date.
*  @param [in]		bondObjectName		Bond Object Name
*  @param [in]		settlementDate		Bond current settle date
*  @param [in]		deliveryDate		Bond future settle date
*  @param [in]		bondPrice			Bond price at settle date, can be dirty or clean depending on the bond's isCleanPrice flag
*  @param [in]		futurePrice			Bond's future price at forward settle date
*  @param [in]		conversionFactor	Bond's conversion factor
*  @param [in]		repoDayCount		Day count for the repo year fraction
*  @param [out]		Returns the bond's implied repo rate
*/
double aqBondObjectRepoRateFromFuture( const std::string& bondObjectName,
                                    const std::string& settlementDate,
                                    const std::string& deliveryDate,
                                    const double& bondPrice,
                                    const double& futurePrice,
                                    const double& conversionFactor,
                                    const std::string& repoDayCount );
