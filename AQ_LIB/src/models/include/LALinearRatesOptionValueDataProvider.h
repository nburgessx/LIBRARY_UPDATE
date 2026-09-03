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



#include "LADataValuation.h"
#include "LAMathPlainVanillaEntity.h"
#include "LAFunctionBase.h"
#include "LABlackScholesCalc.h"
#include "LADate.h"
#include "LABlackScholesBaseFunc.h"
#include "LAPriceDataCalendar.h"
#include "LAPriceDataSlidingRule.h"
#include "LAPriceDataDayCount.h"

class LAInterpolationBase;
/*!
		@brief cache class for performance up
*/
class LALinearRatesOptionValueDataProvider : public LADataProvider
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
	const LAFunctionBase* mVolfunc;
	LADate mAsofDate;
	LADate mValueDate;
	LADate mMaturityDate;
	LADate mDeliveryDate;
	LADate mTradeDate;
	bool mIsStillAlive; // true for options that are still alive on or after the expiry date
	double mMargin;
	LAString mnumerairecur;
	LAString mpvcur;
    LAString mPremiumCurrency;
	bool mIsAnalyticalRisk;
	double mShiftValForRisk;
	bool mIsDiffForRisk;
	LAString mAnalyticalRiskType;
	bool mIsAsofAfterMaturity;
	bool mIsPayOffCalculateAfterMaturity;
	unsigned int mCashletSize;
	LAPriceDataDayCount mBlackDayCount;
	
	bool mIsPremAdjust;
	LADate mPremPayDate;
	double mPremAmount;
	bool mIsFWDInter;
	LAInterpolationBase *mpFWDInter;
	DoubleVector mStartTerms;
	double mPremiumValue;
	double mOptionValue;
	bool mIsAddFwdPremPV;
	double mCashSettlementAmount;
	double mCashSettlementValue;
	LADate mCashSettlementPayDate;
	bool mIsCashSettlementAdjust;
	LAString mCashSettlementCurrency;

	//hishida vannavolga
	LAString mValueModel;

	
};

class LAPriceFXOptionValueDataProvider : public LALinearRatesOptionValueDataProvider
{
public:
	LAPriceFXOptionValueDataProvider();
	
	virtual ~LAPriceFXOptionValueDataProvider();
	LAString mdomcur;
	LAString mforcur;
	LAString mfxcur;
	bool mIsLiborDFForFX;
	LADate mSpotDate;
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
	LAStringVector mUnTenor;
	LAString mUnFrequency;
	LADate mUnStartDate;
	LAPriceDataDayCount* mpUnDayCount;
	LAPriceDataSlidingRule* mpUnSlidingRule;
	LAPriceDataCalendar* mpUnCalendar;
	LAString mUnCurrency;
	LAString mDCurveType;
	LAString mFCurveType;
	LAPriceDataDayCount* mpUnDayCountOfFixedLeg;
	LAPriceDataCalendar* mpUnCalendarOfFixedLeg;
	std::vector<LADate> mMaturityDates;
	LAStringVector mFCurveTypes;
	LAStringVector mDCurveTypes;
	unsigned int mFloatLegNo;
	unsigned int mFixedLegNo;
	bool mIsCalcEquivStrike;
	bool mIsFRateFromCurve;
	bool mIsIRRModel;
	LAStringVector mUnderlying;
};
