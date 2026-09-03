//
//  CurveCalibration.cpp
//  This file used to be called YieldGenerator.cpp and before that LAPriceYieldGenerator.cpp
//
#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include <boost/format.hpp>
#include <algorithm>
#include <numeric>

#include "CurveCalibration.h"
#include "AQLMathDefine.h"
#include "AQLDataBasics.h"
#include "AQLDataVector.h"
#include "AQLDataMatrix.h"
#include "AQLInterpolationBase.h"
#include "AQLBasic.h"
#include "AQLMatrix.h"
#include "AQLPriceDataInterpolation.h"
#include "AQLPriceDataSlidingRule.h"
#include "AQLDataMultiReference.h"
#include "AQLPriceDataManager.h"
#include "AQLDataReference.h"
#include "AQLObjectPool.h"
#include "AQLDataInstance.h"
#include "LADefinitions.h"

#include "BasisComponentCurve.h"
#include "LADateHelpers.h"
#include "LADateScheduleHelpers.h"
#include "LACurveForwardRateHelpers.h"
#include "AQLAlgorithm.h"
#include "AQLLinearInterpolation.h"
#include "AQLLinearSplineInterpolation.h"
#include "AQLLinearMonotoneSplineInterpolation.h"
#include "AQLDate.h"
#include "CurveInstruments.h"
#include "ExceptionMacros.h"

#include "OISComponentCurve.h"
#include "SwapComponentCurve.h"
#include "GlobalCurveCalibrationEngine.h"
#include "RecordMacros.h"
#include "LACoreDataService.h"
#include "LAStaticDataManager.h"
#include "CurveResultsContainer.h" // for curve container, pre- and post-results accessors

namespace etrading
{
	namespace curveParameters
	{

		// Important Note: Here we Deprecate LA Define Statements from Common and IR Projects
		// #define statements have been replaced as AQLString with AlgoQuantLib prefix

		const AQLString AQ_O_N				= "O_N";
		const AQLString AQ_T_N				= "T_N";
		const AQLString AQ_ON				= "ON";
		const AQLString AQ_TN				= "TN";
		const AQLString AQ_LIBOR_DATA		= "ZERORATE";	// Libor or Zero Rate
		const AQLString AQ_ZERORATE		= "ZERORATE";	// Libor or Zero Rate
		const AQLString AQ_SWAP_DATA		= "PARRATE";	// Swap Par Rate
		const AQLString AQ_PARRATE			= "PARRATE";	// Swap Par Rate
		const AQLString AQ_FWD				= "FWDRATE";
		const AQLString AQ_BASIS			= "BASISRATE";
		const AQLString AQ_MPC_SWAP		= "BOJRATE";	// This is meant to read Central Bank MPC Swaps not just BOJ Central Bank
		const AQLString AQ_FOMC_SWAP		= "FFRATE";		// FED FUND MPC SWAP i.e FOMC Swap
		const AQLString AQ_ARR_FUTURE		= "ARRFUTURE";
		const AQLString AQ_STD				= "STD";
		const AQLString AQ_FRA3M			= "FRA3M";
		const AQLString AQ_FRA6M			= "FRA6M";
		const AQLString AQ_TERM_3M			= "3M";
		const AQLString AQ_TERM_6M			= "6M";
		const AQLString AQ_TERM_1M			= "1M";
		const AQLString AQ_TERM_12M		= "12M";

		const AQLString AQ_SIMPLE			= "SIMPLE";
		const AQLString AQ_ANNUAL			= "ANNUAL";
		const AQLString AQ_SEMI_ANNUAL		= "SEMI-ANNUAL";
		const AQLString AQ_QUARTERLY		= "QUARTERLY";
		const AQLString AQ_MONTHLY			= "MONTHLY";
		const AQLString AQ_LUNAR			= "LUNAR";
		const AQLString AQ_BUSINESS_DAYS	= "BUSINESS_DAYS";

		const AQLString AQ_ROLLCONV_NORMAL = "NORMAL";
		const AQLString AQ_ROLLCONV_EOM	= "EOM";
		const AQLString AQ_ROLLCONV_LUNAR	= "LUNAR";

		const AQLString AQ_LIBOR			= "LIBOR";
		const AQLString AQ_FUTURE			= "FUTURE";
		const AQLString AQ_FRA				= "FRA";
		const AQLString AQ_SWAP			= "SWAP";
	}
}

// Namespaces
using namespace etrading;
using namespace etrading::curveParameters;

// Global Parameters
const int RATE_PRIORITY_PROXIMITY_DAY_TOLERANCE = 20;

// Method to Initialize Swap Curve DiscountFactors
SwapCurveDiscountFactors initialiseSwapCurveDiscountFactors( LACurveStaticDataHolder & staticDataObj, const LiborIndex & liborIndex, CalibrationResults & resultsObj )
{
	// Initialize Discount Factor Results
	SwapCurveDiscountFactors results;

	results.swapSpotDateDF_		= 1.0;
	results.swapSpotDateTerm_	= 0.0;
		
	results.dfInterpolator_ = std::unique_ptr<AQLInterpolationBase>( dynamic_cast<AQLInterpolationBase *>( resultsObj.interpolationObj_->interpolatorForDiscountFactors_->clone() ) );
	results.dfInterpolator_->setJoinDateAsDouble(resultsObj.interpolationObj_->interpolationJoinDateAsDouble_);
	
	const bool isCurveSelfDiscounting = (staticDataObj.dfCurveName_ == "" || staticDataObj.dfCurveName_ == ITSELF );
	if (!isCurveSelfDiscounting)
	{
		// Non-STD Curves Require Curve Suffix
		AQLString suffix = "";
		if (staticDataObj.dfCurveName_ != AQ_STD)
		{
			suffix = AQLString("_") + staticDataObj.dfCurveName_;
		}

		// Get Terms from Discount Curve
		if (!staticDataObj.curveDataObjectHolder_.getData(CALIBRATION_DATA_TERMS + suffix).isDefined())
		{
			throw AQLCoreInvalidData("Error: Dependency curves have not been built. Terms error", __FILE__, __LINE__);
		}
		const DoubleVector &terms = dynamic_cast<const AQLDataDoubles&> (staticDataObj.curveDataObjectHolder_.getData(CALIBRATION_DATA_TERMS + suffix, ISNOTNULL).get()).get();

		// Get Discount Factors from Discount Curve
		if (!staticDataObj.curveDataObjectHolder_.getData(IR_CALIBRATION_DATA_DFS + suffix).isDefined())
		{
			throw AQLCoreInvalidData("Error: Dependency curves have not been built. Discount error", __FILE__, __LINE__);
		}
		const DoubleVector &dfs = dynamic_cast<const AQLDataDoubles&> (staticDataObj.curveDataObjectHolder_.getData(IR_CALIBRATION_DATA_DFS + suffix, ISNOTNULL).get()).get();

		// Copy Terms and DFs to our Results Interpolator
		AQLPriceDataInterpolation interpolationDataAttribute = etrading::getYieldGenInterpolationByCurveName(staticDataObj.curveDataObjectHolder_, suffix);
		results.dfInterpolator_ = std::unique_ptr<AQLInterpolationBase>(dynamic_cast<AQLInterpolationBase*>(interpolationDataAttribute.getMethod().clone()));
		results.dfInterpolator_->set( terms, dfs, resultsObj.interpolationObj_->interpolationJoinDateAsDouble_ );

		AQLPriceDataDayCount dc_act365(ACT_365);
		results.swapSpotDateTerm_ = dc_act365.getTerm(resultsObj.asOfDate_, liborIndex.curveSpotDate_ ); // note curve spot date = swap spot date
		results.swapSpotDateDF_ = results.dfInterpolator_->value(results.swapSpotDateTerm_);
	}
	return results;
}

// Curve Data Holder for the 'CalibrateModel' Method
LACurveStaticDataHolder::LACurveStaticDataHolder( AQLObject & curveDataObject,
                                                  CurveCalibrationData & curveData,
                                                  CurveProperties& curveProperties,
                                                  const AQLDataProcedure & curveAttributeData,
                                                  const AQLString & curveCollection,  // curveID
                                                  const AQLString & curveIndex )     // or marketName
	: curveDataObject_(curveDataObject),
      curveCalibrationData_(curveData),
      curveProperties_(curveProperties),
      curveAttributeData_(curveAttributeData),
      curveCollection_(curveCollection),
      curveIndex_(curveIndex)
{
	// 1. Initialise Calibration Data Objects
	// *******************************************************************
	( void )curveAttributeData_;
	const AQLDataReference& dataReference = curveCalibrationData_.getYieldData();
	curveDataObjectHolder_ = dataReference.get();

	// 2. Get the Target Curve and the Target Curve Market Data Suffix
	// *******************************************************************
	targetCurve_			= "";
	targetCurveMktSuffix_	= "";

	const AQLDataHolder *dh = &curveCalibrationData_.getData( IR_CALIBRATION_DATA_GENTARGETDF, NOCHECK );
	if( dh->isDefined() && !dh->isNull() )
	{
		targetCurve_ = dynamic_cast< const AQLDataString & >( dh->get() ).get();
	}

	if( AQLString( targetCurve_ ).toUpper() == AQ_STD )
	{
		targetCurve_ = AQ_STD;
	}
	else
	{
		 targetCurveMktSuffix_ = AQLString( "_" ) + curveCalibrationData_.getMarketForCurve( targetCurve_ );
	}

	// 3. Get the Target Curve Alias List and List of Curves Already Built
	// *******************************************************************
	listOfCurvesAlreadyBuilt_	= curveCalibrationData_.getGCurveGenerateMap();
	targetCurveAliasList_		= curveCalibrationData_.getAssignedCurveMktMap();
	
	// 4. Get the dfCurveName
	// *******************************************************************
	dfCurveName_ = ITSELF;
	dh = &curveCalibrationData_.getData( IR_CALIBRATION_DATA_DFCURVENAME + targetCurveMktSuffix_, NOCHECK );
	if( dh->isDefined() && !dh->isNull() )
	{
		dfCurveName_ = dynamic_cast< const AQLDataString & >( dh->get() ).get();
	}
}

// Update Swap Curve State Variables
void LACurveStaticDataHolder::updateSwapCurveStateVariables( CurveMarketDataHolder & mktDataObj )
{
	curveProperties_.updateSwapCurveStateVariables( *this, mktDataObj );
}

// Curve resultsObj container helper for the CurveDataProvider class
CalibrationResults::CalibrationResults( LACurveStaticDataHolder & staticDataObj, const AQLDate& asOfDate )
	: interpolationObj_(nullptr), asOfDate_(asOfDate), dfResults_(DiscountFactors()), fwdStartEndDatesAsTerms_(DoubleMatrix()), fwdRates_(DoubleArray())
{
	interpolationObj_= std::shared_ptr<InterpolationDataHolder>( new InterpolationDataHolder( staticDataObj ) );

	isHybridDiscountFactorInterpolation_	= isHybridDiscountFactorInterpolation( *this );
	isHybridSwapsInterpolation_				= isHybridSwapInterpolation( *this );
	isHybridFuturesAndFRAsInterpolation_	= isHybridFuturesAndFRAsInterpolation( *this );
	doesCurveUseHybridInterpolation_		= isHybridDiscountFactorInterpolation_ || isHybridSwapsInterpolation_ || isHybridFuturesAndFRAsInterpolation_;
}

// Curve resultsObj container helper for the CurveDataProvider class
CalibrationResults::CalibrationResults( LACurveStaticDataHolder & staticDataObj,
										const AQLDate & asOfDate,
										DiscountFactors & dfResults,
										DoubleMatrix& fwdStartEndDatesAsTerms,
										DoubleArray& fwdRates )
	: interpolationObj_(nullptr), asOfDate_(asOfDate), dfResults_(dfResults), fwdStartEndDatesAsTerms_(fwdStartEndDatesAsTerms), fwdRates_(fwdRates)
{
	interpolationObj_= std::shared_ptr<InterpolationDataHolder>( new InterpolationDataHolder( staticDataObj ) );

	isHybridDiscountFactorInterpolation_	= isHybridDiscountFactorInterpolation( *this );
	isHybridSwapsInterpolation_				= isHybridSwapInterpolation( *this );
	isHybridFuturesAndFRAsInterpolation_	= isHybridFuturesAndFRAsInterpolation( *this );
	doesCurveUseHybridInterpolation_		= isHybridDiscountFactorInterpolation_ || isHybridSwapsInterpolation_ || isHybridFuturesAndFRAsInterpolation_;
}

// Curve resultsObj container helper
void CalibrationResults::resetAll()
{
	// Discount Factors and Forwards
	asOfDate_						= AQLDate();
	dfResults_						= DiscountFactors();
	fwdStartEndDatesAsTerms_		= DoubleMatrix();
	fwdRates_						= DoubleArray();
	
	// Risk
	inverseJacobian_				= DoubleMatrix();
	jacobian_						= DoubleMatrix();
	jacobianStateVariables_			= DoubleArray();

	// Initialize Interpolator
	interpolationObj_				= nullptr;
}

// Curve resultsObj container helper
void CalibrationResults::resetForwardsAndDiscFactors()
{
	// Reset Discount Factors
	dfResults_ = DiscountFactors();
	
	// Reset Forwards
	fwdStartEndDatesAsTerms_.clear();
	fwdStartEndDatesAsTerms_.resize(2);
	fwdRates_.clear();
}

CurveProperties::CurveProperties() 
	: useFutures_(false),
	  useFRAs_(false),
	  ratePriorityStringVector_(nullptr),
	  useTenorBasisSwaps_(false),
	  prioritizeSwaps_(true),
	  implyForwards_(true),
	  stateVariableFRAs_(STATE_VARIABLE_LOG_DF),
	  stateVariableFutures_(STATE_VARIABLE_LOG_DF),
	  stateVariableSwaps_(STATE_VARIABLE_ZERO_RATE_TIMES_TIME),
	  stateVariableTenorBasis_(STATE_VARIABLE_ZERO_RATE_TIMES_TIME)
{};

// Curve Instrument Data helper for the calibrateModel methods
CurveProperties::CurveProperties( CurveCalibrationData & curveData )
{
	// Target Curve Info
	AQLString targetCurve = "";
	AQLString targetCurveMktSuffix = "";

	const AQLDataHolder *dh = &curveData.getData( IR_CALIBRATION_DATA_GENTARGETDF, NOCHECK );
	if( dh->isDefined() && !dh->isNull() )
	{
		targetCurve = dynamic_cast< const AQLDataString & >( dh->get() ).get();
		targetCurve.toUpper();
	}

	if( targetCurve != AQ_STD )
	{
		// Non-STD Swap Curves have a market data suffix
		targetCurveMktSuffix = AQLString( "_" ) + curveData.getMarketForCurve( targetCurve );
	}


	// Update 'useFutures_'
	useFutures_= false;
	dh = &curveData.getData( IR_CALIBRATION_DATA_ISFUTUREUSE + targetCurveMktSuffix, NOCHECK );
	if( dh->isDefined() && !dh->isNull() )
	{
		useFutures_ = dynamic_cast< const AQLDataBool& >( dh->get() );
	}
	
	// Update 'useFRAs_'
	useFRAs_ = false;
	dh = &curveData.getData( IR_CALIBRATION_DATA_ISFRAUSE + targetCurveMktSuffix, NOCHECK );
	if( dh->isDefined() && !dh->isNull() )
	{
		useFRAs_ = dynamic_cast< const AQLDataBool& >( dh->get() );
	}
	
	// Check useFutures and useFRAs not enabled at the same time
	AQ_THROW_IF( useFutures_ && useFRAs_, "Invalid Calibration Settings: Calibration to both Futures and FRAs is not supported" )

	// Update 'ratePriorityStringVector_'
	ratePriorityStringVector_ = nullptr;
	dh = &curveData.getData( PRICING_DATA_RATEPRIORITY + targetCurveMktSuffix, NOCHECK );
	if( dh->isDefined() && !dh->isNull() )
	{
		ratePriorityStringVector_ = &( dynamic_cast< const AQLDataStrings& >( dh->get() ).get() );
	}
	
	// Update 'useTenorBasisSwaps_' also known as 'swapTenorAdjust'
	useTenorBasisSwaps_ = false;
	dh = &curveData.getData( IR_CALIBRATION_DATA_ISSWAPTENORADJUST + targetCurveMktSuffix, NOCHECK );
	if( dh->isDefined() && !dh->isNull() )
	{
		useTenorBasisSwaps_ = dynamic_cast< const AQLDataBool& >( dh->get() );
	}

	// Update 'prioritizeSwaps_' also known as 'generateForwardsFromSwapsOnly'
	prioritizeSwaps_ = true;
	dh = &curveData.getData( IR_CALIBRATION_DATA_GENERATEFORWARDSFROMSWAPSONLY + targetCurveMktSuffix, NOCHECK );
	if( dh->isDefined() && !dh->isNull() )
	{
		prioritizeSwaps_ = dynamic_cast< const AQLDataBool& >( dh->get() );
	}

	// Imply Forwards when calibrating curves
	implyForwards_ = true;
}

// Curve Instrument Data helper for the calibrateModel methods
CurveProperties::CurveProperties( LACurveStaticDataHolder & staticDataObj )
{
	CurveProperties( staticDataObj.curveCalibrationData_ );	
}

