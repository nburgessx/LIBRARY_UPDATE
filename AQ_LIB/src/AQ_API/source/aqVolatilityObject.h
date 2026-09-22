// aqVolatilityObject.h

/*
 * @brief			Swig interface for aqVolatilityObject... functions
 */

#pragma once

#include "SwigTypes.h"

#include <string>
#include <vector>

/* @brief			swig interface for aqVolatilityObjectCreate
*  @param [in]		objectName			Volatility-surface object name
*  @param [in]		volLVB				Volatility-surface definition as a key/value matrix
*  @param [in]		validateKeys		Optional. Default TRUE. Check the LVB keys
*  @return			Volatility-surface object handle
*/
std::string aqVolatilityObjectCreate( const std::string& objectName,
                                       const SWIG_STRINGMATRIX& volLVB,
                                       const bool validateKeys = true );

/* @brief			swig interface for aqVolatilityObjectSabrMarketDataCreate
*  @param [in]		objectName			Name for the SABR market-data object
*  @param [in]		key1				Name of the first data block
*  @param [in]		value1				First data block, as a key/value matrix
*  @param [in]		key2				Optional. Name of the second data block
*  @param [in]		value2				Optional. Second data block, as a key/value matrix
*  @return			SABR market-data object handle
*/
std::string aqVolatilityObjectSabrMarketDataCreate( const std::string& objectName,
                                                     const std::string& key1,
                                                     const SWIG_STRINGMATRIX& value1,
                                                     const std::string& key2 = "",
                                                     const SWIG_STRINGMATRIX& value2 = SWIG_STRINGMATRIX() );

/* @brief			swig interface for aqVolatilityObjectSabrModelCalibrate
*  @param [in]		objectName			Name for the SABR model object, calibrated to market data
*  @param [in]		key1				Name of the first data block
*  @param [in]		value1				First data block, as a key/value matrix
*  @param [in]		key2				Optional. Name of the second data block
*  @param [in]		value2				Optional. Second data block, as a key/value matrix
*  @return			SABR model object handle
*/
std::string aqVolatilityObjectSabrModelCalibrate( const std::string& objectName,
                                                   const std::string& key1,
                                                   const SWIG_STRINGMATRIX& value1,
                                                   const std::string& key2 = "",
                                                   const SWIG_STRINGMATRIX& value2 = SWIG_STRINGMATRIX() );

/* @brief			swig interface for aqVolatilityObjectSabrVolatility
*  @param [in]		volSurfaceName		A volatility-surface handle
*  @param [in]		expiry				Option expiry, e.g. 3M, 1Y
*  @param [in]		tenor				Underlying tenor, e.g. 5Y, 10Y
*  @param [in]		strike				Strike
*  @param [in]		forward				Forward rate
*  @return			SABR-implied volatility at the expiry/tenor/strike/forward point
*/
double aqVolatilityObjectSabrVolatility( const std::string& volSurfaceName,
                                          const std::string& expiry,
                                          const std::string& tenor,
                                          const double strike,
                                          const double forward );

/* @brief			swig interface for aqVolatilityObjectSabrParameter
*  @param [in]		volSurfaceName		A volatility-surface handle
*  @param [in]		expiry				Option expiry, e.g. 3M, 1Y
*  @param [in]		tenor				Underlying tenor, e.g. 5Y, 10Y
*  @param [in]		paramName			Which parameter, e.g. ALPHA, BETA, RHO, NU
*  @return			One SABR parameter at the expiry/tenor point
*/
double aqVolatilityObjectSabrParameter( const std::string& volSurfaceName,
                                         const std::string& expiry,
                                         const std::string& tenor,
                                         const std::string& paramName );

/* @brief			swig interface for aqVolatilityObjectSabrMarketDataDisplay
*  @param [in]		marketDataObjectName	A SABR market-data handle
*  @param [in]		marketDataKey			The data block to display
*  @return			Stored SABR market data, as a string matrix
*/
SWIG_STRINGMATRIX aqVolatilityObjectSabrMarketDataDisplay( const std::string& marketDataObjectName,
                                                            const std::string& marketDataKey );

/* @brief			swig interface for aqVolatilityObjectSabrModelDisplay
*  @param [in]		modelObjectName		A SABR model handle
*  @param [in]		modelKey			The model block to display
*  @return			Stored SABR model, as a string matrix
*/
SWIG_STRINGMATRIX aqVolatilityObjectSabrModelDisplay( const std::string& modelObjectName,
                                                       const std::string& modelKey );
