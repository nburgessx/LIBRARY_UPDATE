// aqVolatilitySABR.h

/*
 * @brief			Swig interface for aqVolatilitySABR... functions
 *
 * The legacy procedural, ID-string-driven SABR calibration workflow (as
 * opposed to the object/handle-based aqVolatilityObjectSabr* functions in
 * aqVolatilityObject.h). Each ID parameter (GridID, ConventionID, ...) is a
 * plain string key into the shared AQLDataInstance object pool, not an
 * AQObj handle - no decoration or instance counter is involved. Mirrors the
 * marshalling already used for these functions in AQ_XLL's xllVolatility.cpp.
 */

#pragma once

#include "SwigTypes.h"

#include <string>
#include <vector>

/* @brief			swig interface for aqVolatilitySABRCalibrate
*  @param [in]		approxMethod		SABR approximation method, e.g. HAGAN
*  @param [in]		calibFlg			Per-parameter calibration flags (TRUE/FALSE)
*  @param [in]		calibMethod			Calibration method
*  @param [in]		curveSetID			ID of the curve set
*  @param [in]		alphaID				ID for the alpha parameter matrix
*  @param [in]		betaID				ID for the beta parameter matrix
*  @param [in]		nuID				ID for the nu parameter matrix
*  @param [in]		rhoID				ID for the rho parameter matrix
*  @param [in]		convID				ID of the convention data. Do not supply together with forwardID
*  @param [in]		capConvID			ID of the cap convention data
*  @param [in]		swapVolID			IDs of the swaption vol matrices to calibrate against
*  @param [in]		target				Calibration target
*  @param [in]		weight				Calibration weights
*  @param [in]		sgn					Calibration signs (1 or -1) per point
*  @param [in]		forwardID			ID for the forward rate matrix. Do not supply together with convID
*  @param [in]		forwardShiftValue	Shift applied to the forward rate
*  @param [in]		numeraireID			ID for the numeraire/annuity matrix
*  @param [in]		curveMat			Curve matrix, required when forwardID is not given
*  @param [in]		volType				Optional. Volatility type, default lognormal
*  @return			Confirmation message
*/
std::string aqVolatilitySABRCalibrate( const std::string& approxMethod,
                                        const std::vector<std::string>& calibFlg,
                                        const std::string& calibMethod,
                                        const std::string& curveSetID,
                                        const std::string& alphaID,
                                        const std::string& betaID,
                                        const std::string& nuID,
                                        const std::string& rhoID,
                                        const std::string& convID,
                                        const std::string& capConvID,
                                        const std::vector<std::string>& swapVolID,
                                        const std::string& target,
                                        const std::vector<double>& weight,
                                        const std::vector<int>& sgn,
                                        const std::string& forwardID,
                                        const double forwardShiftValue,
                                        const std::string& numeraireID,
                                        const SWIG_STRINGMATRIX& curveMat,
                                        const std::string& volType = "" );

/* @brief			swig interface for aqVolatilitySABRGetPrem
*  @param [in]		expPoint		Expiry tenor
*  @param [in]		tenorPoint		Underlying swap tenor
*  @param [in]		strike			Strike
*  @param [in]		sign			1 for Call, -1 for Put
*  @param [in]		forwardID		ID for the forward rate matrix
*  @param [in]		numeraireID		ID for the annuity matrix
*  @param [in]		alphaID			ID for the alpha parameter matrix
*  @param [in]		betaID			ID for the beta parameter matrix
*  @param [in]		nuID			ID for the nu parameter matrix
*  @param [in]		rhoID			ID for the rho parameter matrix
*  @param [in]		approxMethod	Approximation method, e.g. HAGAN
*  @param [in]		shift			Optional. Default 0. Shift size of the shifted SABR
*  @param [in]		volType			Optional. Volatility type, default lognormal
*  @return			SABR-implied premium at the expiry/tenor/strike point
*/
double aqVolatilitySABRGetPrem( const std::string& expPoint,
                                 const std::string& tenorPoint,
                                 const double strike,
                                 const int sign,
                                 const std::string& forwardID,
                                 const std::string& numeraireID,
                                 const std::string& alphaID,
                                 const std::string& betaID,
                                 const std::string& nuID,
                                 const std::string& rhoID,
                                 const std::string& approxMethod,
                                 const double shift = 0.0,
                                 const std::string& volType = "" );

/* @brief			swig interface for aqVolatilitySABRGetVol
*  @param [in]		expPoint		Expiry tenor
*  @param [in]		tenorPoint		Underlying swap tenor
*  @param [in]		strike			Strike
*  @param [in]		forwardID		ID for the forward rate matrix
*  @param [in]		alphaID			ID for the alpha parameter matrix
*  @param [in]		betaID			ID for the beta parameter matrix
*  @param [in]		nuID			ID for the nu parameter matrix
*  @param [in]		rhoID			ID for the rho parameter matrix
*  @param [in]		approxMethod	Approximation method, e.g. HAGAN
*  @param [in]		shift			Optional. Default 0. Shift size of the shifted SABR
*  @param [in]		volType			Optional. Volatility type, default lognormal
*  @return			SABR-implied volatility at the expiry/tenor/strike point
*/
double aqVolatilitySABRGetVol( const std::string& expPoint,
                                const std::string& tenorPoint,
                                const double strike,
                                const std::string& forwardID,
                                const std::string& alphaID,
                                const std::string& betaID,
                                const std::string& nuID,
                                const std::string& rhoID,
                                const std::string& approxMethod,
                                const double shift = 0.0,
                                const std::string& volType = "" );

/* @brief			swig interface for aqVolatilitySABROutputParameter
*  @param [in]		gridID			ID identifying the data grid
*  @return			The stored SABR grid, as a string matrix (row x column)
*/
SWIG_STRINGMATRIX aqVolatilitySABROutputParameter( const std::string& gridID );

/* @brief			swig interface for aqVolatilitySABRSetupConvention
*  @param [in]		conventionID	ID to store the convention data under
*  @param [in]		convData		Convention data, as a key/value matrix
*  @return			TRUE on success
*/
bool aqVolatilitySABRSetupConvention( const std::string& conventionID,
                                       const SWIG_STRINGMATRIX& convData );

/* @brief			swig interface for aqVolatilitySABRSetupParameter
*  @param [in]		gridID			ID of the parameter grid being set up
*  @param [in]		conventionID	ID of the convention data
*  @param [in]		gridData		Data of the parameter grid being set up
*  @return			TRUE on success
*/
bool aqVolatilitySABRSetupParameter( const std::string& gridID,
                                      const std::string& conventionID,
                                      const SWIG_STRINGMATRIX& gridData );

/* @brief			swig interface for aqVolatilitySABRSetupSwaptionVol
*  @param [in]		gridID			ID that identifies the swaption vol matrix set
*  @param [in]		volMat			Matrix of swaption vols
*  @param [in]		strikeMat		Matrix of strikes
*  @param [in]		signMat			Matrix of signs
*  @return			TRUE on success
*/
bool aqVolatilitySABRSetupSwaptionVol( const std::string& gridID,
                                        const SWIG_STRINGMATRIX& volMat,
                                        const SWIG_STRINGMATRIX& strikeMat,
                                        const SWIG_STRINGMATRIX& signMat );
