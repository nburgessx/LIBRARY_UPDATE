#pragma once
#include "BaseInstrument.h"
#include "Swap.h"
#include "LabelValueBlock.h"
#include <boost/shared_array.hpp>
#include "AQLString.h"
#include <boost/shared_array.hpp>

class CurveCalibrationData;
class AQLDataMultiReference;
class AQLDataDouble;
class AQLDataProcedure;

namespace etrading
{
	extern const char PILLAR_DELIMITER;

    class DeltaGenerator
    {
    public:

        DeltaGenerator();

		/* @brief	Constructor for BaseInstruments
        *  Note:    Non-LWO Base Case does not support Xccy Swaps
		*/
        DeltaGenerator( const std::vector<BaseInstrumentPtr>& trades,
						const std::vector<AQLString>& tradeIDs,
						const bool bumpSpreadInstruments,
                        const double bumpSize,
                        const AQLString& bumpMode,
                        const bool aggregateRisks,
						const std::string& riskCutOffTenor );

		/* @brief	Constructor for LWO Swap Legs
		*/
		DeltaGenerator( const std::vector<std::shared_ptr<Leg> >& swapLegs,
						const std::vector<AQLString>& legIDs,
						const std::vector<LabelValueBlock >& fixingTableNames,
                        const std::vector<double>& xccyFXSpotRates,
						const bool bumpSpreadInstruments,
                        const double bumpSize,
						const AQLString& bumpMode,
                        const bool aggregateRisks,
						const bool reportInLegCCY,
						const std::string& riskCutOffTenor,
						const bool useGlobalCurveEngine);

		/* @brief	Constructor for LWO Swaps
		*/
		DeltaGenerator(const std::vector<SwapPtr >& lwoSwaps,
						const std::vector<AQLString>& swapIDs,
						const std::vector<LabelValueBlock>& fixingTableNames,
                        const std::vector<double>& xccyFXSpotRates,
						const bool bumpSpreadInstruments,
						const double bumpSize,
						const AQLString& bumpMode,
						const bool aggregateRisks,
						const std::string& riskCutOffTenor,
						const bool useGlobalCurveEngine);

        ~DeltaGenerator() {}

        /* @brief			Set the yield curves required for delta calculation
        *  @param [in]		curveSetID		Name or handle of the curve set
        *  @param [in]		forecastCurve	Name or handle of the forecast curve
        *  @param [in]		discountCurve	Name or handle of the discount curve
        */
        void setCurves( const AQLString& curveSetID, const AQLString& forecastCurve, const AQLString& discountCurve );

        /* @brief			Method that set a variety of pricing parameters
        *  @param [in]		params		A collection of pricing parameters
        */
        void setPricingParams( const LabelValueBlock& params );

		/* @brief	Examines all the curves which the globalEngine is configured to build,
		*			and returns the curveName corresponding to the SWAP curve
		*
		*  @param[in]	curveCollectionID	The curve collection
		*  @returns		The name of the swap curve, if found. If no swap curve, returns an empty string
		*/
		AQLString getSwapCurveNameViaGlobalEngine( const AQLString& curveCollectionID );

		/* @brief	Examines the list of curves which depend on the specified curveName
		*			(i.e. the list of curves which must be rebuild when the specified curveName is bumped).
		*			The method searches for the SWAP curve within the list of dependent curves
		*
		*  @param[in]	curveCollectionID	The curve collection
		*  @param[in]	curveName			The curveName used when finding dependent curves
		*  @returns		The name of the swap curve, if found. If no swap curve, returns an empty string
		*/
		AQLString getSwapCurveNameViaDependentCurves( const AQLString& curveCollectionID, const AQLString& curveName );

        /* @brief		Calculate flat shift delta risk
        *  @return		Delta risk numbers through curve flat shifting for all the trades
        */
        DoubleVector flatshiftDelta();

        /* @brief			Calculate delta ladder of the given trade(s)
        *  @param [out]		pillarNames		Name of pillar points
        *  @param [out]		deltas			All the deltas
        */
        void deltaLadder( AQLStringVector& pillarNames, DoubleMatrix& deltas );

