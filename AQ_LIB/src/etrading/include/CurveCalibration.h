//
//  CurveCalibration.h
//  This file used to be called YieldGenerator.h and before that LAPriceYieldGenerator.h
//
#pragma once

#ifdef __GNUG__
#pragma interface
#endif
 
#include "AQLCoreProcedure.h"
#include "AQLDate.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"
#include "CurveInstruments.h"
#include "AQLObject.h"
#include "AQLObjectHolder.h"
#include "AQLDataProcedure.h"
#include "CurveCalibrationData.h"
#include "CoreEnumerations.h"
#include <vector>
#include "Solvers.h"

//
//  TODO: Remove all these defines - very bad practice
//
            
// irYieldGenerator Function id
#define FN_IRYIELDGENERATOR                 305

// irYieldGenerator Function Name
#define FN_IRYIELDGENERATOR_STR             "fn_iryieldgenerator"


#ifndef PRICING_DATA_PRICE
#define PRICING_DATA_PRICE			        "Price"
#endif

#ifndef PRICING_DATA_STARTDATE
#define PRICING_DATA_STARTDATE		        "StartDate"
#endif

#ifndef PRICING_DATA_ENDDATE
#define PRICING_DATA_ENDDATE		        "EndDate"
#endif

#ifndef PRICING_DATA_SPREAD
#define PRICING_DATA_SPREAD			        "Spread"
#endif

#ifndef PRICING_DATA_FUTUREVOLATILITY
#define PRICING_DATA_FUTUREVOLATILITY	    "FutureVolatility"
#endif

// *** This is the numerical value of the convexity adjustment itself ***
#ifndef PRICING_DATA_CONVEXADJUSTMENT
#define PRICING_DATA_CONVEXADJUSTMENT	    "ConvexAdjustment"
#endif

// *** Legacy Parameter Meaing ConvexityQuotedAsPrice ... poor name, replaced by ConvexityQuoteType Parameter ***
#ifndef PRICING_DATA_USECONVEXADJUSTMENT
#define PRICING_DATA_USECONVEXADJUSTMENT	"UseConvexAdjustment"
#endif

// *** An alias for UseConvexAdjustment for clarity ***
#ifndef PRICING_DATA_CONVEXITYQUOTETYPE
#define PRICING_DATA_CONVEXITYQUOTETYPE		"ConvexityQuoteType" // Alias for PRICING_DATA_USECONVEXADJUSTMENT
#endif

#ifndef PRICING_DATA_RATEPRIORITY
#define PRICING_DATA_RATEPRIORITY	        "RatePriority"
#endif

#ifndef PRICING_DATA_FUTUREOLDMETHOD
#define PRICING_DATA_FUTUREOLDMETHOD	    "FutureOldMethod"
#endif

#ifndef PRICING_DATA_ISFWDSWAP
#define PRICING_DATA_ISFWDSWAP	            "IsFwdSwap"
#endif

#ifndef PRICING_DATA_ISDATE
#define PRICING_DATA_ISDATE	                "IsDate"
#endif

#ifndef PRICING_DATA_STARTTERM
#define PRICING_DATA_STARTTERM		        "StartTerm"
#endif

#ifndef PRICING_DATA_TENOR
#define PRICING_DATA_TENOR		            "Tenor"
#endif

#ifndef STD
#define STD	                                "STD"
#endif

#ifndef ITSELF
#define ITSELF	                            "ITSELF"
#endif

// Typedefs
typedef std::map<AQLString, bool>					ListOfBuiltCurves;
typedef std::map<AQLString, AQLString>				TargetCurveAliasList;
typedef std::pair<AQLDate, AQLDate>					StartAndEndDate;
typedef std::map<StartAndEndDate, const AQLObject*>	MoneyMarketData;

// Forward Declarations
class AQLObject;
class AQLObjectHolder;
class AQLDataProcedure;
class CurveCalibrationData;
class AQLInterpolationBase;
class AQLPriceDataManager;
class AQLPriceDataCalendar;
class AQLPriceDataDayCount;
class AQLPriceDataSlidingRule;
class LACurveStaticDataHolder;
class LACurveMarketData;
class CurveProperties;
class CurveMarketDataHolder;
class InterpolationDataHolder;
class LiborIndex;
class FuturesInstruments;
class SwapInstruments;
class SwapCashflows;
class TenorBasisInstruments;
class SwapCalibration;
class CalibrationResults;

using namespace etrading;

// Curve Parameters to replace legacy defines
namespace etrading
{
	namespace curveParameters
    {
        //
        // Important Note: Here we Deprecate LA Define Statements from Common and IR Projects
        // #define statements have been replaced as AQLString with AlgoQuantLib prefix
        //
        extern const AQLString AQ_O_N;
        extern const AQLString AQ_T_N;
        extern const AQLString AQ_ON;
        extern const AQLString AQ_TN;
		extern const AQLString AQ_LIBOR_DATA;
        extern const AQLString AQ_ZERO;
		extern const AQLString AQ_SWAP_DATA;
        extern const AQLString AQ_PARRATE;
        extern const AQLString AQ_FWD;
        extern const AQLString AQ_BASIS;
        extern const AQLString AQ_MPC_SWAP;
        extern const AQLString AQ_FOMC_SWAP;
        extern const AQLString AQ_STD;
        extern const AQLString AQ_FRA3M;
        extern const AQLString AQ_FRA6M;
        extern const AQLString AQ_TERM_3M;
        extern const AQLString AQ_TERM_6M;
        extern const AQLString AQ_TERM_1M;
		extern const AQLString AQ_TERM_12M;

