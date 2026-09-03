/*
 * @brief			Class that calculates swap delta risk against multiple yield curves
 * @Created:		11 April 2016
 * @Author:			Joseph Ye
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#pragma once
#include "Swap.h"
#include "BaseInstrument.h"
#include "LabelValueBlock.h"
#include "DeltaRiskGenerator.h"
#include "CurveResultsContainer.h"      // New Non-Object Pool Curve Results Objects

#include <boost/shared_array.hpp>
#include "LAString.h"
#include <boost/shared_array.hpp>

class YieldCurvePro;
class LADataMultiReference;
class LADataDouble;
class LADataProcedure;

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
                                  const LAString curveCollection,
								  const bool bumpSpreadInstruments,
                                  const double bumpSize,
                                  const LAString& bumpMode,
                                  const bool aggregateRisks,
								  const std::string& riskCutOffTenor );

		/* @brief	Constructor for LWO Swaps
        *  Note1:   We must disable the CurveResults Object otherwise Products will price outside the object pool and not incorporate curve bumps and shift results
		*/
		MultiCurveDeltaGenerator( const LAStringVector& swapNames,
			                      const LAStringMatrix& curveCollectionNames,
		                          const LAStringMatrix& fixingTableNames,
                                  const DoubleVector& xccyFXSpotRates,
								  const bool bumpSpreadInstruments,
                                  const double bumpSize,
								  const LAString& bumpMode,
                                  const bool aggregateRisks,
								  const bool reportInLegCCY,
								  const std::string& riskCutOffTenor ); 

		~MultiCurveDeltaGenerator() {}

        /* @brief			Set the yield curves required of delta calculation
        *  @param [in]		curveSetID		Name or handle of the curve set
        *  @param [in]		curves			A collection of yield curves
        */
        void setCurves( const LAString& curveCollectionID, const LAStringVector& curves );

        /* @brief			Calculate delta ladder for the given trade(s)
        *  @param [out]		pillarNames		Name of pillar points
        *  @param [out]		headers			Headers of the delta matrix
        *  @param [out]		deltas			All the deltas
        */
        void deltaLadder( LAStringVector& pillarNames, LAStringVector& headers, LAStringVector& deltaCCYs, DoubleMatrix& deltas );

		/* @brief			Calculate flat-shift delta for the given trade(s)
        *  @param [out]		positionIDs		Name of each SwapID / LegID for which the delta is calculated
        *  @param [out]		deltas			The flat-shift delta of each swap leg.
        */
		void flatShiftDelta( LAStringVector& positionIDs, DoubleVector& deltas, const LAString& groupRiskBy);


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
			LAString curveCollectionID_;
			LAString forecastCurve_;
			LAString discountCurve_;

			CurveDependencies() {}
			CurveDependencies(const LAString& curveCollectionID, const LAString& forecastCurve, const LAString& discountCurve)
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
											       LAStringVector& allLegIDs,
											       LAStringVector& allLegCCYs,
											       std::map< CurveDependencies, std::vector< std::shared_ptr<Leg> > >& lwoSwapLegsByCurves,
											       std::map< CurveDependencies, std::vector< LAString > >& lwoTradeIDsByCurves,
											       std::map< CurveDependencies, std::vector< LAString > >& lwoLegIDsByCurves,
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
												LAStringVector& allTradeIDs,
												LAStringVector& allTradeCcys,
												std::map< CurveDependencies, std::vector< SwapPtr > >& lwoSwapByCurves,
												std::map< CurveDependencies, std::vector< LAString > >& lwoTradeIDsByCurves,
												std::map< CurveDependencies, std::vector< LabelValueBlock > >& fixingTableNamesByCurves);

		/* @brief		Invokes the delta risk generator on a mini portfolio to calculate the delta-ladder, and post-processes the results
		 * @param [in]	riskGen						The DeltaGenerator which will calculate the risk for a given miniPortfolio
		 * @param [in]	miniPortfolioTradeIDs		A vector of tradeIDs from the miniPortfolio
		 * @param [out] deltaMap					An output containing the delta sensitivities. Each element in 'deltaMap' is indexed by the combination of a pillar name and a trade ID.
		 * @param [out]	pillarNamesFromAllCurves	An output containing the instrument pillar names which were bumped
		 * @param [out]	uniquePillarNamesInSet		An output containing the unique instrument names across all curves that were bumped
		*/
		void calculateDeltaLadderAndProcessResults( DeltaGenerator& riskGen,
													const std::vector<LAString>& miniPortfolioTradeIDs,
													std::map< std::pair<LAString, LAString>, double>& deltaMap,
													std::vector<LAString>& pillarNamesFromAllCurves,
													std::set<LAString>& uniquePillarNamesInSet );

        std::vector<BaseInstrumentPtr> portfolio_;

		// For LWO Swaps
		LAStringVector lwoSwapNames_;
		std::vector<std::shared_ptr<Swap> > lwoPortfolio_;
		std::vector<LabelValueBlock> fixingTableNames_;
		std::vector<LabelValueBlock> curveCollections_;
        std::vector<LabelValueBlock> valuationSettingsLVB_;

		// Set to true if the portfolio contains LWO Swaps
		bool usingLWO_;

		// Stores the CurveDependencies for a portfolio of BaseInstrument Swaps
        std::vector<CurveDependencies> forecastAndDiscountCurves_;

        LAStringVector allYieldCurves_;

        LabelValueBlock pricingParams_;

        LAString curveCollectionID_;

		bool bumpSpreadInstruments_;

		double bumpSize_;

        LAString bumpMode_;

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
        std::map<LAString, ValuationSettings> tradeValuationSettingsMap_;

        // Helpers
        double getXccyFXSpotRateByTradeID( const LAString & tradeID )
        {
            const double xccyFXSpotRate = tradeValuationSettingsMap_.find( tradeID )->second.xccyFXSpotRate_;
            return xccyFXSpotRate;
        }

        std::vector<double> getXccyFXSpotRatesByTradeIDs( const std::vector<LAString> tradeIDs )
        {
            std::vector<double> xccyFXSpotRates( tradeIDs.size(), 0.0 );
            for (size_t i = 0; i < tradeIDs.size(); ++i )
            {
                xccyFXSpotRates[i] = getXccyFXSpotRateByTradeID( tradeIDs[i] );
            }
            return xccyFXSpotRates;
        }

		double getXccyFXAsOfDateRateByTradeID(const LAString & tradeID)
		{
			const double xccyFXAsOfDateRate = tradeValuationSettingsMap_.find(tradeID)->second.xccyFXAsOfDateRate_;
			return xccyFXAsOfDateRate;
		}

		std::vector<double> getXccyFXAsOfDateRatesByTradeIDs(const std::vector<LAString> tradeIDs)
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