// Update State Variable Method
void CurveProperties::updateSwapCurveStateVariables( LACurveStaticDataHolder & staticDataObj, CurveMarketDataHolder & mktDataObj )
{
	// State Variable Defaults
	stateVariableFRAs_			= STATE_VARIABLE_LOG_DF;
	stateVariableFutures_		= STATE_VARIABLE_LOG_DF;
	stateVariableSwaps_			= STATE_VARIABLE_ZERO_RATE_TIMES_TIME;
	stateVariableTenorBasis_	= STATE_VARIABLE_ZERO_RATE_TIMES_TIME;

	// Initialize DataHolder
	const AQLDataHolder* dh = 0;

	// Set Swap State Variable 
	dh = &(staticDataObj.curveDataObjectHolder_.getData(IR_CALIBRATION_DATA_STATEVARIABLE + staticDataObj.targetCurveMktSuffix_, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		staticDataObj.curveProperties_.stateVariableSwaps_ = toStateVariableEnum(dynamic_cast<const AQLDataString&>(dh->get()).get().getCString());
		AQ_THROW_IF(stateVariableSwaps_ != STATE_VARIABLE_DF && stateVariableSwaps_ != STATE_VARIABLE_ZERO_RATE_TIMES_TIME,
					  "Invalid Swap Data: Swap Calibration StateVariable must be DiscountFactor or RateTime")
	}

	// Set Tenor Basis State Variable
	if ( useTenorBasisSwaps_ && mktDataObj.mktData_.tenorswap_.size() > 0 )
	{
		// Access Violation Guard
		AQ_REQUIRE( mktDataObj.mktData_.tenorswap_.size() == mktDataObj.mktData_.swap_.size(), "Invalid Tenor Basis Data: Inconsistent number of IRS Swaps and Tenor Basis Swaps" )
		
		bool useRateTime = false;
		dh = &( mktDataObj.mktData_.swap_[0]->getData(IR_CALIBRATION_DATA_ISTIMEINTERPOLATIONSW, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			useRateTime = dynamic_cast<const AQLDataBool &>(dh->get()).get();
		}
	
		if ( !useRateTime )
		{
			stateVariableTenorBasis_ = STATE_VARIABLE_ZERO_RATE;
		}
	}
}

// Default Constructor for Curve Interpolation Results container for the CurveDataProvider class
InterpolationDataHolder::InterpolationDataHolder()
	: interpolatorForDiscountFactors_( NULL ),
	  interpolatorForSwaps_( NULL ),
	  interpolatorForFuturesAndFRAs_( NULL ),
	  interpolationStringDiscountFactors_( "" ),
	  interpolationStringSwaps_( "" ),
	  interpolationJoinDate_( AQLDate() ),
	  interpolationJoinDateAsDouble_( 0.0 )
{
}

// Main Constructor for Curve Interpolation Results container for the CurveDataProvider class
InterpolationDataHolder::InterpolationDataHolder( LACurveStaticDataHolder & staticDataObj )
	:	interpolatorForDiscountFactors_( NULL ),
		interpolatorForSwaps_( NULL ),
		interpolatorForFuturesAndFRAs_( NULL ),	
		interpolationStringDiscountFactors_( "" ),
		interpolationStringSwaps_( "" ),
	    interpolationJoinDate_( AQLDate() ),
	    interpolationJoinDateAsDouble_( 0.0 )
{
	// Initialize interpolation strings
	interpolationStringDiscountFactors_ = staticDataObj.curveCalibrationData_.getInterpolation().convertToString();
	interpolationStringSwaps_			= staticDataObj.curveCalibrationData_.getInterpolation_yg().convertToString();

	// Target Curve Market Suffix
	AQLString targetCurveMktSuffix = staticDataObj.targetCurveMktSuffix_;

	// Initialize Discount Factor Interpolation
	const AQLDataHolder *dh = &staticDataObj.curveCalibrationData_.getData( CALIBRATION_DATA_INTERPOLATION + targetCurveMktSuffix, NOCHECK );
	if( dh->isDefined() && !dh->isNull() )
	{
		AQLPriceDataInterpolation thisInterpolator = dynamic_cast< const AQLPriceDataInterpolation & >( dh->get() );
		interpolationStringDiscountFactors_ = thisInterpolator.convertToString();
		interpolatorForDiscountFactors_ = dynamic_cast< AQLInterpolationBase* >( thisInterpolator.getMethod().clone() );
		fh_DiscountFactors_.set( interpolatorForDiscountFactors_, true );
	}

	// Initialize Swap Instrument Interpolation
	dh = &staticDataObj.curveCalibrationData_.getData( IR_CALIBRATION_DATA_INTERPOLATIONYG + targetCurveMktSuffix, NOCHECK );
	if( dh->isDefined() && !dh->isNull() )
	{
		AQLPriceDataInterpolation thisInterpolator = dynamic_cast< const AQLPriceDataInterpolation & >( dh->get() );
		interpolationStringSwaps_ = thisInterpolator.convertToString();
		interpolatorForSwaps_ = dynamic_cast< AQLInterpolationBase* >( thisInterpolator.getMethod().clone() );
		fh_Swaps_.set( interpolatorForSwaps_, true );
	}

	// Initialize Futures and FRA Instrument Interpolation
	dh = &staticDataObj.curveCalibrationData_.getData( IR_CALIBRATION_DATA_INTERPOLATIONFW + targetCurveMktSuffix, NOCHECK );
	if( dh->isDefined() && !dh->isNull() )
	{
		interpolatorForFuturesAndFRAs_ = dynamic_cast< AQLInterpolationBase* >( dynamic_cast< const AQLPriceDataInterpolation & >( dh->get() ).getMethod().clone() );
		fh_FuturesAndFRAs_.set( interpolatorForFuturesAndFRAs_, true );
	}
}

// Clear and Reset InterpolationDataHolder member variables
void InterpolationDataHolder::reset()
{
	interpolatorForDiscountFactors_		= NULL;
	interpolatorForSwaps_				= NULL;
	interpolatorForFuturesAndFRAs_		= NULL;
	interpolationStringDiscountFactors_	= "";
	interpolationStringSwaps_			= "";
	interpolationJoinDate_				= AQLDate();
	interpolationJoinDateAsDouble_		= 0.0;
}

AQLDate getFirstSwapMaturityDate( const LACurveMarketData & mktData )
{
	AQLDate swapMaturity;

	AQ_REQUIRE( mktData.swap_size_ > 0, "Invalid Market Data: Missing Swap Instruments" )

	const AQLDate& spotdate				= dynamic_cast<const AQLDataDate&> ((mktData.swap_[0]->getData(IR_CALIBRATION_DATA_SPOTDATE, ISNOTNULL)).get());
	const AQLString& term_str			= dynamic_cast<const AQLDataString&> ((mktData.swap_[0]->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get()).get();
	const AQLString& freq				= dynamic_cast<const AQLDataString&> ((mktData.swap_[0]->getData(IR_CALIBRATION_DATA_FREQUENCY, ISNOTNULL)).get()).get();
	const AQLPriceDataCalendar& cal		= dynamic_cast<const AQLPriceDataCalendar&> ((mktData.swap_[0]->getData(CALIBRATION_DATA_CALENDAR, ISNOTNULL)).get());
	const AQLPriceDataSlidingRule& sld	= dynamic_cast<const AQLPriceDataSlidingRule&> ((mktData.swap_[0]->getData(CALIBRATION_DATA_SLIDINGRULE, ISNOTNULL)).get());

	// set isEOMRoll
	bool isEOMRoll = false;
	const AQLDataHolder *dh = &(mktData.swap_[0]->getData(IR_CALIBRATION_DATA_ISEOMROLLSW, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		isEOMRoll = dynamic_cast<const AQLDataBool &>(dh->get()).get();
	}
	// set roll convention
	AQLString roll_conv = getRollConv(freq, isEOMRoll);

	swapMaturity = LADateHelpers::getDate(spotdate, term_str, sld, &cal, true, &roll_conv);
	return swapMaturity;
}

// Get the Spot Date for the Libor Cash Deposit Instruments
AQLDate getLiborSpotDate( const LACurveMarketData & mktData )
{
	AQLDate liborSpotDate;
	for (unsigned int i = 0; i < mktData.libor_size_; i++)
	{
		const AQLDate& spotdate = dynamic_cast<const AQLDataDate&> ((mktData.libor_[i]->getData(IR_CALIBRATION_DATA_SPOTDATE, ISNOTNULL)).get());
		if (i == 0)
		{
			liborSpotDate = spotdate;
		}
		else if (liborSpotDate != spotdate)
		{
			AQ_THROW("Calibration instruments must have the same spotdate")
		}
	}
	return liborSpotDate;
}

// Check if we have been given OIS Market Data
bool isOISMarketData( const LACurveMarketData & mktData )
{
	// MPC Swaps are only used in OIS Curves
	// Libor Cash Deposits are never used in OIS Curves
	return mktData.mpc_swap_size_ || !mktData.libor_size_;
}

// Check if Swaps are Forward Starting
bool isSwapForwardStarting( const LACurveMarketData & mktData )
{
	bool is_fwdswap = false;

	if ( mktData.swap_.empty() ) // Prevent Access Violation
	{ 
		const AQLDataHolder *dh = &(mktData.swap_[0]->getData(PRICING_DATA_ISFWDSWAP, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			is_fwdswap = dynamic_cast<const AQLDataBool &>(dh->get()).get();
		}
	}
	return is_fwdswap;
}

// Group the Curve Market Data by Instrument and Sort the Data
void CurveMarketDataHolder::groupMarketDataByInstrument( LACurveStaticDataHolder & staticDataObj, const AQLDate& asOfDate )
{
	// 0.	Store Market Data AsOfDate
	asOfDate_ = asOfDate;

	// 1.	Group Market Data by Instrument
	// ***************************************************************************************************
	
	AQLString datatype_str;
	AQLPriceDataDayCount dc_act365(ACT_365);
	mktData_.isO_N_ = false;
	mktData_.isT_N_ = false;
	
	unsigned int size_marketdata = rawMarketData_.size();
	for (unsigned i = 0; i < size_marketdata; i++)
	{
		// check use grid
		const AQLDataHolder *dh = &rawMarketData_[i]->getData(IR_CALIBRATION_DATA_GRIDUSEFLAG, NOCHECK);
		if (dh->isDefined() && !dh->isNull() && !dynamic_cast<const AQLDataBool &>(dh->get()).get())
		{
			continue;
		}

		datatype_str = dynamic_cast<const AQLDataString&> ((rawMarketData_[i]->getData(IR_CALIBRATION_DATA_DATATYPE, ISNOTNULL)).get()).get();
		datatype_str.toUpper();

		if (datatype_str == AQ_LIBOR_DATA)
		{
			// libor case
			mktData_.libor_.push_back(rawMarketData_[i]);
		}
		else if (datatype_str == AQ_SWAP_DATA)
		{
			// swap case
			mktData_.swap_.push_back(rawMarketData_[i]);
		}
		else if (datatype_str == AQ_FUTURE)
		{
			// future case
			mktData_.future_.push_back(rawMarketData_[i]);
		}
		else if ( datatype_str == AQ_MPC_SWAP || datatype_str == AQ_FOMC_SWAP || datatype_str == AQ_ARR_FUTURE )
		{
			// Monetary Policy Swaps (Central Bank Swaps)
			mktData_.mpc_swaps_.push_back(rawMarketData_[i]);
		}
		else if (datatype_str == AQ_FRA)
		{
			// fra case
			mktData_.fra_.push_back(rawMarketData_[i]);
		}
		else if (datatype_str == AQ_BASIS)
		{
			// tenor basis swap
			mktData_.tenorswap_.push_back(rawMarketData_[i]);
		}
		else 
		{
			// money market case
			const AQLPriceDataCalendar& cal = dynamic_cast<const AQLPriceDataCalendar&> ((rawMarketData_[i]->getData(CALIBRATION_DATA_CALENDAR, ISNOTNULL)).get());
			const AQLPriceDataSlidingRule& sld = dynamic_cast<const AQLPriceDataSlidingRule&> ((rawMarketData_[i]->getData(CALIBRATION_DATA_SLIDINGRULE, ISNOTNULL)).get());
			if (datatype_str == AQ_O_N)
			{
				AQLDate end = asOfDate;
				end.addDays(1);
				end = sld.getDate(end, cal);

				mktData_.moneyMarket_[StartAndEndDate(asOfDate, end)] = rawMarketData_[i];
				mktData_.isO_N_ = true;
				
				DateVector depositDates(2, asOfDate);
				depositDates[1] = end;
				
				if (!rawMarketData_[i]->getData(IR_CALIBRATION_DATA_CALCDATESFORDVZERO, NOCHECK).isDefined())
				{
					rawMarketData_[i]->add(IR_CALIBRATION_DATA_CALCDATESFORDVZERO, new AQLDataDates(depositDates));
				}
				else
				{
					dynamic_cast<AQLDataDates&>(rawMarketData_[i]->getData(IR_CALIBRATION_DATA_CALCDATESFORDVZERO).get()).set(depositDates);
				}
			}
			else if (datatype_str == AQ_T_N)
			{
				AQLDate start = asOfDate;
				start.addDays(1);
				start = sld.getDate(start, cal);

				AQLDate end = start;
				end.addDays(1);
				end = sld.getDate(end, cal);
				
				mktData_.moneyMarket_[StartAndEndDate(start, end)] = rawMarketData_[i];
				mktData_.isT_N_ = true;
				
				DateVector depositDates(2, start);
				depositDates[1] = end;
				
				if (!rawMarketData_[i]->getData(IR_CALIBRATION_DATA_CALCDATESFORDVZERO, NOCHECK).isDefined())
				{
					rawMarketData_[i]->add(IR_CALIBRATION_DATA_CALCDATESFORDVZERO, new AQLDataDates(depositDates));
				}
				else
				{
					dynamic_cast<AQLDataDates&>(rawMarketData_[i]->getData(IR_CALIBRATION_DATA_CALCDATESFORDVZERO).get()).set(depositDates);
				}
			}
			else
			{
				AQLDate start, end;
				if (datatype_str == "0D_1D") mktData_.isO_N_ = true;
				else if (datatype_str == "1D_1D") mktData_.isT_N_ = true;

				getMoneyMarketDates(asOfDate, datatype_str, cal, sld, start, end);
				
				mktData_.moneyMarket_[StartAndEndDate(start, end)] = rawMarketData_[i];
			}
		}
	}

	// 2.	Sort Market Data in Ascending Order and Store Sizes 
	// ***************************************************************************************************

	sort( mktData_.libor_.begin(),		mktData_.libor_.end(),		InstrumentComp() );
	sort( mktData_.swap_.begin(),		mktData_.swap_.end(),		InstrumentComp() );
	sort( mktData_.mpc_swaps_.begin(),	mktData_.mpc_swaps_.end(),	InstrumentComp() );
	sort( mktData_.tenorswap_.begin(),	mktData_.tenorswap_.end(),	InstrumentComp() );

	if (staticDataObj.curveProperties_.useFRAs_)
	{
		sort(mktData_.fra_.begin(), mktData_.fra_.end(), InstrumentComp());
	}

	if (staticDataObj.curveProperties_.useFutures_)
	{
		sort(mktData_.future_.begin(), mktData_.future_.end(), InstrumentComp());
	}

	// Size Information
	mktData_.libor_size_		= mktData_.libor_.size();
	mktData_.swap_size_			= mktData_.swap_.size();
	mktData_.future_size_		= mktData_.future_.size();
	mktData_.mpc_swap_size_		= mktData_.mpc_swaps_.size();
	mktData_.fra_size_			= mktData_.fra_.size();

	// 3.	Store Market Data Type Information
	// ***************************************************************************************************

	// Store Market Data Type: Is OIS Market Data ??
	mktData_.isOISMarketData_	= isOISMarketData( mktData_ );

	// Check for Forward Starting Swaps
	mktData_.areSwapsForwardStarting_ = isSwapForwardStarting( mktData_ );
	
	// 5.	Market Data Validation
	// ***************************************************************************************************
	if( mktData_.isOISMarketData_ )
	{
		// OIS Curve Specific Validation
		AQ_REQUIRE(mktData_.swap_size_ > 0, "Invalid Curve Market Data: Missing Swap calibration instruments")
	}
	else
	{
		// Libor Curve Specific Validation
		AQ_REQUIRE(mktData_.swap_size_ > 0, "Invalid Curve Market Data: Missing Libor Swap calibration instruments")
		AQ_REQUIRE(mktData_.libor_size_ > 0, "Invalid Curve Market Data: Missing Libor Cash Deposit calibration instruments")
	    AQ_THROW_IF(!mktData_.isO_N_ || !mktData_.isT_N_, "Invalid Curve Market Data: Both O_N and T_N calibration instruments are required")
	}
	
	// All Curve Validation
	AQ_THROW_IF(staticDataObj.curveProperties_.useFRAs_ && staticDataObj.curveProperties_.useFutures_, "Invalid Curve Instrument Settings: We can not use fra and futures calibration instruments at a same time!")

	// Store the first Swap Maturity Date - Can only store this once market data validated
	mktData_.firstSwapMaturityDate_ = getFirstSwapMaturityDate( mktData_ );

}

// Curve Market Data Holder Constructor
CurveMarketDataHolder::CurveMarketDataHolder( LACurveStaticDataHolder & staticDataObj, const AQLDate & asOfDate )
	: asOfDate_(asOfDate)
{
	marketDataSharedObject_ = &dynamic_cast<const AQLDataMultiReference&>
		(staticDataObj.curveCalibrationData_.getData(CALIBRATION_DATA_MARKETDATA + staticDataObj.targetCurveMktSuffix_, ISNOTNULL).get());
	
	for (unsigned int j = 0; j < marketDataSharedObject_->getSize(); j++)
	{
 		rawMarketData_.push_back(&marketDataSharedObject_->get(j).get());
	}

	// Check if Target Curve is a Swap Curve
	const bool isSwapCurve = isTargetSwapCurve( staticDataObj );
	if (isSwapCurve)
	{
		// Append SwapTenorAdjustments to market data if required
		if( staticDataObj.curveProperties_.useTenorBasisSwaps_ )
		{
			AQLString tenorSwapName = dynamic_cast< const AQLDataString& > ( staticDataObj.curveCalibrationData_.getData( IR_CALIBRATION_DATA_TENORSWAPNAME, ISNOTNULL ).get() );
			tenorSwapName.toUpper();

			const AQLDataMultiReference& basisSwapSharedObject = dynamic_cast< const AQLDataMultiReference& > ( staticDataObj.curveCalibrationData_.getData( CALIBRATION_DATA_MARKETDATA + AQLString( "_" ) + tenorSwapName, ISNOTNULL ).get() );
			for( unsigned int i = 0; i < basisSwapSharedObject.getSize(); i++ )
			{
				rawMarketData_.push_back( &basisSwapSharedObject.get( i ).get() );
			}
		}
	}

	// Populate and Group Market Data by Instrument
	groupMarketDataByInstrument( staticDataObj, asOfDate );
}

// Check if the targetCurve is a STD Swap Curve
bool CurveMarketDataHolder::isTargetSwapCurve( LACurveStaticDataHolder & staticDataObj ) const
{
	// Curve Info
	AQLString targetCurve = staticDataObj.targetCurve_;
	ListOfBuiltCurves listOfCurvesAlreadyBuilt = staticDataObj.listOfCurvesAlreadyBuilt_;

	/*
		* Special STD Standard Curve Logic, where STD is a special keyword.
		* The following conditions are used to determine if the build curve is a SWAP curve.

		* Condition 1. Is the MarketName for the target curve "SWAP"
		* Condition 2. Is the STD curve present in the listOfCurvesAlreadyBuilt dictionary
		* Condition 3. Is the target curve STD
	*/
	const bool isSwapMarketName		= staticDataObj.curveCalibrationData_.getMarketForCurve( targetCurve ) == AQ_SWAP;
	const bool isSwapCurveMissing	= listOfCurvesAlreadyBuilt.find( AQ_STD ) == listOfCurvesAlreadyBuilt.end();
	const bool isSwapTargetCurve	= ( targetCurve == AQ_STD );
	
	const bool isSwapCurve			= ( isSwapMarketName && isSwapCurveMissing ) || isSwapTargetCurve;
	return isSwapCurve;
}

// LiborIndex Constructor
// The legacy analytics takes some info from the swap trade
LiborIndex::LiborIndex( CurveMarketDataHolder & mktDataObj )
	: daycountAct365_(ACT_365)
{
	// Validate Libor Index Instrument Definitions & Data
	AQ_REQUIRE( mktDataObj.mktData_.libor_size_ > 0, "Missing Libor Float Index Data" )
	spotDate_			= getLiborSpotDate( mktDataObj.mktData_ );
	
	// *** Important *** The Libor Market Data vector contains the Libor Cash Deposits for All Curves
	// Therefore get the Libor Index Frequency and Tenor from Curve Properties

	// TODO: Get the Libor Index Tenor and Frequency from the Curve for now
	
	
	
	fixingBusDayAdj_	= &dynamic_cast<const AQLPriceDataSlidingRule&> ((mktDataObj.mktData_.libor_[0]->getData(CALIBRATION_DATA_SLIDINGRULE, ISNOTNULL)).get());
	fixingCalendar_		= &dynamic_cast<const AQLPriceDataCalendar&> ((mktDataObj.mktData_.libor_[0]->getData(CALIBRATION_DATA_CALENDAR, ISNOTNULL)).get());

	daycount_			= &dynamic_cast<const AQLPriceDataDayCount&> ((mktDataObj.mktData_.libor_[0]->getData(IR_CALIBRATION_DATA_DAYCOUNT, ISNOTNULL)).get());
	daycountEnum_		= daycount_->dayCountEnum();

	// Curve Spot Date is the Swap Spot Date - if no swaps are present use the Libor spot date
	curveSpotDate_	= spotDate_;
	if( mktDataObj.mktData_.swap_size_ > 0 )
	{
		curveSpotDate_ = dynamic_cast<const AQLDataDate&> ((mktDataObj.mktData_.swap_[0]->getData(IR_CALIBRATION_DATA_SPOTDATE, ISNOTNULL)).get());
	}

	// TODO - Remove the Libor Index Dependency on Swap Instrument Parameters
	// **********************************************************************

	AQ_REQUIRE( mktDataObj.mktData_.swap_size_ > 0, "Missing Swaps Data: At least One Swap is Required" )

	// Libor Index Frequency - Use Swap Float Frequency as a Proxy
	// The Curve Frequency is a better proxy but the parameter has not been made available to the property manager and entity pool
	const AQLDataHolder*dh = &mktDataObj.mktData_.swap_[0]->getData(IR_CALIBRATION_DATA_FREQUENCY_FLOAT, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		frequency_ = dynamic_cast<const AQLDataString&> (dh->get());
		frequency_.toUpper();
	}
	else
	{
		AQ_THROW("Invalid Swap Data: Float Leg Frequency 'FrequencyFloat' is required")
	}

	// Libor Index Tenor - Implied from the Libor Index Frequency
	if (frequency_ == AQ_SEMI_ANNUAL)
	{
		tenor_ = AQ_TERM_6M;
	}
	else if (frequency_ == AQ_QUARTERLY)
	{
		tenor_ = AQ_TERM_3M;
	}
	else if (frequency_ == AQ_MONTHLY)
	{
		tenor_ = AQ_TERM_1M;
	}
	else if (frequency_ == AQ_ANNUAL)
	{
		tenor_ = AQ_TERM_12M;
	}
	else
	{
		AQ_THROW("Invalid Libor Index Data: Libor Index frequency must be 1M, 3M, 6M or 12M.")
	}
}

// Futures Instruments Default Constructor
FuturesInstruments::FuturesInstruments()
	: marketQuotes_(ForwardQuotes()),
	  type_(SERIAL_FUTURES_BY_DF),
	  areAllFuturesContiguous_(false),
	  isEOMRoll_(false),
	  rollConvention_(""),
	  daycount_(ACT_360),
	  daycountAct365_(ACT_365),
	  isConvexityQuotedAsVol_(true),
	  includeSwapsBeforeMPCSwaps_(true),
	  applyTensionFutures_(false),
	  tensionDayGap_(0),
	  lastStartDate_(AQLDate())
{
}

// Main Futures Instruments Constructor
FuturesInstruments::FuturesInstruments( LACurveStaticDataHolder & staticDataObj, CurveMarketDataHolder & mktDataObj, LiborIndex & liborIndex )
	: marketQuotes_(ForwardQuotes()),
	  type_(SERIAL_FUTURES_BY_DF),
	  areAllFuturesContiguous_(false),
	  isEOMRoll_(false),
	  rollConvention_(""),
	  daycount_(ACT_360),
	  daycountAct365_(ACT_365),
	  isConvexityQuotedAsVol_(true),
	  includeSwapsBeforeMPCSwaps_(true),
	  applyTensionFutures_(false),
	  tensionDayGap_(0),
	  lastStartDate_(AQLDate())
{
	if ( !staticDataObj.curveProperties_.useFutures_ || mktDataObj.mktData_.future_size_ == 0 )
	{
		return;
	}
	
	// isEOMRoll
	isEOMRoll_ = false;
	const AQLDataHolder* dh = &(mktDataObj.mktData_.future_[0]->getData(IR_CALIBRATION_DATA_ISEOMROLL, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		isEOMRoll_ = dynamic_cast<const AQLDataBool &>(dh->get()).get();
	}

	// Roll Convention
	rollConvention_ = getRollConv(liborIndex.frequency_, isEOMRoll_);

	// Convexity Adjustment - Quoted as a Price or Volatility
	// --------------------------------------------------------------------------------------------------

	// UseConvexityAdj means Convexity Quoted as a Price not vol
	// not very clear so replaced by CONVEXITYQUOTEDASVOL it's alias and inverse
	isConvexityQuotedAsVol_ = true;
	const AQLDataHolder* dhConvexityQuotedAsPrice = &(mktDataObj.mktData_.future_[0]->getData(PRICING_DATA_USECONVEXADJUSTMENT, NOCHECK));
	if (dhConvexityQuotedAsPrice->isDefined() && !dhConvexityQuotedAsPrice->isNull())
	{
		const bool convexityQuotedAsPrice = dynamic_cast<const AQLDataBool &>(dhConvexityQuotedAsPrice->get()).get();
		isConvexityQuotedAsVol_ = !convexityQuotedAsPrice;
	}

	// Convexity Quoted as Vol ???
	// Alias and replacement for useConvexAdj
	dh = &(mktDataObj.mktData_.future_[0]->getData(PRICING_DATA_CONVEXITYQUOTETYPE, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		AQLString convexityQuoteTypeStr = dynamic_cast<const AQLDataString &>(dh->get()).get();
		convexityQuoteTypeStr.toUpper();
		AQ_REQUIRE( convexityQuoteTypeStr == "VOL" || convexityQuoteTypeStr == "PRICE", "Invalid Futures Convexity Quote Type: ConvexityQuoteType must be VOL or PRICE" )
		isConvexityQuotedAsVol_ = ( convexityQuoteTypeStr == "VOL" ) ? true : false;
		
		// TODO: Property Manager does not allow us to clear parameters once set ... hence the below fails ... prioritize the convexityQuoteType parameter for now
		// // Don't allow the legacy name and alias to be used at the same time
		// if ( dhConvexityQuotedAsPrice->isDefined() && !dhConvexityQuotedAsPrice->isNull() )
		// {
		// 	AQ_THROW("Invalid Futures Convexity Parameter: Cannot use ConvexityQuoteType and UseConvexAdjustment (ConvexityQuotedAsPrice) parameters at the same time.")
		// }
	}
	// --------------------------------------------------------------------------------------------------

	// Apply Tension
	applyTensionFutures_ = false;
	dh = &(mktDataObj.mktData_.future_[0]->getData(IR_CALIBRATION_DATA_APPLYTENSION, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		applyTensionFutures_ = dynamic_cast<const AQLDataBool &>(dh->get()).get();
	}

	// Tension Gap Days
	tensionDayGap_ = dynamic_cast<const AQLDataInt&> ((mktDataObj.mktData_.future_[0]->getData(IR_CALIBRATION_DATA_TENSIONGAP, ISNOTNULL)).get()).get();
	AQ_THROW_IF( applyTensionFutures_ && tensionDayGap_ < 1, "Invalid Futures Parameter: The TensionGap days must be greater than zero")

	// Include Swaps Before MPC Swaps (also known as 'SmoothShortEnd')
	includeSwapsBeforeMPCSwaps_ = true;
	dh = &(mktDataObj.mktData_.future_[0]->getData(IR_CALIBRATION_DATA_SMOOTHSHORTEND, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		includeSwapsBeforeMPCSwaps_ = dynamic_cast<const AQLDataBool &>(dh->get()).get();
	}

	// Future Type
	type_ = SERIAL_FUTURES_BY_DF;
	dh = &(mktDataObj.mktData_.future_[0]->getData(IR_CALIBRATION_DATA_FRAFUTURE_SERIAL_CALC_TYPE, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		AQLString futuresTypeString = dynamic_cast<const AQLDataString&>(dh->get());
		type_ = toFuturesTypeEnum( futuresTypeString.c_str() );
	}

	// Futures Daycount
	daycount_ = dynamic_cast<const AQLPriceDataDayCount&> ((mktDataObj.mktData_.future_[0]->getData(IR_CALIBRATION_DATA_DAYCOUNT, ISNOTNULL)).get());

	// Internal Curve Daycount
	AQLPriceDataDayCount curveDaycount_act365(ACT_365);

	// Populate ForwardRates
	// ******************************************************************
	std::vector<ForwardRate> forwardRatesFromFutures;
	forwardRatesFromFutures.reserve(mktDataObj.mktData_.future_size_);
		
	// True to indicate all the Fra/Futures are in contiguous months
	// If we find a single non-contiguous future below this will be updated to false
	areAllFuturesContiguous_ = true;

	for (unsigned int i = 0; i < mktDataObj.mktData_.future_size_; ++i)
	{
		const AQLDate& startDate = dynamic_cast<const AQLDataDate&> ((mktDataObj.mktData_.future_[i]->getData(PRICING_DATA_STARTDATE, ISNOTNULL)).get()).get();

		// Ignore the future if current future's startDate is before swap spot date
		if (startDate <= liborIndex.curveSpotDate_ )
		{
			continue;
		}

		if (i == mktDataObj.mktData_.future_size_ - 1)
		{
			// Required for swaps calibration
			lastStartDate_ = startDate;
		}

		const AQLDate endDate = dynamic_cast<const AQLDataDate&> ((mktDataObj.mktData_.future_[i]->getData(PRICING_DATA_ENDDATE, ISNOTNULL)).get()).get();
		if (endDate >= mktDataObj.mktData_.firstSwapMaturityDate_)
		{
			break;
		}

		// Futures Market Data Quote
		double marketRate;
		dh = &(mktDataObj.mktData_.future_[i]->getData(CALIBRATION_DATA_RATE, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			marketRate = dynamic_cast<const AQLDataDouble&> (dh->get()).get();
		}
		else
		{
			double price = dynamic_cast<const AQLDataDouble&> ((mktDataObj.mktData_.future_[i]->getData(PRICING_DATA_PRICE, ISNOTNULL)).get()).get();
			marketRate = 1.0 - price * 0.01;
		}

		// Apply spread
		double spread = 0.0;
		dh = &(mktDataObj.mktData_.future_[i]->getData(PRICING_DATA_SPREAD, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			spread = dynamic_cast<const AQLDataDouble&> (dh->get()).get();
		}
		marketRate += spread;

		// Use the Hull-White 1 Factor Model if the mean reversion parameter is populated else use the Ho-Lee Model
		double meanReversion = 0.0;
		dh = &(mktDataObj.mktData_.future_[i]->getData(IR_CALIBRATION_DATA_MEANREVERSION, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			meanReversion = dynamic_cast<const AQLDataDouble &>(dh->get()).get();
		}

		// add convexity adjustment to the future rate
		marketRate = getConvexityAdjustedFutureRate(mktDataObj.mktData_.future_[i], marketRate, mktDataObj.asOfDate_, meanReversion, daycount_);

		// Update Futures Market Quotes 
		marketQuotes_.startDates_.push_back( startDate );
		marketQuotes_.endDates_.push_back( endDate );
		marketQuotes_.marketRates_.push_back( marketRate );
		bool isThisFutureContiguous = etrading::isContiguousFraFuture(liborIndex.curveSpotDate_, startDate, liborIndex.tenor_, staticDataObj.curveProperties_.useFutures_ );
	
		if( !isThisFutureContiguous )
		{
			areAllFuturesContiguous_ = false;
		}
	}

	// Only set futures type to contiguous if all futures are contiguous
	if (areAllFuturesContiguous_)
	{
		type_ = CONTIGUOUS_FUTURES;
	}
}

// Get ForwardRates Object for Backwards Compatibility
std::vector<ForwardRate> FuturesInstruments::getForwardRates() const
{
	size_t nMarketQuotes = marketQuotes_.marketRates_.size();
	std::vector<ForwardRate> forwardRates( nMarketQuotes );

	for( size_t i = 0; i < nMarketQuotes; ++i )
	{
		forwardRates[i].startDate = marketQuotes_.startDates_[i];
		forwardRates[i].endDate = marketQuotes_.endDates_[i];
		forwardRates[i].fwdRate = marketQuotes_.marketRates_[i];
	}
	
	return forwardRates;
}

// SwapInstruments Constructor
SwapInstruments::SwapInstruments()
	:	spotDate_( DateVector() ),
		calendar_( std::vector<const AQLPriceDataCalendar*>(0) ),
		busDayAdj_( std::vector<const AQLPriceDataSlidingRule*>(0) ),
		isEOMRoll_( std::vector<bool>() ),
		rollConvention_( std::vector<AQLString>() ),
		daycountAct365_(ACT_365),
		daycountFixed_( std::vector<const AQLPriceDataDayCount*>(0) ),
		frequencyFixed_( std::vector<AQLString>() ),
		daycountFloat_( std::vector<const AQLPriceDataDayCount*>(0) ),
		frequencyFloat_( std::vector<AQLString>() ),
		compoundFrequencyFloat_( std::vector<AQLString>() ),
		numberOfCompoundPeriodsFloat_( std::vector<int>() ),
		liborIndexFrequency_( std::vector<AQLString>() ),
		liborIndexTenor_( std::vector<AQLString>() )
{}

// Resize Vector Member Data
void SwapInstruments::resize( const size_t newSize )
{
	spotDate_.resize( newSize );
	calendar_.resize( newSize );
	busDayAdj_.resize( newSize );
	isEOMRoll_.resize( newSize );
	rollConvention_.resize( newSize );
	daycountFixed_.resize( newSize );
	frequencyFixed_.resize( newSize );
	daycountFloat_.resize( newSize );
	frequencyFloat_.resize( newSize );
	compoundFrequencyFloat_.resize( newSize );
	numberOfCompoundPeriodsFloat_.resize( newSize );
	liborIndexFrequency_.resize( newSize );
	liborIndexTenor_.resize( newSize );
}

// Method to get the data required and construct swap calibration instruments
SwapInstruments::SwapInstruments( LACurveStaticDataHolder & staticDataObj, CurveMarketDataHolder & mktDataObj )
	: daycountAct365_(ACT_365)
{
	// Validate and Set Size
	AQ_REQUIRE( mktDataObj.mktData_.swap_size_ > 0, "Invalid Calibration Instruments: Missing Swap Instruments" )
	this->resize( mktDataObj.mktData_.swap_size_ );

	const AQLDataHolder *dh = 0;
	
	for (unsigned int i = 0; i < mktDataObj.mktData_.swap_size_; i++)
	{
		// Trade Conventions
		// *************************************************************
		spotDate_[i]	= dynamic_cast<const AQLDataDate&> ((mktDataObj.mktData_.swap_[i]->getData(IR_CALIBRATION_DATA_SPOTDATE, ISNOTNULL)).get());
		calendar_[i]	= &dynamic_cast<const AQLPriceDataCalendar&> ((mktDataObj.mktData_.swap_[i]->getData(CALIBRATION_DATA_CALENDAR, ISNOTNULL)).get());
		busDayAdj_[i]	= &dynamic_cast<const AQLPriceDataSlidingRule&> ((mktDataObj.mktData_.swap_[i]->getData(CALIBRATION_DATA_SLIDINGRULE, ISNOTNULL)).get());
		
		if ( spotDate_[i] != spotDate_[0] )
		{
			AQ_THROW("Each swap calibration instrument must have the same SpotDate")
		}

		// Fixed Leg
		// *************************************************************
		
		// Frequency Fixed
		frequencyFixed_[i] = dynamic_cast<const AQLDataString&> ((mktDataObj.mktData_.swap_[i]->getData(IR_CALIBRATION_DATA_FREQUENCY, ISNOTNULL)).get()).get();
		frequencyFixed_[i].toUpper();
		AQ_THROW_IF(frequencyFixed_[i] == AQ_SIMPLE, "Swap frequency 'SIMPLE' is not supported")
		
		// Daycount Fixed
		daycountFixed_[i] = &dynamic_cast<const AQLPriceDataDayCount&> ((mktDataObj.mktData_.swap_[i]->getData(IR_CALIBRATION_DATA_DAYCOUNT, ISNOTNULL)).get());

		// isEOMRoll
		isEOMRoll_[i] = false;
		dh = &mktDataObj.mktData_.swap_[i]->getData(IR_CALIBRATION_DATA_ISEOMROLLSW, NOCHECK);
		if (dh->isDefined() && !dh->isNull())
		{
			isEOMRoll_[i] = dynamic_cast<const AQLDataBool &>(dh->get()).get();
		}

		// Roll Convention
		rollConvention_[i] = getRollConv( frequencyFixed_[i], isEOMRoll_[i] );

		// Float Leg
		// *************************************************************
		
		// Frequency Float
		dh = &mktDataObj.mktData_.swap_[i]->getData(IR_CALIBRATION_DATA_FREQUENCY_FLOAT, NOCHECK);
		if (dh->isDefined() && !dh->isNull())
		{
			frequencyFloat_[i] = dynamic_cast<const AQLDataString&> (dh->get());
		}
		else
		{
			AQ_THROW("Invalid Swap Data: Float Leg Frequency 'FrequencyFloat' is required")
		}

		// Daycount Float
		dh = &mktDataObj.mktData_.swap_[i]->getData(IR_CALIBRATION_DATA_DAYCOUNT_FLOAT, NOCHECK);
		if (dh->isDefined() && !dh->isNull())
		{
			daycountFloat_[i] = &dynamic_cast<const AQLPriceDataDayCount&> (dh->get());
		}
		else
		{
			AQ_THROW("Invalid Swap Data: Float Leg Daycount 'DaycountFloat' is required")
		}

		// Compound Frequency Float - use float frequency if missing
		compoundFrequencyFloat_[i] = frequencyFloat_[i];
		dh = &mktDataObj.mktData_.swap_[i]->getData(IR_CALIBRATION_DATA_FREQUENCY_COMPOUND, NOCHECK);
		if (dh->isDefined() && !dh->isNull())
		{
			compoundFrequencyFloat_[i] = dynamic_cast<const AQLDataString&> (dh->get());
		}
		
		// Number of Compound Periods Float
		numberOfCompoundPeriodsFloat_[i] = LADateHelpers::calcCompoundingTimes( frequencyFloat_[i], compoundFrequencyFloat_[i] );

		// Libor Index Frequency - use float frequency if missing
		liborIndexFrequency_[i] = frequencyFloat_[i];
		dh = &mktDataObj.mktData_.swap_[i]->getData(IR_CALIBRATION_DATA_BASEFREQUENCY_FLOAT, NOCHECK);
		if (dh->isDefined() && !dh->isNull())
		{
			liborIndexFrequency_[i] = dynamic_cast<const AQLDataString&> (dh->get());
		}

		// Libor Index Tenor
		if (liborIndexFrequency_[i] == AQ_SEMI_ANNUAL)
		{
			liborIndexTenor_[i] = AQ_TERM_6M;
		}
		else if (liborIndexFrequency_[i] == AQ_QUARTERLY)
		{
			liborIndexTenor_[i] = AQ_TERM_3M;
		}
		else if (liborIndexFrequency_[i] == AQ_MONTHLY)
		{
			AQ_THROW_IF( staticDataObj.curveProperties_.useFRAs_ && mktDataObj.mktData_.fra_size_ > 0, "Invalid Swap Curve: Only 3M and 6M Swap curves are allowed to calibrate using FRAs")
			liborIndexTenor_[i] = AQ_TERM_1M;
		}
		else if (liborIndexFrequency_[i] == AQ_ANNUAL)
		{
			AQ_THROW_IF( staticDataObj.curveProperties_.useFRAs_ && mktDataObj.mktData_.fra_size_ > 0,  "Invalid Swap Curve: Only 3M and 6M Swap curves are allowed to calibrate using FRAs")
			AQ_THROW_IF( staticDataObj.curveProperties_.useFutures_ && mktDataObj.mktData_.future_size_ > 0, "Invalid Swap Curve: Only 1M, 3M and 6M Swap curves are allowed to calibrate using Futures")
			liborIndexTenor_[i] = AQ_TERM_12M;
		}
		else
		{
			AQ_THROW("Invalid Swap Curve: Libor Index frequency must be 1M, 3M, 6M or 12M.")
		}
	}
}

// Default Constructor
TenorBasisInstruments::TenorBasisInstruments()
	:	spreadInterpolator_(NULL),
		spreads_(DoubleVector()),
		daycount_(NULL),
		daycountAct365_(ACT_365),
		calendar_(NULL),
		busDayAdj_(NULL),
		leg1Frequency_(""),
		leg2Frequency_(""),
		isTargetLeg1_(false),
		applySpreadLeg2_(false),
		againstLegFrequency_(""),
		isRollEOM_(false),
		rollConv_("")
{};

// Default Destructor
TenorBasisInstruments::~TenorBasisInstruments()
{
	// Clean-Up Tenor Basis Interpolation Object
	delete spreadInterpolator_;
};

TenorBasisInstruments::TenorBasisInstruments( LACurveStaticDataHolder & staticDataObj, CurveMarketDataHolder & mktDataObj, LiborIndex & liborIndex, CalibrationResults & resultsObj )
	:	spreadInterpolator_(NULL),
		spreads_(mktDataObj.mktData_.tenorswap_.size(),0.0),
		daycount_(NULL),
		daycountAct365_(ACT_365),
		calendar_(NULL),
		busDayAdj_(NULL),
		leg1Frequency_(""),
		leg2Frequency_(""),
		isTargetLeg1_(false),
		applySpreadLeg2_(false),
		againstLegFrequency_(""),
		isRollEOM_(false),
		rollConv_("")
{
	if (staticDataObj.curveProperties_.useTenorBasisSwaps_)
	{
		const AQLDataHolder* dh = 0;

		// Validate Data Dimensions
		AQ_THROW_IF( mktDataObj.mktData_.tenorswap_.size() == 0, "Invalid Tenor Basis Swap Data: Market Data is Empty" )
		AQ_THROW_IF( mktDataObj.mktData_.swap_.size() == 0, "Invalid Tenor Basis Swap Data: Cannot use Tenor Basis Swaps when IRS Swap Market Data is Empty" )
		AQ_REQUIRE( mktDataObj.mktData_.tenorswap_.size() == mktDataObj.mktData_.swap_.size(), "Invalid Tenor Basis Swap Data: Inconsistent number of IRS Swaps and Tenor Basis Swaps")
		
		// Safety Check: Dimensions Initialized Above in Constructor Initializer List
		AQ_REQUIRE( spreads_.size() == mktDataObj.mktData_.tenorswap_.size(), "Invalid Tenor Basis Swap Data: Inconsistent number of Tenor Basis Swap Instruments and Market Data Quotes" )

		// Spread Leg	
		applySpreadLeg2_ = dynamic_cast<const AQLDataBool&> ((mktDataObj.mktData_.tenorswap_[0]->getData(IR_CALIBRATION_DATA_ISAGTSPREAD, ISNOTNULL)).get()).get();

		// Tenor Basis Leg Frequencies
		leg1Frequency_ = dynamic_cast<const AQLDataString&> ((mktDataObj.mktData_.tenorswap_[0]->getData(IR_CALIBRATION_DATA_CASHLETFREQUENCY, ISNOTNULL)).get()).get();
		leg2Frequency_ = dynamic_cast<const AQLDataString&> ((mktDataObj.mktData_.tenorswap_[0]->getData(IR_CALIBRATION_DATA_AGTCASHLETFREQUENCY, ISNOTNULL)).get()).get();

		// Against Leg Frequency
		if (applySpreadLeg2_)
		{
			againstLegFrequency_ = leg2Frequency_;
		}
		else
		{
			againstLegFrequency_ = leg1Frequency_;
		}

		// Target Leg Frequency
		if ( leg1Frequency_ == liborIndex.frequency_ )
		{
			isTargetLeg1_ = true;
		}
		else if ( leg2Frequency_ == liborIndex.frequency_ )
		{
			isTargetLeg1_ = false;
		}
		else
		{
			AQ_THROW("Invalid Tenor Basis Swap. The Tenor Basis Frequency and Libor Index Frequency must match")
		}

		// Daycount
		daycount_	= &dynamic_cast<AQLPriceDataDayCount &>(mktDataObj.mktData_.tenorswap_[0]->getData(IR_CALIBRATION_DATA_CASHLETDAYCOUNT, ISNOTNULL).get());
		
		// Calendar
		calendar_	= &dynamic_cast<AQLPriceDataCalendar &>(mktDataObj.mktData_.tenorswap_[0]->getData(IR_CALIBRATION_DATA_CASHLETCALENDAR, ISNOTNULL).get());
		
		// Business Day Adjustment
		busDayAdj_	= &dynamic_cast<AQLPriceDataSlidingRule &>(mktDataObj.mktData_.tenorswap_[0]->getData(IR_CALIBRATION_DATA_CASHLETSLIDINGRULE, ISNOTNULL).get());
		
		// isEOMRoll
		dh = &(mktDataObj.mktData_.tenorswap_[0]->getData(IR_CALIBRATION_DATA_ISEOMROLL, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			isRollEOM_ = dynamic_cast<const AQLDataBool &>(dh->get()).get();
		}
		
		// Roll Convention
		if ( againstLegFrequency_ == AQ_LUNAR )
		{
			rollConv_ = AQ_ROLLCONV_LUNAR;
		}
		else if ( isRollEOM_ )
		{
			rollConv_ = AQ_ROLLCONV_EOM;
		}
		else
		{
			rollConv_ = AQ_ROLLCONV_NORMAL;
		}
	}

	// Populate Tenor Basis Spread Market Data
	populateSpreadMarketData( staticDataObj, mktDataObj, liborIndex, resultsObj );
}

// Populate Tenor Basis Spread Market Data
void TenorBasisInstruments::populateSpreadMarketData( LACurveStaticDataHolder & staticDataObj, CurveMarketDataHolder & mktDataObj, LiborIndex & liborIndex, CalibrationResults & resultsObj )
{
	if ( staticDataObj.curveProperties_.useTenorBasisSwaps_)
	{
		// Tenor Basis Spread Market Data Interpolator
		// *******************************************

		// Boundary Conditions for Tenor Basis Market Data Interpolator
		DoubleArray maturitiesAsTerms(1, 0.0);
		DoubleArray stateVariableValues(1, 0.0);
		spreads_ = DoubleVector( mktDataObj.mktData_.tenorswap_.size(), 0.0 );
		
		const AQLDataHolder* dh = &(mktDataObj.mktData_.tenorswap_[0]->getData(CALIBRATION_DATA_INTERPOLATION, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			spreadInterpolator_ = dynamic_cast<AQLInterpolationBase*> ((dynamic_cast<const AQLPriceDataInterpolation&>
				(mktDataObj.mktData_.tenorswap_[0]->getData(CALIBRATION_DATA_INTERPOLATION, ISNOTNULL).get())).getMethod().clone());
		}
		else
		{
			spreadInterpolator_ = dynamic_cast<AQLInterpolationBase *>(resultsObj.interpolationObj_->interpolatorForSwaps_->clone());
		}

		// Populate Tenor Basis Interpolator
		for (unsigned int i = 0; i < mktDataObj.mktData_.tenorswap_.size(); i++)
		{
			const AQLString &maturityString	= dynamic_cast<const AQLDataString &>((mktDataObj.mktData_.tenorswap_[i]->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get()).get();
			double zeroRate					= dynamic_cast<const AQLDataDouble &>((mktDataObj.mktData_.tenorswap_[i]->getData(CALIBRATION_DATA_RATE, ISNOTNULL)).get()).get();
			AQLDate maturityDate				= LADateHelpers::getDate(liborIndex.curveSpotDate_, maturityString, *busDayAdj_, calendar_, true, &rollConv_);
			double maturityDateAsTerm		= daycount_->getTerm(liborIndex.curveSpotDate_, maturityDate);
			
			// State Variable: Time
			maturitiesAsTerms.push_back(maturityDateAsTerm);
			
			switch( staticDataObj.curveProperties_.stateVariableTenorBasis_ )
			{
				case STATE_VARIABLE_ZERO_RATE_TIMES_TIME:
				{
					stateVariableValues.push_back(zeroRate * maturityDateAsTerm);
					break;
				}
				case STATE_VARIABLE_ZERO_RATE:
				{
					stateVariableValues.push_back(zeroRate);
					break;
				}
				default:
				{
					AQ_THROW("Invalid Tenor Basis StateVariable: Only Zero Rate or ZeroRateTimesTime Supported")
					break;
				}
			}
		}

		// Note: Linear-Spline is not supported here, since we do not optimize for the join-date until we reach the swap optimization routine 
		// Rather than throw, we convert Linear-Spline to spline, which is preferred, by setting the linear-spline joinDateAsDouble to zero.
		spreadInterpolator_->set( maturitiesAsTerms, stateVariableValues, 0.0 );
	}
}

SwapCashflows::SwapCashflows( LACurveStaticDataHolder & staticDataObj, CurveMarketDataHolder & mktDataObj, SwapInstruments & swaps, TenorBasisInstruments & tenorBasisSwaps, CalibrationResults & resultsObj )
{
	// 1.	Initialize Member Variables
	// ************************************************************************

	// Swap Info
	scheduleSize_				= mktDataObj.mktData_.swap_size_;
	areSwapsForwardStarting_	= mktDataObj.mktData_.areSwapsForwardStarting_;
	
	// Swap Instrument Schedule
	swapStartDates_.resize(scheduleSize_);
	swapEndDates_.resize(scheduleSize_);
	swapEndDatesUnadjusted_.resize(scheduleSize_);
	swapRates_.resize(scheduleSize_);

	// Fixed Schedule Parameters
	fixedStartDatesAsTerms_.resize(scheduleSize_);
	fixedCoupons_.resize(scheduleSize_);
	fixedCashflowPaymentDates_.resize(scheduleSize_);
	fixedCashflowPaymentDatesAsTerms_.resize(scheduleSize_);
	fixedAccrualPeriods_.resize(scheduleSize_);

	// Float Schedule Parameters
	floatStartDatesAsTerms_.resize(scheduleSize_);
	floatAccrualPeriods_.resize(scheduleSize_);
	floatCoupons_.resize(scheduleSize_);
	floatCashflowPaymentDates_.resize(scheduleSize_);
	floatCashflowPaymentDatesAsTerms_.resize(scheduleSize_);
		
	// Fixing Schedule Parameters
	fixingLags_.resize(scheduleSize_);
	fixingStarts_.resize(scheduleSize_);
	fixingEnds_.resize(scheduleSize_);
	fixingStartDates_.resize(scheduleSize_);
	fixingEndDates_.resize(scheduleSize_);
	fixingAccrualPeriods_.resize(scheduleSize_);

	// Tenor Basis Float Schedule Parameters
	tenorSwapAccrualPeriods_.resize(scheduleSize_);
	tenorSwapCoupons_.resize(scheduleSize_);
	tenorSwapCashflowPaymentDates_.resize(scheduleSize_);
	tenorSwapCashflowPaymentDatesAsTerms_.resize(scheduleSize_);

	// 2.	Generate Instrument Start-Dates, End-Dates and Market Data Rates
	// ************************************************************************

	updateRatePriority( staticDataObj, mktDataObj );
	updateInstrumentDatesAndRates( staticDataObj, mktDataObj, swaps, tenorBasisSwaps, resultsObj );

	// 3.	Update Swap Cashflows
	// ************************************************************************

	updateSwapFixedCashflows( mktDataObj, swaps );
	updateSwapFloatCashflows( mktDataObj, swaps );
	updateTenorBasisSwapFloatCashflows( staticDataObj, mktDataObj, swaps, tenorBasisSwaps );
}

// Get the Curve Rate Priority
void SwapCashflows::updateRatePriority( LACurveStaticDataHolder & staticDataObj, CurveMarketDataHolder & mktDataObj )
{
	// Result Struct
	LARatePriority ratePriority;
	
	ratePriority.isFuturePriority_	= false;	// future priority than libor
	ratePriority.isSwapPriority_	= false;	// swap priority than other rates

	if (staticDataObj.curveProperties_.ratePriorityStringVector_ != nullptr)
	{
		if (staticDataObj.curveProperties_.useFutures_ && mktDataObj.mktData_.future_size_ != 0)
		{
			AQ_REQUIRE( staticDataObj.curveProperties_.ratePriorityStringVector_->size() == 3, "Invalid Rate priority specified - Must be a size 3 colon seperated list specifying instrument priority e.g. Libor:Futures:Swap" )
		}

		AQLString ratePriorityInstrument = (*staticDataObj.curveProperties_.ratePriorityStringVector_)[0];
		ratePriorityInstrument.toUpper();
		if (ratePriorityInstrument == AQ_SWAP)
		{
			ratePriority.isSwapPriority_ = true;
			ratePriorityInstrument = (*staticDataObj.curveProperties_.ratePriorityStringVector_)[1];
			ratePriorityInstrument.toUpper();
			if (staticDataObj.curveProperties_.useFutures_ && mktDataObj.mktData_.future_size_ != 0 && ratePriorityInstrument == AQ_FUTURE)
			{
				// TODO: Check legacy logic - how can we have swap and future priority = true ??? Looks incorrect ???
				ratePriority.isFuturePriority_ = true;
			}
		}
		else if (staticDataObj.curveProperties_.useFutures_ && mktDataObj.mktData_.future_size_ != 0 && ratePriorityInstrument == AQ_FUTURE)
		{
			ratePriority.isFuturePriority_ = true;
		}
	}
	// Update Rate Priority  Member Variable
	ratePriority_ =  ratePriority;
}

// Instrument Dates & Rates
void SwapCashflows::updateInstrumentDatesAndRates( LACurveStaticDataHolder & staticDataObj, CurveMarketDataHolder & mktDataObj, SwapInstruments & swaps, TenorBasisInstruments & tenorBasisSwaps, CalibrationResults & resultsObj )
{
	// Update Swap Schedule Data
	for (size_t i = 0; i < mktDataObj.mktData_.swap_size_; i++)
	{
		AQLDate thisSwapEndDate;
		if (mktDataObj.mktData_.areSwapsForwardStarting_)
		{
			const bool is_date = dynamic_cast<const AQLDataBool&> ((mktDataObj.mktData_.swap_[i]->getData(PRICING_DATA_ISDATE, ISNOTNULL)).get()).get();
			if (is_date)
			{
				swapStartDates_[i] = dynamic_cast<const AQLDataDate&> ((mktDataObj.mktData_.swap_[i]->getData(PRICING_DATA_STARTDATE, ISNOTNULL)).get()).get();
				swapEndDates_[i] = dynamic_cast<const AQLDataDate&> ((mktDataObj.mktData_.swap_[i]->getData(PRICING_DATA_ENDDATE, ISNOTNULL)).get()).get();
			}
			else
			{
				const AQLString startTermString = dynamic_cast<const AQLDataString&> ((mktDataObj.mktData_.swap_[i]->getData(PRICING_DATA_STARTTERM, ISNOTNULL)).get()).get();
				swapStartDates_[i] = LADateHelpers::getDate(swaps.spotDate_[i], startTermString, *swaps.busDayAdj_[i], swaps.calendar_[i], true, &swaps.rollConvention_[i]);
				
				const AQLString maturityTermString = dynamic_cast<const AQLDataString&> ((mktDataObj.mktData_.swap_[i]->getData(PRICING_DATA_TENOR, ISNOTNULL)).get()).get();
				swapEndDatesUnadjusted_[i] = LADateHelpers::getDate(swapStartDates_[i], maturityTermString, AQLPriceDataSlidingRule(SLIDING_RULE_NO_CHANGE), NULL, true, nullptr);
				swapEndDates_[i] = LADateHelpers::getDate(swapStartDates_[i], maturityTermString, *swaps.busDayAdj_[i], swaps.calendar_[i], true, &swaps.rollConvention_[i]);
			}
			thisSwapEndDate = swapEndDates_[i];
		}
		else
		{
			swapStartDates_[i] = swaps.spotDate_[i];
			const AQLString& maturityTermString = getMaturityAsTermString(i, mktDataObj.mktData_.swap_);
			swapEndDatesUnadjusted_[i] = LADateHelpers::getDate(swaps.spotDate_[i], maturityTermString, AQLPriceDataSlidingRule(SLIDING_RULE_NO_CHANGE), NULL, true, nullptr);
			swapEndDates_[i] = LADateHelpers::getDate(swaps.spotDate_[i], maturityTermString, *swaps.busDayAdj_[i], swaps.calendar_[i], true, &swaps.rollConvention_[i]);
			thisSwapEndDate = swapEndDates_[i];
		}

		// Skip to Next Swap Instrument if there is a Rate Priority Clash
		AQLDate lastLiborEndDate = resultsObj.dfResults_.paymentDates_.back();
		if ( !ratePriority_.isSwapPriority_ && lastLiborEndDate.intervalDays(swapEndDates_[i]) <= RATE_PRIORITY_PROXIMITY_DAY_TOLERANCE)
		{
			continue;
		}

		swapRates_[i] = dynamic_cast<const AQLDataDouble&> ((mktDataObj.mktData_.swap_[i]->getData(CALIBRATION_DATA_RATE, ISNOTNULL)).get()).get();

		if (staticDataObj.curveProperties_.useTenorBasisSwaps_)
		{
			if (swaps.frequencyFloat_[i] != swaps.liborIndexFrequency_[i])
			{
				const AQLString& tenorSwapMaturityString	= dynamic_cast<const AQLDataString&> ((mktDataObj.mktData_.swap_[i]->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get()).get();
				const AQLDate tenorSwapMaturityDate		= LADateHelpers::getDate(swaps.spotDate_[i], tenorSwapMaturityString, *tenorBasisSwaps.busDayAdj_, tenorBasisSwaps.calendar_, true, &tenorBasisSwaps.rollConv_);
				const double tenorSwapMaturityAsTerm	= tenorBasisSwaps.daycount_->getTerm(swaps.spotDate_[i], tenorSwapMaturityDate);
				
				switch( staticDataObj.curveProperties_.stateVariableTenorBasis_ )
				{
					case STATE_VARIABLE_ZERO_RATE_TIMES_TIME:
					{
						AQ_REQUIRE( AQ_IS_GREATER_THAN_ZERO( tenorSwapMaturityAsTerm ), "Invalid Tenor Basis Swap: Instrument Maturities as terms must be greater than zero" )
						tenorBasisSwaps.spreads_[i] = tenorBasisSwaps.spreadInterpolator_->value(tenorSwapMaturityAsTerm) / tenorSwapMaturityAsTerm;
						break;
					}
					case STATE_VARIABLE_ZERO_RATE:
					{
						tenorBasisSwaps.spreads_[i] = tenorBasisSwaps.spreadInterpolator_->value(tenorSwapMaturityAsTerm);
						break;
					}
					default:
					{
							AQ_THROW("Invalid Tenor Basis StateVariable: Only Zero Rate or ZeroRateTimesTime Supported")
							break;
					}
				}

				// Apply Negative Spread if Required
				if ( ( tenorBasisSwaps.applySpreadLeg2_ && !tenorBasisSwaps.isTargetLeg1_ ) || 
						( !tenorBasisSwaps.applySpreadLeg2_ && tenorBasisSwaps.isTargetLeg1_ ) )
				{
					tenorBasisSwaps.spreads_[i] *= -1.0;
				}
			}
		}

		// Swap has priority over libor cash deposits and futures
		if ( ratePriority_.isSwapPriority_ && i == 0 ) 
		{
			DoubleArray::iterator /*it, it2,*/ it_f, it2_f;
			it_f	= resultsObj.dfResults_.paymentDatesAsTerms_.begin();
			it2_f	= resultsObj.dfResults_.discountFactors_.begin();
				
			it_f++;		//skip first element (term = 0.0);
			it2_f++;	//skip first element (df = 1.0);
				
			DateVector::iterator it2_d_f = resultsObj.dfResults_.paymentDates_.begin();
				
			it2_d_f++;	//skip first element (thisEndDate = asOfDate)

			for (; it_f != resultsObj.dfResults_.paymentDatesAsTerms_.end(); it_f++, it2_f++, it2_d_f++)
			{
				// Rate Priority: Remove Discount Factors that are too close the Priority Rates, within proximity tolerance
				if ((*it2_d_f).intervalDays(swapEndDates_[i]) <= RATE_PRIORITY_PROXIMITY_DAY_TOLERANCE)
				{
					resultsObj.dfResults_.paymentDatesAsTerms_.erase(it_f, resultsObj.dfResults_.paymentDatesAsTerms_.end());
					resultsObj.dfResults_.discountFactors_.erase(it2_f, resultsObj.dfResults_.discountFactors_.end());
					resultsObj.dfResults_.paymentDates_.erase(it2_d_f, resultsObj.dfResults_.paymentDates_.end());
					break;
				}
			}
		}
	}
}

// Get the Accrual Period and Payment Dates
void SwapCashflows::updateSwapFixedCashflows( CurveMarketDataHolder & mktDataObj, SwapInstruments & swaps )
{
	for (size_t nthSwap = 0; nthSwap < mktDataObj.mktData_.swap_size_; nthSwap++)
	{
		// 1. Update Swap Fixed Accrual Periods and Payment Dates
		// ******************************************************************

		if( mktDataObj.mktData_.areSwapsForwardStarting_ )
		{ 
			// *** FORWARD STARTING SWAPS ***

			// Forward Starting Swaps: Require Swap Spot Date
			updateAccrualPeriodsAndPaymentDates( swapStartDates_[nthSwap],
												 swapEndDatesUnadjusted_[nthSwap],
												 swaps.frequencyFixed_[nthSwap],
												 *swaps.calendar_[nthSwap],
												 *swaps.busDayAdj_[nthSwap],
												 *swaps.daycountFixed_[nthSwap],
												 fixedCashflowPaymentDates_[nthSwap],
												 fixedCashflowPaymentDatesAsTerms_[nthSwap],
												 fixedAccrualPeriods_[nthSwap],
												 swaps.isEOMRoll_[nthSwap],
												 &swaps.spotDate_[nthSwap] );
		
			// Update Fixed Start Date as Terms
			fixedStartDatesAsTerms_[nthSwap] = swaps.daycountAct365_.getTerm( swaps.spotDate_[nthSwap], swapStartDates_[nthSwap] );
		}
		else
		{
			// *** SPOT STARTING SWAPS ***
			updateAccrualPeriodsAndPaymentDates( swaps.spotDate_[nthSwap],
												 swapEndDatesUnadjusted_[nthSwap],
												 swaps.frequencyFixed_[nthSwap],
												 *swaps.calendar_[nthSwap],
												 *swaps.busDayAdj_[nthSwap],
												 *swaps.daycountFixed_[nthSwap],
												 fixedCashflowPaymentDates_[nthSwap],
												 fixedCashflowPaymentDatesAsTerms_[nthSwap],
												 fixedAccrualPeriods_[nthSwap],
												 swaps.isEOMRoll_[nthSwap] );
		}

		// 2.	Update Swap Fixed Coupons
		// ******************************************************************
		unsigned int size_fixedCoupons = fixedCashflowPaymentDates_[nthSwap].size();
		fixedCoupons_[nthSwap].resize(size_fixedCoupons);
		for (unsigned int j = 0; j < size_fixedCoupons; j++)
		{
			fixedCoupons_[nthSwap][j] = swapRates_[nthSwap] * fixedAccrualPeriods_[nthSwap][j];
		}

		// 3.	Fixed Cashflow Payment Dates For Risk
		// ******************************************************************
		// Legacy Code - Not sure if this works or if it is required anymore

		DateVector thisFixedCashflowPaymentDates(fixedCashflowPaymentDates_[nthSwap]);
		thisFixedCashflowPaymentDates.insert(thisFixedCashflowPaymentDates.begin(), swaps.spotDate_[nthSwap]);
		if (!mktDataObj.mktData_.swap_[nthSwap]->getData(IR_CALIBRATION_DATA_CALCDATESFORDVZERO, NOCHECK).isDefined())
		{
			mktDataObj.mktData_.swap_[nthSwap]->add(IR_CALIBRATION_DATA_CALCDATESFORDVZERO, new AQLDataDates(thisFixedCashflowPaymentDates));
		}
		else
		{
			dynamic_cast<AQLDataDates&>(mktDataObj.mktData_.swap_[nthSwap]->getData(IR_CALIBRATION_DATA_CALCDATESFORDVZERO).get()).set(thisFixedCashflowPaymentDates);
		}
	}
}

// Get the Accrual Period and Payment Dates
void SwapCashflows::updateSwapFloatCashflows( CurveMarketDataHolder & mktDataObj, SwapInstruments & swaps )
{
	for (size_t nthSwap = 0; nthSwap < mktDataObj.mktData_.swap_size_; nthSwap++)
	{
		// 1. Update Swap Fixed Accrual Periods and Payment Dates
		// ******************************************************************

		AQ_REQUIRE( swaps.liborIndexFrequency_[nthSwap] != "", "Invalid Float Schedule: Libor Float Index is Required" )
		AQ_REQUIRE( swaps.daycountFloat_[nthSwap] != NULL ,	 "Invalid Float Schedule: Float Daycount is Required" )
		
		if (mktDataObj.mktData_.areSwapsForwardStarting_)
		{
			// *** FORWARD STARTING SWAPS ***

			// Forward Starting Swaps: Require Swap Spot Date
			updateAccrualPeriodsAndPaymentDates( swapStartDates_[nthSwap],
												 swapEndDatesUnadjusted_[nthSwap],
												 swaps.liborIndexFrequency_[nthSwap],
												 *swaps.calendar_[nthSwap],
												 *swaps.busDayAdj_[nthSwap],
												 *swaps.daycountFloat_[nthSwap],
												 floatCashflowPaymentDates_[nthSwap],
												 floatCashflowPaymentDatesAsTerms_[nthSwap],
												 floatAccrualPeriods_[nthSwap],
												 swaps.isEOMRoll_[nthSwap],
												 &swaps.spotDate_[nthSwap] );

			floatStartDatesAsTerms_[nthSwap] = swaps.daycountAct365_.getTerm(swaps.spotDate_[nthSwap], swapStartDates_[nthSwap]);
		}
		else
		{
			// *** SPOT STARTING SWAPS ***
			updateAccrualPeriodsAndPaymentDates( swapStartDates_[nthSwap],
									             swapEndDatesUnadjusted_[nthSwap],
									             swaps.liborIndexFrequency_[nthSwap],
									             *swaps.calendar_[nthSwap],
									             *swaps.busDayAdj_[nthSwap],
									             *swaps.daycountFloat_[nthSwap],
									             floatCashflowPaymentDates_[nthSwap],
									             floatCashflowPaymentDatesAsTerms_[nthSwap],
									             floatAccrualPeriods_[nthSwap],
									             swaps.isEOMRoll_[nthSwap] );
		}

		// 2. Float Index Start Date
		// ******************************************************************

		// a)	Initialize Float Index Start Dates using Accrual Dates
		fixingStartDates_[nthSwap].push_back(swaps.spotDate_[nthSwap]);
		for (size_t j = 0; j < floatCashflowPaymentDates_[nthSwap].size() - 1; ++j)
		{
			fixingStartDates_[nthSwap].push_back(floatCashflowPaymentDates_[nthSwap][j]);
		}
		
		// b)	Apply Float Index Fixing Lag to Fixing Start Dates
		
		// TODO: Enable Fixing Lags - Not Supported at Present
		// Note: The calcDatesWithLag method is slow!!!
		// ---------------------------------------------------

		//fixingLags_[nthSwap] = "0D";

		//fixingStartDates_[nthSwap]
		//	= LADateScheduleHelpers::calcDatesWithLag ( fixingStartDates_[nthSwap],
		//												fixingLags_[nthSwap],
		//												*swaps.busDayAdj_[nthSwap],
		//												swaps.calendar_[nthSwap],
		//												false,		 // isAfter
		//												nullptr );   // This RollConv = Start, End, IMM, EOM or NULL

		// 3. Float Index End Date
		// ******************************************************************
		fixingEndDates_[nthSwap] = LADateScheduleHelpers::getMultiDate( fixingStartDates_[nthSwap],
																		swaps.liborIndexTenor_[nthSwap],
																		swaps.busDayAdj_[nthSwap]->convertToString(),
																		swaps.calendar_[nthSwap]->convertToString(),
																		nullptr ); // rollconvention* = nullptr

		// 4.	Fixing Accrual Period
		// ******************************************************************
		for (size_t j = 0; j < fixingStartDates_[nthSwap].size(); ++j)
		{
			fixingStarts_[nthSwap].push_back(swaps.daycountAct365_.getTerm(swaps.spotDate_[nthSwap], fixingStartDates_[nthSwap][j]));
			fixingEnds_[nthSwap].push_back(swaps.daycountAct365_.getTerm(swaps.spotDate_[nthSwap], fixingEndDates_[nthSwap][j]));
			double fixingAccrualPeriod = swaps.daycountFloat_[nthSwap]->getTerm(fixingStartDates_[nthSwap][j], fixingEndDates_[nthSwap][j]);
			fixingAccrualPeriods_[nthSwap].push_back(fixingAccrualPeriod);
		}
	}
}

// Get the Swap Fixed Accrual Periods and Payment Dates
void SwapCashflows::updateTenorBasisSwapFloatCashflows( LACurveStaticDataHolder & staticDataObj, CurveMarketDataHolder & mktDataObj, SwapInstruments & swaps, TenorBasisInstruments & tenorBasisSwaps )
{
	// Set up swap date schedules in preparation for pricing
	AQ_THROW_IF( staticDataObj.curveProperties_.useTenorBasisSwaps_ &&
				   tenorBasisSwaps.spreads_.size() != mktDataObj.mktData_.swap_size_,
				   "Invalid Tenor Basis Swap Data: Inconsistent number of IRS Swaps and Tenor Basis Swaps")

	for (size_t i = 0; i < mktDataObj.mktData_.swap_size_; i++)
	{
		// Tenor Swap Coupons
		// ***************************
		if (staticDataObj.curveProperties_.useTenorBasisSwaps_)
		{
			// Update Tenor Basis Float Cashflows
			// **********************************
			
			if (staticDataObj.curveProperties_.useTenorBasisSwaps_)
			{
				updateAccrualPeriodsAndPaymentDates( swaps.spotDate_[i],
													swapEndDatesUnadjusted_[i],
													tenorBasisSwaps.againstLegFrequency_,
													*tenorBasisSwaps.calendar_,
													*tenorBasisSwaps.busDayAdj_,
													*tenorBasisSwaps.daycount_,
													tenorSwapCashflowPaymentDates_[i],
													tenorSwapCashflowPaymentDatesAsTerms_[i],
													tenorSwapAccrualPeriods_[i],
													tenorBasisSwaps.isRollEOM_ );

				unsigned int size_tenorSwapCoupons = tenorSwapCashflowPaymentDates_[i].size();
				tenorSwapCoupons_[i].resize(size_tenorSwapCoupons);
				for (unsigned int j = 0; j < size_tenorSwapCoupons; j++)
				{
					tenorSwapCoupons_[i][j] = tenorBasisSwaps.spreads_[i] * tenorSwapAccrualPeriods_[i][j];
				}
			}
		}
	}
}

// Check if Discount Factor interpolation is using Hybrid / Mixed Interpolation
bool isHybridDiscountFactorInterpolation( CalibrationResults & resultsObj )
{
	bool isHybrid = false;
	if (resultsObj.interpolationObj_->interpolatorForDiscountFactors_ != NULL)
	{
		isHybrid = resultsObj.interpolationObj_->interpolatorForDiscountFactors_->isHybrid();
	}
	return isHybrid;
}

// Check if Swap interpolation is using Hybrid / Mixed Interpolation
bool isHybridSwapInterpolation( CalibrationResults & resultsObj )
{
	bool isHybrid = false;
	if (resultsObj.interpolationObj_->interpolatorForSwaps_ != NULL)
	{
		isHybrid = resultsObj.interpolationObj_->interpolatorForSwaps_->isHybrid();
	}
	return isHybrid;
}

// Check if Futures / FRA interpolation is using Hybrid / Mixed Interpolation
bool isHybridFuturesAndFRAsInterpolation( CalibrationResults & resultsObj )
{
	bool isHybrid = false;
	if (resultsObj.interpolationObj_->interpolatorForFuturesAndFRAs_ != NULL)
	{
		isHybrid = resultsObj.interpolationObj_->interpolatorForFuturesAndFRAs_->isHybrid();
	}
	return isHybrid;
}

// Method to check if we are using mixed hybrid interpolation schemes
// Note resultsObj contains the interpolation results
bool isCurveUsingHybridInterpolation( CalibrationResults & resultsObj )
{
	bool isHybrid = isHybridDiscountFactorInterpolation( resultsObj ) ||
				    isHybridSwapInterpolation( resultsObj ) ||
					isHybridFuturesAndFRAsInterpolation( resultsObj );

	return isHybrid;
}

// Method to set the join date for mixed / hybrid interpolation methods
void importHybridInterpolationJoinDateDefaults( LACurveStaticDataHolder & staticDataObj, CurveMarketDataHolder & mktDataObj, LiborIndex & liborIndex, CalibrationResults & resultsObj )
{
	// Set Output Variable: linear-spline interpolation join date
	resultsObj.interpolationObj_->interpolationJoinDateAsDouble_ = 0.0;
	resultsObj.interpolationObj_->interpolationJoinDate_ = AQLDate();
	resultsObj.joinDateGiven_		= false;
	resultsObj.calculateJoinDate_	= true;

	if ( resultsObj.doesCurveUseHybridInterpolation_ )
	{
		AQ_REQUIRE( mktDataObj.mktData_.swap_size_ > 0, "Invalid Swap Instruments: Swap Instruments are Missing" )
		AQLDate spotDate_swap	= dynamic_cast<const AQLDataDate&> ((mktDataObj.mktData_.swap_[0]->getData(IR_CALIBRATION_DATA_SPOTDATE, ISNOTNULL)).get());

		// Internal curve model daycount required to tranform a date to a double
		AQLPriceDataDayCount dc_act365(ACT_365);

		// If a join date is given by user as an input, we use this join date and don't calculate join date;
		// If no user-given join date exists, we will decide whether we always re-calculate the dynamic
		// join date, or only calculate it once and re-use thie join date in subsequent calibrations of the same curve.

		// Calculate the initial linear spline join date
		const AQLDataHolder* handle = &(staticDataObj.curveDataObjectHolder_.getData(IR_CALIBRATION_DATA_INPUT_INTERPOLATIONJOINDATE + staticDataObj.targetCurveMktSuffix_, NOCHECK));
		if (handle->isDefined() && !handle->isNull())
		{
			// Use a linear spline join date supplied by user
			resultsObj.joinDateGiven_ = true;
			AQLDate joinDate = dynamic_cast<const AQLDataDate&>(handle->get()).get();
			resultsObj.interpolationObj_->interpolationJoinDate_ = joinDate;
			resultsObj.interpolationObj_->interpolationJoinDateAsDouble_ = dc_act365.getTerm(spotDate_swap, joinDate);
		}
		else
		{
			// Always recalculate the dynamic join date?
			bool alwaysRecalcJoinDate = true;
			const AQLDataHolder* handle = &(staticDataObj.curveDataObjectHolder_.getData(IR_CALIBRATION_DATA_ALWAYSCALCJOINDATE + staticDataObj.targetCurveMktSuffix_, NOCHECK));
			if (handle->isDefined() && !handle->isNull())
			{
				alwaysRecalcJoinDate = dynamic_cast<const AQLDataBool&>(handle->get()).get();
			}

			if (!alwaysRecalcJoinDate)
			{
				handle = &(staticDataObj.curveDataObjectHolder_.getData(CALIBRATION_DATA_INTERPOLATION_JOINDATE_ASDOUBLE + staticDataObj.targetCurveMktSuffix_, NOCHECK));
				if (handle->isDefined() && !handle->isNull())
				{
					double existingJoinDateAsDouble = dynamic_cast<const AQLDataDouble&>(handle->get()).get();
					resultsObj.interpolationObj_->interpolationJoinDateAsDouble_ = existingJoinDateAsDouble;

					handle = &(staticDataObj.curveDataObjectHolder_.getData(CALIBRATION_DATA_INTERPOLATION_JOINDATE + staticDataObj.targetCurveMktSuffix_, NOCHECK));
					resultsObj.interpolationObj_->interpolationJoinDate_ = dynamic_cast<const AQLDataDate&>(handle->get()).get();

					resultsObj.calculateJoinDate_ = false;
				}
			}

			// AlgoQuantLib calculates linear spline join date when:
			// 1. User specifies to ALWAYS recalculate join date; or
			// 2. User specifies not to always recalculate join date, but no pre-calculated join date currently exists;

			if (resultsObj.calculateJoinDate_)
			{
				double joinDateAsDouble = 0.0;
				if (staticDataObj.curveProperties_.useFRAs_ && mktDataObj.mktData_.fra_size_ > 0)
				{
					const AQLDate spotDate = dynamic_cast<const AQLDataDate&>((mktDataObj.mktData_.fra_[mktDataObj.mktData_.fra_size_ - 1]->getData(IR_CALIBRATION_DATA_SPOTDATE, ISNOTNULL)).get()).get();
					const AQLPriceDataDayCount& fraDayCount = dynamic_cast<const AQLPriceDataDayCount&>((mktDataObj.mktData_.fra_[mktDataObj.mktData_.fra_size_ - 1]->getData(IR_CALIBRATION_DATA_DAYCOUNT, ISNOTNULL)).get());

					AQ_REQUIRE( liborIndex.frequency_ != "", "Invalid Libor Index: Libor Index Frequency is Missing" )
					AQ_REQUIRE( liborIndex.tenor_ == AQ_TERM_3M || liborIndex.tenor_ == AQ_TERM_6M, "Invalid Libor Index: FRA Instrument Frequency must be 3M or 6M" )

					AQLDate lastFraStartDate, lastFraEndDate;
					calculateFraDates(lastFraStartDate, lastFraEndDate, spotDate, mktDataObj.mktData_.fra_[mktDataObj.mktData_.fra_size_ - 1], liborIndex.tenor_, liborIndex.frequency_, fraDayCount);

					// Do not allow join date to be beyond the first swap maturity date
					if (lastFraEndDate >= mktDataObj.mktData_.firstSwapMaturityDate_)
					{
						lastFraEndDate = mktDataObj.mktData_.firstSwapMaturityDate_;
					}

					resultsObj.interpolationObj_->interpolationJoinDate_ = lastFraEndDate;
					joinDateAsDouble = dc_act365.getTerm(spotDate_swap, lastFraEndDate);
				}
				else if (staticDataObj.curveProperties_.useFutures_ && mktDataObj.mktData_.future_size_ > 0)
				{
					resultsObj.interpolationObj_->interpolationJoinDate_ = determineLinearSplineInterpolationJoinDate(mktDataObj.mktData_.future_[mktDataObj.mktData_.future_size_ - 1], mktDataObj.mktData_.swap_[0], spotDate_swap, mktDataObj.mktData_.areSwapsForwardStarting_ );
					joinDateAsDouble = dc_act365.getTerm(spotDate_swap, resultsObj.interpolationObj_->interpolationJoinDate_);
				}

				resultsObj.interpolationObj_->interpolationJoinDateAsDouble_ = joinDateAsDouble;
			}
		}

		// Set the Linear-Spline Interpolation Join Date for the Disc Factor Interpolation Table
		if (resultsObj.isHybridDiscountFactorInterpolation_)
		{
			resultsObj.interpolationObj_->interpolatorForDiscountFactors_->setJoinDateAsDouble(resultsObj.interpolationObj_->interpolationJoinDateAsDouble_);
		}

		// Set the Linear-Spline Interpolation Join Date for the Swap Rate Interpolation Table
		if (resultsObj.isHybridSwapsInterpolation_)
		{
			resultsObj.interpolationObj_->interpolatorForSwaps_->setJoinDateAsDouble(resultsObj.interpolationObj_->interpolationJoinDateAsDouble_);
		}

		// Set the Linear-Spline Interpolation Join Date for the Forward Interpolation Table
		if (resultsObj.isHybridFuturesAndFRAsInterpolation_)
		{
			resultsObj.interpolationObj_->interpolatorForFuturesAndFRAs_->setJoinDateAsDouble(resultsObj.interpolationObj_->interpolationJoinDateAsDouble_);
		}
	}
}

// Method to rebase discount factors from the swap spotDate to the curve AsOfDate
void rebaseDiscountFactorsToCurveAsOfDate( LACurveStaticDataHolder & staticDataObj, CurveMarketDataHolder & mktDataObj, LiborIndex & liborIndex, const SwapInstruments & swaps, SwapCashflows & swapCashflows, CalibrationResults & resultsObj )
{
	// 1.	Rebase Discount Factors from Swap SpotDate to the Curve AsOfDate
	// *********************************************************************

	// Calculate AsofDate base df
	DoubleMatrix df_moneymarket(2);
	DateVector df_moneymarket_date;
	AQLPriceDataDayCount	daycountAct365(ACT_365);

	// Populate dfSpot, df_moneymarket, df_moneymarket_date
	const double dfSpot = getSpotDFandUpdateMoneyMarket(df_moneymarket, df_moneymarket_date, mktDataObj.mktData_.moneyMarket_, daycountAct365, liborIndex.curveSpotDate_);

	if (df_moneymarket_date.end() != find(df_moneymarket_date.begin(), df_moneymarket_date.end(), resultsObj.dfResults_.paymentDates_[0]))
	{
		// erase first element( t=0, df=1.0, date = spotdate_swap)
		resultsObj.dfResults_.paymentDatesAsTerms_.erase(resultsObj.dfResults_.paymentDatesAsTerms_.begin());
		resultsObj.dfResults_.discountFactors_.erase(resultsObj.dfResults_.discountFactors_.begin());
		resultsObj.dfResults_.paymentDates_.erase(resultsObj.dfResults_.paymentDates_.begin());
	}

	
	const double spotTerm = daycountAct365.getTerm(resultsObj.asOfDate_, liborIndex.curveSpotDate_);
	for (unsigned int i = 0; i < resultsObj.fwdStartEndDatesAsTerms_[0].size(); i++)
	{
		resultsObj.fwdStartEndDatesAsTerms_[0][i] += spotTerm;
		resultsObj.fwdStartEndDatesAsTerms_[1][i] += spotTerm;
	}

	unsigned int numberOfDiscountFactors = resultsObj.dfResults_.discountFactors_.size();
	for (unsigned int i = 0; i < numberOfDiscountFactors; i++)
	{
		resultsObj.dfResults_.paymentDatesAsTerms_[i] += spotTerm;
		resultsObj.dfResults_.discountFactors_[i] *= dfSpot;
	}

	if (resultsObj.interpolationObj_->interpolatorForSwaps_->isHybrid())
	{
		resultsObj.interpolationObj_->interpolationJoinDateAsDouble_ += spotTerm;
	}

	// 2.	Update Curve Results 
	// ***********************************************************************************************************

	// Find FirstPaymentDatePosition, search for "paymentDates[0]" in "df_moneymarket_date"
	unsigned int firstPaymentDatePosition;
	unsigned int size_marketdata = df_moneymarket_date.size();
	AQLAlgorithm::locate(df_moneymarket_date, resultsObj.dfResults_.paymentDates_[0], size_marketdata, firstPaymentDatePosition);
	
	if (firstPaymentDatePosition == size_marketdata)
	{
		resultsObj.dfResults_.paymentDatesAsTerms_.insert(resultsObj.dfResults_.paymentDatesAsTerms_.begin(), df_moneymarket[0].begin(), df_moneymarket[0].end());
		resultsObj.dfResults_.discountFactors_.insert(resultsObj.dfResults_.discountFactors_.begin(), df_moneymarket[1].begin(), df_moneymarket[1].end());
		resultsObj.dfResults_.paymentDates_.insert(resultsObj.dfResults_.paymentDates_.begin(), df_moneymarket_date.begin(), df_moneymarket_date.end());
	}
	else
	{
		resultsObj.dfResults_.paymentDatesAsTerms_.insert(resultsObj.dfResults_.paymentDatesAsTerms_.begin(), df_moneymarket[0].begin(), df_moneymarket[0].begin() + firstPaymentDatePosition);
		resultsObj.dfResults_.discountFactors_.insert(resultsObj.dfResults_.discountFactors_.begin(), df_moneymarket[1].begin(), df_moneymarket[1].begin() + firstPaymentDatePosition);
		resultsObj.dfResults_.paymentDates_.insert(resultsObj.dfResults_.paymentDates_.begin(), df_moneymarket_date.begin(), df_moneymarket_date.begin() + firstPaymentDatePosition);
		
		unsigned int moneyMarketInstrumentPosition;
		for (unsigned int i = firstPaymentDatePosition; i < size_marketdata; i++)
		{

			// Find moneyMarketInstrumentPosition, search for "paymentDates[0]" in "df_moneymarket_date"
			AQLAlgorithm::locate(resultsObj.dfResults_.paymentDates_, df_moneymarket_date[i], resultsObj.dfResults_.paymentDates_.size(), moneyMarketInstrumentPosition);
			
			if (moneyMarketInstrumentPosition == resultsObj.dfResults_.paymentDates_.size())
			{
				resultsObj.dfResults_.paymentDatesAsTerms_.push_back(df_moneymarket[0][i]);
				resultsObj.dfResults_.discountFactors_.push_back(df_moneymarket[1][i]);
				resultsObj.dfResults_.paymentDates_.push_back(df_moneymarket_date[i]);
			}
			else if (resultsObj.dfResults_.paymentDates_[moneyMarketInstrumentPosition] != df_moneymarket_date[i])
			{
				resultsObj.dfResults_.paymentDatesAsTerms_.insert(resultsObj.dfResults_.paymentDatesAsTerms_.begin() + moneyMarketInstrumentPosition, df_moneymarket[0][i]);
				resultsObj.dfResults_.discountFactors_.insert(resultsObj.dfResults_.discountFactors_.begin() + moneyMarketInstrumentPosition, df_moneymarket[1][i]);
				resultsObj.dfResults_.paymentDates_.insert(resultsObj.dfResults_.paymentDates_.begin() + moneyMarketInstrumentPosition, df_moneymarket_date[i]);
			}
		}
	}

	// Boundary Condition Add term = 0 and df = 1.0
	if (resultsObj.dfResults_.paymentDatesAsTerms_[0] != 0.0)
	{
		resultsObj.dfResults_.paymentDatesAsTerms_.insert(resultsObj.dfResults_.paymentDatesAsTerms_.begin(), 0.0);
		resultsObj.dfResults_.discountFactors_.insert(resultsObj.dfResults_.discountFactors_.begin(), 1.0);
	}

	// 3.	Insert Extrapolation Terms
	// ***********************************************************************************************************

	const AQLDataHolder* dh = &staticDataObj.curveDataObjectHolder_.getData(IR_CALIBRATION_DATA_MAXTERM);
	if (dh->isDefined() && !dh->isNull())
	{
		const AQLString maxTerm = dynamic_cast<const AQLDataString&>(dh->get()).get() + "Y";
		const AQLDate& maxDate = LADateHelpers::getDate(liborIndex.curveSpotDate_, maxTerm, AQLPriceDataSlidingRule(SLIDING_RULE_NO_CHANGE), NULL, true, nullptr);
		if (maxDate > swapCashflows.swapEndDates_.back())
		{
			const AQLString maxFreq = dynamic_cast<const AQLDataString&>(staticDataObj.curveDataObjectHolder_.getData(IR_CALIBRATION_DATA_MAXTERMFREQ).get()).get();
			DoubleArray extra_terms;
			DateVector tmp_dates; DoubleArray tmp_taus;
			updateAccrualPeriodsAndPaymentDates(liborIndex.curveSpotDate_, maxDate, maxFreq, *swaps.calendar_[0], *swaps.busDayAdj_[0], *swaps.daycountFixed_[0], tmp_dates, extra_terms, tmp_taus, swaps.isEOMRoll_[0]);
			for (unsigned int i = 0; i < extra_terms.size(); ++i)
			{
				const double term = extra_terms[i] + spotTerm;
				if (term <= resultsObj.dfResults_.paymentDatesAsTerms_.back())
				{
					continue;
				}

				resultsObj.dfResults_.paymentDatesAsTerms_.push_back(term);
				resultsObj.dfResults_.discountFactors_.push_back(resultsObj.dfResults_.discountFactors_.back());
				if (!resultsObj.fwdStartEndDatesAsTerms_[0].empty())
				{
					resultsObj.fwdStartEndDatesAsTerms_[0].push_back(resultsObj.fwdStartEndDatesAsTerms_[1].back());
					resultsObj.fwdStartEndDatesAsTerms_[1].push_back(term);
				}
			}
		}
	}
}

// Method to Bootstrap Libor Cash Deposits
void bootstrapLibors( LACurveStaticDataHolder & staticDataObj, CurveMarketDataHolder & mktDataObj, LiborIndex & liborIndex, CalibrationResults & resultsObj )
{
	resultsObj.dfResults_ = bootstrapLibors( resultsObj.fwdRates_,
												 resultsObj.fwdStartEndDatesAsTerms_,
												 mktDataObj.mktData_.libor_,
												 mktDataObj.mktData_.moneyMarket_,
												 staticDataObj.curveProperties_.useFRAs_,
												 staticDataObj.curveProperties_.useFutures_,
												 liborIndex.curveSpotDate_,
												 liborIndex.spotDate_,
												 liborIndex.tenor_,
												 staticDataObj.curveProperties_.implyForwards_ );
}

// Bootstrap FRAs and Set Results to Object Pool
void bootstrapFRAs( LACurveStaticDataHolder & staticDataObj, CurveMarketDataHolder & mktDataObj, LiborIndex & liborIndex, CalibrationResults & resultsObj )
{
	if( !staticDataObj.curveProperties_.useFRAs_ || mktDataObj.mktData_.fra_size_ == 0 )
	{
		return;
	}

	bootstrapFRAs( resultsObj.dfResults_,
				   resultsObj.fwdRates_,
				   resultsObj.fwdStartEndDatesAsTerms_,
				   mktDataObj.mktData_.fra_,
				   mktDataObj.mktData_.libor_,
				   resultsObj.interpolationObj_->interpolatorForFuturesAndFRAs_,
				   staticDataObj.curveProperties_.stateVariableFRAs_,
				   mktDataObj.mktData_.areSwapsForwardStarting_,
				   liborIndex.frequency_,
				   liborIndex.tenor_,
				   resultsObj.asOfDate_,
				   liborIndex.curveSpotDate_,
				   liborIndex.spotDate_,
				   mktDataObj.mktData_.firstSwapMaturityDate_,
				   staticDataObj.curveProperties_.implyForwards_,
				   resultsObj.interpolationObj_->interpolationJoinDateAsDouble_,
				   resultsObj.dfResults_.lastLiborPosition_,
				   resultsObj.dfResults_.lastLiborEndDate_ );
}

// Bootstrap FRAs and Set Results to Object Pool
void bootstrapFutures( LACurveStaticDataHolder & staticDataObj, CurveMarketDataHolder & mktDataObj, LiborIndex & liborIndex, FuturesInstruments & futures, CalibrationResults & resultsObj )
{
	if ( staticDataObj.curveProperties_.useFutures_  && mktDataObj.mktData_.future_size_ > 0 )
	{
		bootstrapFuturesOrFRAs( resultsObj.dfResults_,
								resultsObj.fwdStartEndDatesAsTerms_,
								resultsObj.fwdRates_,
								futures.type_,
								futures.getForwardRates(),
								mktDataObj.mktData_.libor_,
								mktDataObj.mktData_.future_,
								futures.marketQuotes_.startDates_,
								futures.marketQuotes_.endDates_,
								futures.marketQuotes_.marketRates_,
								resultsObj.asOfDate_,
								liborIndex.curveSpotDate_,
								liborIndex.spotDate_,
								*resultsObj.interpolationObj_->interpolatorForFuturesAndFRAs_,
								liborIndex.tenor_,
								futures.rollConvention_,
								*liborIndex.fixingBusDayAdj_,
								*liborIndex.fixingCalendar_,
								futures.daycount_,
								futures.daycountAct365_,
								staticDataObj.curveProperties_.stateVariableFutures_,
								resultsObj.interpolationObj_->interpolationJoinDateAsDouble_,
								resultsObj.dfResults_.lastLiborEndDate_,
								resultsObj.dfResults_.lastLiborPosition_,
								mktDataObj.mktData_.firstSwapMaturityDate_,
								futures.includeSwapsBeforeMPCSwaps_,
								futures.applyTensionFutures_,
								futures.tensionDayGap_,
								staticDataObj.curveProperties_.useFutures_ );
	}
}

SwapCalibration::SwapCalibration()
	:	resultTolerance_(1.0e-9),
		gradientTolerance_(1.0e-15),
		shiftSize_(1.0e-10),
		maxIterations_(1000),
		fastConvergenceCheck_(false),
		fastRebuild_(false),
		previousSolutionAvailable_(false),
		isSwapCurveSelfDiscounting_(false),
		preCalibrationDFSize_(0),
		initialBumpSize_(1.0e-5),
		swapCurveDiscountFactors_(SwapCurveDiscountFactors()),
		calibrationResults_(solvers::MultiVariateSolverResults())
{
}

SwapCalibration::SwapCalibration( LACurveStaticDataHolder & staticDataObj,
								  CurveMarketDataHolder & mktDataObj,
								  const LiborIndex & liborIndex,
								  const FuturesInstruments & futures,
								  const SwapInstruments & swaps,
								  SwapCashflows & swapCashflows,
								  CalibrationResults & resultsObj )
	:	resultTolerance_(1.0e-9),
		gradientTolerance_(1.0e-15),
		shiftSize_(1.0e-10),
		maxIterations_(1000),
		fastConvergenceCheck_(false),
		fastRebuild_(false),
		previousSolutionAvailable_(false),
		isSwapCurveSelfDiscounting_(false),
		preCalibrationDFSize_(0),
		initialBumpSize_(1.0e-5),
		swapCurveDiscountFactors_(SwapCurveDiscountFactors()),
		calibrationResults_(solvers::MultiVariateSolverResults())
{
	// 1.	Import Calibration Parameters
	importCalibrationParameters( staticDataObj, mktDataObj, resultsObj );
	
	// 2.	FAST REBUILD  - Use Previous Solution as Initial Guess, if available
	applyPreviousSolution( staticDataObj, mktDataObj, swapCashflows, resultsObj );

	// 3.	JOIN DATE - Find the Optimal Futures/Swap Interpolation Join Date
	optimizeFuturesJoinDateWithSwaps( staticDataObj, mktDataObj, liborIndex, futures, swaps, resultsObj );

	// 4.	Initialize Swap Curve OIS Discount Factors & Interpolator
	swapCurveDiscountFactors_ = initialiseSwapCurveDiscountFactors( staticDataObj, liborIndex, resultsObj );

	// 5.	Initialize Solver Target Function
	// ***********************************************************************************************************
	/* @brief The target objective function used by the multivariate Newton-Raphson solver. This function
	*  calculates the PV of all swap calibration instruments for the current estimate of the
	*  curve state variables.
	*
	*  Note that the targetFunction lambda contains a capture-list [&] which captures all locals by reference.
	*  It uses this feature to update the interpolator 'interpolatorForSwaps' with the current solution guess.
	*  When the solver completes, 'interpolatorForSwaps' will contain the final solution 'zeroRateTimesTime' state variables.
	*
	*  @param[in]	stateVariables	The current estimate of the curve state variables we are solving for
	*  @returns		A vector of calibration instrument swap PVs, calculated using the provided stateVariables.
	*/
	auto targetFunction = [&,this](const std::vector<double>& stateVariables) -> std::vector<double>
	{
		// Update the state variable interpolator
		resultsObj.interpolationObj_->interpolatorForSwaps_->set( stateVariableTerms(), stateVariables );

		// Recalculate the PV of all calibration swaps, and return the PV as a vector
		return calculateSwapPVs( staticDataObj, mktDataObj, liborIndex, swaps, swapCashflows, *this, resultsObj );
	};

	// 6.	Run Swap Calibration Solver
	// ***********************************************************************************************************
	// Invoke the Multi-variate Newton Raphson solver to determine the curve state variables ( provided in 'zeroRateTimesTime' )
	// The solver searches for the state variables which cause the calibration swap instruments to PV to 0.
	// The result is the updated stateVariables_.rateTimeValues_

	calibrationResults_ = solvers::multiVariateNewtonRaphson(
		targetFunction,							/* targetFunction */
		stateVariables_.rateTimeValues_,		/* stateVariables by Zero Rate times Time */
		preCalibrationDFSize(),					/* offset into stateVariables vector */
		mktDataObj.mktData_.swap_size_,			/* numPoints to solve for */
		resultTolerance(),						/* tolerance */
		gradientTolerance(),					/* gradientTolerance */
		maxIterations(),						/* maxIterations */
		initialBumpSize(),						/* initialBumpSize */
		shiftSize(),							/* bumpSize for jacobian calculation */
		previousSolutionAvailable(),			/* useInverseJacobian */
		previousInverseJacobian(),				/* initialInverseJacobian */
		fastConvergenceCheck() );		 		/* fastConvergenceCheck */

	// 7.	Store State Variable Results and Jacobian Results
	storeStateVariableAndJacobianResults( mktDataObj, resultsObj );

	// 8.	Store Discount Factors and Implied Forward Rates on each swap cashflow date
	storeDiscountFactorsAndForwardRates( staticDataObj, mktDataObj, liborIndex, swaps, swapCashflows, resultsObj );
}


// Set Swap Calibration State Variables
void SwapCalibration::stateVariableTerms( const DoubleArray & terms )
{
	stateVariables_.terms_ = terms;
}

// Set Swap Calibration State Variables
void SwapCalibration::stateVariableValues( const DoubleArray & rateTimeValues )
{
	stateVariables_.rateTimeValues_ = rateTimeValues;
}

// Set Swap Calibration State Variables
void SwapCalibration::stateVariables( const DoubleArray & terms, const DoubleArray & rateTimeValues )
{
	stateVariables_.terms_			= terms;
	stateVariables_.rateTimeValues_ = rateTimeValues;
}

// Import Calibration Parameters
void SwapCalibration::importCalibrationParameters( LACurveStaticDataHolder & staticDataObj,
												   CurveMarketDataHolder & mktDataObj,
												   CalibrationResults & resultsObj )
{
	// Data Holder Accessor
	const AQLDataHolder* dh = 0;

	// ResultTolerance (epsilson, eps)
	resultTolerance_ = 1.0e-9;
	dh = &(mktDataObj.mktData_.swap_[0]->getData(IR_CALIBRATION_DATA_EPSILON, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		resultTolerance_ = dynamic_cast<const AQLDataDouble &>(dh->get()).get();
	}

	// Newton-Raphson Slope Tolerance
	gradientTolerance_ = 1.0e-15;
	dh = &(mktDataObj.mktData_.swap_[0]->getData(IR_CALIBRATION_DATA_GRADIENTEPSILON, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		gradientTolerance_ = dynamic_cast<const AQLDataDouble &>(dh->get()).get();
	}

	// Shift-Size (Delta)
	shiftSize_ = 1.0e-10;
	dh = &(mktDataObj.mktData_.swap_[0]->getData(IR_CALIBRATION_DATA_DELTA, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		shiftSize_ = dynamic_cast<const AQLDataDouble &>(dh->get()).get();
	}

	// Maximum Iteration Loops
	maxIterations_ = 1000;
	dh = &(mktDataObj.mktData_.swap_[0]->getData(IR_CALIBRATION_DATA_MAXLOOP, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		maxIterations_ = dynamic_cast<const AQLDataInt &>(dh->get()).get();
	}

	// Fast Convergence Check to Optimize Performance
	fastConvergenceCheck_ = false;
	dh = &(mktDataObj.mktData_.swap_[0]->getData(IR_CALIBRATION_DATA_OPTIMIZEPERFORMANCE, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		fastConvergenceCheck_ = dynamic_cast<const AQLDataBool &>(dh->get()).get();
	}

	// Fast Rebuild, Reuse previous solution for fast rebuild
	fastRebuild_ = false;
	dh = &(mktDataObj.mktData_.swap_[0]->getData(IR_CALIBRATION_DATA_FASTREBUILD, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		fastRebuild_ = dynamic_cast<const AQLDataBool &>(dh->get()).get();
	}

	// isSwapCurveSelfDiscounting
	isSwapCurveSelfDiscounting_ = ( staticDataObj.dfCurveName_ == "" || staticDataObj.dfCurveName_ == ITSELF );

	// preCalibrationDFSize
	preCalibrationDFSize_ = resultsObj.dfResults_.discountFactors_.size();
}

// Apply Previous Solution and Update Initial Guess StateVariables with Previous Solution if Available
void SwapCalibration::applyPreviousSolution( LACurveStaticDataHolder & staticDataObj,
										     CurveMarketDataHolder & mktDataObj,
										     const SwapCashflows & swapCashflows,
										     CalibrationResults & resultsObj )
{
	// Data Holder
	const AQLDataHolder* dh = 0;

	// Initialise state variables in preparation of solving
	DoubleArray stateVariableTerms( resultsObj.dfResults_.paymentDatesAsTerms_ );
	DoubleArray stateVariableValues( resultsObj.dfResults_.paymentDatesAsTerms_.size(), 0.0 );

	for ( unsigned int j = 0; j < resultsObj.dfResults_.paymentDatesAsTerms_.size(); ++j )
	{
		stateVariableValues[j] = getStateVariableValueFromDF(resultsObj.dfResults_.discountFactors_[j], staticDataObj.curveProperties_.stateVariableSwaps_ );
	}

	previousSolutionAvailable_ = false;
	if ( fastRebuild_ )
	{
		// Attempt to fetch the previous curve state-variable std::vector
		dh = &(staticDataObj.curveDataObjectHolder_.getData(IR_CALIBRATION_DATA_JACOBIAN_STATE_VARIABLES + staticDataObj.targetCurveMktSuffix_, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			const DoubleArray& previousSolution = dynamic_cast<const AQLDataDoubles &>(dh->get()).get();
			
			// Sanity check: Verify that the previous solution is the correct size
			if (previousSolution.size() == mktDataObj.mktData_.swap_size_)
			{
				for (unsigned int i = 0; i < mktDataObj.mktData_.swap_size_; ++i)
				{
					stateVariableTerms.push_back(swapCashflows.fixedCashflowPaymentDatesAsTerms_[i].back());
					stateVariableValues.push_back(previousSolution[i]);
				}

				// Now attempt to fetch the jacobian (gradient matrix)
				dh = &(staticDataObj.curveDataObjectHolder_.getData(IR_CALIBRATION_DATA_INVERSE_CURVE_JACOBIAN + staticDataObj.targetCurveMktSuffix_, NOCHECK));
				if (dh->isDefined() && !dh->isNull())
				{
					previousInverseJacobian_ = dynamic_cast<const AQLDataDoubleMatrix &>(dh->get()).get();
					
					// Sanity check: Verify the previous jacobian has the correct size
					if ((previousInverseJacobian_.size() == mktDataObj.mktData_.swap_size_) && (previousInverseJacobian_[0].size() == mktDataObj.mktData_.swap_size_))
					{
						previousSolutionAvailable_ = true;
					}
				}
			}
		}
	}

	if (!previousSolutionAvailable_)
	{
		// No previous solution. Setup a default initial guess
		for (unsigned int i = 0; i < mktDataObj.mktData_.swap_size_; ++i)
		{
			stateVariableTerms.push_back(swapCashflows.fixedCashflowPaymentDatesAsTerms_[i].back());
			double initialLogDF = swapCashflows.swapRates_[i] * swapCashflows.fixedCashflowPaymentDatesAsTerms_[i].back();
			stateVariableValues.push_back(getStateVariableValueFromRateTime(initialLogDF, staticDataObj.curveProperties_.stateVariableSwaps_));
		}
	}

	// Update Class State Variables with Previous Solution if Available
	stateVariables( stateVariableTerms, stateVariableValues );
}

// Method to determine the optimal Futures/Swaps join date when using Mixed/Hybrid Interpolation
// Optional: useNetwonRaphsonMinimizer - Default = true (use Newton-Raphson Minimizer instead of date search)
// Optional: searchIntervalDays - Default = 1 days (search every day = 1 (slow) )
void SwapCalibration::optimizeFuturesJoinDateWithSwaps( LACurveStaticDataHolder & staticDataObj,
														CurveMarketDataHolder & mktDataObj,
														const LiborIndex & liborIndex,
														const FuturesInstruments & futures,
														const SwapInstruments & swaps,
														CalibrationResults & resultsObj,
														const bool useNetwonRaphsonMinimizer,
														const size_t searchIntervalDays )
{
	if ( staticDataObj.curveProperties_.useFutures_ && mktDataObj.mktData_.future_size_ > 0 &&
		 resultsObj.isHybridSwapsInterpolation_&&
		 resultsObj.calculateJoinDate_ &&
		 !resultsObj.joinDateGiven_ )
	{
		// Initialise 'joinDateInterpolationByRateTime' by cloning 'interpolatorForSwaps' which holds the ZeroTime i.e. Negative LogDF that are being used to
		// Note: Negative LogDF  (ZeroTime) = LogDF for interpolation purposes when we have continuous / exponential compouding
		// solved for the most optimal interpolation join date. The optimal choice should make the
		// linear-spline forward rates as linear as possible at the linear vs spline joining point.
		// -------------------------------------------------------------------------------
		std::unique_ptr<AQLInterpolationBase> joinDateInterpolationByRateTime(dynamic_cast<AQLInterpolationBase *>(resultsObj.interpolationObj_->interpolatorForSwaps_->clone()));
		joinDateInterpolationByRateTime->setJoinDateAsDouble(resultsObj.interpolationObj_->interpolationJoinDateAsDouble_);
		joinDateInterpolationByRateTime->set( stateVariableTerms(), stateVariableValues() );

		auto actualFutureSize = futures.marketQuotes_.startDates_.size();
		DoubleArray futuresStartDateTerms(actualFutureSize);
		DoubleArray futuresEndDateTerms(actualFutureSize);

		for (unsigned int i = 0; i < actualFutureSize; i++)
		{
			futuresStartDateTerms[i] = futures.daycountAct365_.getTerm(liborIndex.curveSpotDate_, futures.marketQuotes_.startDates_[i]);
			futuresEndDateTerms[i] = futures.daycountAct365_.getTerm(liborIndex.curveSpotDate_, futures.marketQuotes_.endDates_[i]);
		}

		InterpolationJoinDate optimizeJoinDate = optimizeInterpolationJoinDate( joinDateInterpolationByRateTime,
																		        futuresStartDateTerms,
																		        futuresEndDateTerms,
																		        futures.marketQuotes_.marketRates_,
																		        stateVariableTerms(),
																		        stateVariableValues(),
																				useNetwonRaphsonMinimizer,
																				searchIntervalDays );

		// If join date optimisation succeeds, set it to all the relevant interpolators;
		// otherwise, do nothing and move on using the original join date.
		if (optimizeJoinDate.success_ = true)
		{
			resultsObj.interpolationObj_->interpolationJoinDateAsDouble_ = optimizeJoinDate.joinDateAsDouble_;

			// Recover the join date using interpolationJoinDateAsDouble and make sure the final join date is not a holiday
			double joinDateZoneStart = futuresStartDateTerms.back();
			double fractionFromLastFutureStartToJoin = resultsObj.interpolationObj_->interpolationJoinDateAsDouble_ - joinDateZoneStart;

			AQLPriceDataDayCount dc = *swaps.daycountFloat_[0];
			AQLPriceDataCalendar cal = *swaps.calendar_[0];
			AQLPriceDataSlidingRule sl = *swaps.busDayAdj_[0];
			resultsObj.interpolationObj_->interpolationJoinDate_ = LADateScheduleHelpers::getDateFromTerm(futures.lastStartDate_, fractionFromLastFutureStartToJoin, dc);

			// Adjust the joinDate and joinDateAsDouble for holidays
			resultsObj.interpolationObj_->interpolationJoinDate_ = LADateScheduleHelpers::getDate(resultsObj.interpolationObj_->interpolationJoinDate_, "0D", sl.convertToString(), cal.convertToString());
			resultsObj.interpolationObj_->interpolationJoinDateAsDouble_ = futures.daycountAct365_.getTerm(liborIndex.curveSpotDate_, resultsObj.interpolationObj_->interpolationJoinDate_);


			// Update Interpolation Tables with New Join Date
			// ----------------------------------------------

			// 1. Set the Linear-Spline Interpolation Join Date for the Swap Rate / ZeroTime State Variable Interpolation Table
			resultsObj.interpolationObj_->interpolatorForSwaps_->setJoinDateAsDouble(resultsObj.interpolationObj_->interpolationJoinDateAsDouble_);
			resultsObj.interpolationObj_->interpolatorForSwaps_->set( stateVariableTerms(), stateVariableValues() );

			// 2. Set the Linear-Spline Interpolation Join Date for the Disc Factor Interpolation Table
			if (resultsObj.isHybridDiscountFactorInterpolation_)
			{
				resultsObj.interpolationObj_->interpolatorForDiscountFactors_->setJoinDateAsDouble(resultsObj.interpolationObj_->interpolationJoinDateAsDouble_);
			}

			// 3. Set the Linear-Spline Interpolation Join Date for the Forward Interpolation Table

			if (resultsObj.isHybridFuturesAndFRAsInterpolation_)
			{
				resultsObj.interpolationObj_->interpolatorForFuturesAndFRAs_->setJoinDateAsDouble(resultsObj.interpolationObj_->interpolationJoinDateAsDouble_);
			}
			// ----------------------------------------------
		}
	}
}

// Price Swap Calibration Instruments
DoubleArray SwapCalibration::calculateSwapPVs( LACurveStaticDataHolder & staticDataObj,
											   CurveMarketDataHolder & mktDataObj,
											   const LiborIndex & liborIndex,
											   const SwapInstruments & swaps,
											   const SwapCashflows & swapCashflows,
											   SwapCalibration & swapCalibration,
											   CalibrationResults & resultsObj )
{
	// Initialize swapPVs Result
	DoubleArray swapPVs(mktDataObj.mktData_.swap_size_, 0.0);

	// Calculate Swap PVs
	calculateSwapPVs( *resultsObj.interpolationObj_->interpolatorForSwaps_,
					  staticDataObj.curveProperties_.stateVariableSwaps_,
					  swapPVs,
					  swapCalibration.isSwapCurveSelfDiscounting(),
					  staticDataObj.curveProperties_.useTenorBasisSwaps_,
					  mktDataObj.mktData_.areSwapsForwardStarting_,
					  *swapCalibration.swapCurveDiscountFactors_.dfInterpolator_,
					  swapCalibration.swapCurveDiscountFactors_.swapSpotDateDF_,
					  swapCashflows.fixedCoupons_,
					  swapCashflows.tenorSwapCoupons_,
					  swapCashflows.fixedCashflowPaymentDatesAsTerms_,
					  swapCashflows.tenorSwapCashflowPaymentDatesAsTerms_,
					  swapCashflows.floatCashflowPaymentDatesAsTerms_,
					  resultsObj.asOfDate_,
					  liborIndex.daycountEnum_, 
					  swapCalibration.swapCurveDiscountFactors_.swapSpotDateTerm_,
					  IntArray( swaps.size(), swaps.numberOfCompoundPeriodsFloat_[0] ),
					  swapCashflows.floatStartDatesAsTerms_,
					  swapCashflows.floatAccrualPeriods_,
					  swapCashflows.fixingStarts_,
					  swapCashflows.fixingEnds_,
					  swapCashflows.fixingAccrualPeriods_ );

	return swapPVs;
}

// Price Swap Calibration Instruments, where SwapPVs is the result (input/output)
void SwapCalibration::calculateSwapPVs( AQLInterpolationBase &interpolationForForwardsRates,
										const StateVariableEnum& stateVariable,
										DoubleArray& swapPVs, // <--- Result
										const bool isCurveSelfDiscounting,
										const bool useTenorBasisSwaps,
										const bool areSwapsForwardStarting,
										AQLInterpolationBase &interpolationForDiscountFactors,
										const double spotDiscountFactor,
										const DoubleMatrix &fixedCoupons,
										const DoubleMatrix &tenorSwapCoupons,
										const DoubleMatrix &fixedCashflowPaymentDatesAsTerms,
										const DoubleMatrix &tenorSwapCashflowPaymentDatesAsTerms,
										const DoubleMatrix &floatCashflowPaymentDatesAsTerms,
										const AQLDate & asOfDate,
										const DayCountEnum & accrualDaycount,
										const double spotDateAsTerms,
										const IntArray &numberOfCompoundPeriodsFloat,
										const DoubleArray &floatStartDatesAsTerms,
										const DoubleMatrix &floatAccrualPeriods,
										const DoubleMatrix &fixingStarts,
										const DoubleMatrix &fixingEnds,
										const DoubleMatrix &fixingAccrualPeriods )
{
	for (unsigned int i = 0; i < swapPVs.size(); ++i)
	{
		double discountFactor = 1.0;
		double presentValue = 0.0;

		if (!isCurveSelfDiscounting)
		{
			// *** FIXED LEG PV ***
			for (unsigned int j = 0; j < fixedCashflowPaymentDatesAsTerms[i].size(); j++)
			{
				discountFactor = interpolationForDiscountFactors.value(fixedCashflowPaymentDatesAsTerms[i][j] + spotDateAsTerms) / spotDiscountFactor;
				presentValue += fixedCoupons[i][j] * discountFactor;
			}

			// *** FLOAT LEG PV ***
			if (areSwapsForwardStarting)
			{
				if (fixingStarts.size() == 0)
				{
					presentValue -= calcFloatPV(interpolationForForwardsRates, stateVariable, interpolationForDiscountFactors, spotDiscountFactor, spotDateAsTerms, floatCashflowPaymentDatesAsTerms[i], asOfDate, accrualDaycount, SIMPLE_COMPOUNDING, numberOfCompoundPeriodsFloat[i], floatStartDatesAsTerms[i]);
				}
				else
				{
					discountFactor -= calcFloatPV(interpolationForForwardsRates, stateVariable, interpolationForDiscountFactors, spotDiscountFactor, spotDateAsTerms, floatCashflowPaymentDatesAsTerms[i], asOfDate, accrualDaycount, SIMPLE_COMPOUNDING, numberOfCompoundPeriodsFloat[i], floatStartDatesAsTerms[i], floatAccrualPeriods[i], fixingStarts[i], fixingEnds[i], fixingAccrualPeriods[i]);
				}
			}
			else
			{
				if (fixingStarts.size() == 0)
				{
					// Maintaining backward compatibility when no fixing dates are not present
					presentValue -= calcFloatPV(interpolationForForwardsRates, stateVariable, interpolationForDiscountFactors, spotDiscountFactor, spotDateAsTerms, floatCashflowPaymentDatesAsTerms[i], asOfDate, accrualDaycount, SIMPLE_COMPOUNDING, numberOfCompoundPeriodsFloat[i], 0.0);
				}
				else
				{
					presentValue -= calcFloatPV(interpolationForForwardsRates, stateVariable, interpolationForDiscountFactors, spotDiscountFactor, spotDateAsTerms, floatCashflowPaymentDatesAsTerms[i], asOfDate, accrualDaycount, SIMPLE_COMPOUNDING, numberOfCompoundPeriodsFloat[i], 0.0, floatAccrualPeriods[i], fixingStarts[i], fixingEnds[i], fixingAccrualPeriods[i]);
				}
			}
			
			// *** TENOR BASIS FLOAT LEG PV ***
			if (useTenorBasisSwaps)
			{
				for (unsigned int j = 0; j < tenorSwapCashflowPaymentDatesAsTerms[i].size(); j++)
				{
					discountFactor = interpolationForDiscountFactors.value(tenorSwapCashflowPaymentDatesAsTerms[i][j] + spotDateAsTerms) / spotDiscountFactor;
					presentValue += tenorSwapCoupons[i][j] * discountFactor;
				}
			}
		}
		else
		{
			// *** FIXED LEG PV ***
			for (unsigned int j = 0; j < fixedCashflowPaymentDatesAsTerms[i].size(); j++)
			{
				discountFactor = getInterpolatedDiscountfactor(interpolationForForwardsRates, fixedCashflowPaymentDatesAsTerms[i][j], stateVariable, asOfDate, accrualDaycount, SIMPLE_COMPOUNDING );
				presentValue += fixedCoupons[i][j] * discountFactor;
			}

			// *** FLOAT LEG PV ***
			// Note: Using Self-Discounting Formula 
			presentValue += 1.0 * discountFactor;
			presentValue -= 1.0;

			// *** TENOR BASIS FLOAT LEG PV ***
			if (useTenorBasisSwaps)
			{
				for (unsigned int j = 0; j < tenorSwapCashflowPaymentDatesAsTerms[i].size(); j++)
				{
					discountFactor = getInterpolatedDiscountfactor(interpolationForForwardsRates, tenorSwapCashflowPaymentDatesAsTerms[i][j], stateVariable, asOfDate, accrualDaycount, SIMPLE_COMPOUNDING );
					presentValue += tenorSwapCoupons[i][j] * discountFactor;
				}
			}
		}
		swapPVs[i] = presentValue;
	}
}

// Store Calibration Results: State Variables, Jacobian and Inverse Jacobian
void SwapCalibration::storeStateVariableAndJacobianResults( CurveMarketDataHolder & mktDataObj, CalibrationResults & resultsObj )
{
	// *** STORE STATE VARIABLE RESULTS ***
	resultsObj.jacobianStateVariables_.clear();
	for (unsigned int i = 0; i < mktDataObj.mktData_.swap_size_; ++i)
	{
		resultsObj.jacobianStateVariables_.push_back(calibrationResults_.solution[preCalibrationDFSize_ + i]);
	}

	// *** STORE JACOBIAN RESULTS ***
	resultsObj.jacobian_.clear();
	if (calibrationResults_.jacobian.column() == mktDataObj.mktData_.swap_size_)
	{
		for (unsigned int i = 0; i < mktDataObj.mktData_.swap_size_; ++i)
		{
			std::vector<double> dataVec;
			for (unsigned int j = 0; j < mktDataObj.mktData_.swap_size_; ++j)
			{
				dataVec.push_back(calibrationResults_.jacobian.getValue(j, i));
			}
			resultsObj.jacobian_.push_back(dataVec);
		}
	}

	// *** STORE INVERSE JACOBIAN RESULTS ***
	resultsObj.inverseJacobian_.clear();
	for (unsigned int i = 0; i < mktDataObj.mktData_.swap_size_; ++i)
	{
		std::vector<double> thisInverseVector;
		for (unsigned int j = 0; j < mktDataObj.mktData_.swap_size_; ++j)
		{
			thisInverseVector.push_back(calibrationResults_.inverseJacobian.getValue(j, i));
		}
		resultsObj.inverseJacobian_.push_back(thisInverseVector);
	}
}

void SwapCalibration::storeDiscountFactorsAndForwardRates( LACurveStaticDataHolder & staticDataObj,
														   CurveMarketDataHolder & mktDataObj,
														   const LiborIndex & liborIndex,
														   const SwapInstruments & swaps,
														   const SwapCashflows & swapCashflows,
														   CalibrationResults & resultsObj )
{
	// Update and Insert Swap Data Points
	double thisDiscountFactor = 1.0;
	for (unsigned int nthSwap = 0; nthSwap < mktDataObj.mktData_.swap_size_; ++nthSwap)
	{
		// As long as floating leg exists, we use floating leg dates as pillar dates
		if ( !swapCashflows.floatCashflowPaymentDates_.back().empty() )
		{
			for (unsigned int nthFloatCashflow = 0; nthFloatCashflow < swapCashflows.floatCashflowPaymentDatesAsTerms_[nthSwap].size(); ++nthFloatCashflow)
			{
				
				thisDiscountFactor = getInterpolatedDiscountfactor(*resultsObj.interpolationObj_->interpolatorForSwaps_,
																   swapCashflows.floatCashflowPaymentDatesAsTerms_[nthSwap][nthFloatCashflow],
																   staticDataObj.curveProperties_.stateVariableSwaps_,
																   resultsObj.asOfDate_,
																   liborIndex.daycountEnum_,
																   SIMPLE_COMPOUNDING );
				
				insertDFData( resultsObj.dfResults_, thisDiscountFactor, swapCashflows.floatCashflowPaymentDatesAsTerms_[nthSwap][nthFloatCashflow], swapCashflows.floatCashflowPaymentDates_[nthSwap][nthFloatCashflow] );
			}
		}
		else
		{
			for (unsigned int nthFixedCashflow = 0; nthFixedCashflow < swapCashflows.fixedCashflowPaymentDatesAsTerms_[nthSwap].size(); ++nthFixedCashflow)
			{
				
				thisDiscountFactor = getInterpolatedDiscountfactor(*resultsObj.interpolationObj_->interpolatorForSwaps_,
																	swapCashflows.fixedCashflowPaymentDatesAsTerms_[nthSwap][nthFixedCashflow],
																	staticDataObj.curveProperties_.stateVariableSwaps_,
																	resultsObj.asOfDate_,
																	liborIndex.daycountEnum_,
																	SIMPLE_COMPOUNDING );
				
				insertDFData( resultsObj.dfResults_, thisDiscountFactor, swapCashflows.fixedCashflowPaymentDatesAsTerms_[nthSwap][nthFixedCashflow], swapCashflows.fixedCashflowPaymentDates_[nthSwap][nthFixedCashflow] );
			}
		}
	}

	if (!swapCashflows.fixingStarts_.empty())
	{
		// *** FLOAT LEG ***
		insertDFsForFixingStartEnds( resultsObj.dfResults_,
									 *resultsObj.interpolationObj_->interpolatorForSwaps_,
									 staticDataObj.curveProperties_.stateVariableSwaps_,
									 swapCashflows.fixingStarts_.back(),
									 swapCashflows.fixingEnds_.back(),
									 swapCashflows.fixingStartDates_.back(),
									 swapCashflows.fixingEndDates_.back(),
									 staticDataObj.curveProperties_.prioritizeSwaps_,
									 resultsObj.asOfDate_,
									 liborIndex.daycountEnum_ );

		updateImpliedForwardRates(*resultsObj.interpolationObj_->interpolatorForSwaps_,
								  staticDataObj.curveProperties_.stateVariableSwaps_,
								  swapCashflows.fixingStarts_.back(),
								  swapCashflows.fixingEnds_.back(),
								  swapCashflows.fixingAccrualPeriods_.back(),
								  staticDataObj.curveProperties_.prioritizeSwaps_,
								  resultsObj.asOfDate_,
								  liborIndex.daycountEnum_,
								  resultsObj.fwdStartEndDatesAsTerms_,
								  resultsObj.fwdRates_);
	}
	else
	{
		// *** FIXED LEG ***
		const DateVector *cashflowPaymentDates;
		cashflowPaymentDates = &swapCashflows.fixedCashflowPaymentDates_.back();

		DoubleVector fixedLegStartDatesAsTerms;
		DoubleVector fixedLegEndDatesAsTerms;
		DoubleArray fixedLegAccrualPeriods(cashflowPaymentDates->size());
		
		fixedLegAccrualPeriods[0] = liborIndex.daycount_->getTerm(swaps.spotDate_[0], (*cashflowPaymentDates)[0]);
		for (unsigned int nthCashflow = 1; nthCashflow < fixedLegAccrualPeriods.size(); ++nthCashflow)
		{
			fixedLegStartDatesAsTerms.push_back(swapCashflows.fixedCashflowPaymentDatesAsTerms_.back()[nthCashflow - 1]);
			fixedLegEndDatesAsTerms.push_back(swapCashflows.fixedCashflowPaymentDatesAsTerms_.back()[nthCashflow - 1]);
			fixedLegAccrualPeriods[nthCashflow] = liborIndex.daycount_->getTerm((*cashflowPaymentDates)[nthCashflow - 1], (*cashflowPaymentDates)[nthCashflow]);
		}

		fixedLegStartDatesAsTerms.insert(fixedLegStartDatesAsTerms.begin(), 0.0);
		fixedLegEndDatesAsTerms.push_back(swapCashflows.fixedCashflowPaymentDatesAsTerms_.back().back());

		updateImpliedForwardRates(*resultsObj.interpolationObj_->interpolatorForSwaps_,
								  staticDataObj.curveProperties_.stateVariableSwaps_,
								  fixedLegStartDatesAsTerms,
								  fixedLegEndDatesAsTerms,
								  fixedLegAccrualPeriods,
								  staticDataObj.curveProperties_.prioritizeSwaps_,
								  resultsObj.asOfDate_,
								  liborIndex.daycountEnum_,
								  resultsObj.fwdStartEndDatesAsTerms_,
								  resultsObj.fwdRates_);
	}
}

// Constructor
CurveCalibration::CurveCalibration()
{
}

// Destructor
CurveCalibration::~CurveCalibration()
{
}

// Enumerator
bool CurveCalibration::isTypeOf(function_t id) const
{
	return (id == FN_IRYIELDGENERATOR ? true : AQLCoreProcedure::isTypeOf(id));
}

// Clone
AQLCoreFunctionBase* CurveCalibration::clone() const
{
	try
	{
		return new CurveCalibration();
	}
	catch (std::bad_alloc & e)
	{
		throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
	}
}

// Register data used by class
void CurveCalibration::registerData(AQLPriceDataManager& dm) const
{
	dm.setData(CALIBRATION_DATA_ASOFDATE, DATA_DATE);
	dm.setData(CALIBRATION_DATA_TERMS, DATA_DOUBLES);
	dm.setData(CALIBRATION_DATA_CALENDAR, DATA_CALENDAR);
	dm.setData(CALIBRATION_DATA_SLIDINGRULE, DATA_SLIDINGRULE);
	dm.setData(CALIBRATION_DATA_RATE, DATA_DOUBLE);
	dm.setData(CALIBRATION_DATA_INTERPOLATION_JOINDATE_ASDOUBLE, DATA_DOUBLE);

	dm.setData(PRICING_DATA_PRICE, DATA_DOUBLE);
	dm.setData(PRICING_DATA_STARTDATE, DATA_DATE);
	dm.setData(PRICING_DATA_ENDDATE, DATA_DATE);
	dm.setData(PRICING_DATA_SPREAD, DATA_DOUBLE);
	dm.setData(PRICING_DATA_FUTUREVOLATILITY, DATA_DOUBLE);
	dm.setData(PRICING_DATA_RATEPRIORITY, DATA_STRINGS);
	dm.setData(PRICING_DATA_ISFWDSWAP, DATA_BOOL);
	dm.setData(PRICING_DATA_ISDATE, DATA_BOOL);
	dm.setData(PRICING_DATA_STARTTERM, DATA_STRING);
	dm.setData(PRICING_DATA_TENOR, DATA_STRING);

	dm.setData(IR_CALIBRATION_DATA_DFS, DATA_DOUBLES);
	dm.setData(IR_CALIBRATION_DATA_FORWARDRATES, DATA_DOUBLES);
	dm.setData(IR_CALIBRATION_DATA_DATATYPE, DATA_STRING);
	dm.setData(IR_CALIBRATION_DATA_TERM, DATA_STRING);
	dm.setData(IR_CALIBRATION_DATA_FREQUENCY, DATA_STRING);
	dm.setData(IR_CALIBRATION_DATA_SPOTDATE, DATA_DATE);
	dm.setData(IR_CALIBRATION_DATA_DAYCOUNT, DATA_DAYCOUNT);
}

// Enumerator 
function_t CurveCalibration::getType() const
{
	return FN_IRYIELDGENERATOR;
}


/* @brief  Helper function to fetch all the curve names which the global curve engine is configured to build
*
*  @param[in]	curveData	EntityPool properties object
*  @returns	A string vector specifying all of the curve names
*/
AQLStringVector CurveCalibration::getGlobalEngineCurveNames( const CurveCalibrationData& curveData )
{
	// Get the engine name
	AQLString engineName;
	const AQLDataHolder *dh = &curveData.getData( IR_CALIBRATION_DATA_GENTARGETDF, NOCHECK );
	if ( dh->isDefined() && !dh->isNull() )
	{
		engineName = dynamic_cast<const AQLDataString &>(dh->get()).get();
	}

	AQLString suffix_engine = engineName;
	suffix_engine = ( engineName == AQ_STD ) ? "" : "_" + engineName;
	suffix_engine.toUpper();

	AQLStringVector allCurveNames;
	dh = &curveData.getData( IR_CALIBRATION_DATA_GLOBALENGINECURVES_ALLCURVENAMES + suffix_engine, NOCHECK);
	if ( dh->isDefined() && !dh->isNull() )
	{
		AQLString allCurveNamesString = dynamic_cast<const AQLDataString &>(dh->get()).get();
		allCurveNames = allCurveNamesString.toToken( MULTI_STATIC_DATA_DELIMITER );
	}
	return allCurveNames;
}

/*!
@brief  Calibrate yield curves through a yield curve engine
*/
void CurveCalibration::buildEngineCurves(const AQLDate& asOfDate, AQLObject& calibrationDataObject, const AQLDataProcedure& att, bool includeBasisCurves) const
{
	CurveCalibrationData& curveData = dynamic_cast<CurveCalibrationData&>(calibrationDataObject);

	// Get the engine name
	AQLString engineName = "";
	const AQLDataHolder *dh = &curveData.getData(IR_CALIBRATION_DATA_GENTARGETDF, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		engineName = dynamic_cast<const AQLDataString &>(dh->get()).get();
	}

	AQLString suffix_engine = engineName;
	suffix_engine = (engineName == AQ_STD) ? "" : "_" + engineName;
	suffix_engine.toUpper();

	const AQLDataReference& ref = curveData.getYieldData();
	AQLObjectHolder objHolder = ref.get();

	// Fast rebuild?
	bool fastRebuildRequested = true;
	dh = &curveData.getData(IR_CALIBRATION_DATA_GLOBALENGINECURVES_FASTREBUILD + suffix_engine, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		fastRebuildRequested = dynamic_cast<const AQLDataBool &>(dh->get()).get();
	}

	//-------------------------------------------------------------------------
	// Instantiate all the engine curves and add them to the engine

	// Create calibration engine and set all to-be-built curves
	GlobalCurveCalibrationEnginePtr curveEngine(new GlobalCurveCalibrationEngine(curveData, engineName));

	// Get OIS curve
	AQLString oisCurveName = "";
	dh = &curveData.getData(IR_CALIBRATION_DATA_DUALBOOTSTRAP_OISCURVE + suffix_engine, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		oisCurveName = dynamic_cast<const AQLDataString &>(dh->get()).get();
	}

	const AQLStringVector allCurveNames = getGlobalEngineCurveNames( curveData );
	if (allCurveNames.size() == 0)
	{
		AQ_THROW("Yield curve engine has no component curves to build.");
	}

	// Loop through all the curves and create a component curve object for each one of them
	OISComponentCurvePtr oisComponentCurve;
	SwapComponentCurvePtr swapComponentCurve;
	std::vector<BasisComponentCurvePtr> basisComponentCurves;

	for (unsigned int i = 0; i < allCurveNames.size(); ++i)
	{
		AQLString curveName = allCurveNames[i];
		AQLString curveNameUppercase = AQLString(curveName).toUpper();
		AQLString suffix_curve = (curveNameUppercase == AQ_STD) ? "" : "_" + curveNameUppercase;

		AQLString curveType("");
		dh = &curveData.getData(IR_CALIBRATION_DATA_GLOBALENGINECURVES_SINGLECURVETYPE + suffix_engine + suffix_curve, NOCHECK);
		if (dh->isDefined() && !dh->isNull())
		{
			curveType = dynamic_cast<const AQLDataString &>(dh->get()).get();
		}
		else
		{
			AQ_THROW("Curve '" + curveName + "' does not have a curve type.");
		}

		// Depending on the curve type initialise different component curve objects for the engine]
		if (curveType == "OIS")
		{
			oisComponentCurve.reset(new OISComponentCurve(curveName, curveData, asOfDate, fastRebuildRequested));
		}
		else if (curveType == "SWAP")
		{
			swapComponentCurve.reset(new SwapComponentCurve(curveName, curveData, asOfDate, fastRebuildRequested));
		}
		else if (curveType == "TENORBASIS")
		{
			if (includeBasisCurves)
			{
				BasisComponentCurvePtr basisComponentCurve(new BasisComponentCurve(curveName, curveData, asOfDate, fastRebuildRequested));
				basisComponentCurves.push_back(basisComponentCurve);
			}
		}
		else
		{
			AQ_THROW("'" + curveType + "' is not a supported curve type. Currently supports: OIS, SWAP, TENORBASIS.");
		}
	}

	std::shared_ptr<AQLInterpolationBase> oisCurve_stateVarInterp	= oisComponentCurve->getStateVarInterp();
	std::shared_ptr<AQLInterpolationBase> swapCurve_stateVarInterp	= swapComponentCurve->getStateVarInterp();

	// Set OIS curve's dependencies and add to engine
	oisComponentCurve->setLiborInterp(swapCurve_stateVarInterp);
	oisComponentCurve->setDFInterpolator(oisCurve_stateVarInterp);
	curveEngine->addComponentCurve(oisComponentCurve);

	// Let swap curve's dependencies  and add to engine
	swapComponentCurve->setDFInterpolator(oisCurve_stateVarInterp);
	curveEngine->addComponentCurve(swapComponentCurve);

	// Let basis curve's dependencies and add to engine
	if (includeBasisCurves)
	{
		for (unsigned int i = 0; i < basisComponentCurves.size(); ++i)
		{
			BasisComponentCurvePtr basisComponentCurve = basisComponentCurves[i];
			basisComponentCurve->setAgainstLegDFInterpolator(oisCurve_stateVarInterp);
			basisComponentCurve->setTargetLegDFInterpolator(oisCurve_stateVarInterp);
			basisComponentCurve->setTargetLegFwdInterpolator(swapCurve_stateVarInterp);
			basisComponentCurve->setAgainstLegFwdInterpolator(swapCurve_stateVarInterp);
			curveEngine->addComponentCurve(basisComponentCurve);
		}
	}

	curveEngine->calibrate();
	curveEngine->postProcessing(calibrationDataObject);

}

/*!
	@brief  Calibrate swap and OIS curves using dual bootstrapping technique
*/
void CurveCalibration::dualbootstrap(const AQLDate& asOfDate, AQLObject& calibrationDataObject, const AQLDataProcedure& att) const
{
	CurveCalibrationData& curveData = dynamic_cast<CurveCalibrationData&>(calibrationDataObject);

	// Get the engine name
	AQLString engineName = "";
	const AQLDataHolder *dh = &curveData.getData(IR_CALIBRATION_DATA_GENTARGETDF, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		engineName = dynamic_cast<const AQLDataString &>(dh->get()).get();
	}

	AQLString suffix_engine = engineName;
	suffix_engine = (engineName == AQ_STD) ? "" : "_" + engineName;
	suffix_engine.toUpper();

	const AQLDataReference& ref = curveData.getYieldData();
	AQLObjectHolder objHolder = ref.get();

	// Fast rebuild?
	bool fastRebuildRequested = true;
	dh = &curveData.getData(IR_CALIBRATION_DATA_GLOBALENGINECURVES_FASTREBUILD + suffix_engine, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		fastRebuildRequested = dynamic_cast<const AQLDataBool &>(dh->get()).get();
	}

	//-------------------------------------------------------------------------
	// Instantiate all the engine curves and add them to the engine

	// Create calibration engine and set all to-be-built curves
	GlobalCurveCalibrationEnginePtr curveEngine(new GlobalCurveCalibrationEngine(curveData, engineName));

	// Get OIS curve
	AQLString oisCurveName = "";
	dh = &curveData.getData(IR_CALIBRATION_DATA_DUALBOOTSTRAP_OISCURVE + suffix_engine, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		oisCurveName = dynamic_cast<const AQLDataString &>(dh->get()).get();
	}

	oisCurveName.toUpper();
	AQLString suffix_ois = "_" + oisCurveName;

	// Get swap curve
	AQLString swapCurveName = "";
	dh = &curveData.getData(IR_CALIBRATION_DATA_DUALBOOTSTRAP_SWAPCURVE + suffix_engine, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		swapCurveName = dynamic_cast<const AQLDataString &>(dh->get()).get();
	}

	swapCurveName.toUpper();
	AQLString suffix_swap = (swapCurveName == AQ_STD) ? "" : "_" + swapCurveName;

	// Create and initialise individual curve objects
	OISComponentCurvePtr oisComponentCurve(new OISComponentCurve(oisCurveName, curveData, asOfDate, fastRebuildRequested));
	SwapComponentCurvePtr swapComponentCurve(new SwapComponentCurve(swapCurveName, curveData, asOfDate, fastRebuildRequested));

	// For each curve object, set its dependent curves
	std::shared_ptr<AQLInterpolationBase> oisCurve_stateVarInterp = oisComponentCurve->getStateVarInterp();
	std::shared_ptr<AQLInterpolationBase> swapCurve_stateVarInterp = swapComponentCurve->getStateVarInterp();

	oisComponentCurve->setLiborInterp(swapCurve_stateVarInterp);

	oisComponentCurve->setDFInterpolator(oisCurve_stateVarInterp);
	swapComponentCurve->setDFInterpolator(oisCurve_stateVarInterp);

	curveEngine->addComponentCurve(oisComponentCurve);
	curveEngine->addComponentCurve(swapComponentCurve);

	curveEngine->calibrate();
	curveEngine->postProcessing(calibrationDataObject);
}

// ********************************************************************
// Helper Method for the Curve Calibration 'CalibrateModel' Routine

// Get Target Curve and Target Curve Market Data Suffix
void CurveCalibration::getTargetCurveAndMarketDataSuffix( AQLString & targetCurve, AQLString & marketDataSuffix, CurveCalibrationData & curveData ) const
{
	targetCurve				= "";
	marketDataSuffix		= "";

	const AQLDataHolder *dh = &curveData.getData( IR_CALIBRATION_DATA_GENTARGETDF, NOCHECK );
	if( dh->isDefined() && !dh->isNull() )
	{
		targetCurve = dynamic_cast< const AQLDataString & >( dh->get() ).get();
		targetCurve.toUpper();
	}

	if( targetCurve != AQ_STD )
	{
		// Non-STD Swap Curves have a market data suffix
		 marketDataSuffix = AQLString( "_" ) + curveData.getMarketForCurve( targetCurve );
	}
}

// Get Object Pool Target Curve Alias List
TargetCurveAliasList CurveCalibration::getTargetCurveAliasList( LACurveStaticDataHolder & staticDataObj ) const
{
	return dynamic_cast< CurveCalibrationData & >( staticDataObj.curveCalibrationData_ ).getAssignedCurveMktMap();
}

// Get Object Pool List of Curves Built
ListOfBuiltCurves CurveCalibration::getObjectPoolCurveBuildList( LACurveStaticDataHolder & staticDataObj ) const
{
	return dynamic_cast< CurveCalibrationData & >( staticDataObj.curveCalibrationData_ ).getGCurveGenerateMap();
}

// Clear Object Pool List of Curves Built
void CurveCalibration::clearObjectPoolCurveBuildList( LACurveStaticDataHolder & staticDataObj ) const
{
	dynamic_cast< CurveCalibrationData & >( staticDataObj.curveCalibrationData_ ).clearGCurveGenerateMap();
}

// Update Object Pool List of Curves Built
void CurveCalibration::updateObjectPoolCurveBuildListForTargetCurve( LACurveStaticDataHolder & staticDataObj ) const
{
	dynamic_cast< CurveCalibrationData & >( staticDataObj.curveCalibrationData_ ).setGCurveGenerateMap( staticDataObj.targetCurve_ );
}

// Update Object Pool List of Curves Built
void CurveCalibration::updateObjectPoolCurveBuildListForTargetCurve( LACurveStaticDataHolder & staticDataObj, const AQLString & targetCurve ) const
{
	dynamic_cast< CurveCalibrationData & >( staticDataObj.curveCalibrationData_ ).setGCurveGenerateMap( targetCurve );
}

// Update Object Pool List of Curves Built Including Alias Names
void CurveCalibration::updateObjectPoolCurveBuildListForAliasCurves( LACurveStaticDataHolder & staticDataObj ) const
{
	const AQLString targetCurve = staticDataObj.targetCurve_;
	TargetCurveAliasList targetCurveAliasList = staticDataObj.targetCurveAliasList_;

	// Curve Aliases - Update Curve Build list
	for( TargetCurveAliasList::const_iterator it = targetCurveAliasList.begin(); it != targetCurveAliasList.end(); it++ )
	{
		if( it->second == staticDataObj.curveCalibrationData_.getMarketForCurve( targetCurve ) )
		{
			const AQLString& curveName = it->first;
			AQLString suffix_curve = "";
			if( curveName != AQ_STD )
			{
				suffix_curve = "_" + curveName;
			}
			updateObjectPoolCurveBuildListForTargetCurve( staticDataObj, curveName );
		}
	}
}

// Update Object Pool List of Curves Built *** For Target and Alias Curves ***
void CurveCalibration::updateObjectPoolCurveBuildList( LACurveStaticDataHolder & staticDataObj ) const
{
	updateObjectPoolCurveBuildListForTargetCurve( staticDataObj );
	updateObjectPoolCurveBuildListForAliasCurves( staticDataObj );
}

// Helper Method to Update the Object Pool Curve Results
void CurveCalibration::updateObjectPoolCurveResults( LACurveStaticDataHolder & staticDataObj,
													 const CalibrationResults& resultsObj,
													 const AQLString & curveSuffix ) const
{
	// Check Calibration Discount Factor Dates are not in the Past
	DoubleArray::const_iterator min_it = min_element( resultsObj.dfResults_.paymentDatesAsTerms_.begin(), resultsObj.dfResults_.paymentDatesAsTerms_.end() );
	AQ_THROW_IF( !resultsObj.dfResults_.paymentDatesAsTerms_.empty() && *min_it < 0.0, "Invalid calibration instrument dates - Instrument dates are in the past, Instrment Dates and 'Term' parameters must be positive" )

	// Check Calibration Forward Rate Dates are not in the Past
	if( !resultsObj.fwdStartEndDatesAsTerms_.empty() && !resultsObj.fwdStartEndDatesAsTerms_[0].empty() )
	{
		min_it = min_element( resultsObj.fwdStartEndDatesAsTerms_[0].begin(), resultsObj.fwdStartEndDatesAsTerms_[0].end() );
		AQ_THROW_IF( *min_it < 0.0, "Invalid calibration instrument dates - Instrument dates are in the past - Forward dates and 'Fwd_term' parameters must be positive." )
	}

	// Reset and Update Object Pool with New Curve Results
	staticDataObj.curveDataObjectHolder_.remove( CALIBRATION_DATA_ASOFDATE + curveSuffix );
	staticDataObj.curveDataObjectHolder_.remove( CALIBRATION_DATA_TERMS + curveSuffix );
	staticDataObj.curveDataObjectHolder_.remove( CALIBRATION_DATA_FWDTERMSMATRIX + curveSuffix );
	staticDataObj.curveDataObjectHolder_.remove( IR_CALIBRATION_DATA_FORWARDRATES + curveSuffix );
	staticDataObj.curveDataObjectHolder_.remove( IR_CALIBRATION_DATA_DFS + curveSuffix );
	staticDataObj.curveDataObjectHolder_.remove( CALIBRATION_DATA_INTERPOLATION_JOINDATE_ASDOUBLE + curveSuffix );
	staticDataObj.curveDataObjectHolder_.remove( CALIBRATION_DATA_INTERPOLATION_JOINDATE + curveSuffix );
	staticDataObj.curveDataObjectHolder_.remove( IR_CALIBRATION_DATA_JACOBIAN_STATE_VARIABLES + curveSuffix );
	staticDataObj.curveDataObjectHolder_.remove( IR_CALIBRATION_DATA_BUILTBYCURVEENGINE + curveSuffix );

	staticDataObj.curveDataObjectHolder_.add( CALIBRATION_DATA_ASOFDATE + curveSuffix, new AQLDataDate( resultsObj.asOfDate_ ) );
	staticDataObj.curveDataObjectHolder_.add( CALIBRATION_DATA_TERMS + curveSuffix, new AQLDataDoubles( resultsObj.dfResults_.paymentDatesAsTerms_ ) );
	staticDataObj.curveDataObjectHolder_.add( CALIBRATION_DATA_FWDTERMSMATRIX + curveSuffix, new AQLDataDoubleMatrix( resultsObj.fwdStartEndDatesAsTerms_ ) );
	staticDataObj.curveDataObjectHolder_.add( IR_CALIBRATION_DATA_FORWARDRATES + curveSuffix, new AQLDataDoubles( resultsObj.fwdRates_ ) );
	staticDataObj.curveDataObjectHolder_.add( IR_CALIBRATION_DATA_DFS + curveSuffix, new AQLDataDoubles( resultsObj.dfResults_.discountFactors_ ) );
	staticDataObj.curveDataObjectHolder_.add( CALIBRATION_DATA_INTERPOLATION_JOINDATE_ASDOUBLE + curveSuffix, new AQLDataDouble( resultsObj.interpolationObj_->interpolationJoinDateAsDouble_ ) );
	staticDataObj.curveDataObjectHolder_.add( CALIBRATION_DATA_INTERPOLATION_JOINDATE + curveSuffix, new AQLDataDate( resultsObj.interpolationObj_->interpolationJoinDate_ ) );
	staticDataObj.curveDataObjectHolder_.add( IR_CALIBRATION_DATA_JACOBIAN_STATE_VARIABLES + curveSuffix, new AQLDataDoubles( resultsObj.jacobianStateVariables_ ) );
	staticDataObj.curveDataObjectHolder_.add( IR_CALIBRATION_DATA_BUILTBYCURVEENGINE + curveSuffix, new AQLDataBool( false ) );

	// Update Risk Metrics if Available
	if (resultsObj.inverseJacobian_.size() != 0)
	{
		staticDataObj.curveDataObjectHolder_.remove(IR_CALIBRATION_DATA_INVERSE_CURVE_JACOBIAN + curveSuffix);
		staticDataObj.curveDataObjectHolder_.add(IR_CALIBRATION_DATA_INVERSE_CURVE_JACOBIAN + curveSuffix, new AQLDataDoubleMatrix(resultsObj.inverseJacobian_));
	}

	// Update Risk Metrics if Available
	if (resultsObj.jacobian_.size() != 0)
	{
		staticDataObj.curveDataObjectHolder_.remove(IR_CALIBRATION_DATA_CURVE_JACOBIAN + curveSuffix);
		staticDataObj.curveDataObjectHolder_.add(IR_CALIBRATION_DATA_CURVE_JACOBIAN + curveSuffix, new AQLDataDoubleMatrix(resultsObj.jacobian_));
	}
	
	// Update Interpolation Method for Discount Factors if Avaliable
	if ( resultsObj.interpolationObj_->interpolationStringDiscountFactors_ != "" )
	{ 
		staticDataObj.curveDataObjectHolder_.remove(CALIBRATION_DATA_INTERPOLATION + curveSuffix);
		staticDataObj.curveDataObjectHolder_.add(CALIBRATION_DATA_INTERPOLATION + curveSuffix,	new AQLPriceDataInterpolation()).convertFromString(resultsObj.interpolationObj_->interpolationStringDiscountFactors_);
	}

	// Update Interpolation Method for Swaps if Avaliable
	if ( resultsObj.interpolationObj_->interpolationStringSwaps_ != "" )
	{ 
		staticDataObj.curveDataObjectHolder_.remove(IR_CALIBRATION_DATA_INTERPOLATIONYG + curveSuffix);
		staticDataObj.curveDataObjectHolder_.add(IR_CALIBRATION_DATA_INTERPOLATIONYG + curveSuffix, new AQLPriceDataInterpolation()).convertFromString(resultsObj.interpolationObj_->interpolationStringSwaps_);
	}
}

// Update Object Pool Curve Conventions and Results
void CurveCalibration::updateObjectPoolCurveConventionsAndResults( LACurveStaticDataHolder & staticDataObj,
																   CurveMarketDataHolder & mktDataObj,
																   const CalibrationResults& resultsObj ) const
{ 
	// Curve Info
	AQLString targetCurve = staticDataObj.targetCurve_;
	TargetCurveAliasList targetCurveAliasList = staticDataObj.targetCurveAliasList_;

	for( TargetCurveAliasList::const_iterator it = targetCurveAliasList.begin(); it != targetCurveAliasList.end(); it++ )
	{
		AQLString currentMkt		= it->second;
		AQLString mktForCurve	= staticDataObj.curveCalibrationData_.getMarketForCurve( targetCurve );

		if( currentMkt == mktForCurve )
		{
			// Get Curve Name and Suffix (if any)
			const AQLString& curveName = it->first;
			AQLString curveSuffix = ( curveName == AQ_STD ) ? "" : "_" + curveName;

			// Set Object Pool Results
			updateObjectPoolCurveResults( staticDataObj, resultsObj, curveSuffix );

			// LA method that calculates analytical risks, whose resultsObj are unverified
			setCurveConvention( staticDataObj, mktDataObj.rawMarketData_, curveName );
		}
	}
}

// Update Object Pool Curve Conventions, Results and Build List
void CurveCalibration::updateObjectPoolResultsAndBuildList( LACurveStaticDataHolder & staticDataObj,
															CurveMarketDataHolder & mktDataObj,
															const CalibrationResults& resultsObj ) const
{
	updateObjectPoolCurveConventionsAndResults( staticDataObj, mktDataObj, resultsObj );
	updateObjectPoolCurveBuildList( staticDataObj );
}

// Check if Curve Updates are Enabled
bool CurveCalibration::areCurveUpdatesEnabled( LACurveStaticDataHolder & staticDataObj ) const
{
	bool isEnabled = false;
	const AQLDataHolder *dh = &staticDataObj.curveCalibrationData_.getData( IR_CALIBRATION_DATA_ENABLECALCULATION, NOCHECK );
	if( dh->isDefined() && !dh->isNull() )
	{
		isEnabled = dynamic_cast< const AQLDataBool & >( dh->get() ).get();
	}
	return isEnabled;
}

// Helper Method to get the DFCurveName
AQLString CurveCalibration::getDFCurveName( LACurveStaticDataHolder & staticDataObj, const AQLString & curveSuffix ) const
{
	AQLString dfCurveName = ITSELF;
	const AQLDataHolder *dh = &staticDataObj.curveCalibrationData_.getData( IR_CALIBRATION_DATA_DFCURVENAME + curveSuffix, NOCHECK );
	if( dh->isDefined() && !dh->isNull() )
	{
		dfCurveName = dynamic_cast< const AQLDataString & >( dh->get() ).get();
	}
	return dfCurveName;
}

// Check if Curve Exists and Contains Valid Data
bool CurveCalibration::doesValidCurveExist( const AQLString & targetCurve, LACurveStaticDataHolder & staticDataObj ) const
{
	bool doesCurveExist = false;
	const AQLDataHolder *dh = &staticDataObj.curveCalibrationData_.getData( IR_CALIBRATION_DATA_ISCURVEEXISTCHECK, NOCHECK );
	if( dh->isDefined() && !dh->isNull() ) doesCurveExist = dynamic_cast< const AQLDataBool & >( dh->get() ).get();
	bool doCurveResultsExist = staticDataObj.curveCalibrationData_.checkCurveAttr( staticDataObj.curveCalibrationData_.getMarketForCurve( targetCurve ) );
	
	bool result = false;
	if( doesCurveExist && doCurveResultsExist )
	{
		result = true;
	}
	return result;
}

// Get DFCurveName
AQLString CurveCalibration::getDFCurvename( LACurveStaticDataHolder & staticDataObj, const AQLString & targetCurve, const AQLString & curveSuffix ) const
{
	AQLString dfCurveName = ITSELF;
	const AQLDataHolder *dh = &staticDataObj.curveCalibrationData_.getData( IR_CALIBRATION_DATA_DFCURVENAME + curveSuffix, NOCHECK );
	if( dh->isDefined() && !dh->isNull() )
	{
		dfCurveName = dynamic_cast< const AQLDataString & >( dh->get() ).get();
	}
	return dfCurveName;
}

// Virtual Method to Calibrate the Curve Model
void CurveCalibration::calibrateModel( const AQLDate& asOfDate, AQLObject& dataObject, const AQLDataProcedure& laAttributeData ) const
{
	calibrateOISAndSwapCurve( asOfDate, dataObject, laAttributeData, "", "" ); 
}

// Virtual Method to Calibrate the Curve Model for OIS and Swap Curve with additional Curve Collection and Curve Index Info
void CurveCalibration::calibrateOISAndSwapCurve( const AQLDate& asOfDate,
                                                 AQLObject& dataObject,
                                                 const AQLDataProcedure& laAttributeData,
                                                 const AQLString & curveCollection,
                                                 const AQLString & curveIndex ) const
{
	// 1.	Initialize Curve Calibration Data & Curve Properties
	CurveCalibrationData& curveData = dynamic_cast< CurveCalibrationData& >( dataObject );
	CurveProperties curveProperties( curveData );

	// 2.	Initialize Curve Object
	LACurveStaticDataHolder staticDataObj( dataObject, curveData, curveProperties, laAttributeData, curveCollection, curveIndex );
	
	// 3.	Initialize Calibration Interpolators and Results
	CalibrationResults resultsObj( staticDataObj, asOfDate );

	// 4.	Initialize Curve Market Data Holder Object
	CurveMarketDataHolder marketDataObj( staticDataObj, asOfDate );

	// 5.	Main Calibration Routine
	if ( marketDataObj.mktData_.isOISMarketData_ )
	{
		// ***  OIS and ARR CURVE ***
		calibrateOISCurve( staticDataObj, marketDataObj, resultsObj );
	}
	else
	{
		// *** SWAP CURVE ***
		
		// Update State Variables
		staticDataObj.updateSwapCurveStateVariables( marketDataObj );
		
		// Calibrate
		calibrateSwapCurve( staticDataObj, marketDataObj, resultsObj );
	}

	// 6.	Store Curve Conventions and Results for the Target Curve and its Alias Names
	updateObjectPoolResultsAndBuildList( staticDataObj, marketDataObj, resultsObj );
}

// Calibrate Standard Swap Curve - CurveCollection and CurveIndex part of of StaticDataObj
void CurveCalibration::calibrateSwapCurve( LACurveStaticDataHolder & staticDataObj, CurveMarketDataHolder & mktDataObj, CalibrationResults & resultsObj )
{
    // Get Curve Conventions and Market Date from the Curve Results Cache
    // auto curveResults = etrading::getOptionalCurveResultsConventionsAndMarketData( staticDataObj.curveCollection_.c_str(), staticDataObj.curveIndex_.c_str() );

	// 1	Reset Calibration Interpolator Objects & Results
	resultsObj.resetForwardsAndDiscFactors();

	// 2.	Build Instruments
	LiborIndex				liborIndex( mktDataObj );
	FuturesInstruments		futures( staticDataObj, mktDataObj, liborIndex );
	SwapInstruments			swaps( staticDataObj, mktDataObj );
	TenorBasisInstruments	tenorBasisSwaps( staticDataObj, mktDataObj, liborIndex, resultsObj );

	// 3.	Import Interpolation JoinDate Defaults for Mixed/Hybrid Methods
	importHybridInterpolationJoinDateDefaults( staticDataObj, mktDataObj, liborIndex, resultsObj );
	
	// 4.	Calibrate Libor Cash Deposits
	bootstrapLibors( staticDataObj, mktDataObj, liborIndex, resultsObj );

	// 5.	Calibrate FRA Instruments
	bootstrapFRAs( staticDataObj, mktDataObj, liborIndex, resultsObj );
	
	// 6.	Calibrate Futures Instruments
	bootstrapFutures( staticDataObj, mktDataObj, liborIndex, futures, resultsObj );

	// 7.	Initialize Swap and Tenor Basis Cashflows
	SwapCashflows swapCashflows( staticDataObj, mktDataObj, swaps, tenorBasisSwaps, resultsObj );

	// 8.	Calibrate Swap Instruments
	SwapCalibration calibrateSwaps( staticDataObj, mktDataObj, liborIndex, futures, swaps, swapCashflows, resultsObj );

	// 9.	Rebase Discount Factors from SpotDate to AsOfDate
	rebaseDiscountFactorsToCurveAsOfDate( staticDataObj, mktDataObj, liborIndex, swaps, swapCashflows, resultsObj );
}

// Calibrate OIS Swap Curve - CurveCollection and CurveIndex part of of StaticDataObj
void CurveCalibration::calibrateOISCurve( LACurveStaticDataHolder & staticDataObj, CurveMarketDataHolder & mktDataObj, CalibrationResults & resultsObj )
{
	// 1.	Reset Calibration Results
	resultsObj.resetForwardsAndDiscFactors();

	//-------------------------------------------------------------------------------
	// Classify marketData object by marketData type

	std::vector<AQLObject*> data_swap, data_on, data_tn;
	unsigned int size_marketdata = mktDataObj.rawMarketData_.size();
	AQLString datatype_str;

	AQLPriceDataDayCount dc_act365(ACT_365);
	for (unsigned i = 0; i < size_marketdata; i++)
	{
		// check use grid
		const AQLDataHolder *dh = &mktDataObj.rawMarketData_[i]->getData(IR_CALIBRATION_DATA_GRIDUSEFLAG, NOCHECK);
		if (dh->isDefined() && !dh->isNull() && !dynamic_cast<const AQLDataBool &>(dh->get()).get()) continue;

		datatype_str = dynamic_cast<const AQLDataString&> ((mktDataObj.rawMarketData_[i]->getData(IR_CALIBRATION_DATA_DATATYPE, ISNOTNULL)).get()).get();
		datatype_str.toUpper();

		if (datatype_str == AQ_SWAP_DATA)
		{
			const AQLString& term = dynamic_cast<const AQLDataString&> ((mktDataObj.rawMarketData_[i]->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get()).get();
			if (term == AQ_ON)
			{
                // Treat O/N as a regular 1D Swap if reset lag = 0D i.e. if spotDate = asOfDate
                const AQLDate& spotdate_ = dynamic_cast<const AQLDataDate&> ((mktDataObj.rawMarketData_[i]->getData(IR_CALIBRATION_DATA_SPOTDATE, ISNOTNULL)).get());
                if (spotdate_ == resultsObj.asOfDate_ )
                {
                    // data_swap: Treat O/N as a regular OIS Swap
                    data_swap.push_back(mktDataObj.rawMarketData_[i]);
                }
                else
                {
                    // data_on : Append O/N data, not part of swap calibration
                    data_on.push_back(mktDataObj.rawMarketData_[i]);
                }
			}
			else if (term == AQ_TN)
			{
                // Disallow T/N Instruments when Curve SpotLag = 0D i.e. when spotDate = asOfDate
                const AQLDate& spotdate_ = dynamic_cast<const AQLDataDate&> ((mktDataObj.rawMarketData_[i]->getData(IR_CALIBRATION_DATA_SPOTDATE, ISNOTNULL)).get());
                AQ_THROW_IF( spotdate_== resultsObj.asOfDate_, "Invalid Curve Instrument: T/N is not a valid instrument for curves with 0D spot lag" )
                
                // data_tn: Append T/N data, not part of swap calibration    
                data_tn.push_back(mktDataObj.rawMarketData_[i]);
			}
			else
			{
				// swap case
				data_swap.push_back(mktDataObj.rawMarketData_[i]);
			}
		}
	}

	// ascending swap marketData objects
	sort(data_swap.begin(), data_swap.end(), InstrumentComp());
	unsigned int mpc_swap_size = mktDataObj.mktData_.mpc_swap_size_;	// mpc size = monetary policy committee swaps size
	unsigned int swap_size = data_swap.size();							// swap size
	unsigned int size_on = data_on.size();								// on size
	unsigned int size_tn = data_tn.size();								// tn size
	
	AQ_THROW_IF(swap_size == 0, "Invalid market data. At least one outright OIS swap must be specified")

	// Short term swaps overrule central bank swaps?
	bool shortTermSwapOverrules = false;
	const AQLDataHolder *dh = &mktDataObj.rawMarketData_.front()->getData(IR_CALIBRATION_DATA_SHORTTERMSWAPOVERRULES, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		shortTermSwapOverrules = dynamic_cast<const AQLDataBool &>(dh->get()).get();
	}

	//Spot date
	//-------------------------------------------------------------------------------
	AQLDate spotdate;
	size_marketdata = mpc_swap_size + swap_size;
	for (unsigned int i = 0; i < size_marketdata; ++i)
	{
		const AQLObject *thisMarketData = 0;
		if (i < mpc_swap_size)
		{
			thisMarketData = mktDataObj.mktData_.mpc_swaps_[i];
		}
		else
		{
			thisMarketData = data_swap[i - mpc_swap_size];
		}

		const AQLDate& spotdate_ = dynamic_cast<const AQLDataDate&> ((thisMarketData->getData(IR_CALIBRATION_DATA_SPOTDATE, ISNOTNULL)).get());
		if (i == 0)
		{
			spotdate = spotdate_;
		}
		else if (spotdate_ != spotdate)
		{
			AQ_THROW("All market data must have same spotdate")
		}
	}

	//-------------------------------------------------------------------------------
	// Get standard swap conventions

	const AQLPriceDataDayCount& dc = dynamic_cast<const AQLPriceDataDayCount&> ((data_swap[0]->getData(IR_CALIBRATION_DATA_DAYCOUNT, ISNOTNULL)).get());
	const AQLPriceDataCalendar& cal = dynamic_cast<const AQLPriceDataCalendar&> ((data_swap[0]->getData(CALIBRATION_DATA_CALENDAR, ISNOTNULL)).get());
	const AQLPriceDataSlidingRule& sld = dynamic_cast<const AQLPriceDataSlidingRule&> ((data_swap[0]->getData(CALIBRATION_DATA_SLIDINGRULE, ISNOTNULL)).get());
	RateConvention rc = LAMathYieldCurve::setRC(AQ_SIMPLE);
	AQLPriceDataConvention conv(dc.getDayCount(), rc);

	// Get shortterm_date
	// If not given directly as an input, short term date is the LAST central bank swap/future END date. Failing that, it is the spot date
	AQLDate shortterm_date = spotdate;

	dh = &data_swap[0]->getData(IR_CALIBRATION_DATA_SHORTTERMDATE, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		shortterm_date = dynamic_cast<const AQLDataDate&>(dh->get());
		if (mpc_swap_size)
		{
			const AQLDate &mpc_swap_enddate = dynamic_cast<const AQLDataDate&> ((mktDataObj.mktData_.mpc_swaps_.back()->getData(PRICING_DATA_ENDDATE, ISNOTNULL)).get());
			if (shortterm_date > mpc_swap_enddate)
			{
				shortterm_date = mpc_swap_enddate;
			}
		}
	}
	else
	{
		if (mpc_swap_size)
		{
			shortterm_date = dynamic_cast<const AQLDataDate&> ((mktDataObj.mktData_.mpc_swaps_.back()->getData(PRICING_DATA_ENDDATE, ISNOTNULL)).get());
		}
	}

	shortterm_date = sld.getDate(shortterm_date, cal);

	bool isEOMRoll = false;
	dh = &(data_swap[0]->getData(IR_CALIBRATION_DATA_ISEOMROLLOIS, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		isEOMRoll = dynamic_cast<const AQLDataBool &>(dh->get()).get();
	}
	AQLString freq = dynamic_cast<const AQLDataString&> ((data_swap[0]->getData(IR_CALIBRATION_DATA_FREQUENCY, ISNOTNULL)).get()).get();
	freq.toUpper();

	// set roll convention
	AQLString roll_conv("");
	if (freq == AQ_LUNAR)
	{
		roll_conv = AQ_ROLLCONV_LUNAR;
	}
	else if (isEOMRoll)
	{
		roll_conv = AQ_ROLLCONV_EOM;
	}
	else
	{
		roll_conv = AQ_ROLLCONV_NORMAL;
	}

	//-------------------------------------------------------------------------------
	// Group together short end swaps that sit in front of Central Bank Swaps
	
	AQLDate firstCBSStartDate;
	AQLDate lastCBSEndDate;
	if (mpc_swap_size > 0)
	{
		if (shortTermSwapOverrules)
		{
			if (mpc_swap_size > 2)
			{
				firstCBSStartDate = dynamic_cast<const AQLDataDate&> ((mktDataObj.mktData_.mpc_swaps_[1]->getData(PRICING_DATA_STARTDATE, ISNOTNULL)).get());
			}
			else
			{
				firstCBSStartDate = dynamic_cast<const AQLDataDate&> ((mktDataObj.mktData_.mpc_swaps_[0]->getData(PRICING_DATA_STARTDATE, ISNOTNULL)).get());
			}
		}
		else
		{
			firstCBSStartDate = dynamic_cast<const AQLDataDate&> ((mktDataObj.mktData_.mpc_swaps_[0]->getData(PRICING_DATA_STARTDATE, ISNOTNULL)).get());
		}

		lastCBSEndDate = dynamic_cast<const AQLDataDate&> ((mktDataObj.mktData_.mpc_swaps_[mpc_swap_size - 1]->getData(PRICING_DATA_ENDDATE, ISNOTNULL)).get());
	}

	std::vector<AQLObject*> swap_curvefront;
	std::vector<AQLObject*> swap_longend;
	size_t i = 0;
	for (i = 0; i < swap_size; ++i)
	{
		const AQLString& term_str = getMaturityAsTermString(i, data_swap);
		// Unused Variable
		//const AQLPriceDataDayCount* dc = &dynamic_cast<const AQLPriceDataDayCount&> ((data_swap[i]->getData(IR_CALIBRATION_DATA_DAYCOUNT, ISNOTNULL)).get());
		const AQLPriceDataCalendar* cal = &dynamic_cast<const AQLPriceDataCalendar&> ((data_swap[i]->getData(CALIBRATION_DATA_CALENDAR, ISNOTNULL)).get());
		const AQLPriceDataSlidingRule* sld = &dynamic_cast<const AQLPriceDataSlidingRule&> ((data_swap[i]->getData(CALIBRATION_DATA_SLIDINGRULE, ISNOTNULL)).get());
		AQLString freq = dynamic_cast<const AQLDataString&> ((data_swap[i]->getData(IR_CALIBRATION_DATA_FREQUENCY, ISNOTNULL)).get()).get();

		AQLDate maturityDate = LADateHelpers::getDate(spotdate, term_str, *sld, cal, true, &roll_conv);
		double term = dc_act365.getTerm(spotdate, maturityDate);
		AQLDate date1Y = LADateHelpers::getDate(spotdate, "1Y", *sld, cal, true, &roll_conv);
		double term1Y = dc_act365.getTerm(spotdate, date1Y);

		if (mpc_swap_size > 0)
		{
			if (maturityDate < firstCBSStartDate)
			{
				AQLObject* shortEndSwap = data_swap[i];

				dh = &shortEndSwap->getData(IR_CALIBRATION_DATA_COMPOUNDINGMETHOD, NOCHECK);
				if (!dh->isDefined() || !dh->isNull())
				{
					// If short term swaps do not already have a user-specified compounding method, it will be defaulted as specified in the calcEffectiveOISRate method
					AQLString compoundMethod = dynamic_cast<const AQLDataString&> (dh->get());
					shortEndSwap->remove(IR_CALIBRATION_DATA_COMPOUNDINGMETHOD);
					shortEndSwap->add(IR_CALIBRATION_DATA_COMPOUNDINGMETHOD, new AQLDataString()).convertFromString("NONE");
				}

				// When Central Bank Swaps are present in the curve, short term swaps can NOT be built from
				// libor swap + basis. This is consistent with existing behaviour.
				swap_curvefront.push_back(shortEndSwap);
			}
			// SwapDate is greather than firstCBSStartDate, i.e. maturityDate >=  firstCBSStartDate
			else
			{
				// Only add the swap to long end, when there is no overlapped.  When there is overlapped, the Central Bank Swap/FF/ARR Future part takes priority.
				if (maturityDate > lastCBSEndDate)
				{
					swap_longend.push_back(data_swap[i]);
				}

			}
		}
		// mpc_swap_size == 0
		else
		{
			// Short term swaps are shorter than 1Y
			if (term < term1Y)
			{
				// When there is no Central Bank Swaps in the curve, if a swap is built from libor swap + basis, it
				// can not be grouped as a short term swap
				const AQLDataHolder *dh = &data_swap[i]->getData(IR_CALIBRATION_DATA_LONGTERMCONVENTION);
				if (dh->isDefined() && !dh->isNull())
				{
					const OISLongTermInstrumentsEnum longTermConvEnum = toOISLongTermInstrumentsEnum(dynamic_cast<const AQLDataString&> (dh->get()).get().getCString());
					if (longTermConvEnum == LIBOROIS_OIS_LONGTERM_INSTRUMENTS)
					{
						shortterm_date = maturityDate;
						break;
					}
				}

				AQLObject* shortEndSwap = data_swap[i];

				dh = &shortEndSwap->getData(IR_CALIBRATION_DATA_COMPOUNDINGMETHOD, NOCHECK);
				if (!dh->isDefined() || !dh->isNull())
				{
					// If short term swaps do not already have a user-specified compounding method, it will be defaulted as specified in the calcEffectiveOISRate method
					// Arithmetic Instruments Less than 1Y switch to SIMPLE / GEOMETRIC Compounding as a market convention, this is managed in the calcEffectiveOISRate method
					AQLString compoundMethod = dynamic_cast<const AQLDataString&> (dh->get());
					shortEndSwap->remove(IR_CALIBRATION_DATA_COMPOUNDINGMETHOD);
					shortEndSwap->add(IR_CALIBRATION_DATA_COMPOUNDINGMETHOD, new AQLDataString()).convertFromString("NONE");
				}

				swap_curvefront.push_back(shortEndSwap);
			}
			else
			{
				shortterm_date = date1Y;
				break;
			}
		}
	}

	// Group the rest of the swaps to long end
	for (size_t j = i + 1; j < swap_size; ++j)
	{
		swap_longend.push_back(data_swap[j]);
	}

	// This suffix was using the target curve instead of the targetCurveMktSuffix
	AQLString targetSuffix = "_" + staticDataObj.targetCurve_;

	StateVariableEnum stateVariable = STATE_VARIABLE_ZERO_RATE;
	const AQLDataHolder* handle = &(staticDataObj.curveDataObjectHolder_.getData(IR_CALIBRATION_DATA_STATEVARIABLE + targetSuffix, NOCHECK));
	if (handle->isDefined() && !handle->isNull())
	{
		stateVariable = toStateVariableEnum(dynamic_cast<const AQLDataString&>(handle->get()).get().getCString());
		AQ_THROW_IF(stateVariable != STATE_VARIABLE_ZERO_RATE, "For OIS/ARR Curve, interpolator's StateVariable only supports ZeroRate")
	}

	// Set the Join Date for the Linear-Spline Interpolation Method
	// ------------------------------------------------------------

	bool interpolatorForDiscFactors_IsHybrid = false;
	bool interpolatorForSwaps_IsHybrid = false;

	if (resultsObj.interpolationObj_->interpolatorForDiscountFactors_ != NULL)
	{
		interpolatorForDiscFactors_IsHybrid = resultsObj.interpolationObj_->interpolatorForDiscountFactors_->isHybrid();
	}
	if (resultsObj.interpolationObj_->interpolatorForSwaps_ != NULL)
	{
		interpolatorForSwaps_IsHybrid = resultsObj.interpolationObj_->interpolatorForSwaps_->isHybrid();
	}

	// Join Date for LinearInterpolation
	resultsObj.interpolationObj_->interpolationJoinDateAsDouble_ = 0.0;

	// Set interpolation join date
	if (interpolatorForDiscFactors_IsHybrid || interpolatorForSwaps_IsHybrid)
	{
		const AQLDataHolder* handle = &(staticDataObj.curveDataObjectHolder_.getData(IR_CALIBRATION_DATA_INPUT_INTERPOLATIONJOINDATE + targetSuffix, NOCHECK));
		if (handle->isDefined() && !handle->isNull())
		{
			// Use a linear spline join date supplied by user
			AQLDate joinDate = dynamic_cast<const AQLDataDate&>(handle->get()).get();
			resultsObj.interpolationObj_->interpolationJoinDate_ = joinDate;
			resultsObj.interpolationObj_->interpolationJoinDateAsDouble_ = dc_act365.getTerm(spotdate, joinDate);
		}
		else if (mpc_swap_size > 0) // Central Bank section (ECB swaps or futures)
		{
			if (mpc_swap_size > 1)
			{
				// More than one future, default the joinDate to be the second last ECB/ARRfuture's END date
				resultsObj.interpolationObj_->interpolationJoinDate_ = dynamic_cast<const AQLDataDate&> ((mktDataObj.mktData_.mpc_swaps_[mpc_swap_size - 2]->getData(PRICING_DATA_ENDDATE, ISNOTNULL)).get());
			}
			else
			{
				// Only one future, default the joinDate to be the ECB/ARRfuture's START date
				resultsObj.interpolationObj_->interpolationJoinDate_ = dynamic_cast<const AQLDataDate&> ((mktDataObj.mktData_.mpc_swaps_[mpc_swap_size - 1]->getData(PRICING_DATA_STARTDATE, ISNOTNULL)).get());
			}

			resultsObj.interpolationObj_->interpolationJoinDateAsDouble_ = dc_act365.getTerm(spotdate, resultsObj.interpolationObj_->interpolationJoinDate_);
		}

		// Set the Linear-Spline Interpolation Join Date for the Disc Factor Interpolation Table
		if (interpolatorForDiscFactors_IsHybrid)
		{
			resultsObj.interpolationObj_->interpolatorForDiscountFactors_->setJoinDateAsDouble(resultsObj.interpolationObj_->interpolationJoinDateAsDouble_);
		}

		// Set the Linear-Spline Interpolation Join Date for the Swap Rate Interpolation Table
		if (interpolatorForSwaps_IsHybrid)
		{
			resultsObj.interpolationObj_->interpolatorForSwaps_->setJoinDateAsDouble(resultsObj.interpolationObj_->interpolationJoinDateAsDouble_);
		}
	}

	// Start calibrating curve with instruments
	//-------------------------------------------------------------------------------

	// Two containers that will hold the zero rates on key dates and the corresponding 'terms'
	DoubleArray grid(1, 0.0);
	DoubleArray yields(1, 0.0);

	// Calculate DF from swap rates
	DoubleArray terms_grid;
	DoubleArray accrualPeriod;
	DateVector cashflowPaymentDates;

	std::unique_ptr<AQLInterpolationBase> interpolationForSwaps_;
	if (resultsObj.interpolationObj_->interpolatorForSwaps_->getType() == FN_LINEARINTERPOLATION)
	{
		interpolationForSwaps_.reset(new AQLLinearInterpolation(LINEAR_EXTRAPOLATION_TYPE));
	}
	else
	{
		interpolationForSwaps_.reset(dynamic_cast<AQLInterpolationBase *>(resultsObj.interpolationObj_->interpolatorForSwaps_->clone()));
	}

	const double spotterm = dc_act365.getTerm(resultsObj.asOfDate_, spotdate);

	const bool isARRCurve = (getCurveTypeEnum(staticDataObj.curveDataObjectHolder_.get(), staticDataObj.targetCurve_) == ARR_CURVETYPE);

	if (mpc_swap_size == 0)
	{
		// When there are no Central Bank Swaps, simply solve all OIS swaps in one go
		calcOISDFBySwapRates(yields,
							 grid,
							 terms_grid,
							 accrualPeriod,
							 cashflowPaymentDates,
							 resultsObj.interpolationObj_->interpolatorForDiscountFactors_,
							 interpolationForSwaps_.get(),
							 mktDataObj.rawMarketData_,
							 data_swap,
							 staticDataObj,
							 dc_act365,
							 resultsObj.asOfDate_,
							 spotdate,
							 spotdate,
							 &staticDataObj.dfCurveName_,
							 mpc_swap_size,
							 isEOMRoll,
							 true);
	}
	else
	{
		// Bootstrap the short term swaps
		//-------------------------------------------------------------------------------

		if (swap_curvefront.size() > 0)
		{
			calcOISDFBySwapRates(yields,
								 grid,
								 terms_grid,
								 accrualPeriod,
								 cashflowPaymentDates,
								 resultsObj.interpolationObj_->interpolatorForDiscountFactors_,
								 interpolationForSwaps_.get(),
								 mktDataObj.rawMarketData_,
								 swap_curvefront,
								 staticDataObj,
								 dc_act365,
								 resultsObj.asOfDate_,
								 spotdate,
								 shortterm_date,
								 &staticDataObj.dfCurveName_,
								 mpc_swap_size,
								 isEOMRoll,
								 true,
								 swap_curvefront.size());
		}

		// Compute the flat OIS rates used in the Central Bank Swaps section
		//-------------------------------------------------------------------------------

		const AQLString userInputShortTermConvStr = dynamic_cast<const AQLDataString&> ((mktDataObj.mktData_.mpc_swaps_[0]->getData(IR_CALIBRATION_DATA_SHORTTERMCONVENTION, ISNOTNULL)).get()).get().getCString();

		// For CentralBank and FedFund rates
		// The pair contains endDate and rate
		std::map<AQLDate, std::pair<AQLDate, double> > mpcSwapRates;

		// For Future rates
		// The pair contains endDate and rate
		std::map<AQLDate, std::pair<AQLDate, double> > futureRates;

		const AQLPriceDataDayCount& dc = dynamic_cast<const AQLPriceDataDayCount&> ((mktDataObj.mktData_.mpc_swaps_[0]->getData(IR_CALIBRATION_DATA_DAYCOUNT, ISNOTNULL)).get());
		const AQLPriceDataCalendar& cal = dynamic_cast<const AQLPriceDataCalendar&> ((mktDataObj.mktData_.mpc_swaps_[0]->getData(CALIBRATION_DATA_CALENDAR, ISNOTNULL)).get());

		for (unsigned int i = 0; i < mpc_swap_size; ++i)
		{
			const AQLDate& startdate = dynamic_cast<const AQLDataDate&> ((mktDataObj.mktData_.mpc_swaps_[i]->getData(PRICING_DATA_STARTDATE, ISNOTNULL)).get());
			const AQLDate& enddate = dynamic_cast<const AQLDataDate&> ((mktDataObj.mktData_.mpc_swaps_[i]->getData(PRICING_DATA_ENDDATE, ISNOTNULL)).get());
			const double rate = dynamic_cast<const AQLDataDouble&> ((mktDataObj.mktData_.mpc_swaps_[i]->getData(CALIBRATION_DATA_RATE, ISNOTNULL)).get()).get();

			auto instrumentType = getOISMidTermInstrumentsEnum(dynamic_cast<const AQLDataString&> ((mktDataObj.mktData_.mpc_swaps_[i]->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get()).get().getCString());

			OISCompoundingEnum shortTermConv = getDefaultOISCompounding(instrumentType, userInputShortTermConvStr);

			switch (shortTermConv)
			{
			case ARITHMETIC_OIS_COMPOUNDING:
			{
				// for first grid
				if (resultsObj.asOfDate_ >= enddate)
				{
					continue;
				}

				if (startdate < resultsObj.asOfDate_)
				{

					// if the overnight O/N rate exists, then today's ff rate should be the same as O/N rate.
					double todayffrate = 0.0;
					bool istodayrateexist = false;
					if (data_on.size() > 0)
					{
						todayffrate = dynamic_cast<const AQLDataDouble&> ((data_on[0]->getData(CALIBRATION_DATA_RATE, ISNOTNULL)).get()).get();
						istodayrateexist = true;
					}
					double implyrate = getAverageRateFromHistRates(startdate, enddate, dc, cal, rate, mktDataObj.mktData_.mpc_swaps_[i], resultsObj.asOfDate_, istodayrateexist, todayffrate);
					populateOISMidInstrumentRate(mpcSwapRates, futureRates, instrumentType, resultsObj.asOfDate_, enddate, implyrate);
				}
				else
				{
					// Daily Arithmetic Average formula: quotedRate = sumeOf(oisfwdRate_i * tau_i)/tau_T, where tau_T is the yearfraction of the entire tenor, e.g. 1M, 3M, 6M, etc. Reference: formula (3) in "AlgoQuantLib Yield Curves in a Nutshell".
					// Here we assume oisfwdRate is constant, the oisfwdRate is the same as quotedRate, as the sumeOf(tau_i) and tau_T cancel out.

					populateOISMidInstrumentRate(mpcSwapRates, futureRates, instrumentType, startdate, enddate, rate);
				}

				// End Case
				break;
			}
			case GEOMETRIC_OIS_COMPOUNDING:
			{
				// Reference: Geometric Comppunding formula (2) in "AlgoQuantLib Yield Curves in a Nutshell".

				// This function GEOMETRIC compounding between startDate and endDate. The target value is the compounded OIS parRate based on the quote ECB fwd rate, this function solves the fwd rate that produce the same compounded OIS parRate.
				// Here we assume oisfwdRate is constant

				const double onforward = solveOISRate(startdate, enddate, dc, cal, rate);

				populateOISMidInstrumentRate(mpcSwapRates, futureRates, instrumentType, startdate, enddate, onforward);
				// End Case
				break;
			}
			default:
			{
				AQ_THROW("Invalid ShortTermConvention - Must be 'ARITHMETIC' or 'GEOMETRIC'")
			}
			}

			DateVector mpc_swap_dates(2, startdate);
			mpc_swap_dates[1] = enddate;
			if (!mktDataObj.mktData_.mpc_swaps_[i]->getData(IR_CALIBRATION_DATA_CALCDATESFORDVZERO, NOCHECK).isDefined())
			{
				mktDataObj.mktData_.mpc_swaps_[i]->add(IR_CALIBRATION_DATA_CALCDATESFORDVZERO, new AQLDataDates(mpc_swap_dates));
			}
			else
			{
				dynamic_cast<AQLDataDates&>(mktDataObj.mktData_.mpc_swaps_[i]->getData(IR_CALIBRATION_DATA_CALCDATESFORDVZERO).get()).set(mpc_swap_dates);
			}

		}

		//-------------------------------------------------------------------------------
		// Bootstrap the central bank swap section (mid curve)
		const bool hasCentralBank = (mpcSwapRates.size() > 0);
		const bool hasFuture = (futureRates.size() > 0);

		const AQLDate& firstFutureStart = hasFuture ? futureRates.begin()->first : AQLDate();
		const AQLDate& firstCentralBankStart = hasCentralBank ? mpcSwapRates.begin()->first : AQLDate();
        
        //This date is used as the last date for the central bank bootstrap routine
		AQLDate centralBankShortTermDate = shortterm_date;
		if (hasCentralBank)
		{
			const AQLDate& lastKey = mpcSwapRates.rbegin()->first;
			const AQLDate& lastCentralBankSwapEndDate = mpcSwapRates[lastKey].first;
			if (centralBankShortTermDate > lastCentralBankSwapEndDate)
			{
				centralBankShortTermDate = lastCentralBankSwapEndDate;
			}
		}

		AQLDate tmp_date;
		firstCBSStartDate = dynamic_cast<const AQLDataDate&> ((mktDataObj.mktData_.mpc_swaps_[0]->getData(PRICING_DATA_STARTDATE, ISNOTNULL)).get());
		double firstCBSTerm = dc_act365.getTerm(spotdate, firstCBSStartDate);

		if (swap_curvefront.size() != 0)
		{
			// 1.
			// When short term swaps are provided, bootstrap the central bank swaps section from the end of short terms swaps

			// End date of the last short term swap
			const AQLString& term_str = dynamic_cast<const AQLDataString&> ((swap_curvefront.back()->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get());
			AQLDate lastSwapEndDate = LADateHelpers::getDate(spotdate, term_str, sld, &cal, true, &roll_conv);

			double lastYield = yields.back();
			double lastShortSwapTerm = grid.back();

			//There is a gap between short term swap and first ECB swap
			if (lastShortSwapTerm <= firstCBSTerm)
			{
				double lastSwapRate = dynamic_cast<const AQLDataDouble&> ((swap_curvefront.back()->getData(CALIBRATION_DATA_RATE, ISNOTNULL)).get()).get();

				// Linearly interpolate the gap between the end of the last short term swap and the start of the first mid instrument
				insertZeroRatesFromShortTermSwapToMidInstrument(grid, 
																yields, 
																cashflowPaymentDates, 
																lastSwapEndDate, 
																lastSwapRate, 
																mpcSwapRates,
																futureRates,
																spotdate,
																dc_act365, 
																cal, 
																conv);
				
				double initialDF = AQLMath::exp(-yields.back() * grid.back());
				insertZeroRatesForOISMidInstruments(grid,
													yields,
													cashflowPaymentDates,
													spotdate,
													firstCentralBankStart, //fromDate
													centralBankShortTermDate,
													initialDF,
													mpcSwapRates,
													futureRates,
													interpolationForSwaps_,
													dc_act365,
													cal,
													conv,
													isARRCurve);
			}
			else if (lastShortSwapTerm > firstCBSTerm)
			{
				// Overlaps between the last short swap and the first n Central Bank Swaps. Default behaviour is that the Central Bank Swaps overrule

				if (!shortTermSwapOverrules)
				{
					// The first Central Bank Swaps contract rules over the last short term swaps

					DoubleArray thisYieldVector;
					DoubleArray thisGridVector;

					AQ_THROW_IF(yields.size() != grid.size(), "Inconsistent Data: Inconsistent number of dates and rates in the OIS Curve")

					// Remove all the zero rates after the first Central Bank Swaps start date
					double yield = 0.0;
					double term = 0.0;
					for (size_t i = 0; i < yields.size(); ++i)
					{
						yield = yields[i];
						term = grid[i];
						if (term < firstCBSTerm)
						{
							thisYieldVector[i] = yields[i];
							thisGridVector[i] = grid[i];
						}
					}

					yields.clear();
					grid.clear();

					yields = thisYieldVector;
					grid = thisGridVector;

					double initialDF = AQLMath::exp(-yield * term);

					insertZeroRatesForOISMidInstruments(grid,
														yields,
														cashflowPaymentDates,
														spotdate,
														firstCentralBankStart, //fromDate
														centralBankShortTermDate,
														initialDF,
														mpcSwapRates,
														futureRates,
														interpolationForSwaps_,
														dc_act365,
														cal,
														conv,
														isARRCurve);
				}
				else
				{

					double initialDF = AQLMath::exp(-lastYield * lastShortSwapTerm);

					insertZeroRatesForOISMidInstruments(grid,
														yields,
														cashflowPaymentDates,
														spotdate,
														lastSwapEndDate, //fromDate
														centralBankShortTermDate,
														initialDF,
														mpcSwapRates,
														futureRates,
														interpolationForSwaps_,
														dc_act365,
														cal,
														conv,
														isARRCurve,
														lastShortSwapTerm);
				}
			}
		}
		else
		{
			// 2.
		   // When no short term swaps are present, simply bootstrap the Central Bank Swaps section from spot date

		   double initialDF = 1.0;

		   insertZeroRatesForOISMidInstruments(grid,
											   yields,
											   cashflowPaymentDates,
											   spotdate, 
											   spotdate, //fromDate
											   centralBankShortTermDate,
											   initialDF,
											   mpcSwapRates,
											   futureRates,
											   interpolationForSwaps_,
											   dc_act365,
											   cal,
											   conv,
											   isARRCurve);
		}

		//-------------------------------------------------------------------------------
		// Bootstrap the long tenor swaps
		interpolationForSwaps_.reset(dynamic_cast<AQLInterpolationBase *>(resultsObj.interpolationObj_->interpolatorForSwaps_->clone()));
		if (swap_longend.size() != 0)
		{
			calcOISDFBySwapRates(yields,
								 grid,
								 terms_grid,
								 accrualPeriod,
								 cashflowPaymentDates,
								 resultsObj.interpolationObj_->interpolatorForDiscountFactors_,
								 interpolationForSwaps_.get(),
								 mktDataObj.rawMarketData_,
								 swap_longend,
								 staticDataObj,
								 dc_act365,
								 resultsObj.asOfDate_,
								 spotdate,
								 shortterm_date,
								 &staticDataObj.dfCurveName_,
								 mpc_swap_size,
								 isEOMRoll,
								 false,
								 swap_curvefront.size());
		}
		else
		{
			interpolationForSwaps_->set(grid, yields);
		}
	}

	AQ_THROW_IF(grid.size() < 3, "Curve calibration instrument error. There are no calibration instruments specified")

	// Use a map to improve performance when populating yields for terms of terms_grid, in function - getOISdiscountFactor()
	std::unordered_map<double, double> gridYieldMap;
	for (size_t i = 0; i < grid.size(); ++i)
	{
		gridYieldMap[grid[i]] = yields[i];
	}

	// grid is the payment date year fractions of the the pillar point instruments
	// terms_grid is the payment date year fractions of the longest maturity swap/instrument (e.g. all payment dates of the 50Y swap)
	// *** this merge allows more points to be inserted into the discount factor tables, by interpolating yields (zero rates), so that the swaps can be repriced. */
	grid.insert(grid.begin(), terms_grid.begin(), terms_grid.end());

	sort(grid.begin(), grid.end());
	removeDuplicatedTerms(grid);

	//-------------------------------------------------------------------------------
	// Append ON and TN rates to the front if provided, when spotDate > AsOfDate
	// When spotDate = AsOfDate the ON and TN have to be included in the OIS Swap Calibration so all Swaps can incorporate this data and reprice
    // Convert zero rates to dfs

	if (spotdate > resultsObj.asOfDate_ )
	{
		if (size_on > 0 || size_tn > 0)
		{
           AQLDate thisEndDate;
			double rate_on = 0.0;
			if (size_on > 0)
			{
				// O/N Instrument: from asOfDate to tomorrow
				rate_on = dynamic_cast<const AQLDataDouble&> ((data_on[0]->getData(CALIBRATION_DATA_RATE, ISNOTNULL)).get()).get();
				const AQLPriceDataDayCount& dc_on = dynamic_cast<const AQLPriceDataDayCount&> ((data_on[0]->getData(IR_CALIBRATION_DATA_DAYCOUNT, ISNOTNULL)).get());
				RateConvention rc_on = LAMathYieldCurve::setRC(AQ_SIMPLE);
				AQLPriceDataConvention conv_on(dc_on.getDayCount(), rc_on);

				thisEndDate = cal.getBusinessDay(resultsObj.asOfDate_, 1);
				double termON = dc_act365.getTerm(resultsObj.asOfDate_, thisEndDate);
				
                // Add ON data to results object
                resultsObj.dfResults_.discountFactors_.push_back(conv.getDF(rate_on, resultsObj.asOfDate_, thisEndDate));
                resultsObj.dfResults_.paymentDatesAsTerms_.push_back(termON);
			}

			double rate_tn = 0.0;
			if (size_tn > 0 && thisEndDate < spotdate)
			{
				// T/N Instrument: from tomorrow to a day after
				rate_tn = dynamic_cast<const AQLDataDouble&> ((data_tn[0]->getData(CALIBRATION_DATA_RATE, ISNOTNULL)).get()).get();
				const AQLPriceDataDayCount& dc_tn = dynamic_cast<const AQLPriceDataDayCount&> ((data_tn[0]->getData(IR_CALIBRATION_DATA_DAYCOUNT, ISNOTNULL)).get());
				RateConvention rc_tn = LAMathYieldCurve::setRC(AQ_SIMPLE);
				AQLPriceDataConvention conv_tn(dc_tn.getDayCount(), rc_tn);

				AQLDate on_EndDate;
				double on_df = 1.0;
				if (size_on > 0)
				{
					on_EndDate = thisEndDate;
					on_df = resultsObj.dfResults_.discountFactors_.back();
				}
				else
				{
					on_EndDate = resultsObj.asOfDate_;
				}

				thisEndDate = cal.getBusinessDay(on_EndDate, 1);
                double termTN = dc_act365.getTerm(resultsObj.asOfDate_, thisEndDate);

                // Add TN data to results object
				resultsObj.dfResults_.discountFactors_.push_back(on_df * conv.getDF(rate_tn, on_EndDate, thisEndDate));
				resultsObj.dfResults_.paymentDatesAsTerms_.push_back(termTN);
			}

			double shortTermRate;
			if (size_tn > 0 && thisEndDate < spotdate)
			{
				// If TN is provided, use the TN rate for the rest of the spot period
				shortTermRate = rate_tn;
			}
			else
			{
				if (size_on > 0)
				{
					// If TN is not provided but ON is provided, use ON rate for the rest of the spot period
					shortTermRate = rate_on;
				}
			}

			// Go through the rest of the spot period
			AQLDate lastDate = thisEndDate;
			thisEndDate = cal.getBusinessDay(thisEndDate, 1);
			while (thisEndDate <= spotdate)
			{
				double thisTerm = dc_act365.getTerm(resultsObj.asOfDate_, thisEndDate);
				resultsObj.dfResults_.paymentDatesAsTerms_.push_back(thisTerm);
				resultsObj.dfResults_.discountFactors_.push_back(resultsObj.dfResults_.discountFactors_.back() * conv.getDF(shortTermRate, lastDate, thisEndDate));
				lastDate = thisEndDate;
				thisEndDate = cal.getBusinessDay(thisEndDate, 1);
			}

            // Update Results Object with Grid Results and Adjusting by SpotDF
			const double spotdf = resultsObj.dfResults_.discountFactors_.back();
			for (unsigned int i = 1; i < grid.size(); ++i)
			{
                resultsObj.dfResults_.paymentDatesAsTerms_.push_back(spotterm + grid[i]);
				resultsObj.dfResults_.discountFactors_.push_back(getOISdiscountFactor(grid[i], gridYieldMap, interpolationForSwaps_) * spotdf);
			}
		}
		else
		{
			const double spotyield = yields[1];
			// set business date before spotdate
			AQLDate businessDateBeforeSpotDate = cal.getBusinessDay(resultsObj.asOfDate_, 1);
			while (businessDateBeforeSpotDate < spotdate)
			{
				const double thisTerm = dc_act365.getTerm(resultsObj.asOfDate_, businessDateBeforeSpotDate);
				resultsObj.dfResults_.paymentDatesAsTerms_.push_back(thisTerm);
				resultsObj.dfResults_.discountFactors_.push_back(AQLMath::exp(-spotyield * thisTerm));
				businessDateBeforeSpotDate = cal.getBusinessDay(businessDateBeforeSpotDate, 1);
			}

			resultsObj.dfResults_.paymentDatesAsTerms_.push_back(spotterm);
			const double spotdf = AQLMath::exp(-spotyield * spotterm);
			resultsObj.dfResults_.discountFactors_.push_back(spotdf);
			
            for (unsigned int i = 1; i < grid.size(); ++i)
			{
				resultsObj.dfResults_.paymentDatesAsTerms_.push_back(spotterm + grid[i]);
				resultsObj.dfResults_.discountFactors_.push_back(getOISdiscountFactor(grid[i], gridYieldMap, interpolationForSwaps_) * spotdf);
			}
		}
	}
    else
	{
        // When SpotDate == AsOfDate
        // -------------------------
         
        for (unsigned int i = 1; i < grid.size(); ++i)
		{

            resultsObj.dfResults_.paymentDatesAsTerms_.push_back(grid[i]);
			resultsObj.dfResults_.discountFactors_.push_back(getOISdiscountFactor(grid[i], gridYieldMap, interpolationForSwaps_));
		}

	}

	if (interpolatorForSwaps_IsHybrid)
	{
		resultsObj.interpolationObj_->interpolationJoinDateAsDouble_ += spotterm;
	}

	//add term =0 and df = 1.0
	resultsObj.dfResults_.paymentDatesAsTerms_.insert(resultsObj.dfResults_.paymentDatesAsTerms_.begin(), 0.0);
	resultsObj.dfResults_.discountFactors_.insert(resultsObj.dfResults_.discountFactors_.begin(), 1.0);

	// insert extrapolation terms
	dh = &staticDataObj.curveDataObjectHolder_.getData(IR_CALIBRATION_DATA_MAXTERM);
	if (dh->isDefined() && !dh->isNull())
	{
		const AQLString maxTerm = dynamic_cast<const AQLDataString&>(dh->get()).get() + "Y";
		const AQLDate& maxDate = LADateHelpers::getDate(spotdate, maxTerm, AQLPriceDataSlidingRule(SLIDING_RULE_NO_CHANGE), NULL, true, nullptr);
		if (maxDate > cashflowPaymentDates.back())
		{
			const AQLString maxFreq = dynamic_cast<const AQLDataString&>(staticDataObj.curveDataObjectHolder_.getData(IR_CALIBRATION_DATA_MAXTERMFREQ).get()).get();
			DoubleArray extra_terms;
			DateVector tmp_dates; DoubleArray tmp_taus;
			updateAccrualPeriodsAndPaymentDates(spotdate, maxDate, maxFreq, cal, sld, dc, tmp_dates, extra_terms, tmp_taus, isEOMRoll);
			for (unsigned int i = 0; i < extra_terms.size(); ++i)
			{
				const double thisTerm = extra_terms[i] + spotterm;
				if (thisTerm <= resultsObj.dfResults_.paymentDatesAsTerms_.back())
					continue;
				resultsObj.dfResults_.paymentDatesAsTerms_.push_back(thisTerm);
				resultsObj.dfResults_.discountFactors_.push_back(resultsObj.dfResults_.discountFactors_.back());
			}
		}
	}

	/* This allows ARR Curve to have a forward table
	if ( isARRCurve )
	{
		fwdStartEndDatesAsTerms.clear();
		fwdStartEndDatesAsTerms.resize(2);
		fwdRates.clear();

		populateARRCurveForwardRateTable(*interpolationForSwaps_, asOfDate, terms, dfs, cal, dc_act365, dc, fwdStartEndDatesAsTerms, fwdRates);
	}
	*/
}

/*
	@brief solve ois rate by newton raphson

	@param[in] startdate
	@param[in] enddate
	@param[in] dc          daycount
	@param[in] cal         calendar
	@param[in] market_rate targetrate

	@return oisrate
*/
double CurveCalibration::solveOISRateS(const AQLDate& startdate,
									   const AQLDate& enddate,
									   const AQLPriceDataDayCount& dc,
									   const AQLPriceDataCalendar& cal,
									   double market_rate,
									   const std::map<AQLDate, double> &onforward_map)
{
	if (onforward_map.empty() || onforward_map.begin()->first > enddate)
	{
		return solveOISRate(startdate, enddate, dc, cal, market_rate);
	}
	// calc target market rate
	RateConvention rc = LAMathYieldCurve::setRC(AQ_SIMPLE);
	AQLPriceDataConvention conv(dc.getDayCount(), rc);
	double val = 1.0;
	std::map<AQLDate, double>::const_iterator it = onforward_map.begin();
	AQLDate thisFwdDate = it->first;
	while (thisFwdDate <= enddate)
	{
		std::map<AQLDate, double>::const_iterator n_it = it;
		++n_it;
		if (thisFwdDate >= n_it->first)
		{
			++it;
		}

		const double onforward_rate = it->second;
		AQLDate n_date = cal.getBusinessDay(thisFwdDate, 1);
		const double term_d = dc.getTerm(thisFwdDate, n_date);
		val *= AQLPriceDataConvention::rateToRet(onforward_rate, term_d, conv);
		thisFwdDate = n_date;
	}
	const double term = dc.getTerm(startdate, cal.getBusinessDay(enddate, 1));

	AQLDate enddate_ = onforward_map.begin()->first;
	const double term_s = dc.getTerm(startdate, enddate_);
	const double target_rate = ((1.0 + term * market_rate) / val - 1.0) / term_s;

	enddate_ = cal.getBusinessDay(enddate_, -1);
	return solveOISRate(startdate, enddate_, dc, cal, target_rate);
}

/*
	@brief solve ois rate by newton raphson

	@param[in] startdate
	@param[in] enddate
	@param[in] dc          daycount
	@param[in] cal         calendar
	@param[in] market_rate targetrate

	@return oisrate
*/
double
CurveCalibration::solveOISRate(const AQLDate& startdate,
	const AQLDate& enddate,
	const AQLPriceDataDayCount& dc,
	const AQLPriceDataCalendar& cal,
	double market_rate)

{
	const double EPS_PV = 1.0E-8;
	const int MAX_LOOP = 10000;

	double rate0 = market_rate;
	double val0 = market_rate - calcSettleRate(startdate, enddate, dc, cal, rate0);
	double rate1 = rate0 + 0.01;
	double val1 = market_rate - calcSettleRate(startdate, enddate, dc, cal, rate1);
	// newton raphson
	int loopNum = MAX_LOOP;
	try
	{
		while (loopNum--)
		{
			if (AQLMath::abs(val0 - val1) < EPS_PV)
			{
				break;
			}
			double dval = (val1 - val0) / (rate1 - rate0);
			double rate2 = rate1 - val1 / dval;
			rate0 = rate1;
			rate1 = rate2;
			val0 = val1;
			val1 = market_rate - calcSettleRate(startdate, enddate, dc, cal, rate1);
		}
	}
	catch (AQLCoreError &e)
	{
		AQ_THROW("OIS curve calibration solver unable to find a solution with the existing precision and number of iteration settings - " + std::string( e.getMsg() ) )
	}

	AQ_THROW_IF(loopNum < 0, "OIS curve calibration solver unable to find a solution with the existing precision and number of iterations settings")
	return rate1;
}

/*
	@brief calc settle rate

	@param[in] startdate
	@param[in] enddate
	@param[in] dc          daycount
	@param[in] cal         calendar
	@param[in] onforwardrate

	@return settlerate
*/
double CurveCalibration::calcSettleRate(const AQLDate& startdate,
									    const AQLDate& enddate,
									    const AQLPriceDataDayCount& dc,
									    const AQLPriceDataCalendar& cal,
									    double onforward_rate)
{
	RateConvention rc = LAMathYieldCurve::setRC(AQ_SIMPLE);
	AQLPriceDataConvention conv(dc.getDayCount(), rc);
	AQLDate thisFwdDate = startdate;
	AQLPriceDataSlidingRule sld(SLIDING_RULE_FOLLOWING);
	thisFwdDate = sld.getDate(thisFwdDate, cal);
	AQLDate adj_startdate = thisFwdDate;
	double val = 1.0;
	AQLDate n_date = thisFwdDate;
	while (thisFwdDate <= enddate)
	{
		n_date = cal.getBusinessDay(thisFwdDate, 1);
		const double term = dc.getTerm(thisFwdDate, n_date);
		val *= AQLPriceDataConvention::rateToRet(onforward_rate, term, conv);
		thisFwdDate = n_date;
	}
	val -= 1.0;
	return val / dc.getTerm(adj_startdate, n_date);
}

/*
	@brief calc settle rate

	@param[in] startdates
	@param[in] enddates
	@param[in] onforward_rates
	@param[in] dc          daycount
	@param[in] cal         calendar

	@return settlerate
*/
double CurveCalibration::calcSettleRates(const DateVector& startdates,
										const DateVector& enddates,
										const DoubleVector& onforward_rates,
										const AQLPriceDataDayCount& dc)
{
	unsigned int size = startdates.size();
	if (!size)
	{
		return 0.0;
	}
	AQ_THROW_IF(size != enddates.size() || size != onforward_rates.size(), "Unable to calculate settle rates. Inconsistent number of dates and rates specified")

	RateConvention rc = LAMathYieldCurve::setRC(AQ_SIMPLE);
	AQLPriceDataConvention conv(dc.getDayCount(), rc);
	double val = 1.0;
	for (unsigned int i = 0; i < size; ++i)
	{
		const double term = dc.getTerm(startdates[i], enddates[i]);
		val *= AQLPriceDataConvention::rateToRet(onforward_rates[i], term, conv);
	}
	val -= 1.0;
	return val / dc.getTerm(startdates[0], enddates.back());
}


/*
	@brief calculate the effective forward rate of an OIS swap over a certain period

	@param[out] startterms			        The day count fractions to start date of each daily term
	@param[out] endterms			        The day count fractions to end date of each daily term
	@param[in] swapAveragingMethodEnum		The chosen averaging scheme
	@param[in] longTermConvEnum			    Convention of the long term part of OIS curve
	@param[in] longTermGen			        Generating method of the long term part of OIS curve
	@param[in] spotDate				        Spot date of the curve
	@param[in] startDate			        Start date of the averaging period
	@param[in] endDate				        End date of the averaging period
	@param[in] dc					        Day count convention between the daily dates
	@param[in] calc					        Calendar used on the daily dates
	@param[in] interpolateByZeroRate		Interpolator object holding zero rates
	@param[in] accrualPeriod		        Date count fraction of the current period
	@param[in] isLogDF				        Is state variable logDF?

	@return settlerate
*/
double CurveCalibration::calcEffectiveOISRate(DoubleVector& startTerms,
											  DoubleVector& endTerms,
											  const OISCompoundingEnum& swapAveragingMethodEnum,
											  const OISLongTermInstrumentsEnum& longTermConvEnum,
											  const AQLString& longTermGen,
											  const AQLDate& spotDate,
											  const AQLDate& startDate,
											  const AQLDate& endDate,
											  const AQLPriceDataDayCount* dc,
											  const AQLPriceDataCalendar* cal,
											  const AQLInterpolationBase* interpolateByZeroRate,
											  double accrualPeriod,
											  bool isLogDF)
{
	AQLPriceDataDayCount dc_act365(ACT_365);
	double settlerate(0.0);

	switch (longTermConvEnum)
	{
	case NONE_OIS_LONGTERM_INSTRUMENTS:
	{
		//
		// 1. Outright OIS Instruments Only
		// The compounding method is also applied to OIS Outright Instruments
		// -----------------------------------------------------------------------------------------------------
		//

		if (swapAveragingMethodEnum == NONE_OIS_COMPOUNDING)
		{
			// If no averaging methodology is specified, use the formula used in LIBOR swaps to get effective forward rate
			const double startterm = dc_act365.getTerm(spotDate, startDate);
			const double endterm = dc_act365.getTerm(spotDate, endDate);
			if (isLogDF)
			{
				// State Variable: Log Discount Factor, which is equivalent to zero rate times time.
				// DF = exp( - LogDF ) = exp( - zeroRate x time )
				// ForwardRate = exp[ DF(End) - DF(Start) ) ] - 1 / tau
				// ForwardRate = exp( logDF(Start) - logDF(End) - 1 / tau
				settlerate = (AQLMath::exp(interpolateByZeroRate->value(endterm) - interpolateByZeroRate->value(startterm)) - 1.0) / accrualPeriod;
			}
			else
			{
				// State Variable: Zero Rate
				// DF = exp( -ZeroRate.tau )
				// ForwardRate = exp[ DF(End) - DF(Start) ) ] - 1 / tau
				settlerate = (AQLMath::exp(interpolateByZeroRate->value(endterm) * endterm - interpolateByZeroRate->value(startterm) * startterm) - 1.0) / accrualPeriod;
			}
		}
		else if (swapAveragingMethodEnum == ARITHMETIC_OIS_COMPOUNDING)
		{
			// TODO: Arithmetic Instruments Less than 1Y switch to SIMPLE / GEOMETRIC Compounding as a market convention
			//const bool isTenorLessThanOneYear = ( dc_act365.getTerm(spotDate, endDate) < 1.0 );
			//const AQLString compoundMethod = toString( isTenorLessThanOneYear ? GEOMETRIC_OIS_COMPOUNDING : ARITHMETIC_OIS_COMPOUNDING ).c_str();
			const AQLString compoundMethod = toString(ARITHMETIC_OIS_COMPOUNDING).c_str();
			settlerate = calcAverageRate(startTerms, endTerms, swapAveragingMethodEnum, spotDate, startDate, endDate, dc, cal, interpolateByZeroRate, accrualPeriod, isLogDF);

		}
		else if (swapAveragingMethodEnum == GEOMETRIC_OIS_COMPOUNDING)
		{
			const AQLString compoundMethod = toString(GEOMETRIC_OIS_COMPOUNDING).c_str();
			settlerate = calcAverageRate(startTerms, endTerms, swapAveragingMethodEnum, spotDate, startDate, endDate, dc, cal, interpolateByZeroRate, accrualPeriod, isLogDF);
		}
		else
		{
			AQ_THROW("Invalid OIS Compounding Method - Must be ARITHMETIC, GEOMETRIC, or NONE")
		}

		// End of Case
		break;
	}
	case LIBOROIS_OIS_LONGTERM_INSTRUMENTS:
	{
		//
		// 2. Libor-OIS Instruments
		// When longTermConv is LOBASIS, it means this is the part of the OIS curve that uses LOBasis spread + LIBOR swap
		// -----------------------------------------------------------------------------------------------------------------------
		//

		// Note: DailyAveraging must be specified with the SwapAveraging / CompoundMethod set to Arithmetic or Geometric otherwise we apply Simple Averaging i.e. No Averaging
		if (longTermGen == "DAILYAVERAGING" && (swapAveragingMethodEnum == ARITHMETIC_OIS_COMPOUNDING || swapAveragingMethodEnum == GEOMETRIC_OIS_COMPOUNDING))
		{
			settlerate = calcAverageRate(startTerms,
										 endTerms,
										 swapAveragingMethodEnum,
										 spotDate,
										 startDate,
										 endDate,
										 dc,
										 cal,
										 interpolateByZeroRate,
										 accrualPeriod,
										 isLogDF);
		}
		else if (swapAveragingMethodEnum == NONE_OIS_COMPOUNDING || longTermGen.size() == 0 || longTermGen == "NONE" || longTermGen == "")
		{
			// When daily averaging is not employed, use the standard formula used in LIBOR swaps to find effective forward rate
			const double startterm = dc_act365.getTerm(spotDate, startDate);
			const double endterm = dc_act365.getTerm(spotDate, endDate);
			if (isLogDF)
			{
				// State Variable: Log Discount Factor, which is equivalent to zero rate times time.
				// DF = exp( - LogDF ) = exp( - zeroRate x time )
				// ForwardRate = exp[ DF(End) - DF(Start) ) ] - 1 / tau
				// ForwardRate = exp( logDF(Start) - logDF(End) - 1 / tau
				settlerate = (AQLMath::exp(interpolateByZeroRate->value(endterm) - interpolateByZeroRate->value(startterm)) - 1.0) / accrualPeriod;
			}
			else
			{
				// State Variable: Zero Rate
				// DF = exp( -ZeroRate.tau )
				// ForwardRate = exp[ DF(End) - DF(Start) ) ] - 1 / tau
				settlerate = (AQLMath::exp(interpolateByZeroRate->value(endterm) * endterm - interpolateByZeroRate->value(startterm) * startterm) - 1.0) / accrualPeriod;
			}
		}
		else
		{
			AQ_THROW("Invalid LongTerm.GenerateMethod - Must be set to 'DAILYAVERAGING' for compounded rates or 'NONE' for simple compounding")
		}

		// End of Case
		break;
	}
	default:
	{
		AQ_THROW("Invalid LongTermConvention; Must be either 'LOBASIS' to calibrate using Libor-OIS Basis Instruments or 'NONE' to use OIS Outright instruments only")
	}
	}

	// Return the result
	return settlerate;
}


/*
	@brief calculate average rate based on chosen averaging scheme.
			Currently only supports arithmetic and geometric averaging.

	@param[out] startterms			        The day count fractions to start date of each daily term
	@param[out] endterms			        The day count fractions to end date of each daily term
	@param[in] averagingMethodEnum		    The chosen averaging scheme
	@param[in] spotDate				        Spot date of the curve
	@param[in] startDate			        Start date of the averaging period
	@param[in] endDate				        End date of the averaging period
	@param[in] dc					        Day count convention between the daily dates
	@param[in] calc					        Calendar used on the daily dates
	@param[in] interpolateByZeroRate		Interpolator object holding zero rates
	@param[in] accrualPeriod		        Date count fraction of the current period
	@param[in] isLogDF				        Is state variable logDF?

	@return settlerate
*/
double CurveCalibration::calcAverageRate(DoubleVector& startTerms,
										 DoubleVector& endTerms,
										 const OISCompoundingEnum& averagingMethodEnum,
										 const AQLDate& spotDate,
										 const AQLDate& startDate,
										 const AQLDate& endDate,
										 const AQLPriceDataDayCount* dc,
										 const AQLPriceDataCalendar* cal,
										 const AQLInterpolationBase* interpolateByZeroRate,
										 double accrualPeriod,
										 bool isLogDF)
{
	AQLPriceDataDayCount dc_act365(ACT_365);

	// Find the daily date schedule of the current period
	if (startTerms.size() == 0 || endTerms.size() == 0)
	{
		AQLDate tmp_date = startDate;
		while (tmp_date < endDate)
		{
			const AQLDate nextdate = cal->getBusinessDay(tmp_date, 1);

			// TODO: The daycount structure here looks incorrect - Think two sets of terms are required ???
			// 1. The internal daycount for discount factor lookup should be ACT/365
			// 2. The coupon year fractions should be in their natural instrument daycount, typically ACT/360
			startTerms.push_back(dc_act365.getTerm(spotDate, tmp_date));
			endTerms.push_back(dc_act365.getTerm(spotDate, nextdate));

			tmp_date = nextdate;
		}
	}

	double effectiveRate;

	switch (averagingMethodEnum)
	{
	case GEOMETRIC_OIS_COMPOUNDING:
	{
		//
		// GEOMETRIC AVERAGING
		//

		// Daily Geometric Compounding, which is the default behaviour
		RateConvention rc = LAMathYieldCurve::setRC(AQ_SIMPLE);
		AQLPriceDataConvention conv(dc->getDayCount(), rc);

		/* We make use of the geometric effective-rate shortcut:
		 * The product of daily returns may be replaced by the ratio of start / end return.
		 * See "MLIB Yield Curves in a Nutshell" document for the explicit OIS effective rate
		*/

		double val = 1.0;

		const double initialStartTerm = startTerms[0];
		const double finalEndTerm = endTerms[endTerms.size() - 1];

		if (isLogDF)
		{
			// State Variable: Log Discount Factor
			// ForwardRate = [ DF(Start) / DF(End) - 1 ] / tau
			const double initialLogDF = interpolateByZeroRate->value(initialStartTerm);
			const double finalLogDF = interpolateByZeroRate->value(finalEndTerm);

			val = AQLMath::exp(finalLogDF - initialLogDF);
		}
		else
		{
			// State Variable: Zero Rate
			// DiscountFactor = exp(-z.t)
			const double initialZeroTerm = interpolateByZeroRate->value(initialStartTerm) * initialStartTerm;
			const double finalZeroTerm = interpolateByZeroRate->value(finalEndTerm) * finalEndTerm;

			val = AQLMath::exp(finalZeroTerm - initialZeroTerm);
		}

		// Explicit effective rate code, calculating the overnight rate in each time period
		/*
			for (size_t i = 0; i < startTerms.size(); ++i)
			{
				const double startterm = startTerms[i];
				const double endterm = endTerms[i];

				const double term = endterm - startterm;

				//***Tau should be from trade's dayCount
				//auto startDate = LADateScheduleHelpers::getDateFromTerm(spotDate, startterm, dc_act365);
				//auto endDate = LADateScheduleHelpers::getDateFromTerm(spotDate, endterm, dc_act365);
				//const double term = dc->getTerm(startDate, endDate);

				if (isLogDF)
				{
					// State Variable: Log Discount Factor
					// ForwardRate = [ DF(Start) / DF(End) - 1 ] / tau
					effectiveRate = (AQLMath::exp(interpolateByZeroRate->value(endterm) - interpolateByZeroRate->value(startterm)) - 1.0) / term;
				}
				else
				{
					// State Variable: Zero Rate
					// DiscountFactor = exp(-z.t)
					effectiveRate = (AQLMath::exp(interpolateByZeroRate->value(endterm) * endterm - interpolateByZeroRate->value(startterm) * startterm) - 1.0) / term;
				}

				val *= AQLPriceDataConvention::rateToRet(effectiveRate, term, conv);
			}

		*/

		val -= 1.0;
		effectiveRate = val / accrualPeriod;

		// End of Case
		break;

	}
	case ARITHMETIC_OIS_COMPOUNDING:
	{
		//
		// ARITHMETIC AVERAGING
		//

		effectiveRate = calcArithmeticAverageRate(startTerms, endTerms, interpolateByZeroRate, accrualPeriod, isLogDF);

		// End of Case
		break;

	}
	default:
	{
		AQ_THROW("Invalid CompoundingMethod - Must be ARITHMETIC, GEOMETRIC or NONE. For simple compounding set LongTerm.GenerateMethod to NONE")
	}
	}

	return effectiveRate;
}


/*
	@brief calc settle rate by arithmetic average

	@param[in] startterms
	@param[in] endterms
	@param[in] interpolateByZeroRate	inter of on zero rates
	@param[in] wholeterm
	@param[in] isLogDF					Is state variable logDF?

	@return settlerate
*/
double
CurveCalibration::calcArithmeticAverageRate(const DoubleVector& startterms,
											const DoubleVector& endterms,
											const AQLInterpolationBase* interpolateByZeroRate,
											const double wholeterm,
											bool isLogDF)
{
	unsigned int nStartTerms = startterms.size();
	unsigned int nEndTerms = endterms.size();

	if (nStartTerms == 0)
	{
		return 0.0;
	}

	AQ_REQUIRE(nStartTerms == nEndTerms, "Unable to calculate the Arithmetic Average rate - Inconsistent number of instrument start- and end-dates provided.")

	// ForwardRate      = [ DF(Start) / DF(End) - 1 ] / tau
	// tau              = yearFraction(Start, End)
	// ForwardRate.tau  = DF(Start) / DF(End) - 1
	double forwardRatesTimesYearFractions = 0.0;

	for (unsigned int i = 0; i < nStartTerms; ++i)
	{
		if (isLogDF)
		{
			// State Variable: Log Discount Factor, which is equivalent to zero rate times time.
			// DF = exp( - LogDF ) = exp( - zeroRate x time )
			// ForwardRate.tau = exp[ DF(End) - DF(Start) ) ] - 1
			// ForwardRate.tau = exp( logDF(Start) - logDF(End) - 1
			forwardRatesTimesYearFractions += AQLMath::exp(interpolateByZeroRate->value(endterms[i]) - interpolateByZeroRate->value(startterms[i])) - 1.0;
		}
		else
		{
			// State Variable: Zero Rate
			// ForwardRate.tau = exp[ ZeroRate(End).tau(End) - ZeroRate(Start).tau(Start) ] - 1
			// DF = exp( -ZeroRate.tau )
			forwardRatesTimesYearFractions += AQLMath::exp(interpolateByZeroRate->value(endterms[i]) * endterms[i] - interpolateByZeroRate->value(startterms[i]) * startterms[i]) - 1.0;
		}
	}

	const double arithmeticAverageRate = forwardRatesTimesYearFractions / wholeterm;
	return arithmeticAverageRate;
}

/*
	@brief solve ois rate by newton raphson

	@param[in] startdate
	@param[in] enddate
	@param[in] dc          daycount
	@param[in] cal         calendar
	@param[in] market_rate targetrate

	@return oisrate
*/
double CurveCalibration::getAverageRateFromHistRates(const AQLDate& startdate,
													 const AQLDate& enddate,
													 const AQLPriceDataDayCount& dc,
													 const AQLPriceDataCalendar& cal,
													 double market_rate,
													 AQLObject* marketData,
													 const AQLDate& asOfDate,
													 bool istodayrateexist,
													 double todayffrate)
{
	AQLDataHolder* dh;
	dh = &(marketData->getData(IR_CALIBRATION_DATA_HISTORICALDATES, ISNOTNULL));
	DateVector histdates = dynamic_cast<const AQLDataDates &>(dh->get()).get();

	dh = &(marketData->getData(IR_CALIBRATION_DATA_HISTORICALRATES, ISNOTNULL));
	DoubleVector histrates = dynamic_cast<const AQLDataDoubles &>(dh->get()).get();

	AQ_THROW_IF(histdates.size() != histrates.size(), "Invalid ON fixing rates specified. The number of dates and fixings don't match")

	//sort histrical datas
	std::map<AQLDate, double> map_date_rate;
	for (size_t i = 0; i < histdates.size(); ++i)
	{
		AQ_THROW_IF(map_date_rate.find(histdates[i]) != map_date_rate.end(), "Duplicate dates specified in the historical ON fixing rates table")
		map_date_rate[histdates[i]] = histrates[i];
	}
	sort(histdates.begin(), histdates.end());
	histrates.clear();
	for (size_t i = 0; i < histdates.size(); ++i)
	{
		histrates.push_back(map_date_rate[histdates[i]]);
	}

	AQLDate fixingDate = startdate;
	double val = 0.0;
	unsigned int pos = 0;
	int realizednumber = startdate.intervalDays(asOfDate) + 1;
	for (int i = 0; i < realizednumber; i++)
	{
		if (map_date_rate.find(fixingDate) != map_date_rate.end())
		{
			val += map_date_rate[fixingDate];
		}
		else
		{
			AQLAlgorithm::locate<DateVector, AQLDate>(histdates, fixingDate, histdates.size(), pos);

			if (fixingDate == asOfDate && istodayrateexist)
			{
				val += todayffrate;
			}
			else if (pos == histdates.size())
			{
				val += histrates.back();
			}
			else if (pos == 0)
			{
				val += histrates[pos];
			}
			else
			{
				val += histrates[pos - 1];
			}
		}

		fixingDate.addDays(1);
	}

	int averagenum = startdate.intervalDays(enddate) + 1;

	AQ_THROW_IF(enddate <= asOfDate, "Unable to calculate the average effective rate using the fixings specified. EndDate < AsOfDate")

	double implyrate = (static_cast<double>(averagenum) * market_rate - val) / static_cast<double>(averagenum - realizednumber);
	return implyrate;
}

// Store curve conventions at the end of swap curve calibration
void CurveCalibration::setCurveConvention( AQLObjectHolder& objHolder, std::vector<AQLObject*>& mktData, const AQLString& curveName )
{
	const AQLDataHolder *dh;

	AQLString suffix = "";
	if (curveName != AQ_STD)
	{
		suffix = "_" + curveName;
	}

	std::vector<AQLObject*> mktData_swap, mktData_libor;
	for (unsigned i = 0; i < mktData.size(); i++)
	{
		// check use grid
		dh = &mktData[i]->getData(IR_CALIBRATION_DATA_GRIDUSEFLAG, NOCHECK);
		if (dh->isDefined() && !dh->isNull() && !dynamic_cast<const AQLDataBool &>(dh->get()).get())
		{
			continue;
		}

		AQLString datatype_str = dynamic_cast<const AQLDataString&> ((mktData[i]->getData(IR_CALIBRATION_DATA_DATATYPE, ISNOTNULL)).get()).get();
		datatype_str.toUpper();

		if (datatype_str == AQ_SWAP_DATA)
		{
			mktData_swap.push_back(mktData[i]);//swap case
		}
		else if (datatype_str == AQ_LIBOR_DATA)
		{
			mktData_libor.push_back(mktData[i]);//libor case
		}
	}

	AQ_THROW_IF(mktData_swap.size() == 0, "Calibration instrument error. Swap instruments must be provided")

	AQLString freq = "";
	dh = &mktData_swap[0]->getData(IR_CALIBRATION_DATA_BASEFREQUENCY_FLOAT, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		freq = dynamic_cast<const AQLDataString&> (dh->get());
	}
	else
	{
		const AQLDataHolder *dh = &mktData_swap[0]->getData(IR_CALIBRATION_DATA_FREQUENCY_FLOAT, NOCHECK);
		if (dh->isDefined() && !dh->isNull())
		{
			freq = dynamic_cast<const AQLDataString&> (dh->get());
		}
		else
		{
			freq = dynamic_cast<const AQLDataString&> ((mktData_swap[0]->getData(IR_CALIBRATION_DATA_FREQUENCY, ISNOTNULL)).get()).get();
		}
	}

	const AQLPriceDataCalendar* cal = NULL;
	if (mktData_libor.size() != 0)
	{
		cal = &dynamic_cast<const AQLPriceDataCalendar&> ((mktData_libor[0]->getData(CALIBRATION_DATA_CALENDAR, ISNOTNULL)).get());
	}

	const AQLPriceDataSlidingRule* sld = NULL;
	if (mktData_libor.size() != 0)
	{
		sld = &dynamic_cast<const AQLPriceDataSlidingRule&> ((mktData_libor[0]->getData(CALIBRATION_DATA_SLIDINGRULE, ISNOTNULL)).get());
	}

	const AQLPriceDataDayCount* dc = NULL;
	dh = &mktData_swap[0]->getData(IR_CALIBRATION_DATA_DAYCOUNT_FLOAT, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		dc = &dynamic_cast<const AQLPriceDataDayCount&> (dh->get());
	}
	else
	{
		dc = &dynamic_cast<const AQLPriceDataDayCount&> ((mktData_swap[0]->getData(IR_CALIBRATION_DATA_DAYCOUNT, ISNOTNULL)).get());
	}

	AQLString accessary = "";
	if (freq == AQ_ANNUAL) accessary = "12M";
	else if (freq == AQ_SEMI_ANNUAL) accessary = "6M";
	else if (freq == AQ_QUARTERLY) accessary = "3M";
	else if (freq == AQ_MONTHLY || freq == AQ_LUNAR) accessary = "1M";
	else
	{ 
		AQ_THROW("Invalid curve frequency specified: Must be 1M, 3M, 6M or 12M")
	}

	objHolder.remove(IR_CALIBRATION_DATA_FREQUENCY + suffix);
	objHolder.remove(CALIBRATION_DATA_CALENDAR + suffix);
	objHolder.remove(CALIBRATION_DATA_SLIDINGRULE + suffix);
	objHolder.remove(IR_CALIBRATION_DATA_DAYCOUNT + suffix);
	objHolder.remove(IR_CALIBRATION_DATA_ACCESSARY + suffix);

	objHolder.add(IR_CALIBRATION_DATA_FREQUENCY + suffix, new AQLDataString(AQ_SIMPLE));
	if (cal) objHolder.add(CALIBRATION_DATA_CALENDAR + suffix, new AQLPriceDataCalendar(*cal));
	if (sld) objHolder.add(CALIBRATION_DATA_SLIDINGRULE + suffix, new AQLPriceDataSlidingRule(*sld));
	objHolder.add(IR_CALIBRATION_DATA_DAYCOUNT + suffix, new AQLPriceDataDayCount(*dc));
	objHolder.add(IR_CALIBRATION_DATA_ACCESSARY + suffix, new AQLDataString(accessary));
}

// Store curve conventions at the end of swap curve calibration
void CurveCalibration::setCurveConvention(LACurveStaticDataHolder & staticDataObj, std::vector<AQLObject*>& mktData, const AQLString& curveName)
{
	setCurveConvention( staticDataObj.curveDataObjectHolder_, mktData, curveName );
}

/* @brief			Calculate discount factors through swap rates
*  @param [inout]		yields								A vector of bootstrapped zero rates
*  @param [inout]		grid								A vector of day count fractions for the zero rates
*  @param [inout]		terms_grid							A vector of DCFs created in the current function call
*  @param [inout]		accrualPeriod						A vector of date intervals created in the current function call
*  @param [inout]		cashflowPaymentDates				A vector of cash flow dates
*  @param [inout]		interpolatorForDiscountFactors		Interpolator for discount factors
*  @param [inout]		interpolationForSwaps_				Interpolator for Swaps on zero rates
*  @param [in]			marketData							Collection of curve's all instruments
*  @param [in]			data_swap							Collection of curve's swap instruments
*  @param [in]			staticDataObj						Entitiy Pool Object Holder
*  @param [in]			dc_act365							Day count convention based on ACT365ISDA
*  @param [in]			asOfDate							Yield curve's asof date
*  @param [in]			spotdate							Yield curve's spot date
*  @param [in]			shortterm_date						End of the futures section
*  @param [in]			pDFCurveName						Name of discount curve
*  @param [in]			mpc_swap_size						Size of futures section
*  @param [in]			isEOMRoll									Do end of month rolling?
*  @param [in]			useShortEndSwaps					Use short term swaps or not
*  @param [in]			useShortEndSwaps					Number of short term swaps
*/
void CurveCalibration::calcOISDFBySwapRates( DoubleArray& yields,
											 DoubleArray& grid,
											 DoubleArray& terms_grid,
											 DoubleArray& accrualPeriod,
											 DateVector& cashflowPaymentDates,
											 AQLInterpolationBase* interpolatorForDiscountFactors,
											 AQLInterpolationBase* interpolationForSwaps,
											 std::vector<AQLObject*>& marketData,
											 const std::vector<AQLObject*>& data_swap,
											 LACurveStaticDataHolder & staticDataObj,
											 const AQLPriceDataDayCount& dc_act365,
											 const AQLDate& asOfDate,
											 const AQLDate& spotdate,
											 const AQLDate& shortterm_date,
											 const AQLString* pDFCurveName,
											 const size_t& mpc_swap_size,
											 bool isEOMRoll,
											 bool useShortEndSwaps,
											 size_t shortTermSwapSize )
{
	calcOISDFBySwapRates(yields,
						 grid,
						 terms_grid,
						 accrualPeriod,
						 cashflowPaymentDates,
						 interpolatorForDiscountFactors,
						 interpolationForSwaps,
						 marketData,
						 data_swap,
						 staticDataObj.curveDataObjectHolder_,
						 dc_act365,
						 asOfDate,
						 spotdate,
						 shortterm_date,
						 pDFCurveName,
						 mpc_swap_size,
						 isEOMRoll,
						 useShortEndSwaps,
						 shortTermSwapSize);
}

/* @brief			Calculate discount factors through swap rates
*  @param [inout]		yields								A vector of bootstrapped zero rates
*  @param [inout]		grid								A vector of day count fractions for the zero rates
*  @param [inout]		terms_grid							A vector of DCFs created in the current function call
*  @param [inout]		accrualPeriod						A vector of date intervals created in the current function call
*  @param [inout]		cashflowPaymentDates				A vector of cash flow dates
*  @param [inout]		interpolatorForDiscountFactors		Interpolator for discount factors
*  @param [inout]		interpolationForSwaps_				Interpolator for Swaps on zero rates
*  @param [in]			marketData							Collection of curve's all instruments
*  @param [in]			data_swap							Collection of curve's swap instruments
*  @param [in]			objHolder							Entitiy pool pointer
*  @param [in]			dc_act365							Day count convention based on ACT365ISDA
*  @param [in]			asOfDate							Yield curve's asof date
*  @param [in]			spotdate							Yield curve's spot date
*  @param [in]			shortterm_date						End of the futures section
*  @param [in]			pDFCurveName						Name of discount curve
*  @param [in]			mpc_swap_size						Size of futures section
*  @param [in]			isEOMRoll									Do end of month rolling?
*  @param [in]			useShortEndSwaps					Use short term swaps or not
*  @param [in]			useShortEndSwaps					Number of short term swaps
*/
void CurveCalibration::calcOISDFBySwapRates(DoubleArray& yields,
											DoubleArray& grid,
											DoubleArray& terms_grid,
											DoubleArray& accrualPeriod,
											DateVector& cashflowPaymentDates,
											AQLInterpolationBase* interpolatorForDiscountFactors,
											AQLInterpolationBase* interpolationForSwaps,
											std::vector<AQLObject*>& marketData,
											const std::vector<AQLObject*>& data_swap,
											AQLObjectHolder& objHolder,
											const AQLPriceDataDayCount& dc_act365,
											const AQLDate& asOfDate,
											const AQLDate& spotdate,
											const AQLDate& shortterm_date,
											const AQLString* pDFCurveName,
											const size_t& mpc_swap_size,
											bool isEOMRoll,
											bool useShortEndSwaps,
											size_t shortTermSwapSize)
{
	size_t swap_size = data_swap.size();

	// calc df by swap rate

	AQLString suffix = "";
	AQLInterpolationBase *interpolationForDiscountFactors_ = NULL;
	const double spotterm = dc_act365.getTerm(asOfDate, spotdate);
	double d_spotdf = 1.0;
	bool is_selfdf = true;
	if (pDFCurveName && *pDFCurveName != ITSELF)
	{
		is_selfdf = false;
		if (*pDFCurveName != AQ_STD) suffix = AQLString("_") + *pDFCurveName;
		AQ_THROW_IF(!objHolder.getData(CALIBRATION_DATA_TERMS + suffix).isDefined(),"Invalid Curve: Curve with name '" + *pDFCurveName + "' does not exist")

		const DoubleVector &terms_ = dynamic_cast<const AQLDataDoubles&> ((objHolder.getData(CALIBRATION_DATA_TERMS + suffix, ISNOTNULL)).get()).get();
		const DoubleVector &dfs_ = dynamic_cast<const AQLDataDoubles&> ((objHolder.getData(IR_CALIBRATION_DATA_DFS + suffix, ISNOTNULL)).get()).get();

		// Use the discount curve's native interpolation method
		AQLPriceDataInterpolation attrInter = getYieldGenInterpolationByCurveName(const_cast<AQLObjectHolder&>(objHolder), suffix);
		interpolationForDiscountFactors_ = dynamic_cast<AQLInterpolationBase*>(attrInter.getMethod().clone());

		interpolationForDiscountFactors_->set(terms_, dfs_);
		d_spotdf = interpolationForDiscountFactors_->value(spotterm);
	}

	double eps = 1.0e-10;
	const AQLDataHolder *dh = &(marketData[0]->getData(IR_CALIBRATION_DATA_EPSILON, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		eps = dynamic_cast<const AQLDataDouble &>(dh->get()).get();
	}
	int max_loop = 100;
	dh = &(marketData[0]->getData(IR_CALIBRATION_DATA_MAXLOOP, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		max_loop = dynamic_cast<const AQLDataInt &>(dh->get()).get();
	}

	unsigned int calced_size = 0;
	// double _sumCF = 0.0; <-- Unused Variable
	// double _sumT = 0.0;  <-- Unused Variable

	AQLString longTermConv = "NONE";
	AQLString last_longTermConv = "NONE";
	OISLongTermInstrumentsEnum longTermConvEnum = NONE_OIS_LONGTERM_INSTRUMENTS;
	OISLongTermInstrumentsEnum last_longTermConvEnum = NONE_OIS_LONGTERM_INSTRUMENTS;

	AQLString longTermGen;
	// bool reset = false; <-- Unused Variable
	unsigned int calced_swap_size = 0;
	// double _sumT_s = 0.0; <-- Unused Variable
	DoubleArray terms_grid_s, terms_interval_s;
	DateVector dates_s;

	// double settlerate = 0.0; <-- Unused Variable
	AQLDate lastSwapEndDate = spotdate;

	{

		AQLString oldFreq("");
		AQLString oldFreq_s("");
		oldFreq.toUpper();

		std::vector<AQLString> term_strs;
		std::vector<AQLDate> endDates_s;
		std::vector<double> marketRates;
		std::vector<double> marketRates_s;
		std::vector<double> swapEndDateInTerms;
		std::vector<const AQLPriceDataCalendar*> cals;
		std::vector<const AQLPriceDataDayCount*> dateCounts;
		std::vector<size_t> size_cashlets;
		std::vector<size_t> size_cashlets_s;
		std::vector<size_t> calced_sizes;
		std::vector<size_t> calced_sizes_s;
		std::vector<size_t> size_calcs;
		std::vector<size_t> size_calcs_s;
		std::vector<bool> bLiborSwapCalcReset;
		std::vector<bool> bOISSwapCalcReset;
		std::vector<bool> bFullSigmaReset;
		DoubleMatrix terms_grids;
		DoubleMatrix terms_grids_s;
		DoubleMatrix terms_intervals;
		DoubleMatrix terms_intervals_s;
		std::vector<DateVector> swapDates;
		std::vector<DateVector> swapDates_s;
		std::vector<DateVector> fixingStartDates;
		std::vector<DateVector> fixingEndDates;
		std::vector<DoubleVector> fixingAccrualPeriods;

		std::vector<AQLString> longTermGens;
		std::vector< OISLongTermInstrumentsEnum > longTermConvEnums;
		std::vector< OISCompoundingEnum > swapCompoundingMethodEnums;

		for (unsigned int i = 0; i < swap_size; ++i)
		{
			AQLString term_str = dynamic_cast<const AQLDataString&> ((data_swap[i]->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get());
			term_str.toUpper();

			dh = &data_swap[i]->getData(IR_CALIBRATION_DATA_LONGTERMCONVENTION);
			if (dh->isDefined() && !dh->isNull())
			{
				longTermConv = dynamic_cast<const AQLDataString&> (dh->get()).get();
				longTermGen = dynamic_cast<const AQLDataString&> ((data_swap[i]->getData(IR_CALIBRATION_DATA_LONGTERMGENMETHOD, ISNOTNULL)).get());
			}

			longTermConvEnum = toOISLongTermInstrumentsEnum(longTermConv.getCString());
			last_longTermConvEnum = toOISLongTermInstrumentsEnum(last_longTermConv.getCString());

			double marketrate = 0.0;
			const AQLPriceDataDayCount* dc;
			const AQLPriceDataCalendar* cal;
			const AQLPriceDataSlidingRule* sld;
			AQLString freq;

			//---------------------------------------------------------
			// Produce full OIS swap schedules

			// load ois marketdata
			if (longTermConvEnum == NONE_OIS_LONGTERM_INSTRUMENTS)
			{
				marketrate = dynamic_cast<const AQLDataDouble&> ((data_swap[i]->getData(CALIBRATION_DATA_RATE, ISNOTNULL)).get()).get();
				dc = &dynamic_cast<const AQLPriceDataDayCount&> ((data_swap[i]->getData(IR_CALIBRATION_DATA_DAYCOUNT, ISNOTNULL)).get());
				cal = &dynamic_cast<const AQLPriceDataCalendar&> ((data_swap[i]->getData(CALIBRATION_DATA_CALENDAR, ISNOTNULL)).get());
				sld = &dynamic_cast<const AQLPriceDataSlidingRule&> ((data_swap[i]->getData(CALIBRATION_DATA_SLIDINGRULE, ISNOTNULL)).get());
				freq = dynamic_cast<const AQLDataString&> ((data_swap[i]->getData(IR_CALIBRATION_DATA_FREQUENCY, ISNOTNULL)).get()).get();
			}
			// load lobasis marketdata
			else if (longTermConvEnum == LIBOROIS_OIS_LONGTERM_INSTRUMENTS)
			{
				marketrate = dynamic_cast<const AQLDataDouble&> (data_swap[i]->getData(IR_CALIBRATION_DATA_RATE_LOBASIS, ISNOTNULL).get()).get();
				dc = &dynamic_cast<const AQLPriceDataDayCount&> ((data_swap[i]->getData(IR_CALIBRATION_DATA_DAYCOUNT_LOBASIS, ISNOTNULL)).get());
				cal = &dynamic_cast<const AQLPriceDataCalendar&> ((data_swap[i]->getData(IR_CALIBRATION_DATA_CALENDAR_LOBASIS, ISNOTNULL)).get());
				sld = &dynamic_cast<const AQLPriceDataSlidingRule&> ((data_swap[i]->getData(IR_CALIBRATION_DATA_SLIDINGRULE_LOBASIS, ISNOTNULL)).get());
				freq = dynamic_cast<const AQLDataString&> ((data_swap[i]->getData(IR_CALIBRATION_DATA_FREQUENCY_LOBASIS, ISNOTNULL)).get()).get();
			}
			else
			{
				AQ_THROW("Invalid LongTermConvention - Must be set to 'LOBASIS' to calibrate to Libor-OIS Instruments or 'NONE' for OIS Outrights only")
			}

			AQLString roll_conv("");
			freq.toUpper();
			if (freq == AQ_LUNAR)
			{
				roll_conv = AQ_ROLLCONV_LUNAR;
			}
			else if (isEOMRoll)
			{
				roll_conv = AQ_ROLLCONV_EOM;
			}
			else
			{
				roll_conv = AQ_ROLLCONV_NORMAL;
			}

			AQLDate swapEndDate = LADateHelpers::getDate(spotdate, term_str, *sld, cal, true, &roll_conv);
			AQLDate date_unadjusted = LADateHelpers::getDate(spotdate, term_str, AQLPriceDataSlidingRule(SLIDING_RULE_NO_CHANGE), NULL, true, nullptr);

			if (!useShortEndSwaps &&
				(((mpc_swap_size || shortTermSwapSize) && swapEndDate < shortterm_date) || (!mpc_swap_size && i == 0) || (lastSwapEndDate >= swapEndDate)))
			{
				continue;
			}

			// *** Important ***
			// This term parameter determines the calibration grid dates to be used to store the zero rate state variables
			double term = dc_act365.getTerm(spotdate, swapEndDate);

			terms_grid.clear();
			accrualPeriod.clear();
			cashflowPaymentDates.clear();
			updateAccrualPeriodsAndPaymentDates(spotdate, date_unadjusted, freq, *cal, *sld, *dc, cashflowPaymentDates, terms_grid, accrualPeriod, isEOMRoll);

			unsigned int size_cashlet = cashflowPaymentDates.size();
			AQ_THROW_IF(!size_cashlet, "Invalid calibration instruments specified with no cashflows")

			//---------------------------------------------------------
			// Arrange to re-use calculation resultsObj from earlier swaps

			bOISSwapCalcReset.push_back(false);
			if (longTermConvEnum != last_longTermConvEnum)
			{
				lastSwapEndDate = spotdate;
				calced_size = 0;
				calced_swap_size = 0;

				bOISSwapCalcReset.back() = true;
			}
			last_longTermConvEnum = longTermConvEnum;

			bFullSigmaReset.push_back(false);
			if (size_cashlet == calced_size						// 1. Swap with the same cashlets as the last swap is encountered
				|| (oldFreq != freq && oldFreq.size() != 0)		// 2. Swap frequency has changed
				|| ((calced_size > 0) && lastSwapEndDate != cashflowPaymentDates[calced_size - 1] && size_cashlet > calced_size)	// 3. Current swap has an odd date schedule
				)
			{
				// When odd swap date schedule is encountered, reset all sigmas and will build swap date schedule from the beginning
				// rather than by extending from the previous swap's schedule

				if (calced_size == 0 || oldFreq != freq)
				{
					lastSwapEndDate = spotdate;
				}
				else
				{
					lastSwapEndDate = cashflowPaymentDates[calced_size - 1];
				}

				calced_size = 0;

				bFullSigmaReset.back() = true;
			}
			oldFreq = freq;

			unsigned int size_calc = size_cashlet - calced_size;
			if (!size_calc)
			{
				continue;
			}

			// load libor swap data
			double marketrate_s = 0.0;
			unsigned int size_cashlet_s = 0;
			unsigned int size_calc_s = 0;
			bLiborSwapCalcReset.push_back(false);
			if (longTermConvEnum == LIBOROIS_OIS_LONGTERM_INSTRUMENTS)
			{
				marketrate_s = dynamic_cast<const AQLDataDouble&> (data_swap[i]->getData(IR_CALIBRATION_DATA_RATE_SWAP, ISNOTNULL).get()).get();
				const AQLPriceDataDayCount& swapDaycountFixed = dynamic_cast<const AQLPriceDataDayCount&> ((data_swap[i]->getData(IR_CALIBRATION_DATA_DAYCOUNT_SWAP, ISNOTNULL)).get());
				const AQLPriceDataCalendar& swapCalendar = dynamic_cast<const AQLPriceDataCalendar&> ((data_swap[i]->getData(IR_CALIBRATION_DATA_CALENDAR_SWAP, ISNOTNULL)).get());
				const AQLPriceDataSlidingRule& swapBusDayAdj = dynamic_cast<const AQLPriceDataSlidingRule&> ((data_swap[i]->getData(IR_CALIBRATION_DATA_SLIDINGRULE_SWAP, ISNOTNULL)).get());
				AQLString swapFrequencyFixed = dynamic_cast<const AQLDataString&> ((data_swap[i]->getData(IR_CALIBRATION_DATA_FREQUENCY_SWAP, ISNOTNULL)).get()).get();
				swapFrequencyFixed.toUpper();

				if (oldFreq_s.size() != 0 && oldFreq_s != swapFrequencyFixed)
				{
					// Reset when calibration swap frequency changes
					bLiborSwapCalcReset.back() = true;
					calced_swap_size = 0;
				}
				oldFreq_s = swapFrequencyFixed;

				AQLString swapRollConvention("");
				if (freq == AQ_LUNAR) swapRollConvention = AQ_ROLLCONV_LUNAR;
				else if (isEOMRoll) swapRollConvention = AQ_ROLLCONV_EOM;
				else swapRollConvention = AQ_ROLLCONV_NORMAL;

				// scheduling libor swap
				const AQLDate date_s_unadjusted = LADateHelpers::getDate(spotdate, term_str, AQLPriceDataSlidingRule(SLIDING_RULE_NO_CHANGE), NULL, true, nullptr);
				terms_grid_s.clear();
				terms_interval_s.clear();
				dates_s.clear();
				updateAccrualPeriodsAndPaymentDates(spotdate, date_s_unadjusted, swapFrequencyFixed, swapCalendar, swapBusDayAdj, swapDaycountFixed, dates_s, terms_grid_s, terms_interval_s, isEOMRoll);
				size_cashlet_s = dates_s.size();
				AQ_THROW_IF(!size_cashlet_s, "Invalid swap calibration instrument specified with no cashflows.")
				size_calc_s = size_cashlet_s - calced_swap_size;

				// Compute fixing schedule dates
				DateVector thisFixingStartDates;
				thisFixingStartDates.push_back(spotdate);
				for (size_t j = 0; j < cashflowPaymentDates.size() - 1; ++j)
				{
					thisFixingStartDates.push_back(cashflowPaymentDates[j]);
				}

				AQLString indexTenor;
				if (freq == AQ_SEMI_ANNUAL)
				{
					indexTenor = AQ_TERM_6M;
				}
				else if (freq == AQ_QUARTERLY)
				{
					indexTenor = AQ_TERM_3M;
				}
				else if (freq == AQ_MONTHLY)
				{
					indexTenor = AQ_TERM_1M;
				}
				else if (freq == AQ_ANNUAL)
				{
					indexTenor = "1Y";
				}
				else if (freq == AQ_BUSINESS_DAYS)
				{
					indexTenor = "1D";
				}
				else
				{
					AQ_THROW("Invalid Swap Floating Frequency: Only 1D, 1M, 3M, 6M and 12M frequencies are supported")
				}

				// fixing end dates are always a fixed term after the corresponding fixing start dates
				DateVector thisFixingEndDates;
				if (freq == AQ_ANNUAL && (term_str == "18M" || term_str == "1Y6M") && cashflowPaymentDates.size() == 2)
				{
					// Special case. Swap with 6M front stub.
					thisFixingEndDates.push_back(cashflowPaymentDates[0]);
					thisFixingEndDates.push_back(cashflowPaymentDates[1]);
				}
				else
				{
					thisFixingEndDates = LADateScheduleHelpers::getMultiDate(thisFixingStartDates, indexTenor, sld->convertToString(), cal->convertToString(), nullptr); // rollconvention* = nullptr
				}

				DoubleVector thisFixingTaus;
				for (size_t j = 0; j < thisFixingEndDates.size(); ++j)
				{
					// Never allow any fixing dates to be beyond the end of the current swap
					// Otherwise the last forward rates may be susceptible to extrapolation which is unwanted
					if (thisFixingEndDates[j] > cashflowPaymentDates.back())
					{
						thisFixingEndDates[j] = cashflowPaymentDates.back();
					}

					double fixingTau = dc->getTerm(thisFixingStartDates[j], thisFixingEndDates[j]);
					thisFixingTaus.push_back(fixingTau);
				}

				fixingStartDates.push_back(thisFixingStartDates);
				fixingEndDates.push_back(thisFixingEndDates);
				fixingAccrualPeriods.push_back(thisFixingTaus);
			}
			else if (longTermConvEnum == NONE_OIS_LONGTERM_INSTRUMENTS)
			{
				//---------------------------------------------------------
				// Compute fixing schedule dates
				DateVector thisFixingStartDateVector, thisFixingEndDateVector;
				DoubleArray thisFixingTauVector;

				thisFixingStartDateVector.push_back(spotdate);
				for (size_t j = 0; j < cashflowPaymentDates.size() - 1; ++j)
				{
					thisFixingStartDateVector.push_back(cashflowPaymentDates[j]);
					thisFixingEndDateVector.push_back(cashflowPaymentDates[j]);
					thisFixingTauVector.push_back(accrualPeriod[j]);
				}
				thisFixingEndDateVector.push_back(cashflowPaymentDates.back());
				thisFixingTauVector.push_back(accrualPeriod.back());

				fixingStartDates.push_back(thisFixingStartDateVector);
				fixingEndDates.push_back(thisFixingEndDateVector);
				fixingAccrualPeriods.push_back(thisFixingTauVector);
			}
			else
			{
				AQ_THROW("Invalid LongTermConvention - Must be set to 'LOBASIS' to calibrate to Libor-OIS Instruments or 'NONE' for OIS Outrights only")
			}

			lastSwapEndDate = cashflowPaymentDates[size_cashlet - 1];

			// Compounding methods
			AQLString swapCompoundingMethod("");
			dh = &data_swap[i]->getData(IR_CALIBRATION_DATA_COMPOUNDINGMETHOD, NOCHECK);
			if (dh->isDefined() && !dh->isNull())
			{
				swapCompoundingMethod = dynamic_cast<const AQLDataString&> (dh->get()).get();
			}
			swapCompoundingMethodEnums.push_back(toOISCompoundingEnum(swapCompoundingMethod.getCString()));

			//---------------------------------------------------------
			// Store all sorts of information in vector form
			term_strs.push_back(term_str);
			endDates_s.push_back(swapEndDate);
			swapEndDateInTerms.push_back(term);
			cals.push_back(cal);
			dateCounts.push_back(dc);

			swapDates.push_back(cashflowPaymentDates);
			swapDates_s.push_back(dates_s);

			marketRates.push_back(marketrate);
			marketRates_s.push_back(marketrate_s);

			size_cashlets.push_back(size_cashlet);
			size_cashlets_s.push_back(size_cashlet_s);

			size_calcs.push_back(size_calc);
			size_calcs_s.push_back(size_calc_s);

			calced_sizes.push_back(calced_size);
			calced_sizes_s.push_back(calced_swap_size);

			terms_grids.push_back(terms_grid);
			terms_grids_s.push_back(terms_grid_s);

			terms_intervals.push_back(accrualPeriod);
			terms_intervals_s.push_back(terms_interval_s);

			longTermConvEnums.push_back(toOISLongTermInstrumentsEnum(longTermConv.getCString()));
			longTermGens.push_back(longTermGen);

			calced_size = size_cashlet;
			if (longTermConvEnum == LIBOROIS_OIS_LONGTERM_INSTRUMENTS)
			{
				calced_swap_size = size_cashlet_s;
			}
		}

		const double delta = 1.0e-10;
		const double grad_eps = 1.0e-15;

		std::vector<DoubleMatrix> startterms(term_strs.size());
		std::vector<DoubleMatrix> endterms(term_strs.size());

		// Variable 'yields' carries all the key points being calibrated. There can be parts of the curve that have already
		// been calibrated by the time swaps are used. 'preSwapSize' measures the number of key points that have already
		// been calibrated prior to swaps are used.

		// Initialise state variable interpolator
		size_t swapCount = term_strs.size();
		size_t preSwapSize = yields.size();
		for (size_t i = 0; i < swapCount; ++i)
		{
			grid.push_back(swapEndDateInTerms[i]);
			yields.push_back(longTermConvEnum == LIBOROIS_OIS_LONGTERM_INSTRUMENTS ? marketRates_s[i] - marketRates[i] : marketRates[i]);
		}
		interpolationForSwaps->set(grid, yields);

		// The code snippet below attempts to solve for the target curve using newton raphson
		// In the context of newton raphson, we have y = f(x) where y is the PV of each OIS
		// swap (either outright or Libor-OIS basis swap) and x is the market par rate.

		// -----------------------------------------------------------------------
		// 1. Calculate the value of y as in y = f(x) - BEFORE perturbation
		DoubleVector allPVs_old;
		priceOISSwaps(allPVs_old, interpolationForSwaps, interpolationForDiscountFactors_, size_calcs, size_calcs_s, calced_sizes, calced_sizes_s, terms_grids, terms_grids_s, terms_intervals, terms_intervals_s,
			marketRates, marketRates_s, fixingStartDates, fixingEndDates, fixingAccrualPeriods, term_strs, is_selfdf, d_spotdf, spotterm, spotdate, swapCompoundingMethodEnums, longTermConvEnums,
			longTermGens, cals, dateCounts, bOISSwapCalcReset, bFullSigmaReset, bLiborSwapCalcReset, startterms, endterms);

		// Perturb x as in y = f(x)
		for (size_t i = 0; i < swapCount; ++i)
		{
			yields[preSwapSize + i] += delta;
		}
		interpolationForSwaps->set(grid, yields);

		// -----------------------------------------------------------------------
		// 2. Calculate the value of y as in y = f(x) - AFTER perturbation
		DoubleVector allPVs_new;
		priceOISSwaps(allPVs_new, interpolationForSwaps, interpolationForDiscountFactors_, size_calcs, size_calcs_s, calced_sizes, calced_sizes_s, terms_grids, terms_grids_s, terms_intervals, terms_intervals_s,
			marketRates, marketRates_s, fixingStartDates, fixingEndDates, fixingAccrualPeriods, term_strs, is_selfdf, d_spotdf, spotterm, spotdate, swapCompoundingMethodEnums, longTermConvEnums,
			longTermGens, cals, dateCounts, bOISSwapCalcReset, bFullSigmaReset, bLiborSwapCalcReset, startterms, endterms);

		// -----------------------------
		// 3. Start single swap solving
		int loop = max_loop;
		bool solutionFound = false;
		while (loop--)
		{
			bool isEnd = true;

			// Check whether solving can no longer produce meaningful difference to the output.
			for (size_t i = 0; i < swapCount; ++i)
			{
				if (AQLMath::abs(allPVs_new[i] - allPVs_old[i]) >= grad_eps)
				{
					isEnd = false;
					break;
				}
			}

			if (isEnd)
			{
				break;
			}

			// -----------------------------------------------------------------------
			// 3.1 Perturb x by amount 'delta' in order to calculate y_after_bump
			AQLMatrix jacobianMatrix(swapCount, swapCount);
			for (size_t i = 0; i < swapCount; ++i)
			{
				DoubleArray thisYieldVector(yields);
				thisYieldVector[preSwapSize + i] += delta;
				interpolationForSwaps->set(grid, thisYieldVector);

				DoubleVector allPVs_after_bump;
				priceOISSwaps(allPVs_after_bump, interpolationForSwaps, interpolationForDiscountFactors_, size_calcs, size_calcs_s, calced_sizes, calced_sizes_s, terms_grids, terms_grids_s, terms_intervals, terms_intervals_s,
					marketRates, marketRates_s, fixingStartDates, fixingEndDates, fixingAccrualPeriods, term_strs, is_selfdf, d_spotdf, spotterm, spotdate, swapCompoundingMethodEnums, longTermConvEnums,
					longTermGens, cals, dateCounts, bOISSwapCalcReset, bFullSigmaReset, bLiborSwapCalcReset, startterms, endterms);

				for (size_t j = 0; j < swapCount; ++j)
				{
					const double firstOrderDerivative = (allPVs_after_bump[j] - allPVs_new[j]) / delta;
					jacobianMatrix.setValue(j, i, firstOrderDerivative);
				}
			}

			// -----------------------------------------------------------------------
			// 3.2 Update x as in y = f(x) according to Newton Raphson
			AQLMatrix valMat(allPVs_new);

			AQLMatrix inverseJacobian(swapCount, swapCount);
			inverseJacobian = jacobianMatrix.inverseMatrix();

			AQLMatrix deltaMat = inverseJacobian * valMat;

			for (size_t i = 0; i < swapCount; ++i)
			{
				yields[preSwapSize + i] -= deltaMat.getValue(i, 0);
			}

			// -----------------------------------------------------------------------
			// 3.3 Calculate new y as in y = f(x) now that we've obtained a new x
			allPVs_old = allPVs_new;
			interpolationForSwaps->set(grid, yields);
			priceOISSwaps(allPVs_new, interpolationForSwaps, interpolationForDiscountFactors_, size_calcs, size_calcs_s, calced_sizes, calced_sizes_s, terms_grids, terms_grids_s, terms_intervals, terms_intervals_s,
				marketRates, marketRates_s, fixingStartDates, fixingEndDates, fixingAccrualPeriods, term_strs, is_selfdf, d_spotdf, spotterm, spotdate, swapCompoundingMethodEnums, longTermConvEnums,
				longTermGens, cals, dateCounts, bOISSwapCalcReset, bFullSigmaReset, bLiborSwapCalcReset, startterms, endterms);

			solutionFound = true;
			for (unsigned int i = 0; i < swapCount; ++i)
			{
				// A solution is considered acceptable only when all calibration instruments reprice to zero
				if (AQLMath::abs(allPVs_new[i]) >= eps)
				{
					solutionFound = false;
					break;
				}
			}

			if (solutionFound)
			{
				break;
			}
		}

		delete interpolationForDiscountFactors_;
		AQ_REQUIRE(solutionFound, "OIS curve optimisation failed to converge to a solution")
	}
}