        extern const AQLString AQ_SIMPLE;
        extern const AQLString AQ_ANNUAL;
        extern const AQLString AQ_SEMI_ANNUAL;
        extern const AQLString AQ_QUARTERLY;
        extern const AQLString AQ_MONTHLY;
        extern const AQLString AQ_LUNAR;
        extern const AQLString AQ_BUSINESS_DAYS;

        extern const AQLString AQ_ROLLCONV_NORMAL;
        extern const AQLString AQ_ROLLCONV_EOM;
        extern const AQLString AQ_ROLLCONV_LUNAR;
    
        extern const AQLString AQ_LIBOR;
        extern const AQLString AQ_FUTURE;
        extern const AQLString AQ_FRA;
        extern const AQLString AQ_SWAP;
    }
}

// Struct: Swap Curve Initial DiscountFactors Container
struct SwapCurveDiscountFactors
{
	// Constructor / Destructor
	SwapCurveDiscountFactors()
		: dfInterpolator_(nullptr), swapSpotDateDF_(1.0), swapSpotDateTerm_(0.0)
	{}

	~SwapCurveDiscountFactors() {};

	// Member Variables
	std::shared_ptr<AQLInterpolationBase> dfInterpolator_;
	double swapSpotDateDF_;
	double swapSpotDateTerm_;
};

// Method to Initialize Swap Curve DiscountFactors
// Outside the curve class as the curve calibration method is static and cannot access member data parameters
SwapCurveDiscountFactors initialiseSwapCurveDiscountFactors( LACurveStaticDataHolder & staticDataObj, const LiborIndex & liborIndex, CalibrationResults & resultsObj );

// Curve Object Pool for the 'CalibrateModel' and 'CalibrateCurveModel' Method
class LACurveStaticDataHolder
{
public:

	// Main Constructor & Destructor
	LACurveStaticDataHolder( AQLObject & curveDataObject,
                             CurveCalibrationData & curveData,
                             CurveProperties& instrumentSettings,
                             const AQLDataProcedure & curveAttributeData,
                             const AQLString & curveCollection,  // or curveID
                             const AQLString & curveIndex );     // or marketName
	
    virtual ~LACurveStaticDataHolder() {};

	// Update Curve State Variables
	void updateSwapCurveStateVariables( CurveMarketDataHolder & mktDataObj );
	
	// Legacy Curve Data Objects
	AQLObject						curveDataObject_;
	AQLObjectHolder					curveDataObjectHolder_;
	CurveCalibrationData &			curveCalibrationData_;
	AQLDataProcedure					curveAttributeData_;

	// Legacy Curve Name Parameters
	AQLString targetCurve_;
	AQLString targetCurveMktSuffix_;
	AQLString dfCurveName_;

	ListOfBuiltCurves listOfCurvesAlreadyBuilt_;
	TargetCurveAliasList targetCurveAliasList_;

	// Instrument Settings: isFuturesUse, isFRAuse, ratePriority, isSwapTenorAdjust, generateForwardsFromSwapsOnly
	CurveProperties & curveProperties_;

    // Curve Info
    AQLString curveCollection_;  // or curveID
    AQLString curveIndex_;       // or marketName
};

// Curve resultsObj container helper for the CurveDataProvider class
class CalibrationResults
{
	public:
	
	CalibrationResults() : interpolationObj_( nullptr ) {};
	~CalibrationResults() {};

	CalibrationResults( LACurveStaticDataHolder & staticDataObj, const AQLDate& asOfDate );

	CalibrationResults( LACurveStaticDataHolder & staticDataObj,
						const AQLDate & asOfDate,
						DiscountFactors & dfResults,
						DoubleMatrix& fwdStartEndDatesAsTerms,
						DoubleArray& fwdRates );

	void resetAll();
	void resetForwardsAndDiscFactors();

	// Interpolation
	std::shared_ptr<InterpolationDataHolder> interpolationObj_;
	
	bool						joinDateGiven_;
	bool						calculateJoinDate_;
	
	bool						isHybridDiscountFactorInterpolation_;
	bool						isHybridSwapsInterpolation_;
	bool						isHybridFuturesAndFRAsInterpolation_;
	bool						doesCurveUseHybridInterpolation_;
	
	// Discount Factors and Forwards
	AQLDate						asOfDate_;
	DiscountFactors				dfResults_;
	DoubleMatrix				fwdStartEndDatesAsTerms_;
	DoubleArray					fwdRates_;
	
	// Risk
	DoubleMatrix				inverseJacobian_;
	DoubleMatrix				jacobian_;
	DoubleArray					jacobianStateVariables_;
};

// Curve Calibration Properties Data helper for the calibrateModel methods
class CurveProperties
{
	public:

	// Default Constructor / Destructor
	CurveProperties();
	~CurveProperties() {};

	// Constructor
	CurveProperties( CurveCalibrationData& curveData );

	// Constructor
	CurveProperties( LACurveStaticDataHolder & staticDataObj );

