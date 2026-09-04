#include "DeltaRiskGenerator.h"
#include "CommonConstants.h"
#include "TypeHelpers.h"

#include "DateUtilities.h"
#include "AQLCurveForwardRateHelpers.h"
#include "CurveCalibrationData.h"
#include "AQLDataMultiReference.h"
#include "InitializeETrading.h"
#include "AQLObject.h"
#include "AQLMathDefine.h"
#include "AQLPriceDataCalendar.h"
#include "AQLPriceDataSlidingRule.h"
#include "AQLDataReference.h"
#include "AQLDataProcedure.h"
#include "RiskUtilities.h"		// includes utilities to manage curve lists where we have multiple forecast curves

namespace
{
	/* @brief		Retrieve the CurveCalibrationData object for the current curve set
	*  @return		CurveCalibrationData pointer
	*/
	CurveCalibrationData* getYieldCurvePro(const AQLString& curveCollectionID)
	{
		// get yield curve set
		AQLDataInstance* dataInstance = etrading::InitializeETrading::instance().dataInstance();
		AQLObjectPool& objPool = dataInstance->getObjectPool();
		AQLString yieldName(curveCollectionID);

        etrading::AQLCurvePricingObject& bYield = etrading::AQLCurveForwardRateHelpers::getYieldCurveForCurveID(dataInstance, yieldName);

		// get yield data pro
		AQLString CurveIDTool = etrading::AQLCurveForwardRateHelpers::YIELD_CURVE_PRO_NAME_PREFIX + curveCollectionID;
		const AQLObjectHolder ehCur = objPool.getObject(CurveIDTool);
		CurveCalibrationData* curveCalibrationData = NULL;
		if (!ehCur.isDefined())
		{
			curveCalibrationData = new CurveCalibrationData(dataInstance);
			objPool.set(CurveIDTool, curveCalibrationData);
		}
		else
		{
			curveCalibrationData = &dynamic_cast<CurveCalibrationData&>(objPool.getObject(CurveIDTool).get());
		}

		// This object will be used to provide 1. input market data
		// and 2. perform the curve solving.
		return curveCalibrationData;
	}

	/* @brief	Helper function to get the CurveCalibration object associated with a global curve.
	*
	* @param[in]	CurveCalibrationData	EntityPool properties object
	* @returns		CurveCalibration object
	*/
	const CurveCalibration& getCurveCalibrationEngine( const CurveCalibrationData* CurveCalibrationData )
	{
		// Get CurveCalibration out of CurveCalibrationData
		const AQLDataProcedure &modelDataObj = dynamic_cast<const AQLDataProcedure &>(CurveCalibrationData->getData( CALIBRATION_DATA_CURVEGENERATOR, ISNOTNULL).get() );
		const AQLCoreProcedure& yieldGenProcedure = modelDataObj.getMethod();
		const CurveCalibration& curveCalibrationEngine = dynamic_cast<const CurveCalibration&>( yieldGenProcedure );
		return curveCalibrationEngine;
	}

	/* @brief	Helper function to get the CurveCalibration object associated with a global curve.
	*
	* @param[in]	curveCollectionID	Curve Collection used to look up entity pool information
	* @returns		CurveCalibration object
	*/
	const CurveCalibration& getCurveCalibrationEngine( const AQLString& curveCollectionID )
	{
		CurveCalibrationData* CurveCalibrationData = getYieldCurvePro( curveCollectionID );
		return getCurveCalibrationEngine( CurveCalibrationData );
	}

	/* @brief		Given a basis curve input, obtains the 'against' curvename and collection from the object pool
	 *              This method works with tenor basis curves and XCCY basis curves.
	 *              In the case of tenor basis, returns the base forecast curve.
	 *              In the case of XCCY basis, returns the 'against' discount curve
	 * @param [in]	curveCalibrationData					The CurveCalibrationData object corresponding to the input curveCollectionID
	 * @param [in]  curveCollectionID		The curveCollection of the input curveName
	 * @param [in]  curveName				The name of the input basis curve
	 * @param [out] baseCurveCollectionID   The curveCollection of the base curve
	 * @param [out] baseCurveName			The name of the base curve
	 * @return		Returns true if the base curve dataValues were successfully obtained
	 */
	bool getBasisCurveDetails(const CurveCalibrationData* curveCalibrationData, const AQLString& curveCollectionID, const AQLString& curveName, AQLString& baseCurveCollectionID, AQLString& baseCurveName)
	{
		AQLString attrSuffix = "_" + curveName;
		attrSuffix.toUpper();
		const AQLDataHolder* dh = &curveCalibrationData->getData(CALIBRATION_DATA_BASISCURVEBASE + attrSuffix);
		if (dh->isDefined() && !dh->isNull())
		{
			baseCurveName = dynamic_cast<const AQLDataString&> (dh->get());

			// The base curve name can either be in the form of "STD" or "EURYC:STD"
			AQLStringVector partitioned = baseCurveName.toToken(':');
			if (partitioned.size() == 1)
			{
				baseCurveCollectionID = curveCollectionID;
				baseCurveName = partitioned[0];
			}
			else if (partitioned.size() == 2)
			{
				baseCurveCollectionID = partitioned[0];
				baseCurveName = partitioned[1];
			}

			return true;
		}

		return false;
	}

	/* @brief		Calibrate a basis curve using single-curve method
	* @param [in]	curveCalibrationData					The CurveCalibrationData object corresponding to the input curveCollectionID
	* @param [in]	curveName				The name of the input basis curve
	*/
	void calibrateSingleBasisCurve(const AQLString& curveCollectionID, const AQLString& curveName)
	{
		CurveCalibrationData* curveCalibrationData = getYieldCurvePro(curveCollectionID);

		// If the data ISSWAPTENORADJUST exists, preserve the original value before curve rebuild.
		// This data is only present on AUD curves ( true for AUD 3M, false for AUD 6M ).
		bool isSwapTenorAdjustAttrExists = false;
		bool origIsSwapTenorAdjust = false;
		AQLDataHolder* dh = &(curveCalibrationData->getData(IR_CALIBRATION_DATA_ISSWAPTENORADJUST));
		if ( dh->isDefined() && !dh->isNull() )
		{
			// Only enter this block if the data exists
			isSwapTenorAdjustAttrExists = true;
			AQLDataBool& isSwapTenorAdjustAttr = dynamic_cast<AQLDataBool&>( dh->get() );
			origIsSwapTenorAdjust = isSwapTenorAdjustAttr.get();

			AQLString attrSuffix = "";
			if (curveName != STD)
			{
				attrSuffix = "_" + curveName;
				attrSuffix.toUpper();
			}
			bool isSwapTenorAdjust = dynamic_cast<const AQLDataBool& > ((curveCalibrationData->getData(IR_CALIBRATION_DATA_ISSWAPTENORADJUST + attrSuffix, ISNOTNULL)).get());
			isSwapTenorAdjustAttr.set(isSwapTenorAdjust);
		}

		curveCalibrationData->AQLObject::remove(IR_CALIBRATION_DATA_BASISTARGETDF);
		curveCalibrationData->AQLObject::add(IR_CALIBRATION_DATA_BASISTARGETDF, new AQLDataString(curveName));

		// Recalibrate the basis curve here
		curveCalibrationData->setBasisRates();

		if ( isSwapTenorAdjustAttrExists )
		{
			// Only restore the data if it exists
			AQLDataBool& isSwapTenorAdjustAttr = dynamic_cast<AQLDataBool&>( dh->get() );
			isSwapTenorAdjustAttr.set(origIsSwapTenorAdjust);
		}

		curveCalibrationData->AQLObject::remove(IR_CALIBRATION_DATA_BASISTARGETDF);
	}
}


namespace etrading
{

	const char PILLAR_DELIMITER = '.';

    /* @brief	Constructor
    */
    DeltaGenerator::DeltaGenerator()
    {}

    /* @brief	Constructor for BaseInstruments
    *  Note:    Non-LWO Base Case does not support Xccy Swaps
    */
    DeltaGenerator::DeltaGenerator( const std::vector<BaseInstrumentPtr>& trades,
									const std::vector<AQLString>& tradeIDs,
									const bool bumpSpreadInstruments,
                                    const double bumpSize,
                                    const AQLString& bumpMode,
                                    const bool aggregateRisks,
									const std::string& riskCutOffTenor )
										: myTrades_( trades ),
										  myInstrumentIDs_( tradeIDs ),
										  bumpSpreadInstruments_( bumpSpreadInstruments ),
										  bumpSize_( bumpSize ),
										  bumpMode_( bumpMode ),
										  interpolation_( "" ),
										  bumpCurvesCollection_( BumpCurvesCollection() ),
										  aggregateRisks_( aggregateRisks ),
										  reportInLegCCY_( false ),
										  riskCutOffTenor_( riskCutOffTenor ),
										  usingLWO_( false ),
										  isUsingGlobalCurveEngine_(false)

    {
		myLWOSwaps_.clear();
		myLWOSwapLegs_.clear();
	}

	/* @brief	Constructor for LWO Swap legs
    */
	DeltaGenerator::DeltaGenerator( const std::vector<std::shared_ptr<Leg> >& swapLegs,
									const std::vector<AQLString>& legIDs,
									const std::vector<LabelValueBlock>& fixingTableNames,
                                    const std::vector<double>& xccyFXAsOfDateRates,
									const bool bumpSpreadInstruments,
                                    const double bumpSize,
									const AQLString& bumpMode,
                                    const bool aggregateRisks,
									const bool reportInLegCCY,
									const std::string& riskCutOffTenor,
									const bool useGlobalCurveEngine)
										: myLWOSwapLegs_( swapLegs ),
										  myInstrumentIDs_( legIDs ),
										  myLWOFixingTables_( fixingTableNames ),
                                          myLWOXccyFXAsOfDateRates_( xccyFXAsOfDateRates ),
										  bumpSpreadInstruments_( bumpSpreadInstruments ),
										  bumpSize_( bumpSize ),
										  bumpMode_( bumpMode ),
										  bumpCurvesCollection_( BumpCurvesCollection() ),
										  aggregateRisks_( aggregateRisks ),
										  reportInLegCCY_( reportInLegCCY ),
										  riskCutOffTenor_( riskCutOffTenor ),
										  usingLWO_( true ),
										  riskOnLWOLegs_(true),
										  isUsingGlobalCurveEngine_(useGlobalCurveEngine)
    {
		myLWOSwaps_.clear();
	}

	/* @brief	Constructor for LWO Swaps
	*/
	DeltaGenerator::DeltaGenerator(const std::vector<SwapPtr >& lwoSwaps,
											const std::vector<AQLString>& swapIDs,
											const std::vector<LabelValueBlock >& fixingTableNames,
                                            const std::vector<double>& xccyFXAsOfDateRates,
											const bool bumpSpreadInstruments,
											const double bumpSize,
											const AQLString& bumpMode,
											const bool aggregateRisks,
											const std::string& riskCutOffTenor,
											const bool useGlobalCurveEngine)
											: myLWOSwaps_(lwoSwaps),
												myInstrumentIDs_(swapIDs),
												myLWOFixingTables_(fixingTableNames),
                                                myLWOXccyFXAsOfDateRates_( xccyFXAsOfDateRates ),
												bumpSpreadInstruments_(bumpSpreadInstruments),
												bumpSize_(bumpSize),
												bumpMode_(bumpMode),
												bumpCurvesCollection_(BumpCurvesCollection()),
												aggregateRisks_(aggregateRisks),
												reportInLegCCY_(false),
												riskCutOffTenor_(riskCutOffTenor),
												usingLWO_(true),
										        riskOnLWOLegs_(false),
												isUsingGlobalCurveEngine_(useGlobalCurveEngine)
	{
		myLWOSwapLegs_.clear();
	}

	/* @brief			Set the yield curves required for delta calculation
    *  @param [in]		curveCollectionID		Name or handle of the curve set
    *  @param [in]		forecastCurve			Name or handle of the forecast curve
    *  @param [in]		discountCurve			Name or handle of the discount curve
    */
    void DeltaGenerator::setCurves( const AQLString& curveCollectionID, const AQLString& forecastCurve, const AQLString& discountCurve )
    {
		// Add all curves ready to be bumped
        // Note: We allow multiple forecast curves separated by a colon, which can be input as "OIS:USD3ML:USD6ML:USD12ML" for example
		if ( isCurveList( forecastCurve ) )
		{
			// Multiple Forecast Curves
			// Needed for Variable Notional Swaps with Several Floating Curve Indices per Trade Leg
			// -------------------------------------------------------------------------------------
			bool discountCurveAlreadyAdded = false;
			
			std::vector<AQLString> forecastCurveList = generateCurveList(forecastCurve);
			
			for ( AQLString thisforecastCurve : forecastCurveList )
			{
				// Add Forecast Curve Dependencies
				bumpCurvesCollection_.addCurve( thisforecastCurve, curveCollectionID, discountCurve, isUsingGlobalCurveEngine_ );

				// Add discount curve dependencies only once - we assume there is only one discount curve
				if ( thisforecastCurve != discountCurve && !discountCurveAlreadyAdded )
				{
					bumpCurvesCollection_.addCurve( discountCurve, curveCollectionID, "", isUsingGlobalCurveEngine_ );
				}

				// If using the global yield curve engine and if basis curves are used, need to calibrate the basis curve(s) 
				// using the single-curve method because subsequent bumping will be carried out based on the single-curve method
				if ( isUsingGlobalCurveEngine_ )
				{
					std::string curveType = getCurveType( curveCollectionID, thisforecastCurve ).toUpper().getCString();
					if ( curveType == MARKET_KEY::CURVE_TYPE_TENORBASIS || curveType == MARKET_KEY::CURVE_TYPE_BASIS )
					{
						calibrateSingleBasisCurve( curveCollectionID, thisforecastCurve );
					}
				}
			}
		}
		else
		{
			// Single Forecast Curve
			// ---------------------

			// Add Forecast Curve Dependencies
			bumpCurvesCollection_.addCurve( forecastCurve, curveCollectionID, discountCurve, isUsingGlobalCurveEngine_ );

			// Add discount Curve  Dependencies
			if ( forecastCurve != discountCurve )
			{
				bumpCurvesCollection_.addCurve( discountCurve, curveCollectionID, "", isUsingGlobalCurveEngine_ );
			}

			// If using the global yield curve engine and if basis curves are used, need to calibrate the basis curve(s) 
			// using the single-curve method because subsequent bumping will be carried out based on the single-curve method
			if ( isUsingGlobalCurveEngine_ )
			{
				std::string curveType = getCurveType( curveCollectionID, forecastCurve ).toUpper().getCString();
				if ( curveType == MARKET_KEY::CURVE_TYPE_TENORBASIS || curveType == MARKET_KEY::CURVE_TYPE_BASIS )
				{
					calibrateSingleBasisCurve( curveCollectionID, forecastCurve );
				}
			}
		}

		// Market Data Collection Keys and Values
		AQLStringVector addKeys(3);
		addKeys[0] = MARKET_KEY::CURVE_COLLECTION;
		addKeys[1] = MARKET_KEY::FORECAST_CURVE;
		addKeys[2] = MARKET_KEY::DISCOUNT_CURVE;

		AQLStringVector addValues(3);
		addValues[0] = curveCollectionID;
		addValues[1] = forecastCurve;	// Note the forecast curve may be a list of curves
		addValues[2] = discountCurve;

		// Create new market collection for trade pricing
		marketDataCollection_ = LabelValueBlock( marketDataCollection_, addKeys, addValues );
    }
	