		AQLString getCurveDependencyTreeAsString() const;
		AQLString getAllCurvesToBumpAsString() const;

    private:

        //-------------------------------------------------------------------------------------
        //-------------------------------------------------------------------------------------

        class DeltaLadderData
        {
        public:

            DeltaLadderData();

            /* @brief		Add a bucket risk to the delta ladder
            *  @param [in]	curveName						Name of the current curve
            *  @param [in]	curveCollectionID				Name of the relevant curve collection
            *  @param [in]	bucketNameWithoutCurvePrefix	Name of the current pillar
            *  @param [in]	deltas							Pillar delta for all the trades
            */
            void addBucketRisk( const AQLString& curveName, const AQLString& curveCollectionID, const AQLString& bucketName, DoubleVector deltas );

            /* @brief		Set the flag that controls risk aggregation
            *  @param [in]	aggregateRisk	Do we aggregate risk or not
            */
            void setIsRiskAggregated( bool aggregateRisk );

            /* @brief		Return delta ladder in presentable format
            *  @param [out]		pillarNames		Name of pillar points
            *  @param [out]		deltas			All the deltas
            */
            void outputDeltaLadder( AQLStringVector& pillarNames, DoubleMatrix& deltas ) const;

            /* @brief		Build contral delta ladder through up deltas and down deltas
            *  @param [in]	doubleLadderData_up		A delta ladder generated through bumping up
            *  @param [in]	doubleLadderData_down	A delta ladder generated through bumping down
            */
            void buildCentralDeltas( const DeltaLadderData& doubleLadderData_up, const DeltaLadderData& doubleLadderData_down );

            /* @brief		Get delta ladder size
            *  @return		Delta ladder size
            */
            size_t size() const;

        private:

            // A map of bucket deltas (of each trade) to the corresponding bucket name.
            std::map<AQLString, DoubleVector> deltaLadderInMap_;

            // The value for each key is the curve name that is going to be prefixed to the
            // short label name to build the final bucket label for output
            std::map<AQLString, AQLString> bucketNamePrefix_;

            // A vector keeping the order of the output buckets
            // whose risks are not aggregated
            std::vector<AQLString> bucketNames_NonAggregated_;

            // A vector keeping the order of the output buckets
            // whose risks are aggregated
            std::vector<AQLString> bucketNames_Aggregated_;

            bool isRiskAggregated_;
        };

        //-------------------------------------------------------------------------------------
        //-------------------------------------------------------------------------------------


        /* @brief		A collection of all the curves that are to be bumped
        */
        struct BumpCurvesCollection
        {
            /* @brief			Default constructor
            */
            BumpCurvesCollection();

            /* @brief			Add a curve to bump
            *  @param [in]		curveName			Name of the curve being shifted
            *  @param [in]		curveCollectionID	Curve collection ID
            *  @param [in]		discountCurveName	Discount curve
			*  @param [in]		isUsingGlobalCurveEngine	Are curves calibrated by the global curve engine?
            */
            void addCurve( const AQLString& curveName, const AQLString& curveCollectionID, const AQLString& discountCurveName = "", bool isUsingGlobalCurveEngine = false);

			/* @brief	Follows the chain of dependencies and determines whether there is a dependency path
			*           from: startCurveCollectionID:startCurveName
			*			to:   endCollectionID:endCurveName
			*  @param [in]		startCurveCollectionID	The CurveCollectionID of the starting point
			*  @param [in]		startCurveName			The CurveName of the starting point
			*  @param [in]		endCurveCollectionID	The CurveCollectionID of the end point
			*  @param [in]		endCurveName			The CurveName of the end point
			*  @param [out]     Returns true if a path exists
			*/
			bool findDependencyPath( const AQLString& startCurveCollectionID, const AQLString& startCurveName,
									 const AQLString& endCurveCollectionID, const AQLString& endCurveName );

            std::map<AQLString, AQLString> curveTypes_;
            std::map<AQLString, std::set<AQLString> > dependentCurve_;

