// aqSwapDelta.h

/*
 * @brief			Swig interface for aqSwapDelta / aqSwapObjectDelta... risk functions
 */

#pragma once

#include "SwigTypes.h"

#include <string>
#include <vector>

/* @brief			swig interface for the aqSwapDelta function - flat-shift delta for several trades defined inline
*  @param [in]		dealInfoLVBs		Trade definitions table: header row of keys, one row per trade
*  @param [in]		forecastCurveSet	Column of forecasting curve indices
*  @param [in]		curveCollection		The curve collection
*  @param [in]		bumpSize			Bump size, e.g. 0.0001 for 1bp
*  @param [in]		bumpMode			Up, Down, or Central bumping
*  @param [in]		deltaType			Flat-shift or ladder delta type
*  @param [in]		aggregateRisk		Whether to aggregate risk against the same instrument across curves
*  @return			A pillar x header delta matrix (header row, then one row per pillar)
*/
SWIG_STRINGMATRIX aqSwapDelta( const SWIG_STRINGMATRIX & dealInfoLVBs,
                            const std::vector<std::string>& forecastCurveSet,
                            const std::string& curveCollection,
                            double bumpSize,
                            const std::string& bumpMode,
                            const std::string& deltaType,
                            bool aggregateRisk );

/* @brief			swig interface for the aqSwapObjectDelta function - flat-shift delta for a column of cached swaps
*  @param [in]		swapNames				Column of cached swap handles
*  @param [in]		curveCollectionNames	Curve collection name(s) per swap leg
*  @param [in]		fixingTableNames		Fixing table name(s) per swap leg
*  @param [in]		bumpSpreadInstruments	Whether to bump the LIBOR-OIS spread instruments in the OIS curve
*  @param [in]		bumpSize				Bump size, e.g. 0.0001 for 1bp
*  @param [in]		bumpMode				Up, Down, or Central bumping
*  @param [in]		groupRiskBy				How to group the reported risk
*  @param [in]		aggregateRisks			Whether to aggregate risk against the same instrument across curves
*  @param [in]		reportInLegCCY			Whether to report the risk in LegCCY (if true) or valuationCCY (if false)
*  @param [in]		xccyFXSpotRates			Optional. Column of cross-currency FX spot rates
*  @return			A two-column (PositionID, Delta) matrix, one row per position
*/
SWIG_STRINGMATRIX aqSwapObjectDelta( const std::vector<std::string>& swapNames,
                                  const SWIG_STRINGMATRIX & curveCollectionNames,
                                  const SWIG_STRINGMATRIX & fixingTableNames,
                                  bool bumpSpreadInstruments,
                                  double bumpSize,
                                  const std::string& bumpMode,
                                  const std::string& groupRiskBy,
                                  bool aggregateRisks,
                                  bool reportInLegCCY,
                                  const std::vector<double>& xccyFXSpotRates );

/* @brief			swig interface for the aqSwapObjectDeltaLadder function - pillar-by-pillar delta ladder for a column of cached swaps
*  @param [in]		swapNames				Column of cached swap handles
*  @param [in]		curveCollectionNames	Curve collection name(s) per swap leg
*  @param [in]		fixingTableNames		Fixing table name(s) per swap leg
*  @param [in]		bumpSpreadInstruments	Whether to bump the LIBOR-OIS spread instruments in the OIS curve
*  @param [in]		bumpSize				Bump size, e.g. 0.0001 for 1bp
*  @param [in]		bumpMode				Up, Down, or Central bumping
*  @param [in]		aggregateRisk			Whether to aggregate risk against the same instrument across curves
*  @param [in]		reportInLegCCY			Whether to report the risk in LegCCY (if true) or valuationCCY (if false)
*  @param [in]		riskCutOffTenor			The maximum tenor (e.g. 10Y) beyond which curves are no longer bumped
*  @param [in]		xccyFXSpotRates			Optional. Column of cross-currency FX spot rates
*  @return			A pillar x curve delta matrix (header row, then one row per pillar)
*/
SWIG_STRINGMATRIX aqSwapObjectDeltaLadder( const std::vector<std::string>& swapNames,
                                        const SWIG_STRINGMATRIX & curveCollectionNames,
                                        const SWIG_STRINGMATRIX & fixingTableNames,
                                        bool bumpSpreadInstruments,
                                        double bumpSize,
                                        const std::string& bumpMode,
                                        bool aggregateRisk,
                                        bool reportInLegCCY,
                                        const std::string& riskCutOffTenor,
                                        const std::vector<double>& xccyFXSpotRates );

/* @brief			swig interface for the aqSwapObjectDeltaLadderHorizontally function - as aqSwapObjectDeltaLadder,
*					with each curve's pillar/delta pair presented in its own two columns
*  @param [in]		swapNames				Column of cached swap handles
*  @param [in]		curveCollectionNames	Curve collection name(s) per swap leg
*  @param [in]		fixingTableNames		Fixing table name(s) per swap leg
*  @param [in]		bumpSpreadInstruments	Whether to bump the LIBOR-OIS spread instruments in the OIS curve
*  @param [in]		bumpSize				Bump size, e.g. 0.0001 for 1bp
*  @param [in]		bumpMode				Up, Down, or Central bumping
*  @param [in]		aggregateRisk			Whether to aggregate risk against the same instrument across curves
*  @param [in]		reportInLegCCY			Whether to report the risk in LegCCY (if true) or valuationCCY (if false)
*  @param [in]		riskCutOffTenor			The maximum tenor (e.g. 10Y) beyond which curves are no longer bumped
*  @param [in]		xccyFXSpotRates			Optional. Column of cross-currency FX spot rates
*  @return			A header row (each curve name repeated twice) followed by paired PillarName/Delta columns
*/
SWIG_STRINGMATRIX aqSwapObjectDeltaLadderHorizontally( const std::vector<std::string>& swapNames,
                                                    const SWIG_STRINGMATRIX & curveCollectionNames,
                                                    const SWIG_STRINGMATRIX & fixingTableNames,
                                                    bool bumpSpreadInstruments,
                                                    double bumpSize,
                                                    const std::string& bumpMode,
                                                    bool aggregateRisk,
                                                    bool reportInLegCCY,
                                                    const std::string& riskCutOffTenor,
                                                    const std::vector<double>& xccyFXSpotRates );
