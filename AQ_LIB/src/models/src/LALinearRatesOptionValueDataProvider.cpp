#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include <algorithm>

#include "LAObject.h"
#include "LADataBasics.h"
#include "LADataVector.h"
#include "LADataReference.h"
#include "LADataMultiReference.h"
#include "LADataInstance.h"
#include "LAPriceDataManager.h"
#include "LAObjectPool.h"
#include "LACoreTemplateType.h"
#include "LAMathDefine.h"
#include "LAPriceDataCalendar.h"
#include "LAPriceDataSlidingRule.h"
#include "LAPriceDataDayCount.h"
#include "LADataReference.h"
#include "LAPriceDataFunction.h"
#include "LABasic.h"
#include "LAMathDateCalculations.h"
#include "LACoreComponentManager.h"
#include "LALinearRatesOptionValue.h"
#include "LAMathFXEntity.h"
#include "LAMathYieldCurve.h"
#include "LAMathPlainVanillaEntity.h"
#include "LALinearRatesOptionValueDataProvider.h"
#include "LAInterpolationBase.h"

using namespace std;

LALinearRatesOptionValueDataProvider::LALinearRatesOptionValueDataProvider()
: mpvanilla(0), mAnalyticMethod(std::vector<std::vector<LABlackScholesBase *> >(0)),mPayoffMethod(std::vector<std::vector<LABlackScholesBase *> >(0)),mParam(std::vector<std::vector<AnalyticParam *> >(0)),mVolfunc(0)
 ,mAsofDate(),mMaturityDate(),mDeliveryDate(),mTradeDate(),mIsStillAlive(false),mMargin(0.0),mnumerairecur(),mpvcur()
 ,mIsAnalyticalRisk(false),mShiftValForRisk(0.0),mIsDiffForRisk(true),mAnalyticalRiskType()
 ,mIsAsofAfterMaturity(false),mIsPayOffCalculateAfterMaturity(false),mCashletSize(1),mBlackDayCount()
 ,mIsPremAdjust(false),mPremPayDate(),mPremAmount(0.0),mIsFWDInter(false),mpFWDInter(0),mPremiumValue(0.0),mOptionValue(0.0),mCashSettlementAmount(0.0),mCashSettlementValue(0.0),mCashSettlementPayDate(),mIsCashSettlementAdjust(false)
{	
}

LALinearRatesOptionValueDataProvider::~LALinearRatesOptionValueDataProvider()
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

LAPriceFXOptionValueDataProvider::LAPriceFXOptionValueDataProvider()
: LALinearRatesOptionValueDataProvider()
 ,mdomcur(),mforcur(),mfxcur(),mIsLiborDFForFX(false),mSpotDate()
{	
}

LAPriceFXOptionValueDataProvider::~LAPriceFXOptionValueDataProvider()
{
}

LAPriceFXKnockoutRebateValueDataProvider::LAPriceFXKnockoutRebateValueDataProvider()
:LAPriceFXOptionValueDataProvider()
,rebate(0.0),limitval(0.0)
{	
}

LAPriceFXKnockoutRebateValueDataProvider::~LAPriceFXKnockoutRebateValueDataProvider()
{
}

LAPriceIROptionValueDataProvider::LAPriceIROptionValueDataProvider()
:LALinearRatesOptionValueDataProvider()
,mUnTenor(),mUnFrequency(),mUnStartDate(),
mpUnDayCount(0),mpUnSlidingRule(0),mpUnCalendar(0),mUnCurrency(), mDCurveType(), mFCurveType()
,mpUnDayCountOfFixedLeg(0),mpUnCalendarOfFixedLeg(0),mMaturityDates(),mFCurveTypes()
,mFloatLegNo(0), mFixedLegNo(1),mIsCalcEquivStrike(false),mIsFRateFromCurve(true)
,mIsIRRModel(false), mUnderlying()
{	
}

LAPriceIROptionValueDataProvider::~LAPriceIROptionValueDataProvider()
{
}