	// Update Swap Curve State Variables
	void updateSwapCurveStateVariables( LACurveStaticDataHolder & staticDataObj, CurveMarketDataHolder & mktDataObj );

	bool useFutures_;
	bool useFRAs_;
	const AQLStringVector* ratePriorityStringVector_;

	// Adjust swap instrument tenors to allow mixed floating index frequencies in a single curve
	// e.g. to allow both 3M and 6M Libor in AUD curve calibration
	// Also known as isSwapTenorAdjust_
	bool useTenorBasisSwaps_;
	
	// Prioritize Forwards from Swap Instruments when they clash with other instruments such as Futures and FRAs
	// Also Known as generateForwardsFromSwapsOnly
	bool prioritizeSwaps_;

	// Imply Forwards when calibrating curves
	bool implyForwards_;

	// State Variables
	StateVariableEnum stateVariableFRAs_;
	StateVariableEnum stateVariableFutures_;
	StateVariableEnum stateVariableSwaps_;
	StateVariableEnum stateVariableTenorBasis_;
};

// Data container helper for the calibrateCurve method
class InterpolationDataHolder
{
	public:
	
	// Default Construtor / Destructor
	InterpolationDataHolder();
	~InterpolationDataHolder() {};

	// Alternative Constructor
	InterpolationDataHolder(LACurveStaticDataHolder & staticDataObj);

	// Clear and Reset InterpolationDataHolder member variables
	void reset();

	AQLInterpolationBase*		interpolatorForDiscountFactors_;
	AQLInterpolationBase*		interpolatorForSwaps_;
	AQLInterpolationBase*		interpolatorForFuturesAndFRAs_;

	AQLString					interpolationStringDiscountFactors_;
	AQLString					interpolationStringSwaps_;

	AQLCoreFunctionHolder		fh_DiscountFactors_;
	AQLCoreFunctionHolder		fh_Swaps_;
	AQLCoreFunctionHolder		fh_FuturesAndFRAs_;

	AQLDate						interpolationJoinDate_;
	double						interpolationJoinDateAsDouble_;	
};

struct LACurveMarketData
{
	AQLDate asOfDate_;

	bool isOISMarketData_;

	bool isO_N_;
	bool isT_N_;
	
	MoneyMarketData moneyMarket_;
	
	std::vector<AQLObject*> libor_;
	std::vector<AQLObject*> swap_;
	std::vector<AQLObject*> future_;
	std::vector<AQLObject*> mpc_swaps_;
	std::vector<AQLObject*> fra_;
	std::vector<AQLObject*> tenorswap_;

	unsigned int libor_size_;
	unsigned int swap_size_;
	unsigned int future_size_;
	unsigned int mpc_swap_size_;
	unsigned int fra_size_;

	bool areSwapsForwardStarting_;
	AQLDate firstSwapMaturityDate_;
};

// Get the First Swap Maturity Date
AQLDate getFirstSwapMaturityDate( const LACurveMarketData & mktData );

// Get the Spot Date for the Libor Cash Deposit Instruments
AQLDate getLiborSpotDate( const LACurveMarketData & mktData );

// Check if we have been given OIS Market Data
bool isOISMarketData( const LACurveMarketData & mktData );

// Check if Swaps are Forward Starting
bool isSwapForwardStarting( const LACurveMarketData & mktData );

class CurveMarketDataHolder
{
	public:

	// Default Constructor / Destructor
	CurveMarketDataHolder() {};
	~CurveMarketDataHolder() {};

	// Main Constructor
	CurveMarketDataHolder( LACurveStaticDataHolder & staticDataObj, const AQLDate & asOfDate );
	
	// Populate and Group Market Data by Instrument
	void groupMarketDataByInstrument( LACurveStaticDataHolder & staticDataObj, const AQLDate& asOfDate );

	// Check if the targetCurve is a STD Swap Curve
	bool isTargetSwapCurve( LACurveStaticDataHolder & staticDataObj ) const;

	const AQLDataMultiReference* marketDataSharedObject_;
	AQLString targetCurve_;
	AQLString targetCurveMktSuffix_;

	// Raw Market Data - Ungrouped and Unsorted
	std::vector<AQLObject*> rawMarketData_;

	// Market Data Container by Instrument
	LACurveMarketData mktData_;
	AQLDate asOfDate_;
};

class LiborIndex
{
	public:
	
	// Default Constructor / Destructor
	LiborIndex() {};
	~LiborIndex() {};

	// Main Constructor
	LiborIndex( CurveMarketDataHolder & mktDataObj );
	
	AQLDate							spotDate_;
	AQLString						frequency_;
	AQLString						tenor_;
	const AQLPriceDataCalendar *		fixingCalendar_;
	const AQLPriceDataSlidingRule *	fixingBusDayAdj_;
	const AQLPriceDataDayCount *		daycount_;
	etrading::DayCountEnum			daycountEnum_; 

	// Curve & Swap Spot Date
	AQLDate							curveSpotDate_;
	AQLPriceDataDayCount				daycountAct365_;
};

class FuturesInstruments
{
	public:

	// Default Constructor/Destructor
	FuturesInstruments();
	~FuturesInstruments() {};
	
