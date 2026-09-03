#pragma once
#include "Swap.h"
#include "BaseInstrument.h"
#include "LabelValueBlock.h"
#include "DeltaRiskGenerator.h"
#include "CurveResultsContainer.h"      // New Non-Object Pool Curve Results Objects

#include <boost/shared_array.hpp>
#include "AQLString.h"
#include <boost/shared_array.hpp>

class YieldCurvePro;
class AQLDataMultiReference;
class AQLDataDouble;
class AQLDataProcedure;

namespace etrading
{
    /* @brief			Class that calculates swap delta risk against multiple yield curves
    */
    class MultiCurveDeltaGenerator
    {
    public:

        MultiCurveDeltaGenerator();

		/* @brief	Constructor for LabelValueBlock and BaseInstrument
        *  Note1:   Base Instrument Does not support Xccy Swaps
        *  Note2:   We must disable the CurveResults Object otherwise Products will price outside the object pool and not incorporate curve bumps and shift results
		*/
        MultiCurveDeltaGenerator( const std::vector<LabelValueBlock>& dealsInfo,
                                  const AQLString curveCollection,
								  const bool bumpSpreadInstruments,
                                  const double bumpSize,
                                  const AQLString& bumpMode,
                                  const bool aggregateRisks,
								  const std::string& riskCutOffTenor );

		/* @brief	Constructor for LWO Swaps
        *  Note1:   We must disable the CurveResults Object otherwise Products will price outside the object pool and not incorporate curve bumps and shift results
		*/
		MultiCurveDeltaGenerator( const AQLStringVector& swapNames,
			                      const AQLStringMatrix& curveCollectionNames,
		                          const AQLStringMatrix& fixingTableNames,
                                  const DoubleVector& xccyFXSpotRates,
								  const bool bumpSpreadInstruments,
                                  const double bumpSize,
								  const AQLString& bumpMode,
                                  const bool aggregateRisks,
								  const bool reportInLegCCY,
								  const std::string& riskCutOffTenor ); 

		~MultiCurveDeltaGenerator() {}

        /* @brief			Set the yield curves required of delta calculation
        *  @param [in]		curveSetID		Name or handle of the curve set
        *  @param [in]		curves			A collection of yield curves
        */
        void setCurves( const AQLString& curveCollectionID, const AQLStringVector& curves );

        /* @brief			Calculate delta ladder for the given trade(s)
        *  @param [out]		pillarNames		Name of pillar points
        *  @param [out]		headers			Headers of the delta matrix
        *  @param [out]		deltas			All the deltas
        */
        void deltaLadder( AQLStringVector& pillarNames, AQLStringVector& headers, AQLStringVector& deltaCCYs, DoubleMatrix& deltas );

		/* @brief			Calculate flat-shift delta for the given trade(s)
        *  @param [out]		positionIDs		Name of each SwapID / LegID for which the delta is calculated
        *  @param [out]		deltas			The flat-shift delta of each swap leg.
        */
		void flatShiftDelta( AQLStringVector& positionIDs, DoubleVector& deltas, const AQLString& groupRiskBy);


    private:

		// Disable Curve Results Objects for the lifetype of this class i.e. while calculating risk.
		// The Curve Results Objects are used for Pricing outside the Object Pool.
		DisableCurveResults disableCurveResults_;

        //-------------------------------------------------------------------------------------
        //-------------------------------------------------------------------------------------
		
		/* @brief	A portfolio of instruments can be grouped into sub-portfolios such that the instruments in given sub-portfolio
		*           all share the same curve dependencies.
		*			This struct stores those common curve dependencies for a given sub-portfolio of instruments.
		*           The curve dependencies are: curveCollection, forecastCurve, discountCurve.
		*/
		struct CurveDependencies
		{
			AQLString curveCollectionID_;
			AQLString forecastCurve_;
			AQLString discountCurve_;