    /* @brief			Method that set a variety of pricing parameters
    *  @param [in]		params		A collection of pricing parameters
    */
    void DeltaGenerator::setPricingParams( const LabelValueBlock& params )
    {
        interpolation_ = params.getCompulsoryValueAsLAString( PRICING_PARAMS::INTERPOLATION, "DeltaGenerator" );
    }

	/* @brief		Returns the size of the trade portfolio
	*				Examines myTrades_ or myLWOSwapLegs_ depending on whether LWO Swaps are being used
	*/
	size_t DeltaGenerator::getPortfolioSize()
	{
		if (usingLWO_)
		{
			if (riskOnLWOLegs_)
			{
				return myLWOSwapLegs_.size();
			}
			else
			{
				return myLWOSwaps_.size();
			}
		}
		else
		{
			return myTrades_.size();
		}
	}

	/* @brief	Examines all the curves which the globalEngine is configured to build,
	*			and returns the curveName corresponding to the SWAP curve
	*
	*  @param[in]	curveCollectionID	The curve collection
	*  @returns		The name of the swap curve, if found. If no swap curve, returns an empty string
	*/
	AQLString DeltaGenerator::getSwapCurveNameViaGlobalEngine( const AQLString& curveCollectionID )
	{
		AQLString swapCurveName;

		CurveCalibrationData* CurveCalibrationData = getYieldCurvePro( curveCollectionID );
		const AQLStringVector allGlobalEngineCurveNames = CurveCalibration::getGlobalEngineCurveNames( *CurveCalibrationData );

		for ( auto engineCurve : allGlobalEngineCurveNames )
		{
			AQLString engineCurveType = getCurveType( curveCollectionID, engineCurve );
			const std::string engineCurveTypeStr = engineCurveType.toUpper().getCString();
			if ( engineCurveTypeStr == MARKET_KEY::CURVE_TYPE_SWAP )
			{
				swapCurveName = engineCurve;
				break;
			}
		}
		return swapCurveName;
	}

	/* @brief	Examines the list of curves which depend on the specified curveName
	*			(i.e. the list of curves which must be rebuild when the specified curveName is bumped).
	*			The method searches for the SWAP curve within the list of dependent curves
	*
	*  @param[in]	curveCollectionID	The curve collection
	*  @param[in]	curveName			The curveName used when finding dependent curves
	*  @returns		The name of the swap curve, if found. If no swap curve, returns an empty string
	*/
	AQLString  DeltaGenerator::getSwapCurveNameViaDependentCurves( const AQLString& curveCollectionID, const AQLString& curveName )
	{
		AQLString swapCurveName;

		const std::set<AQLString>& dependentCurves = bumpCurvesCollection_.dependentCurve_[ curveCollectionID + curveName ];

		for ( auto dependentCurve : dependentCurves )
		{
			if ( dependentCurve.size() != 0 )
			{
				const AQLStringVector partitioned = dependentCurve.toToken( ':' );
				const AQLString dependentCurveCollectionID = partitioned[0];
				const AQLString dependentCurveName			= partitioned[1];

				const std::string dependentCurveType = bumpCurvesCollection_.curveTypes_[ dependentCurveCollectionID + dependentCurveName ].getCString();
				if ( dependentCurveType == MARKET_KEY::CURVE_TYPE_SWAP )
				{
					swapCurveName = dependentCurveName;
					break;
				}
			}
		}
		return swapCurveName;
	}

	/* @brief		Calculates the PV of the specified instrument
	 * @param [in]	index							The index of the instrument in the portfolio
	 * @param [in]	setMarketDataAndInterpolation	For non LWO-swaps, specifies whether to initialize the trade with MarketData and Interpolation parameters
	*/
	double DeltaGenerator::calculateTradePV(int index, bool setMarketDataAndInterpolation)
	{
		if (usingLWO_)
		{
			const AQLString curveCollectionID = marketDataCollection_.getOptionalValueAsLAString( MARKET_KEY::CURVE_COLLECTION );
			const double xccyFXAsOfDateRate = myLWOXccyFXAsOfDateRates_[index];
			//We assume valuation date is always the asOfDate when calculating risk.
			const AQLDate valuationDate = getCurveAsOfDate(curveCollectionID);

            StandardStringVector lvbKeys(3);
            lvbKeys[0] = VALUATION_SETTING_KEYS::CURVE_COLLECTION;
            lvbKeys[1] = VALUATION_SETTING_KEYS::FX_AS_OF_DATE_RATE;
            lvbKeys[2] = VALUATION_SETTING_KEYS::VALUATION_DATE;

            StandardStringVector lvbValues(3);
            lvbValues[0] = curveCollectionID.getCString();
            const std::string xccyFXAsOfDateRateAsString = std::to_string( static_cast<long double> (xccyFXAsOfDateRate) );
            lvbValues[1] = xccyFXAsOfDateRateAsString;
            lvbValues[2] = valuationDate.convertDateToString().getCString();

            LabelValueBlock valuationSettingsLVB( lvbKeys, lvbValues );
			
			const LabelValueBlock fixingTableNames = myLWOFixingTables_[index];

			if (riskOnLWOLegs_)
			{
				auto leg = myLWOSwapLegs_[index];

				DataProvider dataProvider(ValuationSettings(valuationSettingsLVB, fixingTableNames, leg->getLegName()));

				return leg->pv( dataProvider, reportInLegCCY_);
			}
			else
			{
				double pv = 0.0;
				SwapPtr lwoTrade = myLWOSwaps_[index];
				for (size_t j = 0; j < lwoTrade->getLegSize(); j++)
				{
					auto leg = lwoTrade->getLeg(j);

					DataProvider dataProvider(ValuationSettings(valuationSettingsLVB, fixingTableNames, leg->getLegName()));

					pv += leg->pv( dataProvider, reportInLegCCY_);
				}

				return pv;
			}
		}
		else
		{
			if (setMarketDataAndInterpolation)
			{
				myTrades_[index]->setMarketData( marketDataCollection_ );
				myTrades_[index]->setInterpolation( interpolation_ );		
			}
			return myTrades_[index]->pv();
		}
	}

    /* @brief		Calculate flat shift delta risk
    *  @return		Delta risk numbers through curve flat shifting for all the trades
    */
    DoubleVector DeltaGenerator::flatshiftDelta()
    {
		DoubleVector deltas;
		try
		{
			// Pre-bump PV
			DoubleVector oldPVs;
			for (size_t i = 0; i < getPortfolioSize(); ++i)
			{
				double oldPV = calculateTradePV(i, true /* setMarketDataAndInterpolation */);
				oldPVs.push_back(oldPV);
			}

			// Cache old rates
			cacheOriginalCurveData();

			// Flat shift the curves			
			if (bumpMode_ == "UP")
			{
				flatShiftYieldCurve("UP", bumpSize_);

				for (size_t i = 0; i < getPortfolioSize(); ++i)
				{
					double newPV = calculateTradePV(i);
					double delta = (newPV - oldPVs[i]) / bumpSize_;   // Divide through by bumpSize to get change in PV per bp.
					deltas.push_back(delta);
				}
			}
			else if (bumpMode_ == "DOWN")
			{
				flatShiftYieldCurve("DOWN", bumpSize_);

				for (size_t i = 0; i < getPortfolioSize(); ++i)
				{
					double newPV = calculateTradePV(i);
					double delta = (oldPVs[i] - newPV) / bumpSize_;   // Divide through by bumpSize to get change in PV per bp.
					deltas.push_back(delta);
				}
			}
			else if (bumpMode_ == "CENTRAL")
			{
				flatShiftYieldCurve("UP", bumpSize_);

				DoubleVector upPVs;
				for (size_t i = 0; i < getPortfolioSize(); ++i)
				{
					upPVs.push_back(calculateTradePV(i));
				}


				flatShiftYieldCurve("DOWN", bumpSize_ * 2);
				for (size_t i = 0; i < getPortfolioSize(); ++i)
				{
					double downPV = calculateTradePV(i);
					double delta = (upPVs[i] - downPV) / bumpSize_ / 2.0;  // Divide through by bumpSize to get change in PV per bp.
					deltas.push_back(delta);
				}
			}
			else
			{
				throw AQLCoreInvalidData("#Error: BumpMode must be either 'UP, or 'DOWN', or 'CENTRAL'", __FILE__, __LINE__);
			}

			// Restore the bumped curves to their original states
			restoreCurveMarketDataAndRecalibrate();
		}
		catch (const AQLCoreError& )	
		{						
			// Restore the bumped curves to their original states
			restoreCurveMarketDataAndRecalibrate();
		}
		catch (const std::exception&)
		{
			// Restore the bumped curves to their original states
			restoreCurveMarketDataAndRecalibrate();
		}

        return deltas;
    }


    /* @brief			Calculate delta ladder of the
    *  @param [out]		pillarNames		Name of pillar points
    *  @param [out]		deltas			All the deltas
    */
    void DeltaGenerator::deltaLadder( AQLStringVector& pillarNames, DoubleMatrix& deltas )
    {		
		try
		{
			// Pre-bump PV
			DoubleVector oldPVs;
			AQLString errorCollection;
			for (size_t i = 0; i < getPortfolioSize(); ++i)
			{
				try
				{
					double oldPV = calculateTradePV(i, true /* setMarketDataAndInterpolation */);
					oldPVs.push_back(oldPV);
				}
				catch (const std::exception& ex)
				{
					// Accumulate all errors into a single message
					const AQLString& instrumentID = myInstrumentIDs_[i];
					errorCollection += instrumentID + " : " + ex.what() + "; ";
				}
			}
			if (errorCollection.size() > 0)
			{
				AQLString errorString = "#Error: " + errorCollection;
				throw AQLCoreInvalidData(errorString.getCString(), __FILE__, __LINE__);
			}

			// Cache old rates
			cacheOriginalCurveData();

			// Bump each pillar point for a delta ladder
			DeltaLadderData deltaLadderData;
			deltaLadderData.setIsRiskAggregated(aggregateRisks_);

			if (bumpMode_ == "UP")
			{
				deltaLadderPerCurve(deltaLadderData, oldPVs, "UP", bumpSize_);
			}
			else if (bumpMode_ == "DOWN")
			{
				deltaLadderPerCurve(deltaLadderData, oldPVs, "DOWN", bumpSize_);
			}
			else if (bumpMode_ == "CENTRAL")
			{
				DeltaLadderData deltaLadderData_up;
				deltaLadderData_up.setIsRiskAggregated(false);
				deltaLadderPerCurve(deltaLadderData_up, oldPVs, "UP", bumpSize_);

				DeltaLadderData deltaLadderData_down;
				deltaLadderData_down.setIsRiskAggregated(false);
				deltaLadderPerCurve(deltaLadderData_down, oldPVs, "DOWN", bumpSize_);

				deltaLadderData.buildCentralDeltas(deltaLadderData_up, deltaLadderData_down);
			}
			else
			{
				throw AQLCoreInvalidData("#Error: BumpMode must be either 'UP, or 'DOWN', or 'CENTRAL'", __FILE__, __LINE__);
			}

			// Restore the bumped curves to their original states
			restoreCurveMarketDataAndRecalibrate();

			// Output delta ladder in desired order
			deltaLadderData.outputDeltaLadder(pillarNames, deltas);
		}
		catch (const AQLCoreError&)
		{
			// Restore the bumped curves to their original states
			restoreCurveMarketDataAndRecalibrate();
		}
		catch (const std::exception&)
		{
			// Restore the bumped curves to their original states
			restoreCurveMarketDataAndRecalibrate();
		}
    }


    //-------------------------------------------------------------------------------------
    // Private Calculation methods

    /* @brief			Flat shift a given curve
    *  @param [in]		direction		Curve shifting direction
    *  @param [in]		bumpSize		The size of bumps in basis point
    */
    void DeltaGenerator::flatShiftYieldCurve( const AQLString& direction, double bumpSize )
    {
        AQLDataInstance* dataInstance = etrading::InitializeETrading::instance().dataInstance();

		double directionFactor = 1.0;
        if ( direction == "UP" )
        {
            directionFactor = 1.0;
        }
        else if ( direction == "DOWN" )
        {
            directionFactor = -1.0;
        }
        else
        {
            throw AQLCoreInvalidData( "#Error: Curve can only be bumped either UP or DOWN", __FILE__, __LINE__ );
        }

		// Run the main loop inside a try / catch block so that we have a chance to restore the curves
		// if any error occurs during bumping
		try
		{
			// 1. Loop through each curve collection and apply a flat shift to curves
			for ( auto iter = bumpCurvesCollection_.allCurvesInEachCollection_.begin(); iter != bumpCurvesCollection_.allCurvesInEachCollection_.end(); ++iter )
			{
				AQLString curveCollectionID		= iter->first;
				std::set<AQLString> allCurves    = iter->second;
				CurveCalibrationData* curveCalibrationData			    = getYieldCurvePro( curveCollectionID );
			
				// Loop through all available curves in each curve collection
				for ( auto it = allCurves.begin(); it != allCurves.end(); ++it )
				{
					AQLString curveName = *it;

					// Do not bump an FWDFXCONST curve: It contains no market data instruments
					AQLString curveType = bumpCurvesCollection_.curveTypes_[curveCollectionID + curveName];
					std::string curveTypeStr = curveType.toUpper().getCString();
					if ( curveTypeStr == MARKET_KEY::CURVE_TYPE_FWDFXCONST )
					{
						continue;
					}

					// Do not bump BASIS curve instruments if bumpSpreadInstruments is false
					if ( (curveTypeStr == MARKET_KEY::CURVE_TYPE_BASIS || curveTypeStr == MARKET_KEY::CURVE_TYPE_TENORBASIS || curveTypeStr == MARKET_KEY::CURVE_TYPE_XCCYBASIS) && ! bumpSpreadInstruments_ )
					{
						continue;
					}

					// get constituent market data
					AQLString attrSuffix = "";
					if ( curveName != STD )
					{
						attrSuffix = "_" + curveName;
						attrSuffix.toUpper();
					}

					AQLDataHolder* dh = &curveCalibrationData->getData( CALIBRATION_DATA_MARKETDATA + attrSuffix );

					if ( dh->isDefined() && !dh->isNull() )
					{
						AQLDataMultiReference& refMarketDatas = dynamic_cast<AQLDataMultiReference&> ( dh->get() );

						// Perturb ALL curve building market data
						for ( unsigned int i = 0; i < refMarketDatas.getSize(); ++i )
						{
							AQLObjectHolder& objHolder = refMarketDatas.get( i );

							if ( isBuiltFromBasisSpread( objHolder ) )
							{
								// Libor-OIS basis
								// Only bump the Libor-OIS spread if we have been instructed to do so
								if ( bumpSpreadInstruments_ )
								{
									AQLDataHolder* dh = &objHolder.getData( IR_CALIBRATION_DATA_RATE_LOBASIS );
									if ( dh->isDefined() && !dh->isNull() )
									{
										AQLDataDouble& attrLBasisRate = dynamic_cast<AQLDataDouble&>( dh->get() );
										double rate = attrLBasisRate.get();
										// bumpSize is in bp. We multiply by 0.0001 to make the bumpSize compatible with absolute rate
										attrLBasisRate.set( rate + directionFactor * bumpSize * 0.0001 );
									}
									else
									{
										AQLString err = AQLString( "#Error: No Libor-OIS basis are found in the part of OIS curve built through basis spreads in curve" ) + curveName;
										throw AQLCoreInvalidData( err.getCString(), __FILE__, __LINE__ );
									}
								}

								// Reference swaps
								dh = &objHolder.getData( IR_CALIBRATION_DATA_RATE_SWAP );
								if ( dh->isDefined() && !dh->isNull() )
								{
									AQLDataDouble& attrSwapRate = dynamic_cast<AQLDataDouble&>( dh->get() );
									double rate = attrSwapRate.get();
									// bumpSize is in bp. We multiply by 0.0001 to make the bumpSize compatible with absolute rate
									attrSwapRate.set( rate + directionFactor * bumpSize * 0.0001 );
								}
								else
								{
									if (!isUsingGlobalCurveEngine_)
									{
										AQLString err = AQLString("#Error: No reference swap rates are found in the part of OIS curve built through basis spreads in curve") + curveName;
										throw AQLCoreInvalidData(err.getCString(), __FILE__, __LINE__);
									}
								}
							}
							else
							{
								// Outright swaps
								AQLDataDouble& attrRate = dynamic_cast<AQLDataDouble&>( objHolder.getData( CALIBRATION_DATA_RATE, ISNOTNULL ).get() );
								double rate = attrRate.get();
								// bumpSize is in bp. We multiply by 0.0001 to make the bumpSize compatible with absolute rate
								attrRate.set( rate + directionFactor * bumpSize * 0.0001 );
							}
						}
					}
					else
					{
						AQLString err = AQLString( "#Error: No market data can be found in curve '" ) + curveName + AQLString( "'. Has curve been built?" );
						throw AQLCoreInvalidData( err.getCString(), __FILE__, __LINE__ );
					}
				}
			}

			// 2. Now rebuild all curves
			recalibrateCurves();
		}
		catch( std::exception& e )
		{
			// A problem has occurred during bumping.
			// Here we restore the curves back to the initial state so that pricing is not affected.
			(void)e;
			restoreCurveMarketDataAndRecalibrate();

			// Re-throw the original exception
			throw;
		}
    }