	// Main Constructor
	FuturesInstruments( LACurveStaticDataHolder & staticDataObj, CurveMarketDataHolder & mktDataObj, LiborIndex & liborIndex );

	// Get ForwardRate Object for Backwards Compatibility
	std::vector<ForwardRate> getForwardRates() const;

	// Futures Start-, EndDates and Quotes
	ForwardQuotes marketQuotes_;
	
	FuturesTypeEnum type_;
	bool areAllFuturesContiguous_;
	bool isEOMRoll_;
	AQLString rollConvention_;
	AQLPriceDataDayCount daycount_;
	AQLPriceDataDayCount daycountAct365_;
		
	// Is the Convexity Adjustment Provided as a volatility instead of a price, an alais for as 'useconvexadj' the inverse i.e. quoted as price
	// FALSE = use convexity price given, TRUE = imply from model using volatility
	bool isConvexityQuotedAsVol_;
	
	// Include Swaps Before MPC or Central Bank Swaps, also known as 'smoothShortEnd'
	// TRUE = include swaps before MPC Swaps, FALSE = exclude swaps before MPC swaps
	bool includeSwapsBeforeMPCSwaps_;

	// Insert Linear Points Between Calibration Points to Simulate Linear Interpolation / Tension
	bool applyTensionFutures_;
	unsigned int tensionDayGap_;

	// Required for Swaps Calibration
	AQLDate lastStartDate_;
};

class SwapInstruments
{
	public:

	// Default Constructor/Destructor
	SwapInstruments();
	~SwapInstruments() {};

	// Main Constructor
	SwapInstruments( LACurveStaticDataHolder & staticDataObj, CurveMarketDataHolder & mktDataObj );

	// Size Methods
	size_t size() const							{ return spotDate_.size(); }
	
	// Resize Vector Member Data
	void resize( const size_t newSize );

	// Trade Conventions 
	DateVector									spotDate_;
	std::vector<const AQLPriceDataCalendar*>		calendar_;
	std::vector<const AQLPriceDataSlidingRule*>	busDayAdj_;
	std::vector<bool>							isEOMRoll_;
	std::vector<AQLString>						rollConvention_;
	AQLPriceDataDayCount							daycountAct365_;

	// Fixed Leg
	std::vector<const AQLPriceDataDayCount*>		daycountFixed_;
	std::vector<AQLString>						frequencyFixed_;

	// Float Leg
	std::vector<const AQLPriceDataDayCount*>		daycountFloat_;
	std::vector<AQLString>						frequencyFloat_;
	std::vector<AQLString>						compoundFrequencyFloat_;
	std::vector<int>							numberOfCompoundPeriodsFloat_;
	std::vector<AQLString>						liborIndexFrequency_;
	std::vector<AQLString>						liborIndexTenor_;
};

class TenorBasisInstruments
{
	public:

	// Default Constructor
	TenorBasisInstruments();

	// Default Destructor
	~TenorBasisInstruments();
	
	// Main Constructor
	TenorBasisInstruments( LACurveStaticDataHolder & staticDataObj, CurveMarketDataHolder & mktDataObj, LiborIndex & liborIndex, CalibrationResults & resultsObj );

	// Populate Tenor Basis Spread Market Data
	void populateSpreadMarketData( LACurveStaticDataHolder & staticDataObj, CurveMarketDataHolder & mktDataObj, LiborIndex & liborIndex, CalibrationResults & resultsObj );

	AQLInterpolationBase*			spreadInterpolator_;
	DoubleVector					spreads_;

	bool							isTargetLeg1_;
	bool							applySpreadLeg2_;

	AQLString						leg1Frequency_;
	AQLString						leg2Frequency_;
	AQLString						againstLegFrequency_;

	AQLPriceDataDayCount *			daycount_;
	AQLPriceDataDayCount				daycountAct365_;
	AQLPriceDataCalendar *			calendar_;
	AQLPriceDataSlidingRule *		busDayAdj_;

	bool							isRollEOM_;
	AQLString						rollConv_;
};

struct LARatePriority
{
	bool isFuturePriority_;
	bool isSwapPriority_;
};

class SwapCashflows
{
	public:

	SwapCashflows() {};
	~SwapCashflows() {};

	// Main Constructor
	SwapCashflows( LACurveStaticDataHolder & staticDataObj, CurveMarketDataHolder & mktDataObj, SwapInstruments & swaps, TenorBasisInstruments & tenorBasisSwaps, CalibrationResults & resultsObj );

	// *** CLASS METHODS ***

	// Rate Priority - Called within Main Constructor
	void updateRatePriority( LACurveStaticDataHolder & staticDataObj, CurveMarketDataHolder & mktDataObj );

	// Instrument Dates & Rates - Called within Main Constructor
	void updateInstrumentDatesAndRates( LACurveStaticDataHolder & staticDataObj, CurveMarketDataHolder & mktDataObj, SwapInstruments & swaps, TenorBasisInstruments & tenorBasisSwaps, CalibrationResults & resultsObj );

	// Get the Swap Fixed Accrual Periods and Payment Dates
	void updateSwapFixedCashflows( CurveMarketDataHolder & mktDataObj, SwapInstruments & swaps );

	// Get the Swap Fixed Accrual Periods and Payment Dates
	void updateSwapFloatCashflows( CurveMarketDataHolder & mktDataObj, SwapInstruments & swaps );

