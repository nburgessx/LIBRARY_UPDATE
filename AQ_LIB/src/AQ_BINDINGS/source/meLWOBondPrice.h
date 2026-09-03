// meLWOBondPrice.h

/* 
 * @brief			Swig interface for meLWOBondPrice... functions
 * @Created:		25th June 2018
 * @Author:			Nicholas Burgess
 * @Department:		Quant Research and Analytics
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#pragma once
#include <string>
#include <vector>

/* @brief			swig interface for meLWOBondPrice function
*  @param [in]		bondObjectName		    Bond Object Name
*  @param [in]		settlementDates	        Bond Settlement Date(s)
*  @param [in]		yields     	            Bond Yield(s)
*  @param [in]		yieldCalculationType	Yield Calculation Type
*  @param [out]		Returns a vector of bond prices for each settlement date, quoted clean/dirty based on market and Bond definition
*/
const std::vector<double> meLWOBondPrice( const std::string& bondObjectName,
                                          const std::vector<std::string>& settlementDates,
                                          const std::vector<double>& yields,
                                          const std::string& yieldCalculationType );

/* @brief			swig interface for meLWOBondCleanPrice function
*  @param [in]		bondObjectName		    Bond Object Name
*  @param [in]		settlementDates	        Bond Settlement Date(s)
*  @param [in]		yields     	            Bond Yield(s)
*  @param [in]		yieldCalculationType	Yield Calculation Type
*  @param [out]		Returns a vector of clean bond prices for each settlement date
*/
const std::vector<double> meLWOBondCleanPrice( const std::string& bondObjectName,
                                               const std::vector<std::string>& settlementDates,
                                               const std::vector<double>& yields,
                                               const std::string& yieldCalculationType );

/* @brief			swig interface for meLWOBondDirtyPrice function
*  @param [in]		bondObjectName		    Bond Object Name
*  @param [in]		settlementDates	        Bond Settlement Date(s)
*  @param [in]		yields     	            Bond Yield(s)
*  @param [in]		yieldCalculationType	Yield Calculation Type
*  @param [out]		Returns a vector of clean bond prices for each settlement date
*/
const std::vector<double> meLWOBondDirtyPrice( const std::string& bondObjectName,
                                               const std::vector<std::string>& settlementDates,
                                               const std::vector<double>& yields,
                                               const std::string& yieldCalculationType );

/* @brief			swig interface for meLWOBondYield function
*  @param [in]		bondObjectName		    Bond Object Name
*  @param [in]		settlementDates	        Bond Settlement Date(s)
*  @param [in]		quotedPrices            Bond Quoted Price(s), must input consistently as clean or dirty as quoted in the market and specified in the Bond Definition
*  @param [in]		yieldCalculationType	Yield Calculation Type
*  @param [in]		optimizePerformance	    Optimize for performance
*  @param [out]		Returns a vector of clean bond prices for each settlement date
*/
const std::vector<double> meLWOBondYield( const std::string& bondObjectName,
                                          const std::vector<std::string>& settlementDates,
                                          const std::vector<double>& quotedPrices,
                                          const std::string& yieldCalculationType,
                                          const bool& optimizePerformance );

/* @brief			Validation interface for the meLWOBondForwardPrice method. Calculate Bond's forward price at forward settle date, from repo rate.
*  @param [in]		bondObjectName		Bond object name
*  @param [in]		settleDate			Bond current settle date
*  @param [in]		forwardSettleDate   Bond forward settle date
*  @param [in]		price				Bond price at settle date, can be dirty or clean depends on Bond's isCleanPrice flag
*  @param [in]		repoRate			Bond repo rate
*  @param [in]		repoDayCount		Day count for repo year fraction
*  @return			Bond forward price
*/
double meLWOBondForwardPrice( const std::string& bondObjectName,
                              const std::string& settleDate,
                              const std::string& forwardSettleDate,
                              const double& price,
                              const double& repoRate,
                              const std::string& repoDayCount );