    /* @brief			Find the delta ladder of a particular curve
    *  @param [inout]	deltaLadderData		Delta ladder data wrapper object
    *  @param [out]		bucketLabels		A list of bucket labels
    *  @param [inout]	oldPVs				Instrument's PV before bumping
    *  @param [in]		direction			Bumping direction
    *  @param [in]		bumpSize			Bump size in bps
    */
    void DeltaGenerator::deltaLadderPerCurve( DeltaLadderData& deltaLadderData,
											  DoubleVector oldPVs,
											  const AQLString& direction,
											  double bumpSize )
    {
		double directionFactor = 1.0;
        if ( direction == "UP" )
        {
            directionFactor = 1.0;
        }
        else if ( direction == "DOWN" )
        {
            directionFactor = -1.0;
        }
        else
        {
            throw AQLCoreInvalidData( "#Error: Curve can only be bumped either UP or DOWN", __FILE__, __LINE__ );
        }

		// Determine cut-off tenor in years if provided, beyond which no more bumping is to be performed.
		// 0 years implies no cutoff
		const unsigned int riskCutOffTenorYears = etrading::parseTenorYears( riskCutOffTenor_, true /* throw on parse failure */ );

        // Loop through each curve collection
        for ( auto iter = bumpCurvesCollection_.allCurvesInEachCollection_.begin(); iter != bumpCurvesCollection_.allCurvesInEachCollection_.end(); ++iter )
        {
            AQLString curveCollectionID	 = iter->first;
            std::set<AQLString> allCurves = iter->second;
            CurveCalibrationData* curveCalibrationData			 = getYieldCurvePro( curveCollectionID );

            // Loop through all available curves in each curve collection
            for ( auto it = allCurves.begin(); it != allCurves.end(); ++it )
            {
                AQLString curveName = *it;

				// Do not bump a FWDFXCONST curve: It contains no market data instruments
				AQLString curveType = bumpCurvesCollection_.curveTypes_[curveCollectionID + curveName];
				std::string curveTypeStr = curveType.toUpper().getCString();
				if ( curveTypeStr == MARKET_KEY::CURVE_TYPE_FWDFXCONST )
				{
					continue;
				}

                AQLDataInstance* dataInstance = etrading::InitializeETrading::instance().dataInstance();
                const AQLDataDate& atr = dynamic_cast<const AQLDataDate& >( dataInstance->getObjectPool().getObject( curveCollectionID, ENCHKTYPE_ISDEFINED ).get().getData( CALIBRATION_DATA_ASOFDATE, ISNOTNULL ).get() );
                const AQLDate asofdate = atr.get();

                // get constituent market data
                AQLString attrSuffix = "";
                if ( curveName != STD )
                {
                    attrSuffix = "_" + curveName;
                    attrSuffix.toUpper();
                }

                bool isBasisCurve = checkIsBasisCurve( curveCalibrationData, attrSuffix, bumpCurvesCollection_, curveCollectionID + curveName );

                AQLDataHolder* dh = &curveCalibrationData->getData( CALIBRATION_DATA_MARKETDATA + attrSuffix );

                if ( dh->isDefined() && !dh->isNull() )
                {
                    AQLDataMultiReference& refMarketDatas = dynamic_cast<AQLDataMultiReference&> ( dh->get() );

                    // Perturb the price of EACH constituent instrument and reprice
                    for ( unsigned int i = 0; i < refMarketDatas.getSize(); ++i )
                    {
						AQLObjectHolder& objHolder = refMarketDatas.get( i );

						// Check instrument include flag
						const AQLDataHolder *ahInclude = &( objHolder.getData(IR_CALIBRATION_DATA_GRIDUSEFLAG, NOCHECK) );
						if ( ahInclude->isDefined() && !ahInclude->isNull() ) 
						{
							bool included = dynamic_cast<const AQLDataBool &>(ahInclude->get()).get();
							if ( ! included )
							{						
								continue;
							}
						}

                        if ( isBuiltFromBasisSpread( objHolder ) )
                        {
                            // 1. Libor-OIS basis
							// Only bump the Libor-OIS spread if we have been instructed to do so
							if ( bumpSpreadInstruments_ )
							{
								AQLDataHolder* dh = &objHolder.getData( IR_CALIBRATION_DATA_RATE_LOBASIS );
								if ( dh->isDefined() && !dh->isNull() )
								{
									AQLDataDouble& attrLOBasisRate = dynamic_cast<AQLDataDouble&>( dh->get() );
									AQLString pillarType = BASIS;
									pillarDelta( deltaLadderData, attrLOBasisRate, curveCalibrationData, objHolder, asofdate, bumpSize, oldPVs, curveName, curveCollectionID, pillarType, isBasisCurve, directionFactor );
								}
								else
								{
									AQLString err = AQLString( "#Error: No Libor-OIS basis are found in the part of OIS curve built through basis spreads in curve '" ) + curveName + "'";
									throw AQLCoreInvalidData( err.getCString(), __FILE__, __LINE__ );
								}
							}

                            // 2. Reference swaps
                            dh = &objHolder.getData( IR_CALIBRATION_DATA_RATE_SWAP );

							/* If there is a reference swap associated with the Libor-OIS instrument, then bump the underlying Libor swap instrument.
							*  OR
							*  If there is no Libor Swap data block in the OIS curve, but isUsingGlobalCurveEngine_ is TRUE,
							*  then we can still bump the underlying Libor swap instrument by fetching the Libor swap from the SWAP curve.
							*
							* Note that typically when isUsingGlobalCurveEngine_ is true it likely that the OIS curve market data
							* will not contain it's own outright Libor instruments.
							*/
                            if ( ( dh->isDefined() && !dh->isNull() ) || isUsingGlobalCurveEngine_ )
                            {
                                // Before bumping the reference swap rate, scout all the pillar points in this curve's dependent curve and check if there
                                // is any constituent instrument on the dependent curve that is the same instrument as the reference swap currently being bumped.
                                // If such an instrument exists in the dependent curve, the same instrument has to be bumped in the dependent curve too at the same time.
                                //
                                // Example, if it has been found out that the same 5Y swap is one of the constituent instruments on the STD curve, the 5Y
                                // swap rate on both the OIS curve and the STD curve must be bumped at the same time

                                // Get the TERM of the reference Libor swap in the OIS curve
                                AQLString refSwapTerm;
                                AQLDataHolder* attrHolder = &objHolder.getData( IR_CALIBRATION_DATA_TERM, NOCHECK );
                                if ( attrHolder->isDefined() && !attrHolder->isNull() )
                                {
                                    refSwapTerm = dynamic_cast<const AQLDataString&>( attrHolder->get() ).get();
                                    refSwapTerm.toUpper();
                                }
                                else
                                {
                                    AQLString err = AQLString( "#Error: Can not obtain reference swap term from curve '" ) + curveName + "'";
                                    throw AQLCoreInvalidData( err.getCString(), __FILE__, __LINE__ );
                                }

                                // Get the FREQUENCY of the reference Libor swap in the OIS curve
                                AQLString refSwapFreq;
                                attrHolder = &objHolder.getData( IR_CALIBRATION_DATA_FREQUENCY_LOBASIS, NOCHECK );
                                if ( attrHolder->isDefined() && !attrHolder->isNull() )
                                {
                                    refSwapFreq = dynamic_cast<const AQLDataString&>( attrHolder->get() ).get();
                                    refSwapFreq.toUpper();
                                }
                                else
                                {
                                    AQLString err = AQLString( "#Error: Can not obtain reference swap frequency from curve '" ) + curveName + "'";
                                    throw AQLCoreInvalidData( err.getCString(), __FILE__, __LINE__ );
                                }

								// Depending on whether we are using the global curve engine, attempt to get the SWAP curve name
								// This will be used to locate the Libor swap instrument corresponding to the LOBasis
								AQLString swapCurveName;
								if ( isUsingGlobalCurveEngine_ )
								{
									swapCurveName = getSwapCurveNameViaGlobalEngine( curveCollectionID );
								}
								else
								{
									swapCurveName = getSwapCurveNameViaDependentCurves( curveCollectionID, curveName );
								}

                                // If there is a swap curve name, loop through its constituent instruments to find the same swap currently
                                // being bumped in the OIS curve
                                AQLDataDouble* attrSwapRate = NULL;

								if (swapCurveName.size() > 0 )
								{
									AQLString suffix = "";
									if ( swapCurveName != STD )
									{
										suffix = "_" + swapCurveName;
										suffix.toUpper();
									}

									AQLDataHolder* attrHolder = &curveCalibrationData->getData( CALIBRATION_DATA_MARKETDATA + suffix );

									if ( attrHolder->isDefined() && !attrHolder->isNull() )
									{
										AQLDataMultiReference& refMarketDatas = dynamic_cast<AQLDataMultiReference&> ( attrHolder->get() );

										for ( unsigned int i = 0; i < refMarketDatas.getSize(); ++i )
										{
											AQLObjectHolder& entityHolder = refMarketDatas.get( i );

											AQLString pillarType = dynamic_cast<const AQLDataString&> ( ( entityHolder.getData( IR_CALIBRATION_DATA_DATATYPE, ISNOTNULL ) ).get() ).get();
											if ( pillarType == PAR )
											{
												AQLString pillarTerm = dynamic_cast<const AQLDataString&> ( ( entityHolder.getData( IR_CALIBRATION_DATA_TERM, ISNOTNULL ) ).get() ).get();
												AQLString frequency = dynamic_cast<const AQLDataString&> ( ( entityHolder.getData( IR_CALIBRATION_DATA_FREQUENCY_FLOAT, ISNOTNULL ) ).get() ).get();
												pillarTerm.toUpper();
												frequency.toUpper();

												// Use swap term and frequency to find the match
												if ( pillarTerm == refSwapTerm && frequency == refSwapFreq )
												{
													attrSwapRate = &dynamic_cast<AQLDataDouble&>( entityHolder.getData( CALIBRATION_DATA_RATE, ISNOTNULL ).get() );
													// We found the matching swap. OK to end the search here.
													break;
												}
											}
										}									
									}
								}

								/* If the reference Libor swap was found in the OIS curve then bump it IN ADDITION
								*  to the reference Libor swap found in the SWAP curve.
								*/
								if ( dh->isDefined() && !dh->isNull() )
								{
									AQLDataDouble& attrRefSwapRate = dynamic_cast<AQLDataDouble&>( dh->get() );
									AQLString pillarType = "RefSwap";
									pillarDelta( deltaLadderData, attrRefSwapRate, curveCalibrationData, objHolder, asofdate, bumpSize, oldPVs, curveName, curveCollectionID, pillarType, isBasisCurve, directionFactor, attrSwapRate );
								}
								else
								{
									/* If there is no reference Libor swap in the OIS curve, but we did find a reference swap in the SWAP curve,
									*  then just bump the reference swap from the SWAP curve.
									*/
									if ( attrSwapRate != NULL )
									{
										// SWAP market data missing from OIS curve; but isUsingGlobalCurve_
										AQLString pillarType = "RefSwap";
										pillarDelta( deltaLadderData, *attrSwapRate, curveCalibrationData, objHolder, asofdate, bumpSize, oldPVs, curveName, curveCollectionID, pillarType, isBasisCurve, directionFactor );
									}
								}
                            }
                            else
                            {
								AQLString err = AQLString("#Error: No reference swap rates are found in the part of OIS curve built through basis spreads in curve") + curveName;
								throw AQLCoreInvalidData(err.getCString(), __FILE__, __LINE__);
                            }
                        }
                        else
                        {
                            AQLDataDouble& attrRate = dynamic_cast<AQLDataDouble&>( objHolder.getData( CALIBRATION_DATA_RATE, ISNOTNULL ).get() );

                            AQLString pillarType = dynamic_cast<const AQLDataString&> ( ( objHolder.getData( IR_CALIBRATION_DATA_DATATYPE, ISNOTNULL ) ).get() ).get();
                            pillarType.toUpper();

                            pillarDelta( deltaLadderData, attrRate, curveCalibrationData, objHolder, asofdate, bumpSize, oldPVs, curveName, curveCollectionID, pillarType, isBasisCurve, directionFactor );
                        }
                    }
                }
                else
                {
                    AQLString err = AQLString( "#Error: No market data can be found in curve '" ) + curveName + AQLString( "'. Has curve been built?" );
                    throw AQLCoreInvalidData( err.getCString(), __FILE__, __LINE__ );
                }

				// Even though the bump is removed from the curve market data, we need to recalibrate curves in order to fully restore.
				recalibrateCurveAndAllDependentCurves( curveCalibrationData, curveName, curveCollectionID, isBasisCurve );
            }			
        }
    }


    /* @brief			Calculate the delta risk for a pillar point
    *  @param [inout]	deltaLadderData		Delta ladder data wrapper object
    *  @param [inout]	attrRate			The rate attribution of a pillar point
    *  @param [inout]	curveCalibrationData				Pointer to the yield curve pro object
    *  @param [inout]	objHolder					The current pillar point object
    *  @param [in]		asofdate			As of date of the curve
    *  @param [in]		bumpSize			Delta bump size
    *  @param [in]		oldPVs				Instrument PV before bumping
    *  @param [in]		curveName			Name of current curve
    *  @param [in]		curveCollectionID	Name of the associated curve collection
    *  @param [in]		pillarType			Type of current pillar point
    *  @param [in]		isBasisCurve		Is the current curve a basis curve
    *  @param [in]		directionFactor		Either 1 or -1 depending on bumping up or down
    *  @param [inout]	attr2ndRate			The same instrument to bump that resides in another curve
    *  @return			The name and delta of a curve pillar point
    */
    void DeltaGenerator::pillarDelta( DeltaLadderData& deltaLadderData,
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
                                      AQLDataDouble* attr2ndRate )
    {
        double rate = attrRate.get();
		// bumpSize is in bp. We multiply by 0.0001 to make the bumpSize compatible with absolute rate
        attrRate.set( rate + directionFactor * bumpSize * 0.0001 );

        double rate2;
        if ( attr2ndRate != NULL )
        {
            rate2 = attr2ndRate->get();
			// bumpSize is in bp. We multiply by 0.0001 to make the bumpSize compatible with absolute rate
            attr2ndRate->set( rate2 + directionFactor * bumpSize * 0.0001 );
        }

        // Now that the instrument rates have been updated, re-calibrate the yield curve so
        // that new DFs and new forward rates are generated.
        recalibrateCurveAndAllDependentCurves( curveCalibrationData, curveName, curveCollectionID, isBasisCurve );

        // Pillar delta
        DoubleVector deltas;
		for ( size_t i = 0; i < getPortfolioSize(); ++i )
		{
			double newPV = calculateTradePV(i);
			double delta = directionFactor * ( newPV - oldPVs[i] ) / bumpSize_;
			deltas.push_back( delta );
		}

        // Record pillar name and pillar rate
        AQLString bucketName = buildOutputPillarName( objHolder, pillarType, isBasisCurve );

        deltaLadderData.addBucketRisk( curveName, curveCollectionID, bucketName, deltas );

        // Reset rate for the current constituent instrument
        attrRate.set( rate );
        if ( attr2ndRate != NULL )
        {
            attr2ndRate->set( rate2 );
        }
    }