	// Get the Swap Fixed Accrual Periods and Payment Dates
	void updateTenorBasisSwapFloatCashflows( LACurveStaticDataHolder & staticDataObj, CurveMarketDataHolder & mktDataObj, SwapInstruments & swaps, TenorBasisInstruments & tenorBasisSwaps );

	// *** MEMBER VARIABLES ***

	// Swap Info
	size_t						scheduleSize_;
	bool						areSwapsForwardStarting_;
	LARatePriority				ratePriority_;

	// Swap Instrument Schedule
	DateVector					swapStartDates_;
	DateVector					swapEndDates_;
	DateVector					swapEndDatesUnadjusted_;
	DoubleVector				swapRates_;

	// Fixed Schedule Parameters
	DoubleArray					fixedStartDatesAsTerms_;
	std::vector<DoubleArray>	fixedCoupons_;
	std::vector<DateVector>		fixedCashflowPaymentDates_;
	std::vector<DoubleArray>	fixedCashflowPaymentDatesAsTerms_;
	std::vector<DoubleArray>	fixedAccrualPeriods_;

	// Float Schedule Parameters
	DoubleArray					floatStartDatesAsTerms_;
	std::vector<DoubleArray>	floatAccrualPeriods_;
	std::vector<DoubleArray>	floatCoupons_;
	std::vector<DateVector>		floatCashflowPaymentDates_;
	std::vector<DoubleArray>	floatCashflowPaymentDatesAsTerms_;
		
	// Fixing Schedule Parameters
	std::vector<AQLString>		fixingLags_;
	std::vector<DoubleArray>	fixingStarts_;
	std::vector<DoubleArray>	fixingEnds_;
	std::vector<DateVector>		fixingStartDates_;
	std::vector<DateVector>		fixingEndDates_;
	std::vector<DoubleArray>	fixingAccrualPeriods_;

	// Tenor Basis Float Schedule Parameters
	std::vector<DoubleArray>	tenorSwapAccrualPeriods_;
	std::vector<DoubleArray>	tenorSwapCoupons_;
	std::vector<DateVector>		tenorSwapCashflowPaymentDates_;
	std::vector<DoubleArray>	tenorSwapCashflowPaymentDatesAsTerms_;
};

// Check if Discount Factor interpolation is using Hybrid / Mixed Interpolation
bool isHybridDiscountFactorInterpolation( CalibrationResults & resultsObj );

// Check if Swap interpolation is using Hybrid / Mixed Interpolation
bool isHybridSwapInterpolation( CalibrationResults & resultsObj );

// Check if Futures / FRA interpolation is using Hybrid / Mixed Interpolation
bool isHybridFuturesAndFRAsInterpolation( CalibrationResults & resultsObj );

// Method to check if we are using mixed hybrid interpolation schemes, note resultsObj contains the interpolation results
bool isCurveUsingHybridInterpolation( CalibrationResults & resultsObj );

// Method to set the join date for mixed / hybrid interpolation methods
void importHybridInterpolationJoinDateDefaults( LACurveStaticDataHolder & staticDataObj, CurveMarketDataHolder & mktDataObj, LiborIndex & liborIndex, CalibrationResults & resultsObj );

// Method to rebase discount factors from the swap spotDate to the curve AsOfDate
void rebaseDiscountFactorsToCurveAsOfDate( LACurveStaticDataHolder & staticDataObj, CurveMarketDataHolder & mktDataObj, LiborIndex & liborIndex, const SwapInstruments & swaps, SwapCashflows & swapCashflows, CalibrationResults & resultsObj );

// Method to Bootstrap Libor Cash Deposits
void bootstrapLibors( LACurveStaticDataHolder & staticDataObj, CurveMarketDataHolder & mktDataObj, LiborIndex & liborIndex, CalibrationResults & resultsObj );

// Bootstrap FRAs and Set Results to Object Pool
void bootstrapFRAs( LACurveStaticDataHolder & staticDataObj, CurveMarketDataHolder & mktDataObj, LiborIndex & liborIndex, CalibrationResults & resultsObj );

// Bootstrap FRAs and Set Results to Object Pool
void bootstrapFutures( LACurveStaticDataHolder & staticDataObj, CurveMarketDataHolder & mktDataObj, LiborIndex & liborIndex, FuturesInstruments & futures, CalibrationResults & resultsObj );

// Swap Solver State Variable Struct
struct SwapStateVariables
{
	DoubleArray terms_;
	DoubleArray rateTimeValues_;
};

// Class to Calibrate Swap Instruments
class SwapCalibration
{

public:

	// Default Constructor / Destructor
	SwapCalibration();
	~SwapCalibration() {};

	// Main Constructor
	SwapCalibration( LACurveStaticDataHolder & staticDataObj,
					 CurveMarketDataHolder & mktDataObj,
					 const LiborIndex & liborIndex,
					 const FuturesInstruments & futures,
					 const SwapInstruments & swaps,
					 SwapCashflows & swapCashflows,
					 CalibrationResults & resultsObj );
	
	// Import Calibration Parameters
	void importCalibrationParameters( LACurveStaticDataHolder & staticDataObj,
									  CurveMarketDataHolder & mktDataObj,
									  CalibrationResults & resultsObj );