            // 'allCurvesInEachCollection_' is a collection of curves that will definitely be bumped
            // under each curve collecction in the process of delta ladder calculation
            std::map<AQLString, std::set<AQLString> > allCurvesInEachCollection_;
        };

        //-------------------------------------------------------------------------------------
        //-------------------------------------------------------------------------------------

		/* @brief		Returns the size of the trade portfolio
		*				Examines myTrades_ or myLWOSwapLegs_ depending on whether LWO Swaps are being used
		*/
		size_t getPortfolioSize();

		/* @brief		Calculates the PV of the specified instrument
		 * @param [in]	index							The index of the instrument in the portfolio
		 * @param [in]	setMarketDataAndInterpolation	For non LWO-swaps, specifies whether to initialize the trade with MarketData and Interpolation parameters
		*/
		double calculateTradePV(int index, bool setMarketDataAndInterpolation = false);

        // Trade portfolio
        std::vector<BaseInstrumentPtr> myTrades_;
		std::vector<LegPtr > myLWOSwapLegs_;
		std::vector<SwapPtr > myLWOSwaps_;
		std::vector<AQLString> myInstrumentIDs_;
		std::vector<LabelValueBlock > myLWOFixingTables_;
        std::vector<double> myLWOXccyFXAsOfDateRates_;

        // Delta parameters
		bool bumpSpreadInstruments_;
        double bumpSize_;
        AQLString bumpMode_;

        // Curves
        BumpCurvesCollection bumpCurvesCollection_;
        AQLString interpolation_;
        LabelValueBlock marketDataCollection_;

        // This is the original market data before bumping
        // First key indexes curve; second key indexes constituent market instrument
        std::map<AQLString, std::map<AQLString, double> > originalCurveMarketData_;

		// This stores the instrument IR_CALIBRATION_DATA_GRIDUSEFLAG state before bumping.
		// This data controls whether that instrument is included in the curve build
		// Map from curve collection -> InstrumentName -> include flag
		std::map<AQLString, std::map<AQLString, bool> > originalIncludeInstrumentInCurve_;


        // Aggregate risks against the same market instruments that come from different curves
        bool aggregateRisks_;

		bool reportInLegCCY_;

		// The maximum tenor in years (e.g. 10Y) beyond which the curves are no longer bumped
		std::string riskCutOffTenor_;

		// Set to true if the portfolio contains LWO Swaps
		bool usingLWO_;

		// Calculate risks at the LWO swap leg level
		bool riskOnLWOLegs_;

		// Are we using global curve engine to build curves
		bool isUsingGlobalCurveEngine_;

        //-------------------------------------------------------------------------------------
        // Calculation methods

        /* @brief			Flat shift a given curve
        *  @param [in]		direction		Curve shifting direction
        *  @param [in]		bumpSize		The size of bumps in basis point
        */
        void flatShiftYieldCurve( const AQLString& direction = "UP", double bumpSize = 0.01 );

        /* @brief			Find the delta ladder of a particular curve
        *  @param [inout]	deltaLadderData		Delta ladder data wrapper object
        *  @param [inout]	oldPVs				Instrument's PV before bumping
        *  @param [in]		direction			Bumping direction
        *  @param [in]		bumpSize			Bump size in bps
        */
        void deltaLadderPerCurve( DeltaLadderData& deltaLadderData,
                                  DoubleVector oldPVs,
                                  const AQLString& direction,
                                  double bumpSize );