    //-------------------------------------------------------------------------------------
    //-------------------------------------------------------------------------------------
    // Private Utility methods


	/* @brief			Recalibrate the specified curve only.
    *  @param [inout]	curveCalibrationData				Pointer to the yield curve pro object
    *  @param [in]		curveName			Current curve
    *  @param [in]		curveCollectionID	Collection name where the current curve is in
    *  @param [in]		isBasisCurve		Is the current curve a basis curve?
    */
    void DeltaGenerator::recalibrateSingleCurve( CurveCalibrationData* curveCalibrationData, const AQLString& curveName, const AQLString& curveCollectionID, bool isBasisCurve )
	{
		AQLDataInstance* dataInstance = etrading::InitializeETrading::instance().dataInstance();

		if ( isBasisCurve )
        {
			// Check if this is a XCCY Basis Curve
			AQLString key = curveCollectionID + curveName;
			AQLString curveType = bumpCurvesCollection_.curveTypes_[key];
			std::string curveTypeStr = curveType.toUpper().getCString();

			if ( curveTypeStr == MARKET_KEY::CURVE_TYPE_XCCYBASIS )
			{
				// The CurveCalibrationData can only hold the dataValues for a single XCCY curve at any one time.
				// This next section obtains the 'against' CurveCollectionID for the XCCY.
				// This againstCurveCollectionID is used to locate and attach the foreign yield data to the CurveCalibrationData.
				AQLString againstDiscountCurveIndex;
				AQLString againstCurveCollectionID;
				if ( getBasisCurveDetails( curveCalibrationData, curveCollectionID, curveName, againstCurveCollectionID, againstDiscountCurveIndex ) )
				{
					curveCalibrationData->getForeignYieldData().convertFromString( againstCurveCollectionID );
				}
			}

            // Re-solve for basis curve
			calibrateSingleBasisCurve(curveCollectionID, curveName);
        }
        else
        {
			AQLString key = curveCollectionID + curveName;
			AQLString curveType = bumpCurvesCollection_.curveTypes_[key];
			std::string curveTypeStr = curveType.toUpper().getCString();

			if ( curveTypeStr == MARKET_KEY::CURVE_TYPE_FWDFXCONST )
			{
				// The CurveCalibrationData only supports rebuilding a single FXFWDCONST curve at a time.
				// This next section obtains the 'against' CurveCollectionID for the FXFWDCONST,
				// which represents the collection for the collateral-currency.
				// This againstCurveCollectionID is used to locate and attach the collateral XCCY curve on the CurveCalibrationData.
				AQLString attrSuffix = "_" + curveName;
				attrSuffix.toUpper();
				AQLDataHolder* dh = &curveCalibrationData->getData( CALIBRATION_DATA_MARKETDATA + attrSuffix );
				if ( dh->isDefined() && !dh->isNull() )
				{
					// The dependent curve names of the FX FWD curve are stored as market data
					AQLDataMultiReference& refMarketData = dynamic_cast<AQLDataMultiReference&> ( dh->get() );
					if ( refMarketData.getSize() != 1)
					{
						AQLString errMsg("#Error: Incorrect MarketData size in constant fx forward curve. Expecting a data block of size: 1. Actual size: ");
						errMsg += refMarketData.getSize();
						throw AQLCoreInvalidData( errMsg.getCString(), __FILE__, __LINE__ );
					}

					AQLObjectHolder& objHolder = refMarketData.get( 0 );
					AQLString againstCurveCollectionID = dynamic_cast<const AQLDataString&> ( ( objHolder.getData( IR_CALIBRATION_DATA_AGTCURVECOLLECTION, ISNOTNULL ) ).get() ).get();

					// This next method convertFromString() locates the collateral yield data (the collateral XCCY curve) from within the object pool
					// based on the curveCollection string name. It then attaches this yield data to the CurveCalibrationData.
					curveCalibrationData->getColYieldData().convertFromString(againstCurveCollectionID);

					// Now we can go ahead and rebuild the FWDFXCONST curve
					curveCalibrationData->calcFwdFXConstantCurveUsingMarketName(curveName);
				}
				else
				{
					throw AQLCoreInvalidData( ( boost::format( "#Error: FXFWDCONST curve '%s:%s' has not been built correctly." ) % curveCollectionID.getCString() % curveName.getCString() ).str().c_str() , __FILE__, __LINE__ );
				}
			}
			else
			{
				const AQLDataDate& atr = dynamic_cast<const AQLDataDate&>(dataInstance->getObjectPool().getObject(curveCollectionID, ENCHKTYPE_ISDEFINED).get().getData(CALIBRATION_DATA_ASOFDATE, ISNOTNULL).get());
				const AQLDate asofdate = atr.get();

				if (isUsingGlobalCurveEngine_)
				{
					// Get CurveCalibration out of CurveCalibrationData
					const CurveCalibration& curveEngine = getCurveCalibrationEngine( curveCalibrationData);

					// Peform dual-bootstrapping
					// By setting 'false' to the last argument, we effectively ignore basis curves in the curve engine
					AQLObject* parent = dynamic_cast<AQLObject* >(curveCalibrationData);
                    curveEngine.buildEngineCurves( asofdate, *parent, AQLDataProcedure(), false );
				}
				else
				{
					AQLDataProcedure& modelDataObj = dynamic_cast<AQLDataProcedure&>
						(curveCalibrationData->getData(CALIBRATION_DATA_CURVEGENERATOR, ISNOTNULL).get());					

					// remove curve generate map
					std::map<AQLString, bool>& gCurveMap = curveCalibrationData->getGCurveGenerateMap();
					const std::map<AQLString, AQLString>& assignedCurveMktMap = curveCalibrationData->getAssignedCurveMktMap();
					for (std::map<AQLString, AQLString>::const_iterator it = assignedCurveMktMap.begin(); it != assignedCurveMktMap.end(); it++)
					{
						if (it->second == curveName)
						{
							if (gCurveMap.find(it->first) != gCurveMap.end())
							{
								gCurveMap.erase(it->first);
							}
						}
					}

					curveCalibrationData->AQLObject::remove(IR_CALIBRATION_DATA_GENTARGETDF);
					curveCalibrationData->AQLObject::add(IR_CALIBRATION_DATA_GENTARGETDF, new AQLDataString(curveName));
					curveCalibrationData->AQLObject::remove(IR_CALIBRATION_DATA_ENABLECALCULATION);
					curveCalibrationData->AQLObject::add(IR_CALIBRATION_DATA_ENABLECALCULATION, new AQLDataBool(true));
					
					// If the data ISSWAPTENORADJUST exists, preserve the original value before swap curve rebuild.
					// This data is only present on AUD curves ( true for AUD 3M, false for AUD 6M ).
					bool isSwapTenorAdjustAttrExists = false;
					bool origIsSwapTenorAdjust = false;
					AQLDataHolder* dh = &(curveCalibrationData->getData(IR_CALIBRATION_DATA_ISSWAPTENORADJUST));
					if ( dh->isDefined() && ! dh->isNull() )
					{
						// Only enter this block if the data exists
						isSwapTenorAdjustAttrExists = true;
						AQLDataBool& isSwapTenorAdjustAttr = dynamic_cast<AQLDataBool&>( dh->get() );
						origIsSwapTenorAdjust = isSwapTenorAdjustAttr.get();

						AQLString attrSuffix = "";
						if (curveName != STD)
						{
							attrSuffix = "_" + curveName;
							attrSuffix.toUpper();
						}
						bool isSwapTenorAdjust = dynamic_cast<const AQLDataBool&> ((curveCalibrationData->getData(IR_CALIBRATION_DATA_ISSWAPTENORADJUST + attrSuffix, ISNOTNULL)).get());
						isSwapTenorAdjustAttr.set(isSwapTenorAdjust);
					}

					// Rebuild DFs and Fwd rates
					modelDataObj.calibrateModel(asofdate);

					if ( isSwapTenorAdjustAttrExists )
					{
						// Only restore the data if it exists
						AQLDataBool& isSwapTenorAdjustAttr = dynamic_cast<AQLDataBool&>( dh->get() );
						isSwapTenorAdjustAttr.set(origIsSwapTenorAdjust);
					}

					curveCalibrationData->AQLObject::remove(IR_CALIBRATION_DATA_GENTARGETDF);
					curveCalibrationData->AQLObject::remove(IR_CALIBRATION_DATA_ENABLECALCULATION);
				}
			}
		}
	}


    /* @brief			Recalibrate the specified curve and all dependent curves
    *  @param [inout]	curveCalibrationData				Pointer to the yield curve pro object
    *  @param [in]		curveName			Current curve
    *  @param [in]		curveCollectionID	Collection name where the current curve is in
    *  @param [in]		isBasisCurve		Is the current curve a basis curve?
    */
    void DeltaGenerator::recalibrateCurveAndAllDependentCurves( CurveCalibrationData* curveCalibrationData, const AQLString& curveName, const AQLString& curveCollectionID, bool isBasisCurve )
    {
        // Now that the instrument rates have been updated, re-calibrate the yield curve so
        // that new DFs and new forward rates are generated.

		// ------------------------------------------------------------------------------
        // 1. Re-calibrate the current curve

		recalibrateSingleCurve( curveCalibrationData, curveName, curveCollectionID, isBasisCurve );

        // ------------------------------------------------------------------------------
        // 2. There are curve(s) that are dependent on the current curve. Re-calibrate those curves too
		const std::set<AQLString>& dependentCurves = bumpCurvesCollection_.dependentCurve_[curveCollectionID + curveName];
		for (auto iter = dependentCurves.begin(); iter != dependentCurves.end(); ++iter)
		{
			const AQLString& dependentCurve = *iter;
			if (dependentCurve.size() != 0)
			{
				AQLStringVector partitioned = dependentCurve.toToken(':');
				AQLString dependentCurveCollectionID = partitioned[0];
				AQLString dependentCurveName = partitioned[1];

				std::string dependentCurveType = bumpCurvesCollection_.curveTypes_[dependentCurveCollectionID + dependentCurveName].getCString();

				const bool dependentCurveIsABasisCurve = (dependentCurveType == MARKET_KEY::CURVE_TYPE_BASIS || dependentCurveType == MARKET_KEY::CURVE_TYPE_TENORBASIS || dependentCurveType == MARKET_KEY::CURVE_TYPE_XCCYBASIS);

				if (dependentCurveCollectionID == curveCollectionID)
				{
					recalibrateCurveAndAllDependentCurves(curveCalibrationData, dependentCurveName, dependentCurveCollectionID, dependentCurveIsABasisCurve);
				}
				else
				{
					// 2.2 Dependent curve is not in the same collection. Need to rebuild call both calibrateModel() and setBasisRates()

					CurveCalibrationData* ycProDependent = getYieldCurvePro(dependentCurveCollectionID);

					recalibrateCurveAndAllDependentCurves(ycProDependent, dependentCurveName, dependentCurveCollectionID, dependentCurveIsABasisCurve);
				}
			}
		}
    }