	// Fast Rebuild - Use Previous Solution for Calibration Initial Guess
	void applyPreviousSolution( LACurveStaticDataHolder & staticDataObj,
							    CurveMarketDataHolder & mktDataObj,
							    const SwapCashflows & swapCashflows,
							    CalibrationResults & resultsObj );
	
	// Method to determine the optimal Futures/Swaps join date when using Mixed/Hybrid Interpolation
	// Optional: useNetwonRaphsonMinimizer - Default = true (use Newton-Raphson Minimizer instead of date search)
	// Optional: searchIntervalDays - Default = 1 days (search every day = 1 (slow) )
	void optimizeFuturesJoinDateWithSwaps( LACurveStaticDataHolder & staticDataObj,
									       CurveMarketDataHolder & mktDataObj,
									       const LiborIndex & liborIndex,
									       const FuturesInstruments & futures,
									       const SwapInstruments & swaps,
									       CalibrationResults & resultsObj,
									       const bool useNetwonRaphsonMinimizer = true,
									       const size_t searchIntervalDays = 5 );

	// Price Swap Calibration Instruments
	DoubleArray	calculateSwapPVs( LACurveStaticDataHolder & staticDataObj,
								  CurveMarketDataHolder & mktDataObj,
								  const LiborIndex & liborIndex,
								  const SwapInstruments & swaps,
								  const SwapCashflows & swapCashflows,
								  SwapCalibration & swapCalibration,
								  CalibrationResults & resultsObj );
	
	// Price Swap Calibration Instruments, where SwapPVs is the result (input/output)
	void calculateSwapPVs( AQLInterpolationBase &interpolationForForwardsRates,
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
						   const DoubleMatrix &floatAccrualPeriods = DoubleMatrix(),
						   const DoubleMatrix &fixingStarts = DoubleMatrix(),
						   const DoubleMatrix &fixingEnds = DoubleMatrix(),
						   const DoubleMatrix &fixingAccrualPeriods = DoubleMatrix() );

	// Store Calibration Results: State Variables, Jacobian and Inverse Jacobian
	void storeStateVariableAndJacobianResults( CurveMarketDataHolder & mktDataObj, CalibrationResults & resultsObj );

	// Store Calibration Results: Discount Factors
	void SwapCalibration::storeDiscountFactorsAndForwardRates( LACurveStaticDataHolder & staticDataObj,
															   CurveMarketDataHolder & mktDataObj,
															   const LiborIndex & liborIndex,
															   const SwapInstruments & swaps,
															   const SwapCashflows & swapCashflows,
															   CalibrationResults & resultsObj );

	// Get Accessors
	double resultTolerance() const											{ return resultTolerance_;					}
	double gradientTolerance() const										{ return gradientTolerance_;				}
	double shiftSize()	const												{ return shiftSize_;						}
	double maxIterations()	const											{ return maxIterations_;					}
	bool fastConvergenceCheck() const										{ return fastConvergenceCheck_;				}
	bool fastRebuild() const												{ return fastRebuild_;						}
	bool previousSolutionAvailable() const									{ return previousSolutionAvailable_;		}	
	DoubleMatrix previousInverseJacobian() const							{ return previousInverseJacobian_;			}
	SwapStateVariables stateVariables() const								{ return stateVariables_;					}
	DoubleArray stateVariableTerms() const									{ return stateVariables_.terms_;			}
	DoubleArray stateVariableValues() const									{ return stateVariables_.rateTimeValues_;	}
	bool isSwapCurveSelfDiscounting() const									{ return isSwapCurveSelfDiscounting_;		}
	size_t preCalibrationDFSize() const										{ return preCalibrationDFSize_;				}		
	double initialBumpSize() const											{ return initialBumpSize_;					}	
	SwapCurveDiscountFactors swapCurveDiscountFactors() const				{ return swapCurveDiscountFactors_;			}
	solvers::MultiVariateSolverResults calibrationResults() const			{ return calibrationResults_;				}

	// Set Accessors
	void stateVariables( const SwapStateVariables & stateVariables ) { stateVariables_ = stateVariables; }
	void stateVariableTerms( const DoubleArray & terms );
	void stateVariableValues( const DoubleArray & rateTimeValues );
	void stateVariables( const DoubleArray & terms, const DoubleArray & rateTimeValues );
	void isSwapCurveSelfDiscounting( const bool isSwapCurveSelfDiscounting ) { isSwapCurveSelfDiscounting_ = isSwapCurveSelfDiscounting; };
	void preCalibrationDFSize( const size_t preCalibrationDFSize ) { preCalibrationDFSize_= preCalibrationDFSize; }
	void initialBumpSize( const double & initialBumpSize ) { initialBumpSize_ = initialBumpSize; }

	
private:

	// Member Data
	double resultTolerance_;			// epsilson, eps
	double gradientTolerance_;			// Newton-Raphson Slope Tolerance
	double shiftSize_;					// Delta or Shift-Size
	double maxIterations_;				// Max Iteration Loops
	bool fastConvergenceCheck_;			// Fast Convergence Check to Optimize Performance
	bool fastRebuild_;					// Reuse previous solution for fast rebuild
	
	bool previousSolutionAvailable_;
	DoubleMatrix previousInverseJacobian_;
	bool isSwapCurveSelfDiscounting_;
	size_t preCalibrationDFSize_;

