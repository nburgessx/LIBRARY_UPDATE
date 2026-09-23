// aqSwapObjectPrice.h

/* 
 * @brief			Swig interface for aqObjSwapsPrice... functions
 */

#pragma once

#include "SwigTypes.h"

#include <string>
#include <vector>

/* @brief			Function to calculate the Swap PV
*  @param [in]		swapName			Swap object name
*  @param [in]		curveCollection		Valuation settings
*  @param [in]		legName				Leg name
*  @param [in]		fixingTableNames	Fixing table object names
*  @return			Swap PV
*/
double aqSwapObjectPV( const std::string& swapName,
                    const SWIG_STRINGMATRIX & valuationSettings,
                    const std::string& legName,
                    const SWIG_STRINGMATRIX & fixingTableNames );

/* @brief			Function to calculate the Swap PV01
*  @param [in]		swapName			Swap object name
*  @param [in]		curveCollection		Valuation settings
*  @param [in]		fixingTableNames	Fixing table object names
*  @return			Swap PV01
*/
double aqSwapObjectPV01( const std::string& swapName,
                      const SWIG_STRINGMATRIX & valuationSettings,
                      const SWIG_STRINGMATRIX & fixingTableNames );

/* @brief			Function to calculate the Swap Par Rate
*  @param [in]		swapName		    Swap object name
*  @param [in]		valuationSettings   Valuation settings map
*  @param [in]		fixingTableNames    Fixing table object names
*  @return			Swap par rate
*/
double aqSwapObjectParRate( const std::string& swapName,
                         const SWIG_STRINGMATRIX & valuationSettings,
                         const SWIG_STRINGMATRIX & fixingTableNames );

/* @brief			Function to calculate the Swap Par Rate
*  @param [in]		swapName		    Swap object name
*  @param [in]		valuationSettings   Valuation settings map
*  @param [in]		fixingTableNames    Fixing table object names
*  @param [in]		legName             Leg Name
*  @return			Swap par rate
*/
double aqSwapObjectParRate( const std::string& swapName,
                         const SWIG_STRINGMATRIX & valuationSettings,
                         const SWIG_STRINGMATRIX & fixingTableNames,
                         const std::string& legName );

/* @brief			Function to calculate the Swap Annuity
*  @param [in]		swapName		Swap object name
*  @param [in]		curveCollection Valuation settings
*  @param [in]		legName		    Leg name
*  @return			Swap PV
*/
double aqSwapObjectAnnuity( const std::string& swapName,
                         const SWIG_STRINGMATRIX & valuationSettings,
                         const std::string& legName ) ;

/* @brief			Function to calculate the Par Spread - Excluding any existing spread
*  @param [in]		swapName		Swap name
*  @param [in]		curveCollection Curve collections
*  @param [in]		fixingTableName	Fixing table object names
*  @param [in]		spreadLegName   Leg name the spread will be applied to
*  @return			The spread that make the swap PV zero
*/
double aqSwapObjectParSpread( const std::string& swapName,
                           const SWIG_STRINGMATRIX & valuationSettings,
                           const SWIG_STRINGMATRIX & fixingTableNames,
                           const std::string& spreadLegName ) ;


/* @brief			Function to calculate the Swap Spread - Including any existing spread
*  @param [in]		swapName		Swap name
*  @param [in]		curveCollection Curve collections
*  @param [in]		fixingTableName	Fixing table object names
*  @param [in]		spreadLegName   Leg name the spread will be applied to
*  @return			The spread that make the swap PV zero
*/
double aqSwapObjectSpread( const std::string& swapName,
                        const SWIG_STRINGMATRIX & valuationSettings,
                        const SWIG_STRINGMATRIX & fixingTableNames,
                        const std::string& spreadLegName ) ;

/* @brief			Function to calculate the display swap cashflows
*  @param [in]		swapName			Swap object name
*  @param [in]		curveCollection		Valuation settings
*  @param [in]		legName				Leg name
*  @param [in]		fixingTableNames	Fixing table object names
*  @param [in]		showColumnHeaders	Show Column Headers, true/false
*  @return			Swap Cashflow Table
*/
SWIG_STRINGMATRIX aqSwapObjectDisplayCashflows( const std::string& swapName,
								             const SWIG_STRINGMATRIX & valuationSettings,
								             const std::string& legName,
								             const SWIG_STRINGMATRIX & fixingTableNames,
								             const bool showColumnHeaders );

/* @brief			Function to calculate a swap leg's accrued interest
*  @param [in]		swapName			Swap object name
*  @param [in]		valuationSettings	Valuation settings map
*  @param [in]		legName				Leg name
*  @param [in]		fixingTableName		Fixing table object name
*  @return			The leg's accrued interest
*/
double aqSwapObjectAccruedInterest( const std::string& swapName,
                                 const SWIG_STRINGMATRIX & valuationSettings,
                                 const std::string& legName,
                                 const SWIG_STRINGMATRIX & fixingTableName );

/* @brief			Function to calculate the PV of several cached swaps in one call
*  @param [in]		swapNames				Column of cached swap handles
*  @param [in]		valuationSettingsLVBs	Valuation settings table: header row of keys, one row per swap
*  @param [in]		legNames				Optional. Leg names aligned with swapNames; blank prices the whole swap
*  @param [in]		fixingTableNamesLVBs	Fixing table overrides table: header row of keys, one row per swap
*  @param [in]		optimize				Optimize performance using OMP threading
*  @return			A vector of PVs, aligned with swapNames
*/
std::vector<double> aqSwapObjectPVs( const std::vector<std::string>& swapNames,
                                  const SWIG_STRINGMATRIX & valuationSettingsLVBs,
                                  const std::vector<std::string>& legNames,
                                  const SWIG_STRINGMATRIX & fixingTableNamesLVBs,
                                  const bool optimize );