			CurveDependencies() {}
			CurveDependencies(const AQLString& curveCollectionID, const AQLString& forecastCurve, const AQLString& discountCurve)
				: curveCollectionID_(curveCollectionID), forecastCurve_(forecastCurve), discountCurve_(discountCurve)
				{}

			/* @brief	Less-than operator for comparing CurveDependencies.
			*			Required when using CurveDependencies as a key in a std::map
			*/
			bool operator<(const CurveDependencies& rhs) const;
		};

		/* @brief		Returns the size of the trade portfolio
		*				Examines portfolio_ or lwoPortfolio_ depending on whether LWO Swaps are being used
		*/
		size_t getPortfolioSize();

		/* @brief		Examines the lwoTrade input and adds the trade leg data to a map based on the curve dependencies required to PV that leg
		 * @param [in]	lwoTrade					The LWO Swap to be processed
		 * @param [in]	curveCollectionForTrade		A LabelValueBlock containing the curve collections required to PV this swap
		 * @param [out] fixingTableForTrade			A fixingTableMap containing the fixingTableNames required to PV this swap
		 * @param [out]	allLegIDs					An output containing all LegIDs processed so far
		 * @param [out]	allLegCCYs					An output containing the Leg native currencies
		 * @param [out]	lwoSwapLegsByCurves			An output map containing mini-portfolios of swap legs, keyed by CurveDependencies
		 * @param [out]	lwoLegIDsByCurves			An output map containing mini-portfolios of swap legIDs, keyed by CurveDependencies
		 * @param [out]	fixingTableNamesByCurves		An output map containing mini-portfolios of swap fixingTableNames, keyed by CurveDependencies.
		*/
		void groupLWOTradeLegsByCurveDependencies( const std::shared_ptr<Swap>& lwoTrade,
											       const LabelValueBlock& curveCollectionForTrade,
											       const LabelValueBlock& fixingTableForTrade,
											       AQLStringVector& allLegIDs,
											       AQLStringVector& allLegCCYs,
											       std::map< CurveDependencies, std::vector< std::shared_ptr<Leg> > >& lwoSwapLegsByCurves,
											       std::map< CurveDependencies, std::vector< AQLString > >& lwoTradeIDsByCurves,
											       std::map< CurveDependencies, std::vector< AQLString > >& lwoLegIDsByCurves,
											       std::map< CurveDependencies, std::vector< LabelValueBlock > >& fixingTableNamesByCurves);

		/* @brief		Examines the lwoTrade input and adds the trade data to a map based on the curve dependencies required to PV that leg
		* @param [in]	lwoTrade					The LWO Swap to be processed
		* @param [in]	curveCollectionForTrade		A LabelValueBlock containing the curve collections required to PV this swap
		* @param [out]	allTradeIDs					An output which accumulates the trade IDs that have been processed so far
		* @param [out]	allTradeCCYs				An output which accumulates the trade native currencies
		* @param [out]  fixingTableForTrade			A fixingTableMap containing the fixingTableNames required to PV this swap
		* @param [out]	lwoSwapLegsByCurves			An output map containing mini-portfolios of swap legs, keyed by CurveDependencies
		* @param [out]	fixingTableNamesByCurves		An output map containing mini-portfolios of swap fixingTableNames, keyed by CurveDependencies.
		*/
		void groupLWOTradesByCurveDependencies(const SwapPtr& lwoTrade,
												const LabelValueBlock& curveCollectionForTrade,
												const LabelValueBlock& fixingTableForTrade,
												AQLStringVector& allTradeIDs,
												AQLStringVector& allTradeCcys,
												std::map< CurveDependencies, std::vector< SwapPtr > >& lwoSwapByCurves,
												std::map< CurveDependencies, std::vector< AQLString > >& lwoTradeIDsByCurves,
												std::map< CurveDependencies, std::vector< LabelValueBlock > >& fixingTableNamesByCurves);

