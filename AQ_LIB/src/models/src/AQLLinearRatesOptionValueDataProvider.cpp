#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include <algorithm>

#include "AQLObject.h"
#include "AQLDataBasics.h"
#include "AQLDataVector.h"
#include "AQLDataReference.h"
#include "AQLDataMultiReference.h"
#include "AQLDataInstance.h"
#include "AQLPriceDataManager.h"
#include "AQLObjectPool.h"
#include "AQLCoreTemplateType.h"
#include "AQLMathDefine.h"
#include "AQLPriceDataCalendar.h"
#include "AQLPriceDataSlidingRule.h"
#include "AQLPriceDataDayCount.h"
#include "AQLDataReference.h"
#include "AQLPriceDataFunction.h"
#include "AQLBasic.h"
#include "AQLDateCalculations.h"
#include "AQLCoreComponentManager.h"
#include "AQLLinearRatesOptionValue.h"
#include "AQLMathFXEntity.h"
#include "AQLMathYieldCurve.h"
#include "AQLMathPlainVanillaEntity.h"
#include "AQLLinearRatesOptionValueDataProvider.h"
#include "AQLInterpolationBase.h"

using namespace std;

AQLLinearRatesOptionValueDataProvider::AQLLinearRatesOptionValueDataProvider()
: mpvanilla(0), mAnalyticMethod(std::vector<std::vector<AQLBlackScholesBase *> >(0)),mPayoffMethod(std::vector<std::vector<AQLBlackScholesBase *> >(0)),mParam(std::vector<std::vector<AnalyticParam *> >(0)),mVolfunc(0)
 ,mAsofDate(),mMaturityDate(),mDeliveryDate(),mTradeDate(),mIsStillAlive(false),mMargin(0.0),mnumerairecur(),mpvcur()
 ,mIsAnalyticalRisk(false),mShiftValForRisk(0.0),mIsDiffForRisk(true),mAnalyticalRiskType()
 ,mIsAsofAfterMaturity(false),mIsPayOffCalculateAfterMaturity(false),mCashletSize(1),mBlackDayCount()
 ,mIsPremAdjust(false),mPremPayDate(),mPremAmount(0.0),mIsFWDInter(false),mpFWDInter(0),mPremiumValue(0.0),mOptionValue(0.0),mCashSettlementAmount(0.0),mCashSettlementValue(0.0),mCashSettlementPayDate(),mIsCashSettlementAdjust(false)
{	
}

AQLLinearRatesOptionValueDataProvider::~AQLLinearRatesOptionValueDataProvider()
{
	unsigned int N = mParam.size();
	for (unsigned int i = 0; i < N; i++)
	{
		unsigned int M = mParam[i].size();
		for (unsigned int j = 0; j < M; j++)
		{
			if (mParam[i][j])
				delete mParam[i][j];
		}
		
	}
	if (mpFWDInter)
	{
		delete mpFWDInter;
	}
}

AQLPriceFXOptionValueDataProvider::AQLPriceFXOptionValueDataProvider()
: AQLLinearRatesOptionValueDataProvider()
 ,mdomcur(),mforcur(),mfxcur(),mIsLiborDFForFX(false),mSpotDate()
{	
}

AQLPriceFXOptionValueDataProvider::~AQLPriceFXOptionValueDataProvider()
{
}

AQLPriceFXKnockoutRebateValueDataProvider::AQLPriceFXKnockoutRebateValueDataProvider()
:AQLPriceFXOptionValueDataProvider()
,rebate(0.0),limitval(0.0)
{	
}

AQLPriceFXKnockoutRebateValueDataProvider::~AQLPriceFXKnockoutRebateValueDataProvider()
{
}

AQLPriceIROptionValueDataProvider::AQLPriceIROptionValueDataProvider()
:AQLLinearRatesOptionValueDataProvider()
,mUnTenor(),mUnFrequency(),mUnStartDate(),
mpUnDayCount(0),mpUnSlidingRule(0),mpUnCalendar(0),mUnCurrency(), mDCurveType(), mFCurveType()
,mpUnDayCountOfFixedLeg(0),mpUnCalendarOfFixedLeg(0),mMaturityDates(),mFCurveTypes()
,mFloatLegNo(0), mFixedLegNo(1),mIsCalcEquivStrike(false),mIsFRateFromCurve(true)
,mIsIRRModel(false), mUnderlying()
{	
}

AQLPriceIROptionValueDataProvider::~AQLPriceIROptionValueDataProvider()
{
}

