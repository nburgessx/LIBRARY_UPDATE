// aqCapFloorObject.h

/*
 * @brief			Swig interface for aqCapFloorObject... functions
 */

#pragma once

#include "SwigTypes.h"

#include <string>
#include <vector>

/* @brief			swig interface for aqCapFloorObjectCreate
*  @param [in]		objectName			CapFloor object name
*  @param [in]		tradeLVB			CapFloor definition as a key/value matrix
*  @param [in]		validateKeys		Optional. Default TRUE. Check the LVB keys
*  @return			CapFloor object handle
*/
std::string aqCapFloorObjectCreate( const std::string& objectName,
                                     const SWIG_STRINGMATRIX& tradeLVB,
                                     const bool validateKeys = true );

/* @brief			swig interface for aqCapFloorObjectDisplay, which displays the INPUT parameters of the cached cap/floor
*  @param [in]		objectName			CapFloor object name
*  @return			Display of the cap/floor input parameters
*/
SWIG_STRINGMATRIX aqCapFloorObjectDisplay( const std::string& objectName );

/* @brief			swig interface for aqCapFloorObjectDisplayCashflows
*  @param [in]		objectName			CapFloor object name
*  @param [in]		volObjectName		Volatility object name
*  @param [in]		curveCollection		Discount / forward curve collection
*  @param [in]		fixingTableName		Optional. Fixing table object name for past fixings
*  @param [in]		showColumnHeaders	Optional. Default TRUE. Include a header row
*  @param [in]		columnList			Optional. Column header names to show specified columns. Default to empty list showing all columns.
*  @return			CapFloor cashflow display
*/
SWIG_STRINGMATRIX aqCapFloorObjectDisplayCashflows( const std::string& objectName,
                                                     const std::string& volObjectName,
                                                     const std::string& curveCollection,
                                                     const std::string& fixingTableName = "",
                                                     const bool showColumnHeaders = true,
                                                     const std::vector<std::string>& columnList = std::vector<std::string>() );

/* @brief			swig interface for aqCapFloorObjectPV
*  @param [in]		objectName			CapFloor object name
*  @param [in]		volObjectName		Volatility object name
*  @param [in]		curveCollection		Discount / forward curve collection
*  @param [in]		fixingTableName		Optional. Fixing table object name for past fixings
*  @return			The cap/floor PV
*/
double aqCapFloorObjectPV( const std::string& objectName,
                           const std::string& volObjectName,
                           const std::string& curveCollection,
                           const std::string& fixingTableName = "" );

/* @brief			swig interface for aqCapFloorObjectGreeksAnalytical
*  @param [in]		objectName			CapFloor object name
*  @param [in]		volObjectName		Volatility object name
*  @param [in]		curveCollection		Discount / forward curve collection
*  @param [in]		fixingTableName		Optional. Fixing table object name for past fixings
*  @param [in]		showColumnHeaders	Optional. Default TRUE. Include a header row
*  @return			CapFloor Greeks' display, calculated analytically
*/
SWIG_STRINGMATRIX aqCapFloorObjectGreeksAnalytical( const std::string& objectName,
                                                     const std::string& volObjectName,
                                                     const std::string& curveCollection,
                                                     const std::string& fixingTableName = "",
                                                     const bool showColumnHeaders = true );

/* @brief			swig interface for aqCapFloorObjectGreeks
*  @param [in]		objectName			CapFloor object name
*  @param [in]		volObjectName		Volatility object name
*  @param [in]		curveCollection		Discount / forward curve collection
*  @param [in]		deltaBump			Delta bump size
*  @param [in]		gammaBump			Gamma bump size
*  @param [in]		vegaBump			Vega bump size
*  @param [in]		thetaBump			Theta bump size
*  @param [in]		fixingTableName		Optional. Fixing table object name for past fixings
*  @param [in]		showColumnHeaders	Optional. Default TRUE. Include a header row
*  @return			CapFloor Greeks' display, calculated by bump-and-revalue
*/
SWIG_STRINGMATRIX aqCapFloorObjectGreeks( const std::string& objectName,
                                          const std::string& volObjectName,
                                          const std::string& curveCollection,
                                          const double deltaBump,
                                          const double gammaBump,
                                          const double vegaBump,
                                          const double thetaBump,
                                          const std::string& fixingTableName = "",
                                          const bool showColumnHeaders = true );

/* @brief			swig interface for aqCapFloorObjectDisplayCashflowsFromRates
*  @param [in]		objectName			CapFloor object name
*  @param [in]		volObjectName		Volatility object name
*  @param [in]		rateData			External discount factors and libor rates
*  @param [in]		showColumnHeaders	Optional. Default TRUE. Include a header row
*  @param [in]		columnList			Optional. Column header names to show specified columns. Default to empty list showing all columns.
*  @return			CapFloor cashflow display
*/
SWIG_STRINGMATRIX aqCapFloorObjectDisplayCashflowsFromRates( const std::string& objectName,
                                                              const std::string& volObjectName,
                                                              const std::vector<std::vector<double> >& rateData,
                                                              const bool showColumnHeaders = true,
                                                              const std::vector<std::string>& columnList = std::vector<std::string>() );

/* @brief			swig interface for aqCapFloorObjectGreeksAnalyticalFromRates
*  @param [in]		objectName			CapFloor object name
*  @param [in]		volObjectName		Volatility object name
*  @param [in]		rateData			External discount factors and libor rates
*  @param [in]		showColumnHeaders	Optional. Default TRUE. Include a header row
*  @return			CapFloor Greeks' display, calculated analytically
*/
SWIG_STRINGMATRIX aqCapFloorObjectGreeksAnalyticalFromRates( const std::string& objectName,
                                                              const std::string& volObjectName,
                                                              const std::vector<std::vector<double> >& rateData,
                                                              const bool showColumnHeaders = true );

/* @brief			swig interface for aqCapFloorObjectGreeksFromRates
*  @param [in]		objectName			CapFloor object name
*  @param [in]		volObjectName		Volatility object name
*  @param [in]		rateData			External discount factors and libor rates
*  @param [in]		deltaBump			Delta bump size
*  @param [in]		gammaBump			Gamma bump size
*  @param [in]		vegaBump			Vega bump size
*  @param [in]		thetaBump			Theta bump size
*  @param [in]		showColumnHeaders	Optional. Default TRUE. Include a header row
*  @return			CapFloor Greeks' display, calculated by bump-and-revalue
*/
SWIG_STRINGMATRIX aqCapFloorObjectGreeksFromRates( const std::string& objectName,
                                                    const std::string& volObjectName,
                                                    const std::vector<std::vector<double> >& rateData,
                                                    const double deltaBump,
                                                    const double gammaBump,
                                                    const double vegaBump,
                                                    const double thetaBump,
                                                    const bool showColumnHeaders = true );

/* @brief			swig interface for aqCapFloorObjectPVFromRates
*  @param [in]		objectName			CapFloor object name
*  @param [in]		volObjectName		Volatility object name
*  @param [in]		rateData			External discount factors and libor rates
*  @return			The cap/floor PV
*/
double aqCapFloorObjectPVFromRates( const std::string& objectName,
                                     const std::string& volObjectName,
                                     const std::vector<std::vector<double> >& rateData );