    /* @brief			Construct a meaningful pillar name for output
    *  @param [in]		objHolder				The pillar point object
    *  @param [in]		pillarType		Type of the pillar point
    *  @param [in]		isBasisCurve	Is this a basis curve?
    *  @return			output pillar name
    */
    AQLString DeltaGenerator::buildOutputPillarName( const AQLObjectHolder& objHolder, const AQLString& pillarType, bool isBasisCurve )
    {

        AQLString outputPillarName;
        if ( pillarType == FUTURE
				|| pillarType == YIELD_TYPE_ARR_FUTURE
                || pillarType == FRA6M
                || pillarType == FRA3M
				|| pillarType == FRA
                || pillarType == BASIS )
        {
            AQLString pillarTerm = dynamic_cast<const AQLDataString&> ( ( objHolder.getData( IR_CALIBRATION_DATA_TERM, ISNOTNULL ) ).get() ).get();

            // Look for StartDate and EndDate for bucket name annotation
            AQLString startDate( "" );
            AQLString endDate( "" );
            bool isUsingTenor = false;
            if ( isBasisCurve )
            {
                const AQLDataHolder* dh = &( objHolder.getData( IR_CALIBRATION_DATA_ISFWDBASIS ) );
                if ( dh->isDefined() && !dh->isNull() )
                {
                    const AQLDataBool& isFwdBasis = dynamic_cast<const AQLDataBool&>( dh->get() );
                    if ( isFwdBasis.get() )
                    {
                        bool isDate = dynamic_cast<const AQLDataBool&>( objHolder.getData( PRICING_DATA_ISDATE ).get() ).get();
                        if ( isDate )
                        {
                            // Start Date and End Date as strings
                            AQLDate tempStart = dynamic_cast<const AQLDataDate&>( objHolder.getData( PRICING_DATA_STARTDATE, ISNOTNULL ).get() ).get();
                            startDate = tempStart.stringWithFormat();

                            AQLDate tempEnd = dynamic_cast<const AQLDataDate&>( objHolder.getData( PRICING_DATA_ENDDATE, ISNOTNULL ).get() ).get();
                            endDate = tempEnd.stringWithFormat();
                        }
                        else
                        {
                            // Start Tenor and End Tenor
                            startDate	= dynamic_cast<const AQLDataString&>( objHolder.getData( PRICING_DATA_STARTTERM, ISNOTNULL ).get() ).get();
                            endDate		= dynamic_cast<const AQLDataString&>( objHolder.getData( PRICING_DATA_TENOR, ISNOTNULL ).get() ).get();
                            isUsingTenor = true;
                        }
                    }
                }
            }
            else
            {
                const AQLDataHolder* dh = &( objHolder.getData( PRICING_DATA_STARTDATE ) );
                if ( dh->isDefined() && !dh->isNull() )
                {
                    startDate	= dynamic_cast<const AQLDataDate&>( dh->get() ).get().stringWithFormat();
                }

                dh = &( objHolder.getData( PRICING_DATA_ENDDATE ) );
                if ( dh->isDefined() && !dh->isNull() )
                {
                    endDate		= dynamic_cast<const AQLDataDate&>( dh->get() ).get().stringWithFormat();
                }
            }

            // Annotate the bucket name with start date and end date information if available
            if ( startDate.size() != 0 || endDate.size() != 0 )
            {
                if ( isUsingTenor )
                {
                    outputPillarName = pillarType + PILLAR_DELIMITER  + startDate + PILLAR_DELIMITER + endDate;
                }
                else
                {
					if ( pillarType == YIELD_TYPE_ARR_FUTURE )
					{
						// For ARRFUTURES pillar points, simplify the prefix to be "FUTURE"
						AQLString pillarPrefix( FUTURE );
						outputPillarName = pillarPrefix + PILLAR_DELIMITER + pillarTerm + PILLAR_DELIMITER + startDate + PILLAR_DELIMITER + endDate;
					}
					else
					{
						outputPillarName = pillarType + PILLAR_DELIMITER + pillarTerm + PILLAR_DELIMITER + startDate + PILLAR_DELIMITER + endDate;
					}
                }
            }
            else
            {
                outputPillarName = pillarType + PILLAR_DELIMITER + pillarTerm;
            }

        }
        else if ( pillarType == ZERO )	//libor case
        {
            AQLString pillarTerm = dynamic_cast<const AQLDataString&> ( ( objHolder.getData( IR_CALIBRATION_DATA_TERM, ISNOTNULL ) ).get() ).get();
            outputPillarName = AQLString("LIBOR") + PILLAR_DELIMITER + pillarTerm;
        }
        else if ( pillarType == PAR || pillarType == "RefSwap" )		//swap case
        {
            AQLString swapTypeAttribute;
            AQLString swapTenorAttribute;
            AQLString frequencyAttribute;

            if ( pillarType == PAR )
            {
                swapTypeAttribute = IR_CALIBRATION_DATA_SWAPTYPE;
                swapTenorAttribute = IR_CALIBRATION_DATA_SWAPTENOR;
                frequencyAttribute = IR_CALIBRATION_DATA_FREQUENCY_FLOAT;
            }
            else if ( pillarType == "RefSwap" )
            {
                swapTypeAttribute = IR_CALIBRATION_DATA_REFSWAPTYPE;
                swapTenorAttribute = IR_CALIBRATION_DATA_REFSWAPTENOR;
                frequencyAttribute = IR_CALIBRATION_DATA_FREQUENCY_LOBASIS;
            }

            AQLString swapType = dynamic_cast<const AQLDataString&> ( ( objHolder.getData( swapTypeAttribute.getCString(), ISNOTNULL ) ).get() ).get();
            AQLString pillarTerm = dynamic_cast<const AQLDataString&> ( ( objHolder.getData( IR_CALIBRATION_DATA_TERM, ISNOTNULL ) ).get() ).get();

            if ( swapType.toUpper() == "OIS" )
            {
                outputPillarName = AQLString("OUTRIGHTOISSWAP") + PILLAR_DELIMITER + pillarTerm;
            }
            else
            {
                AQLString swapTenor = dynamic_cast<const AQLDataString&> ( ( objHolder.getData( swapTenorAttribute.getCString(), ISNOTNULL ) ).get() ).get();
                if ( swapTenor.size() == 0 )
                {
                    AQLString frequency = dynamic_cast<const AQLDataString&> ( ( objHolder.getData( frequencyAttribute.getCString(), ISNOTNULL ) ).get() ).get();

                    if ( frequency == ANNUAL )
                    {
                        swapTenor = "12M";
                    }
                    else if ( frequency == SEMI_ANNUAL )
                    {
                        swapTenor = "6M";
                    }
                    else if ( frequency == QUARTERLY )
                    {
                        swapTenor = "3M";
                    }
                    else if ( frequency == MONTHLY )
                    {
                        swapTenor = "1M";
                    }
                    else
                    {
                        //error
                        throw AQLCoreInvalidData( "#Error: Input freq type is not supported", __FILE__, __LINE__ );
                    }
                }

                if ( swapType.toUpper() == "LIBOR" )
                {
                    outputPillarName = swapTenor + AQLString("LIBORSWAP") + PILLAR_DELIMITER + pillarTerm;
                }
                else
                {
                    outputPillarName = swapTenor + AQLString("SWAP") + PILLAR_DELIMITER + pillarTerm;
                }
            }
        }
        else if ( pillarType == BOJ || pillarType == FEDFUNDRATE )		// Central Bank swaps
        {
            // ** Currently all Central Bank swaps are labelled as BOJ regardless of currency during curve construction **
            AQLString pillarTerm = dynamic_cast<const AQLDataString&> ( ( objHolder.getData( IR_CALIBRATION_DATA_TERM, ISNOTNULL ) ).get() ).get();
            AQLString newPillarType = "CBRate";
            outputPillarName = newPillarType + PILLAR_DELIMITER + pillarTerm;
        }
        else if ( pillarType == T_N || pillarType == O_N || pillarType == "0D_1D" || pillarType == "1D_1D" )
        {
            outputPillarName = pillarType;
        }
        else
        {
            outputPillarName = "Unknown";
        }

        return outputPillarName;
    }


    /* @brief			Cache original constituent market data of a curve
    */
    void DeltaGenerator::cacheOriginalCurveData()
    {
		const unsigned int riskCutOffTenorYears = etrading::parseTenorYears( riskCutOffTenor_, true /* throw on parse failure */ );

        // Loop through each curve collection
        for ( auto iter = bumpCurvesCollection_.allCurvesInEachCollection_.begin(); iter != bumpCurvesCollection_.allCurvesInEachCollection_.end(); ++iter )
        {
            AQLString curveCollectionID		= iter->first;
            std::set<AQLString> allCurves = iter->second;
            CurveCalibrationData* curveCalibrationData			= getYieldCurvePro( curveCollectionID );

            // Loop through all available curves in each curve collection
            for ( auto it = allCurves.begin(); it != allCurves.end(); ++it )
            {
                AQLString curveName = *it;

				// An FWDFXCONST curve contains no calibration instruments, so there is no market data to cache
				AQLString curveType = bumpCurvesCollection_.curveTypes_[curveCollectionID + curveName];
				std::string curveTypeStr = curveType.toUpper().getCString();
				if ( curveTypeStr == MARKET_KEY::CURVE_TYPE_FWDFXCONST )
				{
					continue;
				}

                // get constituent market data
                AQLString attrSuffix = "";
                if ( curveName != STD )
                {
                    attrSuffix = "_" + curveName;
                    attrSuffix.toUpper();
                }

                AQLDataHolder* dh = &curveCalibrationData->getData( CALIBRATION_DATA_MARKETDATA + attrSuffix );

                if ( dh->isDefined() && !dh->isNull() )
                {
                    AQLDataMultiReference& refMarketDatas = dynamic_cast<AQLDataMultiReference&> ( dh->get() );

					// Instrument include flag
					std::map<AQLString, bool>& cachedInstrumentIncludeFlags = originalIncludeInstrumentInCurve_[ curveCollectionID + curveName ];
					cachedInstrumentIncludeFlags.clear();

                    // Cache market data
                    std::map<AQLString, double>& cachedCurveRates = originalCurveMarketData_[curveCollectionID + curveName];
                    cachedCurveRates.clear();

                    for ( unsigned int i = 0; i < refMarketDatas.getSize(); ++i )
                    {
                        AQLObjectHolder& objHolder = refMarketDatas.get( i );
                        AQLString name = dynamic_cast<const AQLDataString&> ( ( objHolder.getData( CALIBRATION_DATA_NAME, ISNOTNULL ) ).get() ).get();

						// Check if the instrument will be included in the bumped curves
						if ( riskCutOffTenorYears > 0 )
						{
							// Cache original setting of instrument include flag
							const AQLDataHolder *dh = &( objHolder.getData(IR_CALIBRATION_DATA_GRIDUSEFLAG, NOCHECK) );
							if (dh->isDefined() && !dh->isNull() ) 
							{
								// Store the flag value, if originally set in object pool
								bool includeInstrument  = dynamic_cast<const AQLDataBool &>(dh->get()).get();
								cachedInstrumentIncludeFlags[name] = includeInstrument;
							}

							// Now check the maturity tenor of the instrument.
							// If instrument matures after riskCutOffTenorYears, then exclude this instrument from the curve build.
							const AQLDataHolder* ahTerm = &( objHolder.getData( IR_CALIBRATION_DATA_TERM ) );
							if ( ahTerm->isDefined() && !ahTerm->isNull() )
							{
								AQLString instrumentTenor = dynamic_cast<const AQLDataString&> ( ahTerm->get() );
								std::string instrumentTenorString( instrumentTenor.getCString() );
								unsigned int instrumentTenorYears = parseTenorYears( instrumentTenorString, false );
								if ( instrumentTenorYears > riskCutOffTenorYears)
								{
									// Set instrument include flag to false
									objHolder.remove( IR_CALIBRATION_DATA_GRIDUSEFLAG );
									objHolder.add( IR_CALIBRATION_DATA_GRIDUSEFLAG, new AQLDataBool( false ) );
								}
							}
						}

						// Cache old rate of constituent instrument of curve
                        if ( isBuiltFromBasisSpread( objHolder ) )
                        {
                            // When building OIS curve, it's possible to use spread + reference instrument to build a pillar point
                            // instead of using an outright instrument. In this case need to record both the spread instrument
                            // as well as the reference instrument (which is a vanilla swap)

                            // LOBasis rate
                            double loBasisRate = dynamic_cast<AQLDataDouble&>( objHolder.getData( IR_CALIBRATION_DATA_RATE_LOBASIS, ISNOTNULL ).get() ).get();
                            name = name + "_" + AQLString( IR_CALIBRATION_DATA_RATE_LOBASIS );
                            cachedCurveRates[name] = loBasisRate;

                            // Swap rate
							AQLDataHolder* dh = &(objHolder.getData(IR_CALIBRATION_DATA_RATE_SWAP));
							if (dh->isDefined() && !dh->isNull())
							{
								double swapRate = dynamic_cast<AQLDataDouble&>(dh->get()).get();
								name = name + "_" + AQLString(IR_CALIBRATION_DATA_RATE_SWAP);
								cachedCurveRates[name] = swapRate;
							}
							else if (!isUsingGlobalCurveEngine_)
							{
								AQLString err = AQLString("#Error: Swap par rates must exist in an OIS curve along with Libor-OIS basis rates when NOT using multi-curve building technology.");
								throw AQLCoreInvalidData(err.getCString(), __FILE__, __LINE__);
							}
                        }
                        else
                        {
                            double rate = dynamic_cast<AQLDataDouble&>( objHolder.getData( CALIBRATION_DATA_RATE, ISNOTNULL ).get() ).get();
                            AQLString name = dynamic_cast<const AQLDataString&> ( ( objHolder.getData( CALIBRATION_DATA_NAME, ISNOTNULL ) ).get() ).get();
                            cachedCurveRates[name] = rate;
                        }
                    }
                }
                else
                {
                    AQLString err = AQLString( "#Error: No market data can be found in curve '" ) + curveName + AQLString( "'. Has curve been built?" );
                    throw AQLCoreInvalidData( err.getCString(), __FILE__, __LINE__ );
                }
            }
        }
    }

	void DeltaGenerator::restoreCurveMarketData(const AQLString& curveCollectionID, const AQLString& curveName)
	{
		const unsigned int riskCutOffTenorYears = etrading::parseTenorYears( riskCutOffTenor_, true /* throw on parse failure */ );


		// get constituent market data
        AQLString attrSuffix = "";
        if ( curveName != STD )
        {
            attrSuffix = "_" + curveName;
            attrSuffix.toUpper();
        }

		CurveCalibrationData* curveCalibrationData = getYieldCurvePro( curveCollectionID );
        AQLDataMultiReference& refMarketData = dynamic_cast<AQLDataMultiReference&>
                                                ( curveCalibrationData->getData( CALIBRATION_DATA_MARKETDATA + attrSuffix, ISNOTNULL ).get() );

        // Get the old data back and restore the curves
        AQLString key = curveCollectionID + curveName;
        if ( originalCurveMarketData_.find( key ) != originalCurveMarketData_.end() )
        {
            std::map<AQLString, double> oldMarketData = originalCurveMarketData_[key];
			std::map<AQLString, bool> cachedInstrumentIncludeFlags = originalIncludeInstrumentInCurve_[ curveCollectionID + curveName ];

            for ( unsigned int i = 0; i < refMarketData.getSize(); ++i )
            {
                AQLObjectHolder& objHolder = refMarketData.get( i );
                AQLString name = dynamic_cast<const AQLDataString&> ( ( objHolder.getData( CALIBRATION_DATA_NAME, ISNOTNULL ) ).get() ).get();

				// Check if we need to restore the instrument include state
				if ( riskCutOffTenorYears > 0 )
				{
					if ( cachedInstrumentIncludeFlags.find( name ) != cachedInstrumentIncludeFlags.end() )
					{
						// Drop the include flag, if present
						objHolder.remove( IR_CALIBRATION_DATA_GRIDUSEFLAG );
						bool originalIncludeFlag = cachedInstrumentIncludeFlags[name];
						objHolder.add( IR_CALIBRATION_DATA_GRIDUSEFLAG, new AQLDataBool( originalIncludeFlag ) );
					}
				}

                if ( isBuiltFromBasisSpread( objHolder ) )
                {
                    // When building OIS curve, it's possible to use spread + reference instrument to build a pillar point
                    // instead of using an outright instrument. In this case need to record both the spread instrument
                    // as well as the reference instrument (which is a vanilla swap)

                    // LOBasis Rate
                    name = name + "_" + AQLString( IR_CALIBRATION_DATA_RATE_LOBASIS );
                    if ( oldMarketData.find( name ) == oldMarketData.end() )
                    {
                        AQLString err = AQLString( "#Error: Can not restore back to original market rate for constituent instrument '" ) + name + AQLString( "' in curve '" ) + curveName + AQLString( "'" );
                        throw AQLCoreInvalidData( err.getCString(), __FILE__, __LINE__ );
                    }
                    double oldRate = oldMarketData[name];
                    AQLDataDouble& attrLOBasisRate = dynamic_cast<AQLDataDouble&>( objHolder.getData( IR_CALIBRATION_DATA_RATE_LOBASIS, ISNOTNULL ).get() );
                    attrLOBasisRate.set( oldRate );

                    // Reference swap rate
                    name = name + "_" + AQLString( IR_CALIBRATION_DATA_RATE_SWAP );
                    if ( oldMarketData.find( name ) == oldMarketData.end() )
                    {
						if (!isUsingGlobalCurveEngine_)
						{
							AQLString err = AQLString("#Error: Can not restore back to original market rate for constituent instrument '") + name + AQLString("' in curve '") + curveName + AQLString("'");
							throw AQLCoreInvalidData(err.getCString(), __FILE__, __LINE__);
						}
                    }
					else
					{
						oldRate = oldMarketData[name];
						AQLDataDouble& attrSwapRate = dynamic_cast<AQLDataDouble&>(objHolder.getData(IR_CALIBRATION_DATA_RATE_SWAP, ISNOTNULL).get());
						attrSwapRate.set(oldRate);
					}
                }
                else
                {
                    if ( oldMarketData.find( name ) == oldMarketData.end() )
                    {
                        AQLString err = AQLString( "#Error: Can not restore back to original market rate for constituent instrument '" ) + name + AQLString( "' in curve '" ) + curveName + AQLString( "'" );
                        throw AQLCoreInvalidData( err.getCString(), __FILE__, __LINE__ );
                    }

                    double oldRate = oldMarketData[name];
                    AQLDataDouble& attrRate = dynamic_cast<AQLDataDouble&>( objHolder.getData( CALIBRATION_DATA_RATE, ISNOTNULL ).get() );
                    attrRate.set( oldRate );
                }
            }
		}
	}

