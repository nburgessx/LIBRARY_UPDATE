#pragma once

#ifdef __GNUG__
#pragma interface
#endif


//hishida vannavolga
#ifndef BSVALUEMODEL
#define BSVALUEMODEL "BSValueModel"
#endif
#ifndef VVVALUEMODEL
#define VVVALUEMODEL "VannaVolgaValueModel"
#endif



// plain vanilla option 
#define FN_PLAINVANILLAVALUE	10050
#define FN_PLAINVANILLAVALUE_STR	"fn_plainvanillavalue"
// fx option
#define FN_FXOPTIONVALUE	10051
#define FN_FXOPTIONVALUE_STR	"fn_fxoptionvalue"
// fx digitaloption
#define FN_FXDIGITALOPTIONVALUE	10052
#define FN_FXDIGITALOPTIONVALUE_STR	"fn_fxdigitaloptionvalue"
// fx digital call spread option
#define FN_FXDIGITALCALLSPREADOPTIONVALUE	10053
#define FN_FXDIGITALCALLSPREADOPTIONVALUE_STR	"fn_fxdigitalcallspreadoptionvalue"
// fx knockout rebate value
#define FN_FXKNOCKOUTREBATEVALUE	10054
#define FN_FXKNOCKOUTREBATEVALUE_STR	"fn_fxknockoutrebatevalue"
// fx single barrier value
#define FN_FXSINGLEBARRIEROPTIONVALUE	10055
#define FN_FXSINGLEBARRIEROPTIONVALUE_STR	"fn_fxsinglebarrieroptionvalue"
// fx single barrier value
#define FN_FXDIGITALCALLSPREADSINGLEBARRIEROPTIONVALUE	10056
#define FN_FXDIGITALCALLSPREADSINGLEBARRIEROPTIONVALUE_STR	"fn_fxdigitalcallspreadsinglebarrieroptionvalue"
// ir cap floor option value
#define FN_IR_CAPFLOOROPTIONVALUE	10057
#define FN_IR_CAPFLOOROPTIONVALUE_STR	"fn_ir_capflooroptionvalue"
// ir swaption value
#define FN_IR_SWAPTIONVALUE		10058
#define FN_IR_SWAPTIONVALUE_STR	"fn_ir_swaptionvalue"
// ir swaption value
#define FN_IR_SWAPTIONVALUEFROMCASHFLOW		10061
#define FN_IR_SWAPTIONVALUEFROMCASHFLOW_STR	"fn_ir_swaptionvaluefromcashflow"



#include "AQLDataValuation.h"
#include "LAMathPlainVanillaEntity.h"
#include "AQLFunctionBase.h"
#include "LABlackScholesCalc.h"
#include "AQLDate.h"
#include "LABlackScholesBaseFunc.h"
#include "AQLPriceDataCalendar.h"
#include "AQLPriceDataSlidingRule.h"
#include "AQLPriceDataDayCount.h"

class AQLInterpolationBase;
/*!
		@brief cache class for performance up
*/
class LALinearRatesOptionValueDataProvider : public AQLDataProvider
{
public:
	LALinearRatesOptionValueDataProvider();
	
	virtual ~LALinearRatesOptionValueDataProvider();
	
	
	LAMathPlainVanillaEntity* mpvanilla;
	std::vector< std::vector<LABlackScholesBase* > > mAnalyticMethod;
	std::vector< std::vector<LABlackScholesBase* > > mPayoffMethod;
	std::vector< std::vector<AnalyticParam* > > mParam;
	bool buysell;
	double unit;
	const AQLFunctionBase* mVolfunc;
	AQLDate mAsofDate;
	AQLDate mValueDate;
	AQLDate mMaturityDate;
	AQLDate mDeliveryDate;
	AQLDate mTradeDate;
	bool mIsStillAlive; // true for options that are still alive on or after the expiry date
	double mMargin;
	AQLString mnumerairecur;
	AQLString mpvcur;
    AQLString mPremiumCurrency;
	bool mIsAnalyticalRisk;
	double mShiftValForRisk;
	bool mIsDiffForRisk;
	AQLString mAnalyticalRiskType;
	bool mIsAsofAfterMaturity;
	bool mIsPayOffCalculateAfterMaturity;
	unsigned int mCashletSize;
	AQLPriceDataDayCount mBlackDayCount;
	
	bool mIsPremAdjust;
	AQLDate mPremPayDate;
	double mPremAmount;
	bool mIsFWDInter;
	AQLInterpolationBase *mpFWDInter;
	DoubleVector mStartTerms;
	double mPremiumValue;
	double mOptionValue;
	bool mIsAddFwdPremPV;
	double mCashSettlementAmount;
	double mCashSettlementValue;
	AQLDate mCashSettlementPayDate;
	bool mIsCashSettlementAdjust;
	AQLString mCashSettlementCurrency;

	//hishida vannavolga
	AQLString mValueModel;

	
};

class LAPriceFXOptionValueDataProvider : public LALinearRatesOptionValueDataProvider
{
public:
	LAPriceFXOptionValueDataProvider();
	
	virtual ~LAPriceFXOptionValueDataProvider();
	AQLString mdomcur;
	AQLString mforcur;
	AQLString mfxcur;
	bool mIsLiborDFForFX;
	AQLDate mSpotDate;
};

/*!
		@brief cache class for performance up
	*/
class LAPriceFXKnockoutRebateValueDataProvider : public LAPriceFXOptionValueDataProvider
{
public:
	LAPriceFXKnockoutRebateValueDataProvider();
	
	virtual ~LAPriceFXKnockoutRebateValueDataProvider();
	
	double rebate;
	double limitval;
	
};


/*!
		@brief cache class for performance up
	*/
class LAPriceIROptionValueDataProvider : public LALinearRatesOptionValueDataProvider
{
public:
	LAPriceIROptionValueDataProvider();
	
	virtual ~LAPriceIROptionValueDataProvider();
	AQLStringVector mUnTenor;
	AQLString mUnFrequency;
	AQLDate mUnStartDate;
	AQLPriceDataDayCount* mpUnDayCount;
	AQLPriceDataSlidingRule* mpUnSlidingRule;
	AQLPriceDataCalendar* mpUnCalendar;
	AQLString mUnCurrency;
	AQLString mDCurveType;
	AQLString mFCurveType;
	AQLPriceDataDayCount* mpUnDayCountOfFixedLeg;
	AQLPriceDataCalendar* mpUnCalendarOfFixedLeg;
	std::vector<AQLDate> mMaturityDates;
	AQLStringVector mFCurveTypes;
	AQLStringVector mDCurveTypes;
	unsigned int mFloatLegNo;
	unsigned int mFixedLegNo;
	bool mIsCalcEquivStrike;
	bool mIsFRateFromCurve;
	bool mIsIRRModel;
	AQLStringVector mUnderlying;
};