	// The bump size to apply to the initial solution estimate, before the main Newton-Raphson loop
	// Set in the constructor to 1.0e-5;
	double initialBumpSize_;

	// Swap Solver Results
	SwapStateVariables stateVariables_;

	// Swap Curve OIS Discount Factors and Interpolator
	SwapCurveDiscountFactors swapCurveDiscountFactors_;

	// Solver Results
	solvers::MultiVariateSolverResults calibrationResults_;
};

///////////////////////////////////////////////////////////////////////
/*! 
    @brief Class to calc IR DiscoutFactor
*/
class CurveCalibration : public AQLCoreProcedure
{
public:

    // constructor	
    CurveCalibration();

	// destructor	    
    virtual ~CurveCalibration();
    
	// Check function for this class ID
    virtual bool                isTypeOf(function_t id) const;
    
	// Make copy(clone) of this class
    virtual AQLCoreFunctionBase* clone() const;
   	
	// register dataValues that this class uses
	virtual void				registerData(AQLPriceDataManager& dm) const;
	
	// Return this class type
    virtual function_t          getType() const;
    
	
	// Helper Methods for the Curve Calibration 'CalibrateModel' and 'calibrateCurve' Routines
	// ********************************************************************

	// Get Target Curve & Target Curve Market Data Suffix
	void getTargetCurveAndMarketDataSuffix( AQLString & targetCurve, AQLString & marketDataSuffix, CurveCalibrationData & curveData ) const;
	
	// Get Object Pool Target Curve Alias List
	TargetCurveAliasList getTargetCurveAliasList( LACurveStaticDataHolder & staticDataObj ) const;
	
	// Get Object Pool List of Curves Built
	ListOfBuiltCurves getObjectPoolCurveBuildList( LACurveStaticDataHolder & staticDataObj ) const;

	// Clear Object Pool List of Curves Built
	void clearObjectPoolCurveBuildList( LACurveStaticDataHolder & staticDataObj ) const;

	// Update Object Pool List of Curves Built *** For Target Curve ONLY ***
	void updateObjectPoolCurveBuildListForTargetCurve( LACurveStaticDataHolder & staticDataObj ) const;

	// Update Object Pool List of Curves Built *** For Target Curve ONLY ***
	void updateObjectPoolCurveBuildListForTargetCurve( LACurveStaticDataHolder & staticDataObj, const AQLString & targetCurve ) const;

	// Update Object Pool List of Curves Built *** For Alias Curves ONLY ***
	void updateObjectPoolCurveBuildListForAliasCurves( LACurveStaticDataHolder & staticDataObj ) const;

	// Update Object Pool List of Curves Built *** For Target and Alias Curves ***
	void updateObjectPoolCurveBuildList( LACurveStaticDataHolder & staticDataObj ) const;

	// Update Object Pool Curve Results
	void updateObjectPoolCurveResults( LACurveStaticDataHolder & staticDataObj,
									   const CalibrationResults& resultsObj,
									   const AQLString & curveSuffix = "" ) const;
	
	// Update Object Pool Curve Conventions and Results
	void updateObjectPoolCurveConventionsAndResults( LACurveStaticDataHolder & staticDataObj,
													 CurveMarketDataHolder & mktDataObj,
													 const CalibrationResults& resultsObj ) const;

	// Update Object Pool Curve Conventions, Results and Build List
	void updateObjectPoolResultsAndBuildList( LACurveStaticDataHolder & staticDataObj,
											  CurveMarketDataHolder & mktDataObj,
											  const CalibrationResults& resultsObj ) const;

	// Check if Curve Updates are Enabled
	bool areCurveUpdatesEnabled( LACurveStaticDataHolder & staticDataObj ) const;

	// Get the DF Curve Name
	AQLString getDFCurveName( LACurveStaticDataHolder & staticDataObj, const AQLString & curveSuffix = "" ) const;
	
	// Check if Curve Exists and Contains Valid Data
	bool doesValidCurveExist( const AQLString & targetCurve, LACurveStaticDataHolder & staticDataObj ) const;

	// Calibrate DFCurveName
	AQLString getDFCurvename( LACurveStaticDataHolder & staticDataObj, const AQLString & targetCurve, const AQLString & curveSuffix ) const;

	// ********************************************************************

	// Virtual Method to Calibrate the Curve Model
    virtual void				calibrateModel( const AQLDate& asOfDate, AQLObject& dataObject, const AQLDataProcedure& laAttributeData ) const;

    // Virtual Method to Calibrate the Curve Model with Curve Collection and Curve Index Info
    virtual void				calibrateOISAndSwapCurve( const AQLDate& asOfDate,
                                                          AQLObject& dataObject,
                                                          const AQLDataProcedure& laAttributeData,
                                                          const AQLString & curveCollection,
                                                          const AQLString & curveIndex ) const;

    // Calibrate Standard Swap Curve - CurveCollection and CurveIndex part of of StaticDataObj
    static void                 calibrateSwapCurve( LACurveStaticDataHolder & staticDataObj,
													CurveMarketDataHolder & mktDataObj,
										            CalibrationResults & resultsObj );
	
