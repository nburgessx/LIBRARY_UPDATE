/*
 * @brief			Class that calculates swap delta risk against multiple yield curves
 * @Created:		03 June 2016
 * @Author:			Joseph Ye
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#include "MultiCurveDeltaRiskGenerator.h"
#include "LWOUtilities.h"
#include "CommonConstants.h"
#include "LabelValueBlock.h"
#include "TypeHelpers.h"
#include "VanillaInterestRateSwap.h"
#include "OISSwap.h"
#include "CurveValidation.h"
#include "LabelValueBlockValidation.h"
#include "ScheduleValidation.h"
#include "SwapValidation.h"
#include "SwapUtilities.h"
#include "ParameterValidation.h"
#include "InitializeMLibETrading.h"
#include "CurveCalibrationData.h"
#include "SettingsValidation.h"
#include "DataUtilities.h"      // for MLIB_TO_STRING macros
#include "RiskUtilities.h"		// includes utilities to manage curve lists where we have multiple forecast curves
#include <iterator>
#include <numeric>
#include <boost/algorithm/string.hpp>

namespace etrading
{
	// Helper Method to check if any curves were built using the curve engine
	// If any curve was built from global curve engine, set useGlobalCurveEngine_ to true
	bool wereCurvesBuiltUsingCurveEngine( const LAString & curveCollection, const LAString & forecastCurve, const LAString & discountCurve )
	{
		bool builtUsingCurveEngine = false;

		// We have a single discount curve, but can have multiple forecast curves e.g. VNS and custom schedule swaps can
		// have multiple float indices per trade leg
		bool isDiscountCurveFromEngine = etrading::isCurveBuiltFromCurveEngine( curveCollection, discountCurve );

		std::vector<LAString> forecastCurveList = generateCurveList( forecastCurve );
		for( LAString thisForecastCurve : forecastCurveList )
		{
			bool isForecastCurveFromEngine = etrading::isCurveBuiltFromCurveEngine( curveCollection, thisForecastCurve );
			
			// It is sufficient to check that all forecast curves were built the same way as the discount curve
			// i.e. all curves were built the same way.
			if( isForecastCurveFromEngine && isDiscountCurveFromEngine )
			{
				builtUsingCurveEngine = true;
			}
			else
			{
				if( isForecastCurveFromEngine != isDiscountCurveFromEngine )
				{
					MLIB_THROW( "Both forecast curve and discount curve must be consistently calibrated by either using yield curve engine or using single-curve calibration methods" )
				}
			}
		}

		return builtUsingCurveEngine;
	}

    /* @brief	Constructor
    */
    MultiCurveDeltaGenerator::MultiCurveDeltaGenerator()
    {}

    /* @brief	Constructor for LabelValueBlock and BaseInstrument
    *  Note1:   Base Instrument Does not support Xccy Swaps
    *  Note2:   We must disable the CurveResults Object otherwise Products will price outside the object pool and not incorporate curve bumps and shift results
    */
    MultiCurveDeltaGenerator::MultiCurveDeltaGenerator( const std::vector<LabelValueBlock>& dealsInfo,
														const LAString curveCollection,
														const bool bumpSpreadInstruments,
														const double bumpSize,
														const LAString& bumpMode,
														const bool aggregateRisks,
														const std::string& riskCutOffTenor )
															:
																usingLWO_( false ),
																bumpSpreadInstruments_( bumpSpreadInstruments ),
																bumpSize_( bumpSize ),
																bumpMode_( bumpMode ),
																aggregateRisks_( aggregateRisks ),
																riskCutOffTenor_( riskCutOffTenor ),
                                                                reportInLegCCY_( false )
    {
        // Build the trade portfolio
        for ( size_t i = 0; i < dealsInfo.size(); ++i )
        {
            LabelValueBlock deal = dealsInfo[i];

            LAString tradeType = deal.getCompulsoryValueAsLAString( IRS_KEY::TRADE_TYPE );
            LAString tradeID = deal.getCompulsoryValueAsLAString( IRS_KEY::TRADE_ID );
            LAString forecastCurve = deal.getCompulsoryValueAsLAString( MARKET_KEY::FORECAST_CURVE, tradeID.getCString(), false );
            LAString discountCurve = deal.getCompulsoryValueAsLAString( MARKET_KEY::DISCOUNT_CURVE, tradeID.getCString(), false );

            //Throw exception if the curve has not been built and return the Market Names i.e. PropertyFileName /  StaticDataTable
            LAString forecastCurveMarketName = etrading::getCurveStaticDataTableName( curveCollection, forecastCurve, false ); // false = don't convert marketName to Uppercase
            LAString discountCurveMarketName = etrading::getCurveStaticDataTableName( curveCollection, discountCurve, false ); // false = don't convert marketName to Uppercase

            // Record the trade object pointer
            BaseInstrumentPtr trade;
            if ( boost::iequals( tradeType.getCString(), etrading::TRADE_TYPE_KEY::TRADETYPE_VANILLA_IRS.c_str() ) )			// Vanilla IRS
            {
                trade = BaseInstrumentPtr( new VanillaInterestRateSwap( deal ) );
            }
            else if ( boost::iequals( tradeType.getCString(), etrading::TRADE_TYPE_KEY::TRADETYPE_OIS.c_str() ) )		// OIS Swap
            {
                trade = BaseInstrumentPtr( new OISSwap( deal ) );
            }
            else
            {
                throw LACoreInvalidData( "#Error: TradeType key is either not specified or carries invalid value", __FILE__, __LINE__ );
            }

            portfolio_.push_back( trade );

            // Record the Discount and Forecast Curves i.e. Market Data Names
            forecastAndDiscountCurves_.push_back( CurveDependencies( curveCollection, forecastCurveMarketName, discountCurveMarketName ) );
        }
    }

	/* @brief	Constructor for LWO Swaps
    *  Note1:   We must disable the CurveResults Object otherwise Products will price outside the object pool and not incorporate curve bumps and shift results
	*/
	MultiCurveDeltaGenerator::MultiCurveDeltaGenerator( const LAStringVector& swapNames,
														const LAStringMatrix& curveCollectionNames,
														const LAStringMatrix& fixingTableNames,
                                                        const DoubleVector& xccyFXSpotRates,
														const bool bumpSpreadInstruments,
														const double bumpSize,
														const LAString& bumpMode,
														const bool aggregateRisks,
														const bool reportInLegCCY,
														const std::string& riskCutOffTenor )
															:	
																usingLWO_( true ),
																lwoSwapNames_( swapNames ),
																bumpSpreadInstruments_( bumpSpreadInstruments ),
																bumpSize_( bumpSize ),
																bumpMode_( bumpMode ),
																aggregateRisks_( aggregateRisks ),
																reportInLegCCY_( reportInLegCCY ),
																riskCutOffTenor_( riskCutOffTenor ),
                                                                isCalcDeltaByLeg_(false),
																useGlobalCurveEngine_(false)
	{
		// At minimum a curveCollection should be provided for each swap
		MLIB_REQUIRE( swapNames.size() == curveCollectionNames.size(), "Inconsistent Data: The number of Swap Trades and Curve Collections does not match" )
        
		// Fixing tables are optional, but if present the range should be the same size as the number of swaps
		const bool usingFixingTables = ( fixingTableNames.size() > 0 ) ? true : false;
        if ( usingFixingTables )
		{
            MLIB_REQUIRE( swapNames.size() == fixingTableNames.size(), "Inconsistent Data: The number of Swap Trades and Fixing Tables does not match" )
		}
        
        // Xccy FX Spot Rates are also optional, but if present the range should be the same size as the number of swaps
        const bool usingXccyFXSpotRates = ( xccyFXSpotRates.size() > 0 ) ? true : false;
        if ( usingXccyFXSpotRates )
        {
            MLIB_REQUIRE( swapNames.size() == xccyFXSpotRates.size(), "Inconsistent Data: The number of Swap Trades and Xccy FX Spot Rates does not match" )
        }

		std::vector<double> asOfDateFxRates(swapNames.size());

        // Build the swap portfolio
		for (size_t i=0; i<swapNames.size(); i++)
		{
			// Gracefully handle the case where one or more swapNames is missing from the range
			const LAString& swapName = swapNames[i];
			if ( ! swapName.isDefined() || swapName == "" )
				continue;

			auto swap = etrading::getSwap( swapName.getCString() );
			lwoPortfolio_.push_back( swap );
			
			// We only calculate delta by swap legs for non-vanilla swaps
			SwapTypeEnum swapType = swap->getSwapType();
			if (swapType == CROSS_CURRENCY_SWAP || swapType == XCCY_ZERO_COUPON_SWAP)
			{
				isCalcDeltaByLeg_ = true;
			}
			else if (swapType == VANILLA_SWAP)
			{
				if (swap->getLegSize() == 2)
				{
					if (swap->getLeg(0)->getType() == FLOAT_SCHEDULE_TYPE && swap->getLeg(1)->getType() == FLOAT_SCHEDULE_TYPE)
					{
						isCalcDeltaByLeg_ = true;
					}
				}
			}
			
			/* Process CurveCollections for this trade
			 * The CurveCollectionLVB can have two forms:
			 * 1.
			 *    legName1,   curveCollection1
			 *    legName2,   curveCollection2
			 * OR
			 * 2. curveCollection,  ""
			 * i.e. if only a single curveCollection is given, it applies to all legs in the swap
			 */
			LabelValueBlock curveCollectionsLVB;
            
			LAStringVector curveCollectionsForTrade = curveCollectionNames[i];
			size_t collectionCount = 0;

            for (size_t j=0; j<curveCollectionsForTrade.size(); j++)
			{
				// Check how many non-blank collections have been provided
				if (curveCollectionsForTrade[j].isDefined() && curveCollectionsForTrade[j].size() > 0)
				{
					collectionCount++;
				}
			}

            // 1. Update the Curve Collections for Risk Calculations and Curve Dependency Management

            // Xccy Swap Check: Xccy Swaps Require > 1 Curve Collection
            // --------------------------------------------------------
			if ( collectionCount > 1 )
			{
                const size_t numLegs = swap->getLegSize();
				LAStringVector legNames( numLegs );
                LAStringVector curveNames( numLegs );

                // Two or more curve collections have been given
				for (size_t j = 0; j<collectionCount; j++)
				{
					if ( j >= numLegs )
					{
						// Only include as many curve collections as there are swap legs
						break;
					}
					legNames[j]     = swap->getLeg(j)->getLegName();
                    curveNames[j]   = curveCollectionsForTrade[j];
				}
                // Create new updated LVB
                curveCollectionsLVB = LabelValueBlock( legNames, curveNames );
			}
			else
			{
				// A single curve collection has been given
				curveCollectionsLVB = LabelValueBlock( curveCollectionsForTrade[0], LAString("") );
			}
            
            curveCollections_.push_back( curveCollectionsLVB );
			

            // 2. Update the Valuation Settings, which contains Curve Collections, Xccy FX Spot Rates and Other Pricing Data
            
            // Copy Curve Collection Data and Append Extra Valuation Data as Needed
            valuationSettingsLVB_ = curveCollections_;

            LAString fxSpotKey;
            LAString fxSpotValue;

            // Xccy Swap Check: Xccy Swaps Require > 1 Curve Collection
            // --------------------------------------------------------
			if ( collectionCount > 1 && !swap->isSingleCurrency() )
			{
                // Access Violation & Pricing Consistency Check
                MLIB_REQUIRE( xccyFXSpotRates.size() > 0, "Xccy FX Spot Rates Required for Xccy Swaps & trades with multiple curve collections" )
                
                // Xccy Swaps Require FXSpot Rate from Valuation Settings - Only add one FX_SPOT per trade
                fxSpotKey    = VALUATION_SETTING_KEYS::FX_SPOT;

                // Use index i, since Swap Name Count = Xccy FX Spot Rate Count
                fxSpotValue  = MLIB_TO_STRING_FROM_DOUBLE( xccyFXSpotRates[i] ); 
            }
            else
            {
                // Non-Xccy Swaps Do Not Require FXSpot Rate from Valuation Settings, so we default set the FXSpot to 1.0
                fxSpotKey    = VALUATION_SETTING_KEYS::FX_SPOT;
                fxSpotValue  = MLIB_TO_STRING_FROM_DOUBLE( 1.0 );
            }

            // Update Valuation Settings LVB member variable
            LabelValueBlock valulationSettingsLVB( curveCollectionsLVB, fxSpotKey, fxSpotValue );
            valuationSettingsLVB_.push_back( valulationSettingsLVB );
            
			/* Process FixingTables for this trade
			 * The FixingTableLVB can have two forms:
			 * 1.
			 *    legName1,   fixingTable1
			 *    legName2,   fixingTable2
			 * OR
			 * 2. fixingTable,  ""
			 * i.e. if only a single fixingTable is given, it applies to all legs in the swap
			 */
			LabelValueBlock fixingTablesLVB;
            
			if ( usingFixingTables )
			{
				LAStringVector fixingTablesForTrade = fixingTableNames[i];
				size_t fixingTableCount = 0;

				for (size_t j=0; j<fixingTablesForTrade.size(); j++)
				{
					// Check how many non-blank fixingTables have been provided
					if (fixingTablesForTrade[j].isDefined() && fixingTablesForTrade[j].size() > 0)
					{
						fixingTableCount++;
					}
				}

				// 1. Update the Fixing Tables for Risk Calculations and Curve Dependency Management

				// Xccy or tenor basis Swap Check: These Swaps Require > 1 Fixing Tables
				// --------------------------------------------------------
				if ( fixingTableCount > 1 )
				{
					const size_t numLegs = swap->getLegSize();
					LAStringVector legNames( numLegs );
					LAStringVector fixingNames( numLegs );

					// Two or more fixing tables have been given
					for (size_t j = 0; j<fixingTableCount; j++)
					{
						if ( j >= numLegs )
						{
							// Only include as many fixing tables as there are swap legs
							break;
						}
						legNames[j]     = swap->getLeg(j)->getLegName();
						fixingNames[j]  = fixingTablesForTrade[j];
					}
					// Create new updated LVB
					fixingTablesLVB = LabelValueBlock( legNames, fixingNames );
				}
				else
				{
					// A single fixing table has been given
					fixingTablesLVB = LabelValueBlock( fixingTablesForTrade[0], LAString("") );
				}
            }
			fixingTableNames_.push_back( fixingTablesLVB );

            // Xccy Fixing Spot Rates

			// Now call PV on each swap in the portfolio.
			// This serves two purposes:
			// 1. Validate all the input data and fail early if there is a problem
			// 2. Setup of the swaps (in the case of MTM XCCY, which adjusts notional exchanges).
			LAString legName = "";
			swap->pv(valulationSettingsLVB, fixingTablesLVB, legName);

			if (swap->getSwapType() == CROSS_CURRENCY_SWAP || swap->getSwapType() == XCCY_ZERO_COUPON_SWAP)
			{
				asOfDateFxRates[i] = swap->getFxAsOfDateRate(valulationSettingsLVB);
			}
			else
			{
				asOfDateFxRates[i] = 1.0;
			}

		}

		// Update Trade Valulation Settings Map
		for (size_t i = 0; i < swapNames.size(); ++i)
		{
			ValuationSettings thisTradeValuationSettings;

			if (usingXccyFXSpotRates)
			{
				thisTradeValuationSettings.xccyFXSpotRate_ = xccyFXSpotRates[i];
				thisTradeValuationSettings.xccyFXAsOfDateRate_ = asOfDateFxRates[i];
			}
			else
			{
				thisTradeValuationSettings.xccyFXSpotRate_ = 1.0; // Default Value
				thisTradeValuationSettings.xccyFXAsOfDateRate_ = 1.0; // Default Value
			}

			// Update Trade-ValuationSettings Map
			tradeValuationSettingsMap_[swapNames[i]] = thisTradeValuationSettings;
		}
    }


    /* @brief			Set the yield curves required of delta calculation
    *  @param [in]		curveCollectionID		Name or handle of the curve set
    *  @param [in]		curves					A collection of yield curves
    */
    void MultiCurveDeltaGenerator::setCurves( const LAString& curveCollectionID, const LAStringVector& curves )
    {
        curveCollectionID_ = curveCollectionID;
        allYieldCurves_.clear();
        for ( size_t i = 0; i < curves.size(); ++i )
        {
            LAString curve = curves[i];

            // Check curve exists and get Curve Market Name
            LAString curveMarketName = etrading::getCurveStaticDataTableName( curveCollectionID, curve, false ); // false = don't convert marketName to Uppercase

            allYieldCurves_.push_back( curveMarketName.toLower() );
        }
    }

	/* @brief		Returns the size of the trade portfolio
	*				Examines portfolio_ or lwoPortfolio_ depending on whether LWO Swaps are being used
	*/
	size_t MultiCurveDeltaGenerator::getPortfolioSize()
	{
		if (usingLWO_)
		{
			return lwoPortfolio_.size();
		}
		else
		{
			return portfolio_.size();
		}
	}

	/* @brief		Examines the lwoTrade input and adds the trade data to a map based on the curve dependencies required to PV that leg
	* @param [in]	lwoTrade					The LWO Swap to be processed
	* @param [in]	curveCollectionForTrade		A LabelValueBlock containing the curve collections required to PV this swap
	* @param [out]	allTradeIDs					An output which accumulates the trade IDs that have been processed so far
	* @param [out]	allTradeCCYs				An output which accumulates the trade native currencies
	* @param [out]  fixingTableForTrade			A fixingTableMap containing the fixingTableNames required to PV this swap
	* @param [out]	lwoSwapLegsByCurves			An output map containing mini-portfolios of swap legs, keyed by CurveDependencies
	* @param [out]	fixingTableNamesByCurves		An output map containing mini-portfolios of swap fixingTableNames, keyed by CurveDependencies.
	*/
	void MultiCurveDeltaGenerator::groupLWOTradesByCurveDependencies(const SwapPtr& lwoTrade,
																	const LabelValueBlock& curveCollectionForTrade,
																	const LabelValueBlock& fixingTableForTrade,
																	LAStringVector& allTradeIDs,
																	LAStringVector& allTradeCcys,
																	std::map< CurveDependencies, std::vector< SwapPtr > >& lwoSwapByCurves,
																	std::map< CurveDependencies, std::vector< LAString > >& lwoTradeIDsByCurves,
																	std::map< CurveDependencies, std::vector< LabelValueBlock > >& fixingTableNamesByCurves)
	{
		auto swapID = lwoTrade->getRefToName().c_str();
		allTradeIDs.push_back(swapID);

		// Iterate over swap legs to determine curve requirements
		for (size_t j = 0; j<lwoTrade->getLegSize(); j++)
		{
			const LegPtr& leg = lwoTrade->getLeg(j);

			if (leg->getType() == FLOAT_SCHEDULE_TYPE)
			{
				LAString curveCollectionID = getLWOCurveCollectionFromValuationSettings(curveCollectionForTrade, leg->getLegName());
				LAString discountCurveIndex = leg->getStaticData()->getDiscountCurve();
				LAString discountCurve = getCurveStaticDataTableName(curveCollectionID, discountCurveIndex, false);

				auto forecastCurveIndex = leg->getStaticData()->getForecastCurve();
				
				// Convert the curveIndex to the static table name
				// Note: The forecast curve may be a list of curves, e.g. when pricing a VNS trade with multiple curve indices per trade leg
				LAString forecastCurveAsStaticDataList = generateStaticDataListAsString( curveCollectionID, forecastCurveIndex );
				
				// If any curve was built from global curve engine, set useGlobalCurveEngine_ to true
				if (!useGlobalCurveEngine_)
				{
					// **** IMPORTANT ****
					//Must use the forecast index here!!!
					useGlobalCurveEngine_ = wereCurvesBuiltUsingCurveEngine( curveCollectionID, forecastCurveIndex, discountCurveIndex );
				}
				
				// Put this swap leg in a map that is indexed by its curveDependencies	
				// **** IMPORTANT ****
				// Must use the static data table name of curve index here!!!
				CurveDependencies key( curveCollectionID, forecastCurveAsStaticDataList, discountCurve );

				// Look up the portfolio of swaps which share these CurveDependencies
				// If this swap has new dependencies, this will insert an entry into the map.
				std::vector<SwapPtr >& lwoSwaps = lwoSwapByCurves[key];
				lwoSwaps.push_back(lwoTrade);

                // TODO - Is the fixing table needed to check curve dependencies ??? Should xccyFXSpotRates be here also ???
				std::vector< LabelValueBlock >& lwoFixingTables = fixingTableNamesByCurves[key];
				lwoFixingTables.push_back(fixingTableForTrade);
				std::vector<LAString>& tradeIDs = lwoTradeIDsByCurves[key];
				tradeIDs.push_back(swapID);

				const CCY legCCY = leg->getStaticData()->getCurrency();
				allTradeCcys.push_back(toString(legCCY).c_str());
			}
		}
	}

	/* @brief		Examines the lwoTrade input and adds the trade leg data to a map based on the curve dependencies required to PV that leg
	* @param [in]	lwoTrade					The LWO Swap to be processed
	* @param [in]	curveCollectionForTrade		A LabelValueBlock containing the curve collections required to PV this swap
	* @param [out]  fixingTableForTrade			A map containing the fixingTableNames required to PV this swap
	* @param [out]	allLegIDs					An output which accumulates the legIDs that have been processed so far
	* @param [out]	allLegCCYs					An output which accumulates the Leg native currencies
	* @param [out]	lwoSwapLegsByCurves			An output map containing mini-portfolios of swap legs, keyed by CurveDependencies
	* @param [out]	lwoLegIDsByCurves			An output map containing mini-portfolios of swap legIDs, keyed by CurveDependencies
	* @param [out]	fixingTableNamesByCurves		An output map containing mini-portfolios of swap fixingTableNames, keyed by CurveDependencies.
	*/
	void MultiCurveDeltaGenerator::groupLWOTradeLegsByCurveDependencies( const std::shared_ptr<Swap>& lwoTrade,
																	 const LabelValueBlock& curveCollectionForTrade,
																	 const LabelValueBlock& fixingTableForTrade,
																	 LAStringVector& allLegIDs,
																	 LAStringVector& allLegCCYs,
																	 std::map< CurveDependencies, std::vector< std::shared_ptr<Leg> > >& lwoSwapLegsByCurves,
																	 std::map< CurveDependencies, std::vector< LAString > >& lwoTradeIDsByCurves,
																	 std::map< CurveDependencies, std::vector< LAString > >& lwoLegIDsByCurves,
																	 std::map< CurveDependencies, std::vector< LabelValueBlock > >& fixingTableNamesByCurves)
	{
		auto swapID = lwoTrade->getRefToName().c_str();

		// Iterate over swap legs to determine curve requirements
		for (size_t j=0; j<lwoTrade->getLegSize(); j++)
		{
			const LegPtr& leg = lwoTrade->getLeg(j);
			auto legID = swapID + LAString("_") + leg->getLegName();
			allLegIDs.push_back( legID );

			// Get the currency that the risk will be reported in
			const LegStaticDataPtr&  legStaticData = leg->getStaticData();

			const CCY valuationCCY = legStaticData->getValuationCurrency();
			const CCY legCCY       = legStaticData->getCurrency();

			CCY reportingCCY = NO_CCY;
			if ( reportInLegCCY_ )
			{
				reportingCCY = legCCY;
			}
			else
			{
				if (valuationCCY != NO_CCY)
				{
					reportingCCY = valuationCCY;
				}
				else
				{
					// Not all swaps will have ValuationCCY set.
					// If this field is missing, use the Leg CCY.
					reportingCCY = legCCY;
				}
			}
			if ( reportingCCY == NO_CCY)
			{
				throw LACoreInvalidData( ( boost::format( "#Error: Could not determine the currency for swap leg: %s" )
										% legID.getCString() ).str().c_str() , __FILE__, __LINE__ );
			}
			allLegCCYs.push_back( toString( reportingCCY ).c_str() );

			LAString curveCollectionID = getLWOCurveCollectionFromValuationSettings( curveCollectionForTrade, leg->getLegName() );
			LAString discountCurveIndex = leg->getStaticData()->getDiscountCurve();
			LAString discountCurve = getCurveStaticDataTableName( curveCollectionID, discountCurveIndex, false );
				
			LAString forecastCurveIndex;
			LAString forecastCurve;
			if (leg->getType() == FLOAT_SCHEDULE_TYPE)
			{
				forecastCurveIndex = leg->getStaticData()->getForecastCurve();

				// Convert the curveIndex to the static table name
				forecastCurve = getCurveStaticDataTableName(curveCollectionID, forecastCurveIndex, false);
			}
			else
			{
				// This is a Fixed leg or fee leg, which only depends on a discount curve.
				// For efficiency while calculating risk we wish to group this leg with OIS float legs
				// (which specify the OIS curve as both the discount and forecast dependency)
				forecastCurve = discountCurve;
				forecastCurveIndex = forecastCurve;
			}
				
			// If any curve was built from global curve engine, set useGlobalCurveEngine_ to true
			if( !useGlobalCurveEngine_ )
			{
				// *** IMPORTANT *** Use Curve Index Here (not static data table name)
				useGlobalCurveEngine_ = wereCurvesBuiltUsingCurveEngine( curveCollectionID, forecastCurveIndex, discountCurveIndex );
			}

			// Put this swap leg in a map that is indexed by its curveDependencies			
			// *** IMPORTANT *** Use Static Data table name here (not curve index name)
			CurveDependencies key(curveCollectionID, forecastCurve, discountCurve);
			
			// Look up the portfolio of swapLegs which share these CurveDependencies
			// If this leg has new dependencies, this will insert an entry into the map.
			std::vector<std::shared_ptr<Leg> >& lwoSwapLegs = lwoSwapLegsByCurves[key];
			lwoSwapLegs.push_back( leg );

            // TODO - Is the fixing table needed to check curve dependencies ??? Should xccyFXSpotRates be here also ???
			std::vector< LabelValueBlock >& lwoFixingTables = fixingTableNamesByCurves[key];
			lwoFixingTables.push_back(fixingTableForTrade);
			std::vector<LAString>& tradeIDs = lwoTradeIDsByCurves[key];
			tradeIDs.push_back( swapID );

			std::vector<LAString>& legIDs = lwoLegIDsByCurves[key];
			legIDs.push_back( legID );
		}
	}

    /* @brief			Calculate delta ladder of the
    *  @param [out]		pillarNames		Name of pillar points
    *  @param [out]		headers			Headers of the delta matrix
    *  @param [out]		deltas			All the deltas
    */
    void MultiCurveDeltaGenerator::deltaLadder( LAStringVector& pillarNames, LAStringVector& headers, LAStringVector& deltaCCYs, DoubleMatrix& deltas )
    {
        pillarNames.clear();
        deltas.clear();
        headers.clear();
		deltaCCYs.clear();

        // Exit if no trades are provided
        if ( getPortfolioSize() == 0 )
        {
            return;
        }

        // Group instruments in the given portfolio by their common curve dependencies

		// These maps store dependency information for portfolios of LWO SwapLegs
		std::map< CurveDependencies, std::vector< LegPtr > > lwoSwapLegsByCurves;
		std::map< CurveDependencies, std::vector< SwapPtr > > lwoSwapsByCurves;
		std::map< CurveDependencies, std::vector< LAString > > lwoTradeIDsByCurves;
		std::map< CurveDependencies, std::vector< LAString > > lwoLegIDsByCurves;
		std::map< CurveDependencies, std::vector< LabelValueBlock > > fixingTableNamesByCurves;

		// These maps store dependency information for portfolios of BaseInstrument
        std::map< CurveDependencies, std::vector<BaseInstrumentPtr> > instrumentsByCurves;
        std::map< CurveDependencies, LAString > interpolations;
		std::map< CurveDependencies, std::vector< LAString > > tradeIDsByCurves;

        LAStringVector allTradeIDs;
        for ( size_t i = 0; i < getPortfolioSize(); ++i )
        {
			LAString forecastCurve;
			LAString discountCurve;

			if ( usingLWO_ )
			{
				auto lwoTrade = lwoPortfolio_[i];
				auto curveCollectionForTrade = curveCollections_[i];
				auto fixingTableForTrade = fixingTableNames_[i];

				if (isCalcDeltaByLeg_)
				{
					groupLWOTradeLegsByCurveDependencies(lwoTrade, curveCollectionForTrade, fixingTableForTrade, allTradeIDs, deltaCCYs, lwoSwapLegsByCurves, lwoTradeIDsByCurves, lwoLegIDsByCurves, fixingTableNamesByCurves);
				}
				else
				{
					groupLWOTradesByCurveDependencies(lwoTrade, curveCollectionForTrade, fixingTableForTrade, allTradeIDs, deltaCCYs, lwoSwapsByCurves, lwoTradeIDsByCurves, fixingTableNamesByCurves);
				}
			}
			else
			{
				BaseInstrumentPtr trade = portfolio_[i];
				LAString tradeID = trade->getTradeID();
				allTradeIDs.push_back( tradeID );
				forecastCurve = forecastAndDiscountCurves_[i].forecastCurve_;
				discountCurve = forecastAndDiscountCurves_[i].discountCurve_;

				// For BaseInstrumentPtr trades there is no currency field.
				// Use the curveCullectionID_ as a reasonable placeholder.
				deltaCCYs.push_back( curveCollectionID_ );

				LAString forecastCurveLower = forecastCurve;
				forecastCurveLower.toLower();
				auto pos = std::find( allYieldCurves_.begin(), allYieldCurves_.end(), forecastCurveLower );
				if ( pos == allYieldCurves_.end() )
				{
					LAString err = "#Error: Forecast curve for instrument '" + tradeID + "' is not found in the given group of yield curves";
					throw LACoreInvalidData( err.getCString(), __FILE__, __LINE__ );
				}

				LAString discountCurveLower = discountCurve;
				discountCurveLower.toLower();
				pos = std::find( allYieldCurves_.begin(), allYieldCurves_.end(), discountCurveLower );
				if ( pos == allYieldCurves_.end() )
				{
					LAString err = "#Error: Discount curve for instrument '" + tradeID + "' is not found in the given group of yield curves";
					throw LACoreInvalidData( err.getCString(), __FILE__, __LINE__ );
				}

				// Validate the forecast and discounting curves
				etrading::validateStringEmptiness( forecastCurve, "#Error: The Swap 'forecast Curve' must be specified." );

				if( discountCurve == LAString( "" ) )
				{
					throw LACoreInvalidData( "#Error: The Swap 'discount Curve' must be specified.", __FILE__, __LINE__ );
				}

				// Put this trade in a map that is indexed by the pairing of its forecast curve and discount curve
				CurveDependencies key( curveCollectionID_, forecastCurve, discountCurve );
				std::vector<BaseInstrumentPtr>& trades = instrumentsByCurves[key];
				trades.push_back( trade );

				std::vector<LAString>& tradeIDs = tradeIDsByCurves[key];
				tradeIDs.push_back( tradeID );

				// Interpolations
				LAString interp = etrading::getCurveInterpolation( curveCollectionID_, forecastCurve );
				interpolations[key] = interp;
			}
        }

        //-----------------------------------------------------------------------------------------------
        // Iterate over each mini portfolio and calculate their respective delta ladder
        // Each mini portfolio contain trades that use the same forecast and discounting curves

        std::vector<LAString> pillarNamesFromAllCurves;
        std::set<LAString> uniquePillarNamesInSet;

        // Each element in 'deltaMap' is indexed by the combination of a pillar name and a trade ID.
        // This works similar to a coordinate system where the pillar names form the y axis and the trade IDs form the x axis
        std::map< std::pair<LAString, LAString>, double> deltaMap;

		if (usingLWO_)
		{
			if (isCalcDeltaByLeg_)
			{
				for (auto iter = lwoSwapLegsByCurves.begin(); iter != lwoSwapLegsByCurves.end(); ++iter)
				{
					const CurveDependencies& key = iter->first;
					std::vector<std::shared_ptr<Leg> >& miniPortfolio = iter->second;

					std::vector<LAString>& legIDs = lwoLegIDsByCurves[key];
					std::vector< LabelValueBlock >& fixingTableNames = fixingTableNamesByCurves[key];
                    
                    // TODO: Check and Ensure tradeIDs and legIDs are consistent and unique by key
                    std::vector<LAString>& tradeIDs = lwoTradeIDsByCurves[key];
                    const std::vector<double> xccyFXAsOfDateRates = getXccyFXAsOfDateRatesByTradeIDs( tradeIDs );

					// Run the delta ladder on a mini portfolio where trades share the same forecast and discount curves
					DeltaGenerator riskGen(miniPortfolio, legIDs, fixingTableNames, xccyFXAsOfDateRates, bumpSpreadInstruments_, bumpSize_, bumpMode_, aggregateRisks_, reportInLegCCY_, riskCutOffTenor_, useGlobalCurveEngine_);
					riskGen.setCurves(key.curveCollectionID_, key.forecastCurve_, key.discountCurve_);

					calculateDeltaLadderAndProcessResults(riskGen, legIDs, deltaMap, pillarNamesFromAllCurves, uniquePillarNamesInSet);
				}
			}
			else
			{
				for (auto iter = lwoSwapsByCurves.begin(); iter != lwoSwapsByCurves.end(); ++iter)
				{
					const CurveDependencies& key = iter->first;
					std::vector<SwapPtr >& miniPortfolio = iter->second;

					std::vector<LAString>& tradeIDs = lwoTradeIDsByCurves[key];
					std::vector< LabelValueBlock >& fixingTableNames = fixingTableNamesByCurves[key];
                    
					const std::vector<double> xccyFXAsOfDateRates = getXccyFXAsOfDateRatesByTradeIDs(tradeIDs);

					// Run the delta ladder on a mini portfolio where trades share the same forecast and discount curves
					DeltaGenerator riskGen(miniPortfolio, tradeIDs, fixingTableNames, xccyFXAsOfDateRates, bumpSpreadInstruments_, bumpSize_, bumpMode_, aggregateRisks_, riskCutOffTenor_, useGlobalCurveEngine_);
					riskGen.setCurves(key.curveCollectionID_, key.forecastCurve_, key.discountCurve_);

					calculateDeltaLadderAndProcessResults(riskGen, tradeIDs, deltaMap, pillarNamesFromAllCurves, uniquePillarNamesInSet);
				}
			}
		}
		else
		{
			for( auto iter = instrumentsByCurves.begin(); iter != instrumentsByCurves.end(); ++iter )
			{
				CurveDependencies key = iter->first;
				const std::vector<BaseInstrumentPtr>& miniPortfolio = iter->second;

				std::vector<LAString>& tradeIDs = tradeIDsByCurves[key];
				
				// Run the delta ladder on a mini portfolio where trades share the same forecast and discount curves
				// Note: Non-LWO Base Case does not support Xccy Swaps - No need for Xccy FX Spot Rates here
                DeltaGenerator riskGen( miniPortfolio, tradeIDs, bumpSpreadInstruments_, bumpSize_, bumpMode_, aggregateRisks_, riskCutOffTenor_ );
				riskGen.setCurves( key.curveCollectionID_, key.forecastCurve_, key.discountCurve_ );

				LabelValueBlock pricingParams( etrading::PRICING_PARAMS::INTERPOLATION, interpolations[key].c_str() );
				riskGen.setPricingParams( pricingParams );

				calculateDeltaLadderAndProcessResults( riskGen, tradeIDs, deltaMap, pillarNamesFromAllCurves, uniquePillarNamesInSet );
			}
		}

        // Form the final un-sorted list of pillar names using pillar names from all curves
        for ( size_t i = 0; i < pillarNamesFromAllCurves.size(); ++i )
        {
            LAString pillarName = pillarNamesFromAllCurves[i];
            auto pos = uniquePillarNamesInSet.find( pillarName );
            if ( pos != uniquePillarNamesInSet.end() )
            {
                // Save pillar name for output
                pillarNames.push_back( pillarName );
                uniquePillarNamesInSet.erase( pos );
            }
        }

        // Build the final delta matrix for output for all the trades in the original portfolio
        for ( size_t i = 0; i < pillarNames.size(); ++i )
        {
            LAString pillarName = pillarNames[i];

            DoubleVector allDeltasUnderSinglePillarName;

            for ( size_t j = 0; j < allTradeIDs.size(); ++j )
            {
                LAString tradeID = allTradeIDs[j];

                // Build a delta matrix on a coordinate system defined by pillar name and trade ID
                std::pair<LAString, LAString> deltaKey = std::make_pair( pillarName, tradeID );
                auto iter = deltaMap.find( deltaKey );
                double delta( 0.0 );
                if ( iter != deltaMap.end() )
                {
                    delta = iter->second;
                }
                else
                {
                    delta = 0.0;
                }

                allDeltasUnderSinglePillarName.push_back( delta );
            }

            deltas.push_back( allDeltasUnderSinglePillarName );
        }

        headers = allTradeIDs;
    }

	/* @brief		Invokes the delta risk generator on a mini portfolio and post-processes the results
	 * @param [in]	riskGen						The DeltaGenerator which will calculate the risk for a given miniPortfolio
	 * @param [in]	miniPortfolioTradeIDs		A vector of tradeIDs from the miniPortfolio
	 * @param [out] deltaMap					An output containing the delta sensitivities. Each element in 'deltaMap' is indexed by the combination of a pillar name and a trade ID.
	 * @param [out]	pillarNamesFromAllCurves	An output containing the instrument pillar names which were bumped
	 * @param [out]	uniquePillarNamesInSet		An output containing the unique instrument names across all curves that were bumped
	*/
	void MultiCurveDeltaGenerator::calculateDeltaLadderAndProcessResults( DeltaGenerator& riskGen,
																		  const std::vector<LAString>& miniPortfolioTradeIDs,
																		  std::map< std::pair<LAString, LAString>, double>& deltaMap,
																		  std::vector<LAString>& pillarNamesFromAllCurves,
																		  std::set<LAString>& uniquePillarNamesInSet )
	{
		LAStringVector pillarNamesForTwoCurves;
		DoubleMatrix deltasForTwoCurves;
		riskGen.deltaLadder( pillarNamesForTwoCurves, deltasForTwoCurves );

		// Loop through the current mini portfolio's delta results and index each delta number with
		// the combination of the pillar name and trade ID
		for ( size_t i = 0; i < pillarNamesForTwoCurves.size(); ++i )
		{
			LAString pillarName = pillarNamesForTwoCurves[i];

			for ( size_t j = 0; j < miniPortfolioTradeIDs.size(); j ++ )
			{
				const LAString& tradeID = miniPortfolioTradeIDs[j];

				std::pair<LAString, LAString> deltaKey = std::make_pair( pillarName, tradeID );
				deltaMap[deltaKey] = deltasForTwoCurves[i][j];
			}
		}

		// Keep all the pillar names in one single vector, ignoring duplication for now
		pillarNamesFromAllCurves.insert( pillarNamesFromAllCurves.end(), pillarNamesForTwoCurves.begin(), pillarNamesForTwoCurves.end() );

		// Copy all pillar names into a set which guarantees uniqueness of the pillar names in this set
		std::copy( pillarNamesFromAllCurves.begin(), pillarNamesFromAllCurves.end(), std::inserter( uniquePillarNamesInSet, uniquePillarNamesInSet.end() ) );
	}

	/* @brief			Calculate flat-shift delta for the given trade(s)
    *  @param [out]		positionIDs		Name of each SwapID / LegID for which the delta is calculated
    *  @param [out]		deltas			The flat-shift delta of each swap leg.
    */
	void MultiCurveDeltaGenerator::flatShiftDelta( LAStringVector& positionIDs, DoubleVector& deltas, const LAString& groupRiskBy)
	{
		positionIDs.clear();
        deltas.clear();

		if (! usingLWO_ )
		{
			throw LACoreInvalidData( "#Error: flatShiftDelta is only supported for Light Weight Object Swaps.", __FILE__, __LINE__ );
		}

        // Exit if no trades are provided
        if ( getPortfolioSize() == 0 )
        {
            return;
        }
		
		// Group instruments in the given portfolio by their common curve dependencies

		// These maps store dependency information for portfolios of LWO SwapLegs
		std::map< CurveDependencies, std::vector< LegPtr > > lwoSwapLegsByCurves;
		std::map < CurveDependencies, std::vector< SwapPtr > > lwoSwapsByCurves;
		std::map< CurveDependencies, std::vector< LAString > > lwoTradeIDsByCurves;
		std::map< CurveDependencies, std::vector< LAString > > lwoLegIDsByCurves;
		std::map< CurveDependencies, std::vector< LabelValueBlock> > fixingTableNamesByCurves;

		LAStringVector allLegIDs;
		LAStringVector allTradeIDs;
		LAStringVector deltaCCYs;
        for ( size_t i = 0; i < getPortfolioSize(); ++i )
        {
			LAString forecastCurve;
			LAString discountCurve;

			auto lwoTrade = lwoPortfolio_[i];
			auto curveCollectionForTrade = curveCollections_[i];
			auto fixingTableForTrade = fixingTableNames_[i];
			
			if (isCalcDeltaByLeg_ || groupRiskBy == "LEG")
			{
				groupLWOTradeLegsByCurveDependencies(lwoTrade, curveCollectionForTrade, fixingTableForTrade, allTradeIDs, deltaCCYs, lwoSwapLegsByCurves, lwoTradeIDsByCurves, lwoLegIDsByCurves, fixingTableNamesByCurves);
			}
			else
			{
				groupLWOTradesByCurveDependencies(lwoTrade, curveCollectionForTrade, fixingTableForTrade, allTradeIDs, deltaCCYs, lwoSwapsByCurves, lwoTradeIDsByCurves, fixingTableNamesByCurves);
			}
		}

		// Calculate the flat delta for each group of trades with the same curve dependencies
		LAStringVector tradeIDs;
		LAStringVector legIDs;
		DoubleVector deltaPerLeg;
		DoubleVector deltaPerTrade;
		if (isCalcDeltaByLeg_ || groupRiskBy == "LEG")
		{
			for (auto iter = lwoSwapLegsByCurves.begin(); iter != lwoSwapLegsByCurves.end(); ++iter)
			{
				const CurveDependencies& key = iter->first;
				std::vector<std::shared_ptr<Leg> >& miniPortfolio = iter->second;

				std::vector<LAString>& tradeIDsForPortfolio = lwoTradeIDsByCurves[key];
				std::vector<LAString>& legIDsForPortfolio = lwoLegIDsByCurves[key];
				std::vector< LabelValueBlock >& fixingTableNames = fixingTableNamesByCurves[key];
                
				const std::vector<double> xccyFXAsOfDateRates = getXccyFXAsOfDateRatesByTradeIDs(tradeIDsForPortfolio);

                // Run the flat shift delta on a mini portfolio where trades share the same forecast and discount curves
				DeltaGenerator riskGen(miniPortfolio, legIDsForPortfolio, fixingTableNames, xccyFXAsOfDateRates, bumpSpreadInstruments_, bumpSize_, bumpMode_, aggregateRisks_, reportInLegCCY_, riskCutOffTenor_, useGlobalCurveEngine_);
				riskGen.setCurves(key.curveCollectionID_, key.forecastCurve_, key.discountCurve_);
				DoubleVector deltaForPortfolio = riskGen.flatshiftDelta();

				// Copy the delta results from each mini portfolio into final result vectors
				std::copy(deltaForPortfolio.begin(), deltaForPortfolio.end(), std::inserter(deltaPerLeg, deltaPerLeg.end()));
				std::copy(tradeIDsForPortfolio.begin(), tradeIDsForPortfolio.end(), std::inserter(tradeIDs, tradeIDs.end()));
				std::copy(legIDsForPortfolio.begin(), legIDsForPortfolio.end(), std::inserter(legIDs, legIDs.end()));
			}
		}
		else
		{
			for (auto iter = lwoSwapsByCurves.begin(); iter != lwoSwapsByCurves.end(); ++iter)
			{
				const CurveDependencies& key = iter->first;
				std::vector< SwapPtr >& miniPortfolio = iter->second;

				std::vector<LAString>& tradeIDsForPortfolio = lwoTradeIDsByCurves[key];
				std::vector<LabelValueBlock>& fixingTableNames = fixingTableNamesByCurves[key];
                
				const std::vector<double> xccyFXAsOfDateRates = getXccyFXAsOfDateRatesByTradeIDs(tradeIDsForPortfolio);

				// Run the flat shift delta on a mini portfolio where trades share the same forecast and discount curves
				DeltaGenerator riskGen(miniPortfolio, tradeIDsForPortfolio, fixingTableNames, xccyFXAsOfDateRates, bumpSpreadInstruments_, bumpSize_, bumpMode_, aggregateRisks_, riskCutOffTenor_, useGlobalCurveEngine_);
				riskGen.setCurves(key.curveCollectionID_, key.forecastCurve_, key.discountCurve_);
				DoubleVector deltaForPortfolio = riskGen.flatshiftDelta();

				// Copy the delta results from each mini portfolio into final result vectors
				std::copy(deltaForPortfolio.begin(), deltaForPortfolio.end(), std::inserter(deltaPerTrade, deltaPerTrade.end()));
				std::copy(tradeIDsForPortfolio.begin(), tradeIDsForPortfolio.end(), std::inserter(tradeIDs, tradeIDs.end()));
			}
		}

		// Finally aggregate the results 
		if ( groupRiskBy == "SWAP" )
		{
			std::map<LAString, double> deltaByTradeID;
			// 1. Populate a map of delta by TradeID, and accumulate the delta from each leg
			if (isCalcDeltaByLeg_)
			{
				for (size_t i = 0; i < deltaPerLeg.size(); i++)
				{
					double& total = deltaByTradeID[tradeIDs[i]];
					total += deltaPerLeg[i];
				}
			}
			else
			{
				for (size_t i = 0; i < deltaPerTrade.size(); i++)
				{
					double& total = deltaByTradeID[tradeIDs[i]];
					total += deltaPerTrade[i];
				}
			}

			// 2. Now iterate through the trades in the order they were given to us, and populate the result vectors
			for (size_t i = 0; i < lwoSwapNames_.size(); ++i)
			{
				LAString& swapName = lwoSwapNames_[i];
				if (!swapName.isDefined() || swapName == "")
				{
					// The swapname was blank / missing in the input
					// Set a placeholder value here
					positionIDs.push_back(swapName);
					deltas.push_back(0.0);
				}
				else if (deltaByTradeID.find(swapName) == deltaByTradeID.end())
				{
					throw LACoreInvalidData((boost::format("#Error: Missing risk for swap: %s")
						% swapName.getCString()).str().c_str(), __FILE__, __LINE__);
				}
				else
				{
					positionIDs.push_back(swapName);
					deltas.push_back(deltaByTradeID[swapName]);
				}
			}

		}
		else if ( groupRiskBy == "LEG" )
		{
			// The risk is already calculated by LegID. Simply populate the output result vectors.
			std::copy(legIDs.begin(), legIDs.end(), std::inserter(positionIDs, positionIDs.end()));
			std::copy(deltaPerLeg.begin(), deltaPerLeg.end(), std::inserter(deltas, deltas.end()));
		}
		else if ( groupRiskBy == "TOTAL" )
		{
			double delta = 0.0;
			if ( isCalcDeltaByLeg_ )
			{
				// Aggregate the delta across all swap legs
				delta = std::accumulate( deltaPerLeg.begin(), deltaPerLeg.end(), 0.0 );
			}
			else
			{
				// Aggregate the delta across all trades
				delta = std::accumulate( deltaPerTrade.begin(), deltaPerTrade.end(), 0.0 );
			}
			positionIDs.push_back( "TotalDelta" );
			deltas.push_back( delta );	
		}
		else
		{
			LAString errMsg( "#Error: Invalid value for groupRiskBy parameter: " );
			errMsg += groupRiskBy;
			errMsg += ". Valid values: LEG, SWAP, TOTAL";
			throw LACoreInvalidData( errMsg.getCString(), __FILE__, __LINE__ );
		}
	}

	/* @brief	Less-than operator for comparing CurveDependencies.
	*			Required when using CurveDependencies as a key in a std::map
	*/
	bool MultiCurveDeltaGenerator::CurveDependencies::operator<(const CurveDependencies& rhs) const
	{
		// We choose curveCollectionID_ to have the highest priority in the comparison
		if (curveCollectionID_ < rhs.curveCollectionID_) return true;
		if (curveCollectionID_ > rhs.curveCollectionID_) return false;

		// The two valuationSettingsLVB are equal. Now consider forecastCurve_ comparison.
		if (forecastCurve_ < rhs.forecastCurve_) return true;
		if (forecastCurve_ > rhs.forecastCurve_) return false;

		// The two forecastCurves are equal. Now consider discountCurve_ comparison.
		if (discountCurve_ < rhs.discountCurve_) return true;
		if (discountCurve_ > rhs.discountCurve_) return false;

		// If we reach this point, both this and rhs LegDependencies are equal
		return false;
	}

}

