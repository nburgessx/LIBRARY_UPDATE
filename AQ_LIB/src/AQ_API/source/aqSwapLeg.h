// aqSwapLeg.h

/*
 * @brief			Swig interface for aqSwapLeg... and aqSwapObjectLeg... functions
 */

#pragma once

#include "SwigTypes.h"

#include <string>
#include <vector>

/* -------------------------------------------------------------------------
 *  Stateless swap leg (a single-leg LVB in, value out)
 * ---------------------------------------------------------------------- */

/* @brief			swig interface for the aqSwapLegLVBKeys function - the expected keys for a swap leg label/value block
*  @param [in]		legName		Leg type name, e.g. FIXED, FLOAT
*  @return			expected keys
*/
std::vector<std::string> aqSwapLegLVBKeys( const std::string& legName );

/* @brief			swig interface for the aqSwapLegDisplay function
*  @param [in]		legLVB				The leg definition as a label value block
*  @param [in]		validateKeys		True to validate the all keys provided are valid. Default to True
*  @param [in]		showColumnHeaders	Optional. Default TRUE. Include a header row
*  @param [in]		columnList			Column header names to show specified columns. Default to empty list showing all columns
*  @return			Matrix of leg schedule and cashflows
*/
SWIG_STRINGMATRIX aqSwapLegDisplay( const SWIG_STRINGMATRIX & legLVB,
                                 bool validateKeys,
                                 bool showColumnHeaders,
                                 const std::vector<std::string>& columnList );

/* @brief			swig interface for the aqSwapLegPV function
*  @param [in]		legLVB			A label value block defining the swap leg
*  @param [in]		validateKeys	True to validate the all keys provided are valid. Default to True
*  @return			Swap Leg PV
*/
double aqSwapLegPV( const SWIG_STRINGMATRIX & legLVB, bool validateKeys );

/* @brief			swig interface for the aqSwapLegAnnuity function
*  @param [in]		legLVB			A label value block defining the swap leg
*  @param [in]		validateKeys	True to validate the all keys provided are valid. Default to True
*  @return			Swap Leg Annuity
*/
double aqSwapLegAnnuity( const SWIG_STRINGMATRIX & legLVB, bool validateKeys );

/* -------------------------------------------------------------------------
 *  Swap leg object lifecycle
 * ---------------------------------------------------------------------- */

/* @brief			swig interface for the aqSwapObjectLegCreate function
*  @param [in]		legObjectName	Leg object name
*  @param [in]		legLVB			Leg label value block
*  @param [in]		validateKeys	True to validate the all keys provided are valid. Default to True
*  @return			The leg object handle name
*/
std::string aqSwapObjectLegCreate( const std::string& legObjectName,
                                const SWIG_STRINGMATRIX & legLVB,
                                bool validateKeys );

/* @brief			swig interface for the aqSwapObjectLegCreateFromSchedule function
*  @param [in]		legObjectName	Leg object name
*  @param [in]		scheduleName	A cached schedule handle
*  @param [in]		legLVB			Leg label value block
*  @param [in]		validateKeys	True to validate the all keys provided are valid. Default to True
*  @return			The leg object handle name
*/
std::string aqSwapObjectLegCreateFromSchedule( const std::string& legObjectName,
                                            const std::string& scheduleName,
                                            const SWIG_STRINGMATRIX & legLVB,
                                            bool validateKeys );

/* @brief			swig interface for the aqSwapObjectLegPV function
*  @param [in]		legObjectName			Leg object name
*  @param [in]		valuationSettingsLVB	A LVB containing ModelName, CurveCollection, ValuationDate, etc.
*  @param [in]		fixingTableName			Optional. A cached fixing-table handle
*  @return			Leg PV
*/
double aqSwapObjectLegPV( const std::string& legObjectName,
                       const SWIG_STRINGMATRIX & valuationSettingsLVB,
                       const std::string& fixingTableName );

/* @brief			swig interface for the aqSwapObjectLegDisplay function, displays the INPUT parameters of the cached swap leg
*  @param [in]		legObjectName	Leg object name
*  @return			Leg display of the input parameters
*/
SWIG_STRINGMATRIX aqSwapObjectLegDisplay( const std::string& legObjectName );

/* @brief			swig interface for the aqSwapObjectLegDisplayCashflows function
*  @param [in]		legObjectName			Leg object name
*  @param [in]		valuationSettingsLVB	A LVB containing ModelName, CurveCollection, ValuationDate, etc.
*  @param [in]		fixingTableName			Optional. A cached fixing-table handle
*  @param [in]		showColumnHeaders		Optional. Default TRUE
*  @param [in]		columnList				Column header names to show specified columns. Default to empty list showing all columns
*  @return			Leg display of leg output
*/
SWIG_STRINGMATRIX aqSwapObjectLegDisplayCashflows( const std::string& legObjectName,
                                                const SWIG_STRINGMATRIX & valuationSettingsLVB,
                                                const std::string& fixingTableName,
                                                bool showColumnHeaders,
                                                const std::vector<std::string>& columnList );