    /* @brief			Restore yield curve back to pre-bump state
    */
    void DeltaGenerator::restoreCurveMarketDataAndRecalibrate()
    {
        AQLDataInstance* dataInstance = etrading::InitializeETrading::instance().dataInstance();

        // 1. Restore all Market Data
        for ( auto iter = bumpCurvesCollection_.allCurvesInEachCollection_.begin(); iter != bumpCurvesCollection_.allCurvesInEachCollection_.end(); ++iter )
        {
            AQLString curveCollectionID	 = iter->first;
            std::set<AQLString> allCurves = iter->second;

            // Loop through all available curves in each curve collection
            for ( auto it = allCurves.begin(); it != allCurves.end(); ++it )
            {
                AQLString curveName = *it;
				restoreCurveMarketData(curveCollectionID, curveName);
			}
		}

		// 2. Now rebuild all curves
		recalibrateCurves();
    }
	
	/* @brief			Recalibrate all curves used by this DeltaRiskGenerator
	 *					The curves are rebuilt by curveType in the following sequence: OIS, STD, TenorBasis, XccyBasis, FwdFXConst.
	 */
	void DeltaGenerator::recalibrateCurves()
	{
		if (isUsingGlobalCurveEngine_)
		{
			AQLDataInstance* dataInstance = etrading::InitializeETrading::instance().dataInstance();

			for (auto iter = bumpCurvesCollection_.allCurvesInEachCollection_.begin(); iter != bumpCurvesCollection_.allCurvesInEachCollection_.end(); ++iter)
			{
				AQLString curveCollectionID = iter->first;
				const AQLDataDate& atr = dynamic_cast<const AQLDataDate&>(dataInstance->getObjectPool().getObject(curveCollectionID, ENCHKTYPE_ISDEFINED).get().getData(CALIBRATION_DATA_ASOFDATE, ISNOTNULL).get());
				const AQLDate asofdate = atr.get();

				// Get CurveCalibration out of CurveCalibrationData
				CurveCalibrationData* curveCalibrationData = getYieldCurvePro( curveCollectionID );
				const CurveCalibration& curveEngine = getCurveCalibrationEngine( curveCalibrationData );

				// Peform dual-bootstrapping
				AQLObject* parent = dynamic_cast<AQLObject*>(curveCalibrationData);
                curveEngine.buildEngineCurves( asofdate, *parent, AQLDataProcedure() );
			}
		}
		else
		{
			// The available curveTypes, specified in the correct build sequence
			const char* curveTypesRebuildSequence[] = { MARKET_KEY::CURVE_TYPE_ARR.c_str(),
														MARKET_KEY::CURVE_TYPE_OIS.c_str(),
														MARKET_KEY::CURVE_TYPE_SWAP.c_str(),
														MARKET_KEY::CURVE_TYPE_BASIS.c_str(),        // A tenor basis curve may be labelled as "BASIS"
														MARKET_KEY::CURVE_TYPE_TENORBASIS.c_str(),
														MARKET_KEY::CURVE_TYPE_XCCYBASIS.c_str(),
														MARKET_KEY::CURVE_TYPE_FWDFXCONST.c_str() };
			const int numCurveTypes = sizeof(curveTypesRebuildSequence) / sizeof(char*);

			const std::vector<std::string> rebuildSequence(curveTypesRebuildSequence, curveTypesRebuildSequence + numCurveTypes);

			// Rebuild the curves in the correct sequence: OIS, STD, TenorBasis etc...
			for (int index = 0; index < numCurveTypes; index++)
			{
				const std::string& rebuildCurveType = rebuildSequence[index];

				// Loop over all curve collections, searching for curves of the required type to rebuild
				for (auto iter = bumpCurvesCollection_.allCurvesInEachCollection_.begin(); iter != bumpCurvesCollection_.allCurvesInEachCollection_.end(); ++iter)
				{
					AQLString curveCollectionID = iter->first;
					std::set<AQLString> allCurves = iter->second;

					CurveCalibrationData* curveCalibrationData = getYieldCurvePro(curveCollectionID);

					// Loop through all available curves within each curve collection
					for (auto it = allCurves.begin(); it != allCurves.end(); ++it)
					{
						AQLString curveName = *it;

						AQLString curveType = bumpCurvesCollection_.curveTypes_[curveCollectionID + curveName];
						std::string curveTypeStr = curveType.toUpper().getCString();

						if (curveTypeStr == rebuildCurveType)
						{
							// Found a matching curveType. Rebuild it.

							AQLString attrSuffix = "";
							if (curveName != STD)
							{
								attrSuffix = "_" + curveName;
								attrSuffix.toUpper();
							}
							bool isBasisCurve = checkIsBasisCurve(curveCalibrationData, attrSuffix, bumpCurvesCollection_, curveCollectionID + curveName);

							if ( isBasisCurve )
							{
								/* It is possible that the curve-collection may contain more than one basis curve.
								*  For example the JSCC Tibor basis curves.
								*  In this situation, the order in which the curves are rebuilt becomes important.
								*  This means we cannot simply build the current curve - we also have to rebuild
								*  all of its dependent curves too.
								*/
								recalibrateCurveAndAllDependentCurves(curveCalibrationData, curveName, curveCollectionID, isBasisCurve);
							}
							else
							{
								/*
								* If this is a non-basis curve, it is sufficient to simply build this curve.
								* Rely on curveTypesRebuildSequence to ensure all curves in the collection are
								* rebuilt in the correct sequence.
								*/
								recalibrateSingleCurve(curveCalibrationData, curveName, curveCollectionID, isBasisCurve);
							}
							
							
						}
					}
				}
			}
		}
    }

    /* @brief			Check if the current pillar point is built out of spread + swap
    *  @param [in]		objHolder				The pillar point object
    *  @return			TRUE means being built from spread + swap
    */
    bool DeltaGenerator::isBuiltFromBasisSpread( const AQLObjectHolder& objHolder )
    {
        bool ret = false;
        const AQLDataHolder* longTermConvAttr = &objHolder.getData( IR_CALIBRATION_DATA_LONGTERMCONVENTION );
        if ( longTermConvAttr->isDefined() && !longTermConvAttr->isNull() )
        {
            ret = true;
        }
        return ret;
    }


    /* @brief			Check if the given curve is a basis curve
    *  @param [inout]	curveCalibrationData					Pointer to the yield curve pro object
    *  @param [in]		attrSuffix				suffix string used to locate data
    *  @param [in]		bumpCurvesCollection	Collection of curves being bumped
    *  @param [in]		curveName				Current curve
    *  @return			boolean that tells if curve is a basis curve
    */
    bool DeltaGenerator::checkIsBasisCurve( CurveCalibrationData* curveCalibrationData, const AQLString& attrSuffix, const BumpCurvesCollection& bumpCurvesCollection, const AQLString& curveName )
    {
        bool isBasisCurve = false;

        std::string curveType = bumpCurvesCollection.curveTypes_.find( curveName )->second.getCString();
        if ( curveType.size() != 0 )
        {
            if ( curveType == MARKET_KEY::CURVE_TYPE_BASIS || curveType == MARKET_KEY::CURVE_TYPE_TENORBASIS || curveType == MARKET_KEY::CURVE_TYPE_XCCYBASIS )
            {
                isBasisCurve = true;
            }
        }
        else
        {
            AQLDataInstance* dataInstance = etrading::InitializeETrading::instance().dataInstance();
            AQLDataHolder* dh = &curveCalibrationData->getData( IR_CALIBRATION_DATA_MARKETTYPE + attrSuffix );
            if ( dh->isDefined() && !dh->isNull() )
            {
                AQLString marketType = dynamic_cast<AQLDataString&> ( dh->get() );
                if ( marketType.getCString() == etrading::MARKET_KEY::MARKET_TYPE_BASISSWAP )
                {
                    isBasisCurve = true;
                }
            }
        }

        return isBasisCurve;
    }


    //-------------------------------------------------------------------------------------
    // DeltaLadderData


    /* @brief		Constructor
    */
    DeltaGenerator::DeltaLadderData::DeltaLadderData() : isRiskAggregated_( true )
    {
        deltaLadderInMap_.clear();
        bucketNamePrefix_.clear();
        bucketNames_NonAggregated_.clear();
        bucketNames_Aggregated_.clear();
    }

    /* @brief		Add a bucket risk to the delta ladder
    *  @param [in]	curveName							Name of the current curve
    *  @param [in]	curveCollectionID					Name of the relevant curve collection
    *  @param [in]	bucketNameWithoutPrefixCurveName	Name of the current pillar
    *  @param [in]	deltas								Pillar delta for all the trades
    */
    void DeltaGenerator::DeltaLadderData::addBucketRisk( const AQLString& curveName, const AQLString& curveCollectionID, const AQLString& bucketNameWithoutPrefixCurveName, DoubleVector deltas )
    {
        AQLString bucketNameWithPrefixCurveName;
        bool toAggregateRisk = false;
        AQLString keyWithoutPrefixCurveName = curveCollectionID + ":" + bucketNameWithoutPrefixCurveName;
        if ( isRiskAggregated_ )
        {
            // Check if the current piece of risk can be aggregated to any existing risks
            std::map<AQLString, AQLString>::const_iterator iter1 = bucketNamePrefix_.find( keyWithoutPrefixCurveName );
            if ( iter1 != bucketNamePrefix_.end() )
            {
                AQLString prefix = iter1->second;
                if ( prefix.size() != 0 )
                {
                    bucketNameWithPrefixCurveName = prefix + ":" + keyWithoutPrefixCurveName;
                }

                std::map<AQLString, DoubleVector>::const_iterator iter2 = deltaLadderInMap_.find( bucketNameWithPrefixCurveName );
                if ( iter2 != deltaLadderInMap_.end() )
                {
                    toAggregateRisk = true;
                }
            }
        }

        if ( toAggregateRisk )
        {
            // A risk already exists against the same instrument
            auto currentRisk = deltaLadderInMap_.find( bucketNameWithPrefixCurveName );

            DoubleVector newDeltas;
            for ( size_t i = 0; i < deltas.size(); ++i )
            {
                DoubleVector existingDeltas = currentRisk->second;
                double newDelta = existingDeltas[i] + deltas[i];
                newDeltas.push_back( newDelta );
            }

            // Remove the old risk from the map and add a new one in
            // The name of the new risk does not have a curve name prefix
            deltaLadderInMap_.erase( currentRisk );
            deltaLadderInMap_[keyWithoutPrefixCurveName] = newDeltas;

            bucketNamePrefix_[keyWithoutPrefixCurveName] = "";

            auto currentName = std::find( bucketNames_NonAggregated_.begin(), bucketNames_NonAggregated_.end(), bucketNameWithPrefixCurveName );
            bucketNames_NonAggregated_.erase( currentName );

            bucketNames_Aggregated_.push_back( keyWithoutPrefixCurveName );
        }
        else
        {
            AQLString bucketNameWithPrefixCurveName = curveName + ":" + keyWithoutPrefixCurveName;

            deltaLadderInMap_[bucketNameWithPrefixCurveName] = deltas;
            bucketNames_NonAggregated_.push_back( bucketNameWithPrefixCurveName );
            bucketNamePrefix_[keyWithoutPrefixCurveName] = curveName;
        }
    }

    /* @brief		Set the flag that controls risk aggregation
    *  @param [in]	aggregateRisk	Do we aggregate risk or not
    */
    void DeltaGenerator::DeltaLadderData::setIsRiskAggregated( bool aggregateRisk )
    {
        isRiskAggregated_ = aggregateRisk;
    }


    /* @brief		Return delta ladder in presentable format
    *  @param [out]		pillarNames		Name of pillar points
    *  @param [out]		deltas			All the deltas
    */
    void DeltaGenerator::DeltaLadderData::outputDeltaLadder( AQLStringVector& pillarNames, DoubleMatrix& deltas ) const
    {
        pillarNames.clear();
        deltas.clear();

        // Concatenate bucketNames_Aggregated_ to bucketNames_NonAggregated_
        std::vector<AQLString> bucketNames;
        bucketNames.reserve( bucketNames_NonAggregated_.size() + bucketNames_Aggregated_.size() );
        bucketNames.insert( bucketNames.end(), bucketNames_NonAggregated_.begin(), bucketNames_NonAggregated_.end() );
        bucketNames.insert( bucketNames.end(), bucketNames_Aggregated_.begin(), bucketNames_Aggregated_.end() );

        for ( size_t i = 0; i < bucketNames.size(); ++i )
        {
            AQLString bucketName = bucketNames[i];

            DoubleVector deltaVector = deltaLadderInMap_.find( bucketName )->second;

            AQLString outputBucketName;
            AQLStringVector partitioned = bucketName.toToken( ':' );
            if ( partitioned.size() == 2 )
            {
                AQLString collectionName = partitioned[0];
                AQLString shortName		= partitioned[1];

                outputBucketName = collectionName + PILLAR_DELIMITER + shortName;
            }
            else if ( partitioned.size() == 3 )
            {
                AQLString curveName		= partitioned[0];
                AQLString collectionName = partitioned[1];
                AQLString shortName		= partitioned[2];

                outputBucketName = collectionName + PILLAR_DELIMITER + curveName + PILLAR_DELIMITER + shortName;
            }

            pillarNames.push_back( outputBucketName );
            deltas.push_back( deltaVector );
        }
    }

    /* @brief		Get delta ladder size
    *  @return		Delta ladder size
    */
    size_t DeltaGenerator::DeltaLadderData::size() const
    {
        return deltaLadderInMap_.size();
    }

    /* @brief		Build contral delta ladder through up deltas and down deltas
    *  @param [in]	deltaLadderData_up		A delta ladder generated through bumping up
    *  @param [in]	deltaLadderData_down	A delta ladder generated through bumping down
    */
    void DeltaGenerator::DeltaLadderData::buildCentralDeltas( const DeltaLadderData& deltaLadderData_up, const DeltaLadderData& deltaLadderData_down )
    {
        if ( deltaLadderData_up.size() != deltaLadderData_down.size() )
        {
            throw AQLCoreInvalidData( "#Error: Invalid Risk Results: Inconsistent number of pillar risks have been generated between up bumping and down bumping", __FILE__, __LINE__ );
        }

        isRiskAggregated_ = deltaLadderData_up.isRiskAggregated_;

        deltaLadderInMap_.clear();
        bucketNamePrefix_.clear();
        bucketNames_NonAggregated_.clear();
        bucketNames_Aggregated_.clear();

        // Non-aggregated risks
        for ( size_t i = 0; i < deltaLadderData_up.bucketNames_NonAggregated_.size(); ++i )
        {
            AQLString name_up = deltaLadderData_up.bucketNames_NonAggregated_[i];
            DoubleVector deltas_up = deltaLadderData_up.deltaLadderInMap_.find( name_up )->second;

            auto iter_down = deltaLadderData_down.deltaLadderInMap_.find( name_up );
            if ( iter_down == deltaLadderData_down.deltaLadderInMap_.end() )
            {
                throw AQLCoreInvalidData( "#Error: Invalid Risk Results: Inconsistent number of pillar risks have been generated between up bumping and down bumping", __FILE__, __LINE__ );
            }

            DoubleVector deltas_down = deltaLadderData_down.deltaLadderInMap_.find( name_up )->second;

            DoubleVector deltas_central;
            for ( size_t i = 0; i < deltas_up.size(); ++i )
            {
                double deltaCentral = ( deltas_up[i] + deltas_down[i] ) / 2.0;
                deltas_central.push_back( deltaCentral );
            }

            deltaLadderInMap_[name_up] = deltas_central;
            bucketNames_NonAggregated_.push_back( name_up );
        }

        // Aggregated risks
        for ( size_t i = 0; i < deltaLadderData_up.bucketNames_Aggregated_.size(); ++i )
        {
            AQLString name_up = deltaLadderData_up.bucketNames_Aggregated_[i];
            DoubleVector deltas_up = deltaLadderData_up.deltaLadderInMap_.find( name_up )->second;

            auto iter_down = deltaLadderData_down.deltaLadderInMap_.find( name_up );
            if ( iter_down == deltaLadderData_down.deltaLadderInMap_.end() )
            {
                throw AQLCoreInvalidData( "#Error: Invalid Risk Results: Inconsistent number of pillar risks have been generated between up bumping and down bumping", __FILE__, __LINE__ );
            }

            DoubleVector deltas_down = deltaLadderData_down.deltaLadderInMap_.find( name_up )->second;

            DoubleVector deltas_central;
            for ( size_t i = 0; i < deltas_up.size(); ++i )
            {
                double deltaCentral = ( deltas_up[i] + deltas_down[i] ) / 2.0;
                deltas_central.push_back( deltaCentral );
            }
            bucketNames_Aggregated_.push_back( name_up );
        }
    }