	// Calibrate OIS Swap Curve - CurveCollection and CurveIndex part of of StaticDataObj
	static void					calibrateOISCurve( LACurveStaticDataHolder & staticDataObj,
												   CurveMarketDataHolder & mktDataObj,
										           CalibrationResults & resultsObj );

	// Calculate the effective forward rate using OIS curve
	static double				calcEffectiveOISRate( DoubleVector& startTerms,
													  DoubleVector& endTerms,
													  const etrading::OISCompoundingEnum& swapAveragingMethodEnum,
							                          const etrading::OISLongTermInstrumentsEnum& longTermConvEnum,
													  const AQLString& longTermGen,
													  const AQLDate& spotDate,
													  const AQLDate& startDate,
													  const AQLDate& endDate,
													  const AQLPriceDataDayCount* dc,
													  const AQLPriceDataCalendar* cal,
													  const AQLInterpolationBase* interpolateByZeroRate,
													  double terms_interval,
													  bool isLogDF = false );
	
    static double				calcAverageRate( DoubleVector& startterms,
											     DoubleVector& endterms,
											     const etrading::OISCompoundingEnum& averagingMethodEnum,
											     const AQLDate& spotDate,
											     const AQLDate& startDate,
											     const AQLDate& endDate,
											     const AQLPriceDataDayCount* dc,
											     const AQLPriceDataCalendar* cal,
											     const AQLInterpolationBase* interpolateByZeroRate,
											     double terms_interval,
											     bool isLogDF = false );

	static double                calcArithmeticAverageRate( const DoubleVector& startterms,
											                const DoubleVector& endterms,
											                const AQLInterpolationBase* interpolateByZeroRate,
											                const double wholeterm,
											                bool isLogDF = false );

	/* @brief  Helper function to fetch all the curve names which the global curve engine is configured to build
	*
	*  @param[in]	curveData	EntityPool properties object
	*  @returns	A string vector specifying all of the curve names
	*/
	static AQLStringVector		getGlobalEngineCurveNames( const CurveCalibrationData& curveData );

	// Calibrate swap and OIS curves using dual bootstrapping technique
	void						dualbootstrap(const AQLDate& asOfDate, AQLObject& calibrationDataObject, const AQLDataProcedure& att) const;

	// Calibrate yield curves through a yield curve engine
	void						buildEngineCurves(const AQLDate& asOfDate, AQLObject& calibrationDataObject, const AQLDataProcedure& att, bool includeBasisCurves = true) const;

	// Solve for effective OIS rate given a market rate
	static double				solveOISRate( const AQLDate& startdate,
											  const AQLDate& enddate,
											  const AQLPriceDataDayCount& dc,
											  const AQLPriceDataCalendar& cal,
											  double market_rate );

	// Calculate average rate out of historical rates
	static double				getAverageRateFromHistRates( const AQLDate& startdate,
												             const AQLDate& enddate,
												             const AQLPriceDataDayCount& dc,
												             const AQLPriceDataCalendar& cal,
												             double market_rate,
												             AQLObject* marketData,
												             const AQLDate& asOfDate,
												             bool istodayrateexist,
												             double todayffrate );


	// Store curve conventions at the end of swap curve calibration
	static void					setCurveConvention( AQLObjectHolder& objHolder,
												    std::vector<AQLObject*>& mktData,
												    const AQLString& curveName );

	// Store curve conventions at the end of swap curve calibration
	static void					setCurveConvention( LACurveStaticDataHolder & staticDataObj,
												    std::vector<AQLObject*>& mktData,
												    const AQLString& curveName );

	// Calibrate OIS curve's discount factors using market OIS swap rates
	static void					calcOISDFBySwapRates( DoubleArray& yields,
													  DoubleArray& grid,
													  DoubleArray& terms_grid, 
													  DoubleArray& terms_interval,
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
													  const size_t& size_mpc_swaps,
													  bool isEOMRoll,
													  bool useShortEndSwaps = false,
													  size_t shortTermSwapSize = 0 );

	// Calibrate OIS curve's discount factors using market OIS swap rates
	static void					calcOISDFBySwapRates( DoubleArray& yields,
													  DoubleArray& grid,
													  DoubleArray& terms_grid, 
													  DoubleArray& terms_interval,
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
													  const size_t& size_mpc_swaps,
													  bool isEOMRoll,
													  bool useShortEndSwaps = false,
													  size_t shortTermSwapSize = 0 );

protected:

    // disable copy constructor - not sure why this was disabled ?
	// Think the original author wanted users to create a new class instance and reset the marketdata instead of copying it 
    CurveCalibration(const CurveCalibration& p) = delete;

	static double                solveOISRateS(const AQLDate& startdate,
											   const AQLDate& enddate,
											   const AQLPriceDataDayCount& dc,
											   const AQLPriceDataCalendar& cal,
											   double market_rate,
											   const std::map<AQLDate, double> &onforward_map);
		
	static double                calcSettleRate(const AQLDate& startdate,
											    const AQLDate& enddate,
											    const AQLPriceDataDayCount& dc,
											    const AQLPriceDataCalendar& cal,
											    double onforward_rate);

	static double                calcSettleRates(const DateVector& startdates,
											     const DateVector& enddates,
											     const DoubleVector& onforward_rates,
											     const AQLPriceDataDayCount& dc);
};


