// aqSwaptionObject.h

/*
 * @brief			Swig interface for aqSwaptionObject... functions
 */

#pragma once

#include "SwigTypes.h"

#include <string>
#include <vector>

/* @brief			swig interface for aqSwaptionObjectCreate
*  @param [in]		objectName			Swaption object name
*  @param [in]		tradeLVB			Swaption definition as a key/value matrix
*  @param [in]		validateKeys		Optional. Default TRUE. Check the LVB keys
*  @return			Swaption object handle
*/
std::string aqSwaptionObjectCreate( const std::string& objectName,
                                     const SWIG_STRINGMATRIX& tradeLVB,
                                     const bool validateKeys = true );

/* @brief			swig interface for aqSwaptionObjectDisplay, which displays the INPUT parameters of the cached swaption
*  @param [in]		objectName			Swaption object name
*  @return			Display of the swaption input parameters
*/
SWIG_STRINGMATRIX aqSwaptionObjectDisplay( const std::string& objectName );

/* @brief			swig interface for aqSwaptionObjectPV
*  @param [in]		objectName			Swaption object name
*  @param [in]		valuationSettingsLVB	Valuation settings as a key/value matrix, e.g. curveCollection, volatilityModel
*  @return			The swaption PV
*/
double aqSwaptionObjectPV( const std::string& objectName,
                           const SWIG_STRINGMATRIX& valuationSettingsLVB );

/* @brief			swig interface for aqSwaptionObjectImpliedVol
*  @param [in]		objectName			Swaption object name
*  @param [in]		price				Swaption price
*  @param [in]		valuationSettingsLVB	Valuation settings as a key/value matrix
*  @return			The swaption implied vol
*/
double aqSwaptionObjectImpliedVol( const std::string& objectName,
                                    const double price,
                                    const SWIG_STRINGMATRIX& valuationSettingsLVB );

/* @brief			swig interface for aqSwaptionObjectDelta
*  @param [in]		objectName			Swaption object name
*  @param [in]		valuationSettingsLVB	Valuation settings as a key/value matrix
*  @return			The swaption Delta
*/
double aqSwaptionObjectDelta( const std::string& objectName,
                               const SWIG_STRINGMATRIX& valuationSettingsLVB );

/* @brief			swig interface for aqSwaptionObjectGamma
*  @param [in]		objectName			Swaption object name
*  @param [in]		valuationSettingsLVB	Valuation settings as a key/value matrix
*  @return			The swaption Gamma
*/
double aqSwaptionObjectGamma( const std::string& objectName,
                               const SWIG_STRINGMATRIX& valuationSettingsLVB );

/* @brief			swig interface for aqSwaptionObjectVega
*  @param [in]		objectName			Swaption object name
*  @param [in]		valuationSettingsLVB	Valuation settings as a key/value matrix
*  @return			The swaption Vega
*/
double aqSwaptionObjectVega( const std::string& objectName,
                              const SWIG_STRINGMATRIX& valuationSettingsLVB );

/* @brief			swig interface for aqSwaptionObjectTheta
*  @param [in]		objectName			Swaption object name
*  @param [in]		valuationSettingsLVB	Valuation settings as a key/value matrix
*  @return			The swaption Theta
*/
double aqSwaptionObjectTheta( const std::string& objectName,
                               const SWIG_STRINGMATRIX& valuationSettingsLVB );