        /* @brief			Calculate the delta risk for a pillar point
        *  @param [inout]	deltaLadderData			Delta ladder data wrapper object
        *  @param [inout]	attrRate				The rate attribution of a pillar point
        *  @param [inout]	curveCalibrationData						Pointer to the yield curve pro object
        *  @param [inout]	objHolder						The current pillar point object
        *  @param [in]		asofdate				As of date of the curve
        *  @param [in]		bumpSize				Delta bump size
        *  @param [in]		oldPVs					Instrument PV before bumping
        *  @param [in]		curveName				Name of current curve
        *  @param [in]		curveCollectionID		Name of the associated curve collection
        *  @param [in]		pillarType				Type of current pillar point
        *  @param [in]		isBasisCurve			Is the current curve a basis curve
        *  @param [in]		directionFactor			Either 1 or -1 depending on bumping up or down
        *  @param [inout]	attr2ndRate				The same instrument to bump that resides in another curve
        *  @return									The name and delta of a curve pillar point
        */
        void pillarDelta( DeltaLadderData& deltaLadderData,
                          AQLDataDouble& attrRate,
                          CurveCalibrationData* curveCalibrationData,
                          const AQLObjectHolder& objHolder,
                          const AQLDate& asofdate,
                          double bumpSize,
                          DoubleVector oldPVs,
                          const AQLString& curveName,
                          const AQLString& curveCollectionID,
                          const AQLString& pillarType,
                          bool isBasisCurve,
                          double directionFactor,
                          AQLDataDouble* attr2ndRate = NULL );

        //-------------------------------------------------------------------------------------
        // Utility methods

        /* @brief			Restore yield curve back to pre-bump state
        */
		void restoreCurveMarketData(const AQLString& curveCollectionID, const AQLString& curveName);

		/* @brief			Restore all bumped yield curves back to pre-bump state
        */
        void restoreCurveMarketDataAndRecalibrate();

        /* @brief			Cache original constituent market data of a curve
        */
        void cacheOriginalCurveData();

        /* @brief			Construct a meaning full pillar name for output
        *  @param [in]		objHolder				The pillar point object
        *  @param [in]		pillarType		Type of the pillar point
        *  @param [in]		isBasisCurve	Is this a basis curve?
        *  @return			output pillar name
        */
        AQLString buildOutputPillarName( const AQLObjectHolder& objHolder, const AQLString& pillarType, bool isBasisCurve );

        /* @brief			Check if the current pillar point is built out of spread + swap
        *  @param [in]		objHolder				The pillar point object
        *  @return			TRUE means being built from spread + swap
        */
        bool isBuiltFromBasisSpread( const AQLObjectHolder& objHolder );

        /* @brief			Check if the given curve is a basis curve
        *  @param [inout]	curveCalibrationData						Pointer to the yield curve pro object
        *  @param [in]		attrSuffix				suffix string used to locate data
        *  @param [in]		bumpCurvesCollection	Collection of curves being bumped
        *  @param [in]		curveName				Current curve
        *  @return			boolean that tells if curve is a basis curve
        */
        bool checkIsBasisCurve( CurveCalibrationData* curveCalibrationData, const AQLString& attrSuffix, const BumpCurvesCollection& bumpCurvesCollection, const AQLString& curveName );

        /* @brief			Recalibrate the specified curve only.
        *  @param [inout]	curveCalibrationData					Pointer to the yield curve pro object
        *  @param [in]		curveName			Current curve
        *  @param [in]		curveCollectionID	Collection name where the current curve is in
        *  @param [in]		isBasisCurve		Is the current curve a basis curve?
        */
        void recalibrateSingleCurve( CurveCalibrationData* curveCalibrationData, const AQLString& curveName, const AQLString& curveCollectionID, bool isBasisCurve );

        /* @brief			Recalibrate the specified curve and all dependent curves
        *  @param [inout]	curveCalibrationData					Pointer to the yield curve pro object
        *  @param [in]		curveName			Current curve
        *  @param [in]		curveCollectionID	Collection name where the current curve is in
        *  @param [in]		isBasisCurve		Is the current curve a basis curve?
        */
        void recalibrateCurveAndAllDependentCurves( CurveCalibrationData* curveCalibrationData, const AQLString& curveName, const AQLString& curveCollectionID, bool isBasisCurve );

		/* @brief			Recalibrate all curves used by this DeltaRiskGenerator
		 *					The curves are rebuilt by curveType in the following sequence: OIS, STD, TenorBasis, XccyBasis, FwdFXConst.
		 */
		void recalibrateCurves();
    };


}