	AQLString DeltaGenerator::getCurveDependencyTreeAsString() const
	{
		AQLString output("Dependencies: ");

		for (auto iter = bumpCurvesCollection_.dependentCurve_.begin(); iter != bumpCurvesCollection_.dependentCurve_.end(); ++iter)
		{
			AQLString baseCurve = iter->first;

			output += baseCurve + " -> ( ";
			const std::set<AQLString>& dependencies = iter->second;
			for (auto it = dependencies.begin(); it != dependencies.end(); ++it)
			{
				output += *it + ", ";
			}
			output += "), ";
		}

		return output;
	}

	AQLString DeltaGenerator::getAllCurvesToBumpAsString() const
	{
		AQLString output("AllCurves: ");

		for (auto iter = bumpCurvesCollection_.allCurvesInEachCollection_.begin(); iter != bumpCurvesCollection_.allCurvesInEachCollection_.end(); ++iter)
		{
			AQLString collectionID = iter->first;

			output += collectionID + " -> ( ";
			const std::set<AQLString>& curvenames = iter->second;
			for (auto it = curvenames.begin(); it != curvenames.end(); ++it)
			{
				output += *it + ", ";
			}
			output += "), ";
		}

		return output;
	}
	

    //-------------------------------------------------------------------------------------
    // CurvesToBump

    /* @brief			Default constructor
    */
    DeltaGenerator::BumpCurvesCollection::BumpCurvesCollection()
    {
        allCurvesInEachCollection_.clear();
        curveTypes_.clear();
        dependentCurve_.clear();
    }

