#pragma once

#include "AQLCoreTemplateType.h"
#include "AQLDataInstance.h"
#include "LabelValueBlock.h"

using etrading::LabelValueBlock;

namespace validation
{
    /* @brief			Validation interface for the aqSwapsDeltaLadder method. This method calculates a Delta Ladder for a vector of swaps
	*  @param [out]		headers						Headers of the delta report
    *  @param [out]		pillarNames					The final list of pillar names
    *  @param [out]		deltas						All the deltas
    *  @param [in]		swapNames					A vector of strings representing the AQObj Swap names
	*  @param [in]		curveCollectionNames		A vector of strings representing the CurveCollection names for each leg of swap
    *  @param [in]		fixingTableNames			A vector of strings represening the fixing table names for each leg of swap
	*  @param [in]		bumpSpreadInstruments		Whether to bump the LIBOR_OIS spread instruments in the OIS curve. Default is TRUE.
    *  @param [in]		bumpSize					Bumping size. Default to 0.01 (basis point).
    *  @param [in]		bumpMode					Bumping mode. Up, down, or central bumping.
    *  @param [in]		aggregateRisk				Whether to aggregate risks against the same market instruments from different curves
	*  @param [in]		reportInLegCCY				Whether to report the risk in LegCCY (if true) or valuationCCY (if false)
	*  @param [in]		riskCutOffTenor	    		The maximum tenor in years (e.g. 10Y) beyond which the curves are no longer bumped
    *  @param [in]		xccyFXSpotRates			        A vector of doubles representing Xccy FX Spot Rates
    */
	 void tryAqSwapObjectDeltaLadder( AQLStringVector& headers,
								   AQLStringVector& pillarNames,
								   DoubleMatrix& deltas,
								   const AQLStringVector& swapNames,
								   const AQLStringMatrix& curveCollectionNames,
								   const AQLStringMatrix& fixingTableNames,
								   const bool bumpSpreadInstruments,
								   const double bumpSize,
								   const AQLString& bumpMode,
								   const bool aggregateRisk,
								   const bool reportInLegCCY,
								   const std::string& riskCutOffTenor,
                                   const DoubleVector& xccyFXSpotRates );

   /* @brief			Validation interface for the aqSwapsDeltaLadder method. This method calculates a Delta Ladder for a vector of swaps
    *                   This version presents the delta buckets for each curve in separate columns. Each consecutive pairs of columns
	*                   (pillarName, delta) are aligned horizontally.
	*  @param [out]		headers						Headers of the delta report
    *  @param [out]		pillarNames					A vector of AQLStringVector, with each AQLStringVector representing a column of pillarNames for a curve
    *  @param [out]		deltas						A vector of DoubleVector, with each DoubleVector representing a column of delta sensitivities for a curve
    *  @param [in]		swapNames					A vector of strings representing the AQObj Swap names
	*  @param [in]		curveCollectionNames		A vector of strings representing the CurveCollection names for each leg of swap
    *  @param [in]		fixingTableNames			A vector of strings represening the fixing table names for each leg of swap
	*  @param [in]		bumpSpreadInstruments		Whether to bump the LIBOR_OIS spread instruments in the OIS curve. Default is TRUE.
    *  @param [in]		bumpSize					Bumping size. Default to 0.01 (basis point).
    *  @param [in]		bumpMode					Bumping mode. Up, down, or central bumping.
    *  @param [in]		aggregateRisk				Whether to aggregate risks against the same market instruments from different curves
	*  @param [in]		reportInLegCCY				Whether to report the risk in LegCCY (if true) or valuationCCY (if false)
	*  @param [in]		riskCutOffTenor	    		The maximum tenor in years (e.g. 10Y) beyond which the curves are no longer bumped
    *  @param [in]		xccyFXSpotRates			        A vector of doubles representing Xccy FX Spot Rates
    */
	 void tryAqSwapObjectDeltaLadderHorizontally( AQLStringVector& headers,
											   std::vector<AQLStringVector>& pillarNames,
											   std::vector<DoubleVector>& deltas,
											   const AQLStringVector& swapNames,
											   const AQLStringMatrix& curveCollectionNames,
											   const AQLStringMatrix& fixingTableNames,
											   const bool bumpSpreadInstruments,
											   const double bumpSize,
											   const AQLString& bumpMode,
											   const bool aggregateRisk,
											   const bool reportInLegCCY,
											   const std::string& riskCutOffTenor,
                                               const DoubleVector& xccyFXSpotRates );


    /* @brief			validation interface for the aqSwapDelta method. This method calculates a Flat-Shift Delta for a vector of swaps
	*  @param [out]		positionIDs					The vector of positions for which the flat-shift delta is calculated
    *  @param [out]		deltas						The delta value for each SwapLeg
    *  @param [in]		swapNames					A vector of strings representing the AQObj Swap names
	*  @param [in]		CurveCollectionNamesLeg1	A vector of strings representing the CurveCollection names for Leg1 of each swap
	*  @param [in]		CurveCollectionNamesLeg2	A vector of strings representing the CurveCollection names for Leg2 of each swap
    *  @param [in]		fixingTableNamesLeg1		A vector of strings represening the fixing table names for Leg1 of each swap
	*  @param [in]		fixingTableNamesLeg2		A vector of strings represening the fixing table names for Leg2 of each swap
	*  @param [in]		bumpSpreadInstruments		Whether to bump the LIBOR_OIS spread instruments in the OIS curve. Default is TRUE.
    *  @param [in]		bumpSize					Bumping size. Default to 0.01 (basis point).
    *  @param [in]		bumpMode					Bumping mode. Up, down, or central bumping.
    *  @param [in]		aggregateRisk				Whether to aggregate risks against the same market instruments from different curves
	*  @param [in]		reportInLegCCY				Whether to report the risk in LegCCY (if true) or valuationCCY (if false)
    *  @param [in]		xccyFXSpotRates			        A vector of doubles representing Xccy FX Spot Rates
    */
	void tryAqSwapObjectDelta( AQLStringVector& positionIDs,
							DoubleVector& deltas,
							const AQLStringVector& swapNames,
							const AQLStringMatrix& curveCollectionNames,
							const AQLStringMatrix& fixingTableNames,
							const bool bumpSpreadInstruments,
							const double bumpSize,
							const AQLString& bumpMode,
							const AQLString& groupRiskBy,
							const bool aggregateRisks,
							const bool reportInLegCCY,
                            const DoubleVector& xccyFXSpotRates);

}