		/* @brief		Invokes the delta risk generator on a mini portfolio to calculate the delta-ladder, and post-processes the results
		 * @param [in]	riskGen						The DeltaGenerator which will calculate the risk for a given miniPortfolio
		 * @param [in]	miniPortfolioTradeIDs		A vector of tradeIDs from the miniPortfolio
		 * @param [out] deltaMap					An output containing the delta sensitivities. Each element in 'deltaMap' is indexed by the combination of a pillar name and a trade ID.
		 * @param [out]	pillarNamesFromAllCurves	An output containing the instrument pillar names which were bumped
		 * @param [out]	uniquePillarNamesInSet		An output containing the unique instrument names across all curves that were bumped
		*/
		void calculateDeltaLadderAndProcessResults( DeltaGenerator& riskGen,
													const std::vector<AQLString>& miniPortfolioTradeIDs,
													std::map< std::pair<AQLString, AQLString>, double>& deltaMap,
													std::vector<AQLString>& pillarNamesFromAllCurves,
													std::set<AQLString>& uniquePillarNamesInSet );

        std::vector<BaseInstrumentPtr> portfolio_;

		// For LWO Swaps
		AQLStringVector lwoSwapNames_;
		std::vector<std::shared_ptr<Swap> > lwoPortfolio_;
		std::vector<LabelValueBlock> fixingTableNames_;
		std::vector<LabelValueBlock> curveCollections_;
        std::vector<LabelValueBlock> valuationSettingsLVB_;

		// Set to true if the portfolio contains LWO Swaps
		bool usingLWO_;

		// Stores the CurveDependencies for a portfolio of BaseInstrument Swaps
        std::vector<CurveDependencies> forecastAndDiscountCurves_;

        AQLStringVector allYieldCurves_;

        LabelValueBlock pricingParams_;

        AQLString curveCollectionID_;

		bool bumpSpreadInstruments_;

		double bumpSize_;

        AQLString bumpMode_;

        bool aggregateRisks_;

		bool reportInLegCCY_;

		bool isCalcDeltaByLeg_;

		bool useGlobalCurveEngine_;

        // Valuation Settings
        // ------------------------------------------------

        // Valuation Settings Struct - Will be potentially extended to hold more information
        struct ValuationSettings
        {
            double xccyFXSpotRate_;
			double xccyFXAsOfDateRate_;
		};

        // Trade - Valuation Settings Map
        std::map<AQLString, ValuationSettings> tradeValuationSettingsMap_;

        // Helpers
        double getXccyFXSpotRateByTradeID( const AQLString & tradeID )
        {
            const double xccyFXSpotRate = tradeValuationSettingsMap_.find( tradeID )->second.xccyFXSpotRate_;
            return xccyFXSpotRate;
        }

        std::vector<double> getXccyFXSpotRatesByTradeIDs( const std::vector<AQLString> tradeIDs )
        {
            std::vector<double> xccyFXSpotRates( tradeIDs.size(), 0.0 );
            for (size_t i = 0; i < tradeIDs.size(); ++i )
            {
                xccyFXSpotRates[i] = getXccyFXSpotRateByTradeID( tradeIDs[i] );
            }
            return xccyFXSpotRates;
        }

		double getXccyFXAsOfDateRateByTradeID(const AQLString & tradeID)
		{
			const double xccyFXAsOfDateRate = tradeValuationSettingsMap_.find(tradeID)->second.xccyFXAsOfDateRate_;
			return xccyFXAsOfDateRate;
		}

		std::vector<double> getXccyFXAsOfDateRatesByTradeIDs(const std::vector<AQLString> tradeIDs)
		{
			std::vector<double> xccyFXAsOfDateRates(tradeIDs.size(), 0.0);
			for (size_t i = 0; i < tradeIDs.size(); ++i)
			{
				xccyFXAsOfDateRates[i] = getXccyFXAsOfDateRateByTradeID(tradeIDs[i]);
			}
			return xccyFXAsOfDateRates;
		}


        // ------------------------------------------------
		// The maximum tenor in years (e.g. 10Y) beyond which the curves are no longer bumped
		std::string riskCutOffTenor_;
    };


}