    /* @brief			Add a curve to bump
    *  @param [in]		currentCurveName			Name of the curve being shifted
    *  @param [in]		curveCollection				Curve collection ID
    *  @param [in]		discountCurveName			Discount curve
	*  @param [in]		isUsingGlobalCurveEngine	Are curves calibrated by the global curve engine?
    */
    void DeltaGenerator::BumpCurvesCollection::addCurve( const AQLString& currentCurveName, const AQLString& curveCollection, const AQLString& discountCurveName, bool isUsingGlobalCurveEngine)
    {
        AQLString curveCollectionID = curveCollection;
        AQLString curveName = currentCurveName;
		AQ_REQUIRE( curveName.size() > 0, "Unable to set risk dependencies, curveName is missing" )

        // 'allCurvesInEachCollection_' is a collection of curves that will definitely be bumped
        // under each curve collection in the process of delta ladder calculation
        std::set<AQLString>& allCurves = allCurvesInEachCollection_[curveCollectionID];
        allCurves.insert( curveName );
		
		CurveCalibrationData* curveCalibrationData = getYieldCurvePro( curveCollectionID );

		// First check if the discountCurveName is a XCCYBASIS curve.
		// Find the actual discount curve for this curveCollection
		AQLString discountCurve( discountCurveName );

		if (discountCurve.size() > 0)
		{
			AQLString discCurveType = getCurveType( curveCollectionID, discountCurve);
			std::string discCurveTypeStr = discCurveType.toUpper().getCString();

			if ( discCurveTypeStr == MARKET_KEY::CURVE_TYPE_XCCYBASIS || discCurveTypeStr == MARKET_KEY::CURVE_TYPE_FWDFXCONST )
			{
				// Look up the discount curve dependency of this curveCollection
				AQLDataHolder* dh = &curveCalibrationData->getData( IR_CALIBRATION_DATA_DFCURVENAME );
				if ( dh->isDefined() && !dh->isNull() )
				{
					AQLString discountCurveIndex = dynamic_cast<AQLDataString&> ( dh->get() );

					if ( discountCurveIndex != ITSELF )
					{
						// Add this discountCurve dependency to our list of curves to bump
						discountCurve = getCurveStaticDataTableName( curveCollectionID, discountCurveIndex, false );
						allCurves.insert( discountCurve );
					}
				}
				else
				{
					throw AQLCoreInvalidData( "#Error: Unable to find the discount curve on the target side of the xccy swap", __FILE__, __LINE__ );
				}
			}
		}

        // get curve type from curve
		AQLString curveType = getCurveType( curveCollectionID, curveName );
        AQLString key = curveCollectionID + curveName;
        if ( dependentCurve_.find( key ) == dependentCurve_.end() )
        {
            dependentCurve_		[key] = std::set<AQLString>();
			curveTypes_			[key] = curveType;
		}
        
        std::string curveTypeStr = curveType.toUpper().getCString();
		AQ_REQUIRE( curveTypeStr.size() > 0, "Unable to set risk dependencies, unable to determine the curveType for curve index: " + curveName );

        //-------------------------------------------------------------------------------------------------------------------
        // Below we build a number of special curve relationships so that cross curve impacts are accounted for
        //-------------------------------------------------------------------------------------------------------------------

        if ( curveTypeStr == MARKET_KEY::CURVE_TYPE_SWAP )
        {
            // For swap curves, need to build curve dependency so that cross-curve effect is accounted for

			if (!isUsingGlobalCurveEngine)
			{
			AQLString key = curveCollectionID + discountCurve;
			AQLString curveType = getCurveType(curveCollectionID, discountCurve);
			const std::string discountCurveTypeStr = curveType.toUpper().getCString();
			curveTypes_[key] = discountCurveTypeStr.c_str();

			/* Only include the dependency  discountCurve -> forecastCurve if the discount curve is not a XCCYBASIS.
			*  For XCCYBASIS, the build dependency is actually  forecastCurve -> XCCYBASIS and is handled separately
			*/
			if ( discountCurveTypeStr != MARKET_KEY::CURVE_TYPE_XCCYBASIS )
			{
				std::set<AQLString>& dependentCurves = dependentCurve_[key];
				dependentCurves.insert(curveCollectionID + AQLString(":") + curveName);
				}
			}
        }
        else if ( curveTypeStr == MARKET_KEY::CURVE_TYPE_BASIS || curveTypeStr == MARKET_KEY::CURVE_TYPE_TENORBASIS )
        {
			// When the curve concerned is a basis curve we need to add its base curve (a swap curve or another basis curve) into the bump curve collection

			AQLString againstCurveCollectionID;
			AQLString forecastCurveOnOppositeSideIndex;
			if ( getBasisCurveDetails( curveCalibrationData, curveCollectionID, curveName, againstCurveCollectionID, forecastCurveOnOppositeSideIndex ) )
			{
				// Successfully obtained details of the underlying 'against' curve.
				// Perform basic sanity checking:
				// The against curveCollection CCY  should match the curveCollection CCY.
				// If the two currencies do not match, this looks more like a mis-configured XCCY curve.
				AQLString targetCCY  = getCurveCurrency( curveCollectionID );
				AQLString againstCCY = getCurveCurrency( againstCurveCollectionID );
				if ( againstCCY != targetCCY )
				{
					// This looks like a XCCY basis curve, not a regular basis curve
					throw AQLCoreInvalidData( ( boost::format( "#Error: Curve '%s:%s' has not been built correctly. Against curve currency '%s' does not match target curve currency '%s' " ) % curveCollectionID.getCString()
																																								   						 % curveName.getCString()
																																								   						 % againstCCY
																																								   						 % targetCCY ).str().c_str() , __FILE__, __LINE__ );
				}
			}
			else
			{
				// Could not get basis curve information
				throw AQLCoreInvalidData( ( boost::format( "#Error: Could not get base curve information for curve '%s:%s'." ) % curveCollectionID.getCString() % curveName.getCString() ).str().c_str() , __FILE__, __LINE__ );
			}

			// Add forecast curve from the opposite side to the bump curve collection
			AQLString forecastCurveOnOppositeSide = getCurveStaticDataTableName(againstCurveCollectionID, forecastCurveOnOppositeSideIndex, false);

			std::set<AQLString>& allCurves = allCurvesInEachCollection_[againstCurveCollectionID];
			allCurves.insert(forecastCurveOnOppositeSide);

			AQLString key = againstCurveCollectionID + forecastCurveOnOppositeSide;
			std::set<AQLString>& dependentCurves = dependentCurve_[key];
			dependentCurves.insert(curveCollectionID + AQLString(":") + curveName);

			AQLString curveType = getCurveType(againstCurveCollectionID, forecastCurveOnOppositeSide);
			std::string againstCurveTypeStr = curveType.toUpper().getCString();
			curveTypes_[key] = againstCurveTypeStr.c_str();

			if (isUsingGlobalCurveEngine)
			{

				AQLString discountCurveOnOppositeSide = getCurveStaticDataTableName(againstCurveCollectionID, discountCurve, false);
				std::set<AQLString>& allCurves = allCurvesInEachCollection_[againstCurveCollectionID];
				allCurves.insert(discountCurveOnOppositeSide);
				AQLString key = againstCurveCollectionID + discountCurveOnOppositeSide;
				std::set<AQLString>& dependentCurves = dependentCurve_[key];
				dependentCurves.insert(curveCollectionID + AQLString(":") + curveName);
				AQLString curveType = getCurveType(againstCurveCollectionID, discountCurveOnOppositeSide);
				std::string againstCurveTypeStr = curveType.toUpper().getCString();
				curveTypes_[key] = againstCurveTypeStr.c_str();
			}
			else
			{
			//------------------------------------------------------------------------------------------------
			// Need to explicitly establish the dependency between
			// the discount curve and forecast curve on the Against leg

			// Capture additional dependencies when we have two curve collections in the same currency e.g. JPY_JSCC and JPY_TIBOR
			AQLString discountCurveOnOppositeSide = discountCurve;
			if (againstCurveCollectionID != curveCollectionID)
			{
				// Since the opposite-side forecast curve is in a different collection, look up the corresponding discount curve
				CurveCalibrationData* ycProAgainst = getYieldCurvePro(againstCurveCollectionID);
				AQLDataHolder* dh = &ycProAgainst->getData(IR_CALIBRATION_DATA_DFCURVENAME);
				if (dh->isDefined() && !dh->isNull())
				{
					AQLString discountCurveOnOppositeSideIndex = dynamic_cast<AQLDataString&> (dh->get());

					if (discountCurveOnOppositeSideIndex != ITSELF)
					{
						discountCurveOnOppositeSide = getCurveStaticDataTableName(againstCurveCollectionID, discountCurveOnOppositeSideIndex, false);

						AQLString key = againstCurveCollectionID + discountCurveOnOppositeSide;
						std::set<AQLString>& dependentCurves = dependentCurve_[key];
						dependentCurves.insert(againstCurveCollectionID + AQLString(":") + forecastCurveOnOppositeSide);

						AQLString curveType = getCurveType( againstCurveCollectionID, discountCurveOnOppositeSide );
						const std::string discountCurveTypeStr = curveType.toUpper().getCString();
						curveTypes_[key] = discountCurveTypeStr.c_str();

						std::set<AQLString>& againstCurves = allCurvesInEachCollection_[againstCurveCollectionID];
						againstCurves.insert(discountCurveOnOppositeSide);
					}
				}

				// Add dependency on same side discount curve
				AQLString key = curveCollectionID + discountCurve;
				std::set<AQLString>& dependentCurves = dependentCurve_[key];
				dependentCurves.insert(curveCollectionID + AQLString(":") + curveName);

			}

			// Recursively call addCurve() for the 'against' forecast curve
			addCurve(forecastCurveOnOppositeSide, againstCurveCollectionID, discountCurveOnOppositeSide, isUsingGlobalCurveEngine);
			}
        }
        else if ( curveTypeStr == MARKET_KEY::CURVE_TYPE_XCCYBASIS )
        {
            //-----------------------------------------------------------------------------------------
            // 1. Deal with the discount curve on the 'against' side of the xccy basis swap

            // get discount curve (OIS curve in other currency) on the 'against' side of the xccy basis swap. Add it the bump curve collection
            AQLString discountCurveOnOppositeSideIndex;
            AQLString againstCurveCollectionID;

			if (! getBasisCurveDetails( curveCalibrationData, curveCollectionID, curveName, againstCurveCollectionID, discountCurveOnOppositeSideIndex ) )
			{
				throw AQLCoreInvalidData( "#Error: Unable to find the discount curve on the side of the xccy swap that is not the target side", __FILE__, __LINE__ );
			}

			AQLString discountCurveOnOppositeSide = getCurveStaticDataTableName( againstCurveCollectionID, discountCurveOnOppositeSideIndex, false );

            // Add discount curve on the opposite side to the bump curve collection
            std::set<AQLString>& allCurves = allCurvesInEachCollection_[againstCurveCollectionID];
            allCurves.insert( discountCurveOnOppositeSide );

			AQLString key = againstCurveCollectionID + discountCurveOnOppositeSide;

			AQLString againstCurveType = getCurveType( againstCurveCollectionID, discountCurveOnOppositeSide );
			const std::string againstCurveTypeStr = againstCurveType.toUpper().getCString();
			curveTypes_[key] = againstCurveTypeStr.c_str();

            //-----------------------------------------------------------------------------------------
            // 2. Deal with the forecast curve on the 'against' side of the xccy basis swap

            // get the forecast curve (swap curve in other currency) from xccy basis curve. Add it the bump curve collection
            AQLString forecastCurveOnOppositeSideIndex;

			AQLString attrSuffix = "_" + curveName;
			attrSuffix.toUpper();
            AQLDataHolder* dh = &curveCalibrationData->getData( CALIBRATION_DATA_BASISCURVESECONDARYBASE + attrSuffix );
            if ( dh->isDefined() && !dh->isNull() )
            {
                forecastCurveOnOppositeSideIndex = dynamic_cast<AQLDataString&> ( dh->get() );

                // The base curve name can either be in the form of "STD" or "EURYC:STD"
                AQLStringVector partitioned = forecastCurveOnOppositeSideIndex.toToken( ':' );
                if ( partitioned.size() == 1 )
                {
                    againstCurveCollectionID = curveCollectionID;
                    forecastCurveOnOppositeSideIndex = partitioned[0];
                }
                else if ( partitioned.size() == 2 )
                {
                    againstCurveCollectionID = partitioned[0];
                    forecastCurveOnOppositeSideIndex = partitioned[1];
                }
            }
            else
            {
                throw AQLCoreInvalidData( "#Error: Unable to find the forecast curve on the side of the xccy swap that is not the target side", __FILE__, __LINE__ );
            }

			AQLString forecastCurveOnOppositeSide = getCurveStaticDataTableName(againstCurveCollectionID, forecastCurveOnOppositeSideIndex, false);

            // Add forecast curve on the opposite side to the bump curve collection
			std::set<AQLString>& allCurves2 = allCurvesInEachCollection_[againstCurveCollectionID];
            allCurves2.insert( forecastCurveOnOppositeSide );

			key = againstCurveCollectionID + forecastCurveOnOppositeSide;
			std::set<AQLString>& dependentCurves2 = dependentCurve_[key];
            dependentCurves2.insert( curveCollectionID + AQLString( ":" ) + curveName );

			AQLString curveType = getCurveType( againstCurveCollectionID, forecastCurveOnOppositeSide );
			std::string forecastOppositeCurveTypeStr = curveType.toUpper().getCString();
            curveTypes_			[key] = forecastOppositeCurveTypeStr.c_str();

			// Recursively call addCurve() for the 'against' curve
			addCurve( forecastCurveOnOppositeSide, againstCurveCollectionID, discountCurveOnOppositeSide, isUsingGlobalCurveEngine);

			// Only add in the dependency from the discountCurveOnOppositeSide to the XCCY curve
			// if this dependency is not indirectly present via other curves
			if ( ! findDependencyPath( againstCurveCollectionID, discountCurveOnOppositeSide, curveCollectionID, curveName ) )
			{
				AQLString key = againstCurveCollectionID + discountCurveOnOppositeSide;
				std::set<AQLString>& dependentCurves = dependentCurve_[key];
				dependentCurves.insert( curveCollectionID + AQLString( ":" ) + curveName );
			}

			//-----------------------------------------------------------------------------------------
            // 3. Deal with the forecast curve on the 'same' side of the xccy basis swap

            // get the forecast curve (swap curve) from xccy basis curve. Add it the bump curve collection
			attrSuffix = "_" + curveName;
			attrSuffix.toUpper();
			AQLString sameSideCurveCollectionID;
            AQLString forecastCurveOnSameSideIndex;
            dh = &curveCalibrationData->getData( CALIBRATION_DATA_XCCYSAMESIDEFORECASTCURVE + attrSuffix );
            if ( dh->isDefined() && !dh->isNull() )
            {
                forecastCurveOnSameSideIndex = dynamic_cast<AQLDataString&> ( dh->get() );

				// The base curve name can either be in the form of "STD" or "EURYC:STD"
                AQLStringVector partitioned = forecastCurveOnSameSideIndex.toToken( ':' );
                if ( partitioned.size() == 1 )
                {
                    sameSideCurveCollectionID = curveCollectionID;
                    forecastCurveOnSameSideIndex = partitioned[0];
                }
                else if ( partitioned.size() == 2 )
                {
                    sameSideCurveCollectionID = partitioned[0];
                    forecastCurveOnSameSideIndex = partitioned[1];
                }
            }
            else
            {
                throw AQLCoreInvalidData( "#Error: Unable to find the forecast curve on the target side of the xccy swap", __FILE__, __LINE__ );
            }

			// Add the dependency forecastCurve -> XCCY curve, provided the forecast curve is not set to FIXED_RATE
			if ( ! same( forecastCurveOnSameSideIndex, IRS_KEY::FIXED_RATE ) )
			{
				AQLString forecastCurveOnSameSide = getCurveStaticDataTableName( sameSideCurveCollectionID, forecastCurveOnSameSideIndex, false );

				// Add forecast curve on the same side to the bump curve collection
				std::set<AQLString>& allCurves3 = allCurvesInEachCollection_[sameSideCurveCollectionID];
				allCurves3.insert( forecastCurveOnSameSide );

				// Set the same side forecast curve as a dependency of the XCCYBASIS curve
				key = sameSideCurveCollectionID + forecastCurveOnSameSide;
				std::set<AQLString>& dependentCurves3 = dependentCurve_[key];
				dependentCurves3.insert( curveCollectionID + AQLString( ":" ) + curveName );

				AQLString forecastCurveType = getCurveType( sameSideCurveCollectionID, forecastCurveOnSameSide );
				std::string forecastSameCurveTypeStr  = forecastCurveType.toUpper().getCString();
				curveTypes_	[key] = forecastSameCurveTypeStr.c_str();
			
				// Look up sameside discount curve here
				AQLString discountCurveOnSameSideIndex;

				dh = &curveCalibrationData->getData( IR_CALIBRATION_DATA_DFCURVENAME );
				if ( dh->isDefined() && !dh->isNull() )
				{
					discountCurveOnSameSideIndex = dynamic_cast<AQLDataString&> ( dh->get() );

					if ( discountCurveOnSameSideIndex != ITSELF )
					{
						// Add discount curve on the same side to the bump curve collection
						AQLString discountCurveOnSameSide = getCurveStaticDataTableName( curveCollectionID, discountCurveOnSameSideIndex, false );

						std::set<AQLString>& allCurves = allCurvesInEachCollection_[curveCollectionID];
						allCurves.insert( discountCurveOnSameSide );

						// Recursively call addCurve() for the 'sameside' curves
						addCurve( forecastCurveOnSameSide, curveCollectionID, discountCurveOnSameSide, isUsingGlobalCurveEngine);
					}
				}
				else
				{
					throw AQLCoreInvalidData( "#Error: Unable to find the discount curve on the target side of the xccy swap", __FILE__, __LINE__ );
				}
			}

        }
		else if ( curveTypeStr == MARKET_KEY::CURVE_TYPE_FWDFXCONST )
		{
			// Get the dependency data associated with the constant FX Forward curve
			AQLString attrSuffix = "_" + curveName;
			attrSuffix.toUpper();
			AQLDataHolder* dh = &curveCalibrationData->getData( CALIBRATION_DATA_MARKETDATA + attrSuffix );
			if ( dh->isDefined() && !dh->isNull() )
			{
				// The  dependent curve names of the FX FWD curve are stored as market data
				AQLDataMultiReference& refMarketData = dynamic_cast<AQLDataMultiReference&> ( dh->get() );
				if ( refMarketData.getSize() != 1)
				{
					AQLString errMsg("#Error: Incorrect MarketData size in constant fx forward curve. Expecting a data block of size: 1. Actual size: ");
					errMsg += refMarketData.getSize();
					throw AQLCoreInvalidData( errMsg.getCString(), __FILE__, __LINE__ );
				}

				AQLObjectHolder& objHolder = refMarketData.get( 0 );
				AQLString name = dynamic_cast<const AQLDataString&> ( ( objHolder.getData( CALIBRATION_DATA_NAME, ISNOTNULL ) ).get() ).get();
				AQLString dataType =  dynamic_cast<const AQLDataString&> ( ( objHolder.getData( IR_CALIBRATION_DATA_DATATYPE, ISNOTNULL ) ).get() ).get();
				AQLString sameSideForecastIndex =  dynamic_cast<const AQLDataString&> ( ( objHolder.getData( IR_CALIBRATION_DATA_FORECAST, ISNOTNULL ) ).get() ).get();
				AQLString againstForecastIndex =  dynamic_cast<const AQLDataString&> ( ( objHolder.getData( IR_CALIBRATION_DATA_AGTFORECAST, ISNOTNULL ) ).get() ).get();
				AQLString againstDiscountIndex =  dynamic_cast<const AQLDataString&> ( ( objHolder.getData( IR_CALIBRATION_DATA_AGTDISCOUNT, ISNOTNULL ) ).get() ).get();
				AQLString againstCurveCollectionID = dynamic_cast<const AQLDataString&> ( ( objHolder.getData( IR_CALIBRATION_DATA_AGTCURVECOLLECTION, ISNOTNULL ) ).get() ).get();

				//-----------------------------------------------------------------------------------------
				// 1. Deal with the discount curve on the 'against' side of the fx fwd const curve
				
				// get discount curve (OIS curve in other currency) on the 'against' side of the constant fx forward curve. Add it the bump curve collection
				AQLString againstDiscountCurve = getCurveStaticDataTableName( againstCurveCollectionID, againstDiscountIndex, false );

				// Add discount curve on the opposite side to the bump curve collection
				std::set<AQLString>& allCurves = allCurvesInEachCollection_[againstCurveCollectionID];
				allCurves.insert( againstDiscountCurve );

				AQLString key = againstCurveCollectionID + againstDiscountCurve;

				AQLString discCurveType = getCurveType( againstCurveCollectionID, againstDiscountCurve );
				std::string againstDiscountCurveTypeStr = discCurveType.toUpper().getCString();
				curveTypes_	[key] = againstDiscountCurveTypeStr.c_str();
				
				//-----------------------------------------------------------------------------------------
				// 2. Deal with the forecast curve on the 'against' side of the constant fx-fwd curve
				
				// get the forecast curve (swap curve in other currency) from constant fx forward curve. Add it the bump curve collection
				AQLString againstForecastCurve = getCurveStaticDataTableName(againstCurveCollectionID, againstForecastIndex, false);

				// Add forecast curve on the opposite side to the bump curve collection
				std::set<AQLString>& allCurves2 = allCurvesInEachCollection_[againstCurveCollectionID];
				allCurves2.insert( againstForecastCurve );

				key = againstCurveCollectionID + againstForecastCurve;
				std::set<AQLString>& dependentCurves2 = dependentCurve_[key];
				dependentCurves2.insert( curveCollectionID + AQLString( ":" ) + curveName );

				AQLString forecastCurveType = getCurveType( againstCurveCollectionID, againstForecastCurve );
				std::string forecastOppositeCurveTypeStr = forecastCurveType.toUpper().getCString();
				curveTypes_	[key] = forecastOppositeCurveTypeStr.c_str();

				// Recursively call addCurve() for the 'against' curve
				addCurve( againstForecastCurve, againstCurveCollectionID, againstDiscountCurve, isUsingGlobalCurveEngine);
				
				// Only add in the dependency from the againstDiscountCurve to the forward-fx curve
				// if this dependency is not indirectly present via other curves
				if ( ! findDependencyPath( againstCurveCollectionID, againstDiscountCurve, curveCollectionID, curveName ) )
				{
					AQLString key = againstCurveCollectionID + againstDiscountCurve;

					std::set<AQLString>& dependentCurves = dependentCurve_[key];
					dependentCurves.insert( curveCollectionID + AQLString( ":" ) + curveName );
				}

				//-----------------------------------------------------------------------------------------
				// 3. Deal with the forecast curve on the 'same' side of the constant fx-fwd curve

				// get the forecast curve (swap curve) from constant fx forward curve. Add it the bump curve collection
				AQLString sameSideForecastCurve = getCurveStaticDataTableName( curveCollectionID, sameSideForecastIndex, false );

				// Add forecast curve on the same side to the bump curve collection
				std::set<AQLString>& allCurves3 = allCurvesInEachCollection_[curveCollectionID];
				allCurves3.insert( sameSideForecastCurve );
				
				key = curveCollectionID + sameSideForecastCurve;
				AQLString forecastSameCurveType = getCurveType( curveCollectionID, sameSideForecastCurve );
				std::string forecastSameCurveTypeStr  = forecastSameCurveType.toUpper().getCString();
				curveTypes_	[key] = forecastSameCurveTypeStr.c_str();

				// Only add in the dependency from the sameSideForecastCurve to the forward-fx curve
				// if this dependency is not indirectly present via other curves
				if (! findDependencyPath( curveCollectionID, sameSideForecastCurve, curveCollectionID, curveName ) )
				{
					std::set<AQLString>& dependentCurves3 = dependentCurve_[key];
					dependentCurves3.insert( curveCollectionID + AQLString( ":" ) + curveName );
				}

				// Look up sameside discount curve here
				AQLString discountCurveOnSameSideIndex;

				dh = &curveCalibrationData->getData( IR_CALIBRATION_DATA_DFCURVENAME );
				if ( dh->isDefined() && !dh->isNull() )
				{
					discountCurveOnSameSideIndex = dynamic_cast<AQLDataString&> ( dh->get() );

					if ( discountCurveOnSameSideIndex != ITSELF )
					{
						// Add discount curve on the same side to the bump curve collection
						AQLString sameSideDiscountCurve = getCurveStaticDataTableName( curveCollectionID, discountCurveOnSameSideIndex, false );

						std::set<AQLString>& allCurves = allCurvesInEachCollection_[curveCollectionID];
						allCurves.insert( sameSideDiscountCurve );

						// Recursively call addCurve() for the 'sameside' curves
						addCurve( sameSideForecastCurve, curveCollectionID, sameSideDiscountCurve, isUsingGlobalCurveEngine);
					}
				}
				else
				{
					throw AQLCoreInvalidData( "#Error: Unable to find the discount curve on the target side of the constant fx forward curve", __FILE__, __LINE__ );
				}
			}
			else
			{
				throw AQLCoreInvalidData( ( boost::format( "#Error: FXFWDCONST curve '%s:%s' has not been built correctly." ) % curveCollectionID.getCString() % curveName.getCString() ).str().c_str() , __FILE__, __LINE__ );
			}
		}
		else if ( curveTypeStr == MARKET_KEY::CURVE_TYPE_OIS )
		{
			// For OIS curves, need to build curve dependency so that cross-curve effect is accounted for

			// If we have been given a discountCurve, and it is different to the curveName,
			// Then include the dependency.
			if ( ( discountCurve.size() > 0 ) && ( curveName != discountCurve ) )
			{
				AQLString key = curveCollectionID + discountCurve;
				std::set<AQLString>& dependentCurves = dependentCurve_[key];
				dependentCurves.insert(curveCollectionID + AQLString(":") + curveName);

				AQLString curveType = getCurveType( curveCollectionID, discountCurve );
				const std::string discountCurveTypeStr = curveType.toUpper().getCString();
				curveTypes_[key] = discountCurveTypeStr.c_str();
			}

		}
		else if ( curveTypeStr == MARKET_KEY::CURVE_TYPE_ARR )
		{
			// No further work to do for ARR. 
			// It does not depend on any other curves
		}
		else
		{
			AQLString errMsg("#Error: Invalid CurveType for CurveIndex: " );
			errMsg += curveName.c_str();
			errMsg += ", Risk Calculation does not support CurveType: ";
			errMsg += curveTypeStr.c_str();
			throw AQLCoreInvalidData( errMsg.getCString(), __FILE__, __LINE__ );
		}
    }
	
	/* @brief	Follows the chain of dependencies and determines whether there is a dependency path
	*           from: startCurveCollectionID:startCurveName
	*			to:   endCollectionID:endCurveName
	*  @param [in]		startCurveCollectionID	The CurveCollectionID of the starting point
	*  @param [in]		startCurveName			The CurveName of the starting point
	*  @param [in]		endCurveCollectionID	The CurveCollectionID of the end point
	*  @param [in]		endCurveName			The CurveName of the end point
	*  @param [out]     Returns true if a path exists
	*/
	bool DeltaGenerator::BumpCurvesCollection::findDependencyPath( const AQLString& startCurveCollectionID, const AQLString& startCurveName,
																   const AQLString& endCurveCollectionID, const AQLString& endCurveName )
	{
		if ( startCurveCollectionID == endCurveCollectionID && startCurveName == endCurveName )
		{
			// A dependency path has been found
			return true;
		}

		bool pathExists = false;

		const std::set<AQLString>& dependentCurves = dependentCurve_[startCurveCollectionID + startCurveName];
		for ( auto iter = dependentCurves.begin(); iter != dependentCurves.end(); ++iter )
		{
			const AQLString& dependentCurve = *iter;
			if ( dependentCurve.size() != 0 )
			{
				AQLStringVector partitioned = dependentCurve.toToken( ':' );
				AQLString dependentCurveCollectionID = partitioned[0];
				AQLString dependentCurveName			= partitioned[1];

				if ( findDependencyPath( dependentCurveCollectionID, dependentCurveName, endCurveCollectionID, endCurveName ) )
				{
					pathExists = true;
					break;
				}
			}
		}
		return pathExists;
	}

}

