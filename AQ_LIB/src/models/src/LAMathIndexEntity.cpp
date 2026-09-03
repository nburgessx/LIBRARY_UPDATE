/*! @file
    @brief Source code for class to represent index.

			Following dataValues are registered automatically to data master<BR>
			1.CALIBRATION_DATA_NAME(LADataString)<BR>			
			2.IR_MODEL_DATA_INDEXTYPE(LADataString)<BR>			
			3.IR_MODEL_DATA_ACCESSORY(LADataString)<BR>			
			4.IR_MODEL_DATA_CURRENCY(LADataString)<BR>			
			5.IR_MODEL_DATA_DAYCOUNT(LAPriceDataDayCount)<BR>
			6.IR_MODEL_DATA_FREQUENCY(LADataString)<BR>			
			7.IR_MODEL_DATA_CALENDAR(LAPriceDataCalendar)<BR>
			8.IR_MODEL_DATA_SLIDINGRULE(LAPriceDataSlidingRule)<BR>
			9.IR_MODEL_DATA_PATHENTITY(LADataReference)<BR>
			10.IR_MODEL_DATA_FXRATE(LADataReference)<BR>
			11.IR_MODEL_DATA_CACHESIZE(LADataInt)<BR>

*/
//  2006, AlgoQuantHub..
#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include "LADate.h"
#include "LAMathIndexEntity.h"
#include "LAMathDefine.h"
#include "LABasic.h"
#include "LADataBasics.h"
#include "LADataVector.h"
#include "LADataMatrix.h"
#include "LADataReference.h"
#include "LAPriceDataManager.h"
#include "LADataProcedure.h"
#include "LADataInstance.h"
#include "LADataVector.h"

#include "LAPriceDataDayCount.h"
#include "LAPriceDataCalendar.h"
#include "LAPriceDataSlidingRule.h"
#include "LAPriceDataDayCount.h"
#include "LAMathAttrSDE.h"
#include "LAMathPathEntity.h"
#include "LAMathFXEntity.h"
#include "LAMathDateCalculations.h"
#include "LAMathFXUtility.h"
#include "LAPriceDataInterpolation.h"

#include "LAAlgorithm.h"
#include "LAModelDynamicsCurve.h"
#include "LAMathPlainVanillaEntity.h"
#include "LADataMultiReference.h"
#include "LAMathYieldCurve.h"
#include "LAMathYieldCurvePro.h"
#include "LA1DDataSet.h"
#include "LAMathVolFuncIRSABR.h"
#include "LABlackScholesBaseFunc.h"
#include "LAPricePayOff.h"
#include "LAMathIRVanillaFuncUtility.h"
#include "LALinearSplineInterpolation.h"
#include "LALinearMonotoneSplineInterpolation.h"

#include <cmath>

using namespace std;

#define LIBOR	"LIBOR"
#define CMS		"CMS"
#define FX_RATE	"FXRATE"
#define CMT	"CMT"
#define OIS	"OIS"

#define SIMPLE	"SIMPLE"
#define ANNUAL	"ANNUAL"
#define SEMIANNUAL	"SEMI-ANNUAL"
#define QUARTERLY	"QUARTERLY"
#define MONTHLY		"MONTHLY"

#define SPOTLAG		0
#define SPOT		"SPOT"

#define STD "STD"

// (copied by LADate.cpp) 
static const unsigned short LOOKUP_TABLE_NUMBER_OF_DAYS_IN_A_MONTH_OR_YEAR[2][2][12] =
    {{{31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
      { 0, 31, 59, 90,120,151,181,212,243,273,304,334}},
     {{31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
      { 0, 31, 60, 91,121,152,182,213,244,274,305,335}}};


/*!
    @brief default constructor

	@param[in] dataInstance pointer of LADataInstance object

*/
LAMathIndexEntity::LAMathIndexEntity(LADataInstance* dataInstance) : 
LAObject(), /*mCacheSize(0), */mCachePos(-1), mAntiCachePos(-1), mPos(0),
mIsAntithetic(false), mSDEPos(-1), mpPath(0), mpFX(0), mIndexVersion(0), mSDEType(IR)
,mpVanilla(0), mIsSameFwds(true), mIsDaycountAdj(false), mCurveType(STD)
{
	setDataInstance(dataInstance);

	LAPriceDataManager& dm = dataInstance->getDataMaster();
	dm.setData(CALIBRATION_DATA_NAME, DATA_STRING);
	dm.setData(IR_MODEL_DATA_INDEXTYPE, DATA_STRING);
	dm.setData(IR_MODEL_DATA_ACCESSORY, DATA_STRING);
	dm.setData(IR_MODEL_DATA_CURRENCY, DATA_STRING);
	dm.setData(IR_MODEL_DATA_DAYCOUNT, DATA_DAYCOUNT);
	dm.setData(IR_MODEL_DATA_FREQUENCY, DATA_STRING);
	dm.setData(CALIBRATION_DATA_CALENDAR, DATA_CALENDAR);
	dm.setData(CALIBRATION_DATA_SLIDINGRULE, DATA_SLIDINGRULE);
	dm.setData(IR_MODEL_DATA_PATHENTITY, DATA_REFERENCE);
	dm.setData(IR_MODEL_DATA_FXRATE, DATA_REFERENCE);
	dm.setData(IR_MODEL_DATA_CACHESIZE, DATA_INT);
	dm.setData(PRICING_DATA_DISCOUNTCURVE, DATA_STRING);
	dm.setData(PRICING_DATA_BASISCURVE, DATA_STRING);
	dm.setData(PRICING_DATA_BASISINTERPOLATION, DATA_INTERPOLATION);
	dm.setData(PRICING_DATA_ISFWDINTERPOLATION, DATA_BOOL);
	dm.setData(PRICING_DATA_FWDINTERPOLATION, DATA_INTERPOLATION);
	dm.setData(PRICING_DATA_VOLATILITYUNDERLYING, DATA_STRING);
	dm.setData(PRICING_DATA_CAMODEL, DATA_STRING);
	dm.setData(PRICING_DATA_CATHRESHOLD, DATA_DOUBLE);
	dm.setData(PRICING_DATA_CAINTEGRALCONDITION, DATA_DOUBLES);
	dm.setData(PRICING_DATA_CAPREMIUMINTEGRALSTEP, DATA_INT);
	dm.setData(PRICING_DATA_CFDAYCOUNT, DATA_DAYCOUNT);
	dm.setData(PRICING_DATA_CFFREQUENCY, DATA_STRING);
	dm.setData(PRICING_DATA_PAYMENTLAG, DATA_STRING);
	dm.setData(PRICING_DATA_CFCALCSTARTDATES, DATA_DATES);
	dm.setData(PRICING_DATA_CFCALCENDDATES, DATA_DATES);

	mpName			= &add(CALIBRATION_DATA_NAME);
	mpIndexType		= &add(IR_MODEL_DATA_INDEXTYPE);
	mpAccessory		= &add(IR_MODEL_DATA_ACCESSORY);
	mpCurrency 		= &add(IR_MODEL_DATA_CURRENCY);
	mpDC			= &add(IR_MODEL_DATA_DAYCOUNT);
	mpFrequency		= &add(IR_MODEL_DATA_FREQUENCY);
	mpCalendar		= &add(CALIBRATION_DATA_CALENDAR);
	mpSlidingRule	= &add(CALIBRATION_DATA_SLIDINGRULE);
	mpPathEntity	= &add(IR_MODEL_DATA_PATHENTITY);
	mpFXEntity		= &add(IR_MODEL_DATA_FXRATE);
	mpCacheSize     = &add(IR_MODEL_DATA_CACHESIZE);
	mpDiscountCurve = &add(PRICING_DATA_DISCOUNTCURVE);
	mpBasis         = &add(PRICING_DATA_BASISCURVE);
	mpBasisInter    = &add(PRICING_DATA_BASISINTERPOLATION);
	mpIsFWDInter    = &add(PRICING_DATA_ISFWDINTERPOLATION);
	mpFWDInter      = &add(PRICING_DATA_FWDINTERPOLATION);
	mpVolUnderlying = &add(PRICING_DATA_VOLATILITYUNDERLYING);
	dynamic_cast<LADataInt&>(mpCacheSize->get()).set(0);

	mpIndexArray = &mIndexArray;
}
/*!
    @brief copy constructor

	@param[in] index LAMathIndexEntity object
*/
LAMathIndexEntity::LAMathIndexEntity(
	const LAMathIndexEntity& index) : 
	LAObject(index),
/*mCacheSize(index.mCacheSize), */
mCache(index.mCache), mAntiCache(index.mAntiCache), 
mCachePos(index.mCachePos), mAntiCachePos(index.mAntiCachePos), mPos(index.mPos),
mDateGrid(index.mDateGrid), mTimeGrid(index.mTimeGrid), mIndexArray(index.mIndexArray),
mIsAntithetic(index.mIsAntithetic), mSDEPos(index.mSDEPos), mpPath(index.mpPath), 
mpFX(index.mpFX), mIndexVersion(index.mIndexVersion), mSDEType(index.mSDEType), mGridMat(index.mGridMat), 
mTermMat(index.mTermMat), mFloatTermMat(index.mFloatTermMat), mSpreadMat(index.mSpreadMat), mSpreadMat_DF(index.mSpreadMat_DF),
mDFMat(index.mDFMat), mForwardTimes(index.mForwardTimes), mFromCurrency(index.mFromCurrency), mToCurrency(index.mToCurrency),
mDateGrid_old(index.mDateGrid_old), mpVanilla(index.mpVanilla),mSpotLag(index.mSpotLag),
mDatesForGenerate(index.mDatesForGenerate), mConvexityAdjust(index.mConvexityAdjust), mConvexityAdjustModel(index.mConvexityAdjustModel), 
mConvexityAdjustThreshold(index.mConvexityAdjustThreshold), mCouponTerm(index.mCouponTerm), mConvexityAdjustIC(index.mConvexityAdjustIC), mConvexityAdjustPremIntegralStep(index.mConvexityAdjustPremIntegralStep),
mConvexityAdjustVol(index.mConvexityAdjustVol), mIsSameFwds(index.mIsSameFwds)
{
	mpName			= &getData(CALIBRATION_DATA_NAME);
	mpIndexType		= &getData(IR_MODEL_DATA_INDEXTYPE);
	mpAccessory		= &getData(IR_MODEL_DATA_ACCESSORY);
	mpCurrency 		= &getData(IR_MODEL_DATA_CURRENCY);
	mpDC			= &getData(IR_MODEL_DATA_DAYCOUNT);
	mpFrequency		= &getData(IR_MODEL_DATA_FREQUENCY);
	mpCalendar		= &getData(CALIBRATION_DATA_CALENDAR);
	mpSlidingRule	= &getData(CALIBRATION_DATA_SLIDINGRULE);
	mpPathEntity	= &getData(IR_MODEL_DATA_PATHENTITY);
	mpFXEntity		= &getData(IR_MODEL_DATA_FXRATE);
	mpCacheSize		= &getData(IR_MODEL_DATA_CACHESIZE);
	mpDiscountCurve = &add(PRICING_DATA_DISCOUNTCURVE);
	mpBasis         = &add(PRICING_DATA_BASISCURVE);
	mpBasisInter    = &add(PRICING_DATA_BASISINTERPOLATION);
	mpIsFWDInter    = &add(PRICING_DATA_ISFWDINTERPOLATION);
	mpFWDInter      = &add(PRICING_DATA_FWDINTERPOLATION);

	mpIndexArray = &mIndexArray;
}
/*!
    @brief destructor
*/
LAMathIndexEntity::~LAMathIndexEntity()
{

}

// QUERY
/*!
    @brief Return this class type
	
	@return this function type
*/
object_t	
LAMathIndexEntity::getType(void) const
{
	return ENTITY_INDEX;
}
/*!
    @brief Check function for this Object class ID
    @param[in] id ID to check Object type
    @return True or False
*/
bool
LAMathIndexEntity::isTypeOf(object_t id) const
{
	return (id == ENTITY_INDEX ? true : LAObject::isTypeOf(id));
}


/*!
    @brief get this IndexEntity-name.
	@return name
*/
const LADataString&	
LAMathIndexEntity::getName() const	
{
	return dynamic_cast<const LADataString&>(mpName->get());
}
/*!
    @brief get this IndexEntity-name.The setting of name is also possible.
	@return name
*/
LADataString&	
LAMathIndexEntity::getName()
{
	return dynamic_cast<LADataString&>(mpName->get());
}
/*!
    @brief get index type.
	@return index type
*/
const LADataString&	
LAMathIndexEntity::getIndexType() const	
{
	return dynamic_cast<const LADataString&>(mpIndexType->get());
}
/*!
    @brief get index tpye.The setting of index type is also possible.
	@return index type
*/
LADataString&	
LAMathIndexEntity::getIndexType()
{
	return dynamic_cast<LADataString&>(mpIndexType->get());
}

/*!
    @brief get index accesory.
	@return index accessory
*/
const LADataString&	
LAMathIndexEntity::getAccessory() const	
{
	return dynamic_cast<const LADataString&>(mpAccessory->get());
}
/*!
    @brief get index accesory.The setting of index accesory is also possible.
	@return index accesory
*/
LADataString&	
LAMathIndexEntity::getAccessory()
{
	return dynamic_cast<LADataString&>(mpAccessory->get());
}
/*!
    @brief get this index currency.
	@return currncy
*/
const LADataString&	
LAMathIndexEntity::getCurrency() const	
{
	return dynamic_cast<const LADataString&>(mpCurrency->get());
}
/*!
    @brief get this index currency.The setting of index currency is also possible.
	@return currency
*/
LADataString&	
LAMathIndexEntity::getCurrency()
{
	return dynamic_cast<LADataString&>(mpCurrency->get());
}
/*!
    @brief get index daycount
	@return index daycount
*/
const LAPriceDataDayCount&
LAMathIndexEntity::getDayCount() const
{
	return dynamic_cast<const LAPriceDataDayCount&>(mpDC->get());
}
/*!
    @brief get index daycount. The setting of index daycount is also possible.
	@return index daycount
*/
LAPriceDataDayCount&
LAMathIndexEntity::getDayCount()
{
	return dynamic_cast<LAPriceDataDayCount&>(mpDC->get());
}
/*!
    @brief get index frequency
	@return index frequency
*/
const LADataString&
LAMathIndexEntity::getFrequency() const
{
	return dynamic_cast<const LADataString&>(mpFrequency->get());
}
/*!
    @brief get index frequency. The setting of index frequency is also possible.
	@return index frequency
*/
LADataString&
LAMathIndexEntity::getFrequency()
{
	return dynamic_cast<LADataString&>(mpFrequency->get());
}
/*!
    @brief get index calendar
	@return index calendar
*/
const LAPriceDataCalendar&
LAMathIndexEntity::getCalendar() const
{
	return dynamic_cast<const LAPriceDataCalendar&>(mpCalendar->get());
}
/*!
    @brief get index calendar. The setting of index calendar is also possible.
	@return index calendar
*/
LAPriceDataCalendar&
LAMathIndexEntity::getCalendar()
{
	return dynamic_cast<LAPriceDataCalendar&>(mpCalendar->get());
}	
/*!
    @brief get index sliding rule
	@return index sliding rule
*/
const LAPriceDataSlidingRule&
LAMathIndexEntity::getSlidingRule() const
{
	return dynamic_cast<const LAPriceDataSlidingRule&>(mpSlidingRule->get());
}
/*!
    @brief get index sliding rule. The setting of index sliding rule is also possible.
	@return index sliding rule
*/
LAPriceDataSlidingRule&
LAMathIndexEntity::getSlidingRule()
{
	return dynamic_cast<LAPriceDataSlidingRule&>(mpSlidingRule->get());
}
/*!
    @brief get path object.
	@return path object
*/
const LADataReference&	
LAMathIndexEntity::getPathEntity() const	
{
	return dynamic_cast<const LADataReference&>(mpPathEntity->get());
}
/*!
    @brief get path object.The setting of path object is also possible.
	@return path object
*/
LADataReference&	
LAMathIndexEntity::getPathEntity()
{
	return dynamic_cast<LADataReference&>(mpPathEntity->get());
}
/*!
    @brief get fx object.
	@return fx object
*/
const LADataReference&	
LAMathIndexEntity::getFXEntity() const	
{
	return dynamic_cast<const LADataReference&>(mpFXEntity->get());
}
/*!
    @brief get fx object.The setting of fx object is also possible.
	@return fx object
*/
LADataReference&	
LAMathIndexEntity::getFXEntity()
{
	return dynamic_cast<LADataReference&>(mpFXEntity->get());
} 

/*!
    @brief get basis
	@return basis
*/
const LADataString&	
LAMathIndexEntity::getBasis() const	
{
	return dynamic_cast<const LADataString&>(mpBasis->get());
}
/*!
    @brief get basis.The setting of basis is also possible.
	@return basis
*/
LADataString&	
LAMathIndexEntity::getBasis()
{
	return dynamic_cast<LADataString&>(mpBasis->get());
} 

/*!
    @brief get discount curve
	@return discount curve
*/
const LADataString&	
LAMathIndexEntity::getDiscountCurve() const	
{
	return dynamic_cast<const LADataString&>(mpDiscountCurve->get());
}
/*!
    @brief get discount curve.The setting of discount curve is also possible.
	@return discount curve
*/
LADataString&	
LAMathIndexEntity::getDiscountCurve()
{
	return dynamic_cast<LADataString&>(mpDiscountCurve->get());
} 

/*!
    @brief get basis interpolation
	@return basis interpolation
*/
const LAPriceDataInterpolation&	
LAMathIndexEntity::getBasisInterpolation() const	
{
	return dynamic_cast<const LAPriceDataInterpolation&>(mpBasisInter->get());
}
/*!
    @brief basis interpolation.The setting of basis interpolation is also possible.
	@return basis interpolation
*/
LAPriceDataInterpolation&	
LAMathIndexEntity::getBasisInterpolation()
{
	return dynamic_cast<LAPriceDataInterpolation&>(mpBasisInter->get());
}


/*!
    @brief get isfwdinterpolation
	@return isfwdinterpolation
*/
const LADataBool&	
LAMathIndexEntity::getIsFWDInterpolation() const	
{
	return dynamic_cast<const LADataBool&>(mpIsFWDInter->get());
}
/*!
    @brief get isfwdinterpolation.The setting of isfwdinterpolation is also possible.
	@return isfwdinterpolation
*/
LADataBool&	
LAMathIndexEntity::getIsFWDInterpolation()
{
	return dynamic_cast<LADataBool&>(mpIsFWDInter->get());
} 

/*!
    @brief get fwd interpolation
	@return fwd interpolation
*/
const LAPriceDataInterpolation&	
LAMathIndexEntity::getFWDInterpolation() const	
{
	return dynamic_cast<const LAPriceDataInterpolation&>(mpFWDInter->get());
}
/*!
    @brief fwd interpolation.The setting of fwd interpolation is also possible.
	@return fwd interpolation
*/
LAPriceDataInterpolation&	
LAMathIndexEntity::getFWDInterpolation()
{
	return dynamic_cast<LAPriceDataInterpolation&>(mpFWDInter->get());
}

/*!
    @brief get volatility underlying
	@return volatility underlying
*/
const LADataString&	
LAMathIndexEntity::getVolUnderlying() const	
{
	return dynamic_cast<const LADataString&>(mpVolUnderlying->get());
}
/*!
    @brief get volatility underlying
	@return volatility underlying
*/
LADataString&	
LAMathIndexEntity::getVolUnderlying()
{
	return dynamic_cast<LADataString&>(mpVolUnderlying->get());
} 

/*!
    @brief set next index. 
*/
void
LAMathIndexEntity::setNextIndex()
{
	if (!mIsAntithetic)// not antithetic
	{
		if (mCachePos < mPos) 
		{
			calcIndex();
			mpIndexArray = &mIndexArray;
			
			if (getCacheSize() > mPos)
			{
				mCache[mPos] = mIndexArray; 
				mCachePos = mPos;
			}
		}
		else
			mpIndexArray = &mCache[mPos];		
	}
	else if (mpPath->isOdd()) //antithetic and odd
	{
		if (mCachePos < mPos / 2) 
		{
			calcIndex();
			mpIndexArray = &mIndexArray;

			if (getCacheSize() > mPos / 2)
			{
				mCache[mPos / 2] = mIndexArray; 
				mCachePos = mPos / 2;
			}
		}
		else
			mpIndexArray = &mCache[mPos / 2];	
	}
	else	// antithetic and even
	{
		if (mAntiCachePos < mPos / 2) 
		{	
			calcIndex();
			mpIndexArray = &mIndexArray;
			
			if (getCacheSize() > mPos / 2)
			{
				mAntiCache[mPos / 2] = mIndexArray; 
				mAntiCachePos = mPos / 2;
			}
		}
		else
			mpIndexArray = &mAntiCache[mPos / 2];	
	}	
	mPos++;

}
/*!
	@brief get cache size
	@return cache size
*/
int
LAMathIndexEntity::getCacheSize() const
{
	return dynamic_cast<const LADataInt&>(mpCacheSize->get()).get();
}


/*!
	@brief set cache size
	@param[in] size cache size
*/
void
LAMathIndexEntity::setCacheSize(unsigned int size)
{
	bool flag = (mIndexVersion == getModel());
	dynamic_cast<LADataInt&>(mpCacheSize->get()).set(size);
//	update(TYPE_CACHESIZE_CHANGE);
    if (flag) mIndexVersion = getModel();
}
/*!
    @brief add to time grid. 
	@param[in] date date added to time grid
*/
void
LAMathIndexEntity::addGrid(const LADate& date)
{
	double term = 0.0;
	if (getPathEntity().get().get().isTypeOf(ENTITY_PATH))
	{
		const LAMathPathEntity& path = dynamic_cast<const LAMathPathEntity&>(getPathEntity().get().get());
		term = path.getDayCount().getTerm(path.getAsOfDate().get(), date);
	}
	else if (getPathEntity().get().get().isTypeOf(ENTITY_PLAINVANILLA))
	{
		const LAMathPlainVanillaEntity& path = dynamic_cast<const LAMathPlainVanillaEntity&>(getPathEntity().get().get());
		term = path.getDayCount().getTerm(path.getAsOfDate().get(), date);
	}
	else 
		throw LACoreInvalidData("Error add grid",__FILE__,__LINE__);
	
	//const LAMathPathEntity& path = dynamic_cast<const LAMathPathEntity&>(getPathEntity().get().get());
	//double term = path.getDayCount().getTerm(path.getAsOfDate().get(), date);
	if (mDateGrid.size() == 0)
	{ 
		mDateGrid.push_back(date);
		mTimeGrid.push_back(term);
	}
	else
	{
		unsigned int pos;
		LAAlgorithm::locate<DateVector, LADate>(mDateGrid, date, mDateGrid.size(), pos);
		if (pos == mDateGrid.size())
		{
			mDateGrid.push_back(date);
			mTimeGrid.push_back(term);
		}
		else if (mDateGrid[pos] == date) return;
		else 
		{
			DateVector::iterator it = mDateGrid.begin() + pos;
			mDateGrid.insert(it, date);
			DoubleArray::iterator it2 = mTimeGrid.begin() + pos;
			mTimeGrid.insert(it2, term);
		}
	}
//	update();
}
/*!
	@brief add to time grids used for calculation of convexity adjustment.
*/
void LAMathIndexEntity::addConvexityGrids(const LADate& fixingdate, const LADate& paymentdate, const LADate& enddate)
{
	double fixingterm  = 0.0;
	double paymentterm = 0.0;
	if (getPathEntity().get().get().isTypeOf(ENTITY_PATH))
	{
		const LAMathPathEntity& path = dynamic_cast<const LAMathPathEntity&>(getPathEntity().get().get());
		fixingterm  = path.getDayCount().getTerm(path.getAsOfDate().get(), fixingdate);
		paymentterm = path.getDayCount().getTerm(path.getAsOfDate().get(), paymentdate);
	}
	else if (getPathEntity().get().get().isTypeOf(ENTITY_PLAINVANILLA))
	{
		const LAMathPlainVanillaEntity& path = dynamic_cast<const LAMathPlainVanillaEntity&>(getPathEntity().get().get());
		fixingterm  = path.getDayCount().getTerm(path.getAsOfDate().get(), fixingdate);
		paymentterm = path.getDayCount().getTerm(path.getAsOfDate().get(), paymentdate);
	}
	mFixingGrid .push_back(LAMath::max(fixingterm, 0.0));
	mPaymentGrid.push_back(LAMath::max(paymentterm, 0.0));

	double timingterm = getDayCount().getTerm(paymentdate, enddate);
	mTimingTerm.push_back(timingterm);
}
/*!
    @brief clear cash data. 
*/
void
LAMathIndexEntity::clearCache(void)
{
	for (unsigned int i = 0; i < mCache.size(); i++)
		mCache[i].clear();
	mCache.clear();
	for (unsigned int i = 0; i < mAntiCache.size(); i++)
		mAntiCache[i].clear();
	mAntiCache.clear();
	mCachePos = -1;
	mAntiCachePos = -1;
	
}
/*!
    @brief clear grid. 
*/
void
LAMathIndexEntity::clearGrid(void)
{
	mDateGrid.clear();
	mTimeGrid.clear();
	mFixingGrid .clear();
	mPaymentGrid.clear();
	mTimingTerm .clear();

//	update();
}

/*!
    @brief set up for MC simulation. 
*/
void
LAMathIndexEntity::setUpforMC(void)
{
	if (mDateGrid.size() != mDateGrid_old.size()) update();
	else
	{
		for (unsigned int i = 0; i < mDateGrid.size(); i++)
		{
			if (mDateGrid[i] != mDateGrid_old[i])
			{
				update();
				break;
			}
		}
	}

	mDateGrid_old = mDateGrid; 
	if (mIndexVersion != getModel())//first call or calculation condition is changed, so set up from first
	{		
		//path
		mpPath = &dynamic_cast<LAMathPathEntity&>(getPathEntity().get().get());
		
		//antithetic or not
		mIsAntithetic = mpPath->isAntithetic();
		
		//set up cash
		clearCache();
		mCache.resize(getCacheSize());
		if (mIsAntithetic) mAntiCache.resize(getCacheSize());
		for (int i = 0; i < getCacheSize(); i++)
		{
			mCache[i].resize(mTimeGrid.size());
			if (mIsAntithetic) mAntiCache[i].resize(mTimeGrid.size());
		}
		mCachePos = -1;
		mAntiCachePos = -1;

		mIndexArray.resize(mTimeGrid.size());

		//set up this class for index calculation
		setUp();

	}
	else
	{
		//antithetic or not
		mIsAntithetic = mpPath->isAntithetic();
		
		//setUp Cache
		if ((unsigned int)getCacheSize() > mCache.size())
		{
			unsigned int oldsize = mCache.size();
			mCache.resize(getCacheSize());
			for (int i = oldsize; i < getCacheSize(); i++)			
				mCache[i].resize(mTimeGrid.size());	
							
		}
		else if ((unsigned int)getCacheSize() < mCache.size())
		{
			for (int i = getCacheSize(); i < (int)mCache.size(); i++)
				mCache[i].clear();
			mCache.resize(getCacheSize());
			mCachePos = mCachePos < getCacheSize() - 1 ? mCachePos : getCacheSize() - 1; 

		}		
		
		if (mIsAntithetic && (unsigned int)getCacheSize() > mAntiCache.size())
		{
			unsigned int oldsize = mAntiCache.size();
			mAntiCache.resize(getCacheSize());
			for (int i = oldsize; i < getCacheSize(); i++)			
				mAntiCache[i].resize(mTimeGrid.size());		
		}
		else if (mIsAntithetic && (unsigned int)getCacheSize() < mAntiCache.size())
		{
			for (int i = getCacheSize(); i < (int)mAntiCache.size(); i++)
				mAntiCache[i].clear();
			mAntiCache.resize(getCacheSize());
			mAntiCachePos = mAntiCachePos < getCacheSize() - 1 ? mAntiCachePos : getCacheSize() - 1; 

		}			
	}

	mPos = 0;

	mIndexVersion = getModel();

}


/*!
    @brief calculate index
*/
void
LAMathIndexEntity::calcIndex()
{
#ifndef VISUAL_STUDIO_2010_ANALYTICS
	if (mSDEType == IR)
	{
		if (mIsSameFwds && mpIsFWDInter->isDefined() && !mpIsFWDInter->isNull() && dynamic_cast<const LADataBool &>(mpIsFWDInter->get()).get())
		{
			DoubleArray rates(mFwdsGrid.size());
			if (mSpreadMat.empty())
			{
				for (unsigned int i = 0; i < mFwdsGrid.size(); i++)
				{
					const LARatesPathElementCurve& curve = 
						dynamic_cast<const LARatesPathElementCurve&>(mpPath->getPath(mSDEPos, mFwdsGrid[i]));
				
					//calc index
					double ret = 0.0;
					for (unsigned int j = 1; j < mFwdsGridMat[i].size(); j++)
						ret += mFwdsTermMat[i][j - 1] * curve.getP(mFwdsGridMat[i][j]);

					if (ret == 0.0)
					{
						rates[i] = 0.0;
					}
					else
					{
						rates[i] = (curve.getP(mFwdsGridMat[i][0]) - curve.getP(mFwdsGridMat[i].back())) / ret;
					}			
				}
			}
			else
			{
				for (unsigned int i = 0; i < mFwdsGrid.size(); i++)
				{
					const LARatesPathElementCurve& curve = 
						dynamic_cast<const LARatesPathElementCurve&>(mpPath->getPath(mSDEPos, mFwdsGrid[i]));
				
					//calc index reflect basis spread
					double ret = 0.0;
					for (unsigned int j = 1; j < mFwdsGridMat[i].size(); j++)
						ret += mFwdsTermMat[i][j - 1] * (curve.getP(mFwdsGridMat[i][j]) * mSpreadMat[i][j]);

					if (ret == 0.0)
					{
						rates[i] = 0.0;
					}
					else
					{
						rates[i] = ((curve.getP(mFwdsGridMat[i][0]) * mSpreadMat[i][0]) - (curve.getP(mFwdsGridMat[i].back()) * mSpreadMat[i].back())) / ret;
					}
				}
			}

			dynamic_cast<LAPriceDataInterpolation &>(mpFWDInter->get()).set(mFwdsStartGrid, rates);
			const LAInterpolationBase &inter = dynamic_cast<const LAPriceDataInterpolation &>(mpFWDInter->get()).getMethod(); 
			for (unsigned int i = 0; i < mTimeGrid.size(); i++)
			{
				mIndexArray[i] = inter.value(mGridMat[i][0]);
			}
		}
		else
		{
			if (mSpreadMat.empty())
			{
				for (unsigned int i = 0; i < mTimeGrid.size(); i++)
				{
					const LARatesPathElementCurve& curve = 
						dynamic_cast<const LARatesPathElementCurve&>(mpPath->getPath(mSDEPos, mTimeGrid[i]));
				
					//calc index
					double ret = 0.0;
					for (unsigned int j = 1; j < mGridMat[i].size(); j++)
						ret += mTermMat[i][j - 1] * curve.getP(mGridMat[i][j]);

					if (ret == 0.0)
					{
						mIndexArray[i] = 0.0;
					}
					else
					{
						mIndexArray[i] = (curve.getP(mGridMat[i][0]) - curve.getP(mGridMat[i].back())) / ret;
					}			
				}
			}
			else
			{
				for (unsigned int i = 0; i < mTimeGrid.size(); i++)
				{
					const LARatesPathElementCurve& curve = 
						dynamic_cast<const LARatesPathElementCurve&>(mpPath->getPath(mSDEPos, mTimeGrid[i]));
				
					//calc P vector
					DoubleVector P_vec;
					for (unsigned int j = 0; j < mGridMat[i].size(); ++j)
						P_vec.push_back(curve.getP(mGridMat[i][j]));
					//calc index reflect basis spread
					double ret = 0.0;
					/*if (mGridMat[i].size() <= 2)
					{
						for (unsigned int j = 1; j < mGridMat[i].size(); j++)
							ret += mTermMat[i][j - 1] * (curve.getP(mGridMat[i][j]) * mSpreadMat[i][j]);

						if (ret == 0.0)
						{
							mIndexArray[i] = 0.0;
						}
						else
						{
							mIndexArray[i] = ((curve.getP(mGridMat[i][0]) * mSpreadMat[i][0]) - (curve.getP(mGridMat[i].back()) * mSpreadMat[i].back())) / ret;
						}
					}
					else
					{*/
					for (unsigned int j = 1; j < mGridMat[i].size(); j++)
						ret += mTermMat[i][j - 1] * (P_vec[j] * mSpreadMat_DF[i][j]);

					if (ret == 0.0)
					{
						mIndexArray[i] = 0.0;
					}
					else
					{
						double floatPV = 0.0;
						for (unsigned int k = 1;  k < mGridMat[i].size(); k++)
						{
							const double df1 = P_vec[k-1] * mSpreadMat[i][k - 1];
							const double df2 = P_vec[k] * mSpreadMat[i][k];
							const double rate_tau = (df1 - df2) / df2;
							const double d_df = P_vec[k] * mSpreadMat_DF[i][k];
							floatPV += rate_tau * d_df;
						}
						mIndexArray[i] = floatPV / ret;
					}
					//}
				}
			}
		}
	}
	else if(mSDEType == FX)
	{
		for (unsigned int i = 0; i < mIndexArray.size(); i++)
			mIndexArray[i] = mpFX->getForwardRate(mFromCurrency, mToCurrency, mTimeGrid[i], mForwardTimes[i], mpPath->getDayCount().getDayCount());
	}
#endif
}

/*!
    @brief calculate index
*/
void
LAMathIndexEntity::calcIndexforPlainVanilla()
{
	if (mSDEType == IR)
	{
		const LAMathYieldCurve& curve = dynamic_cast<const LAMathYieldCurve &>(mpVanilla->getIRCurves().get(mSDEPos).get());
		if (mpBasis->isDefined() && !mpBasis->isNull())
		{
			calcSpreadMat(curve, mGridMat);
		}

		if (mIsSameFwds && mpIsFWDInter->isDefined() && !mpIsFWDInter->isNull() && dynamic_cast<const LADataBool &>(mpIsFWDInter->get()).get())
		{	
			// set forwardrates
			DoubleArray taus;
			if (mIsDaycountAdj)
			{
				// get daycount adjusted forwardrates
				curve.getAdjForwardRate(mCurveType, getDayCount(), mFwdsStartGrid, mFwdsGridMat, taus, mFwds, &mFwdsStartEndDate);
			}
			else
			{
				// get base forwardrates
				curve.getBaseForwardRate(mCurveType, mFwdsStartGrid, mFwdsGridMat, taus, mFwds, NULL, &mFwdsStartEndDate);
			}

			if (dynamic_cast<LAPriceDataInterpolation &>(mpFWDInter->get()).isHybrid())
			{
				LADataInstance* dataInstance = getDataInstance();
				LAObjectPool& objPool = dataInstance->getObjectPool();
				LADataHolder *dh = &objPool.getObject("YIELD_SDE_" + getCurrency() + "_IR_DATA", ENCHKTYPE_ISDEFINED).get().getData(CALIBRATION_DATA_INTERPOLATION_JOINDATE_ASDOUBLE + LAString("_") + mCurveType, NOCHECK);
				double JoinDateAsDouble = 0.;
				if (dh->isDefined() && !dh->isNull())
				{
					JoinDateAsDouble = dynamic_cast<LADataDouble&>(dh->get()).get();
				}
				dynamic_cast<LAPriceDataInterpolation &>(mpFWDInter->get()).setJoinDateAsDouble(JoinDateAsDouble);
			}
			dynamic_cast<LAPriceDataInterpolation &>(mpFWDInter->get()).set(mFwdsStartGrid, mFwds);
			const LAInterpolationBase &inter = dynamic_cast<const LAPriceDataInterpolation &>(mpFWDInter->get()).getMethod();
			const unsigned int offset = (mTimeGrid.size() - mPaymentGrid.size());

			for (unsigned int i = 0; i < mTimeGrid.size(); i++)
			{
				if (mGridMat[i].size() <= 2)
				{
					mIndexArray[i] = inter.value(mGridMat[i][0]);
#ifndef VISUAL_STUDIO_2010_ANALYTICS
					if ((i >= offset) && isDelayedConvexityAdjusted())
					{
						LADataMultiReference* pirvols = &(mpVanilla->getIRVolatilitys());
						if (pirvols == 0 || pirvols->getSize() == 0)
						{
							throw LACoreInvalidData("IR Calibration Model For Convexity Adjust is not set", __FILE__, __LINE__);
						}

						const LAMathVolFuncIRSABR* method = dynamic_cast<const LAMathVolFuncIRSABR*>(mpVanilla->getIRVolFunc(getCurrency().get()));
						if (!method->isTypeOf(FN_VOLFUNCIRSABR))
						{
							throw LACoreInvalidData("Convexity Adjust needs only SABR model now", __FILE__, __LINE__);
						}

						const LAInterpolationBase* oiscurve = &mpVanilla->getIRCurve(getCurrency().get()).getBasisDFInterpolation();
						const auto& curvePro = mpVanilla->getIRCurvePro(getCurrency().get());
						const auto& assignedCurveMktMap = curvePro.getAssignedCurveMktMap();
						for (auto iac = assignedCurveMktMap.begin(); iac != assignedCurveMktMap.end(); ++iac)
						{
							if (iac->second == OISCURVE)
							{
								oiscurve = &curvePro.getDFInterpolation(&iac->first);
								break;
							}
						}

						const double dfEnd = oiscurve->value(mGridMat[i][1]);
						const double dfPayment = oiscurve->value(mPaymentGrid[i - offset]);

						mIndexArray[i] += getLIBORConvexityAdjust(mIndexArray[i], mFixingGrid[i - offset], mTimingTerm[i - offset], dfEnd, dfPayment, i);
					}
#endif
				}
				else
				{
					if (mSpreadMat_DF.empty())
						throw LACoreInvalidData("mSpreadMat_DF is not set.", __FILE__, __LINE__);

					double annuity = 0.;
					for (unsigned int j = 1; j < mGridMat[i].size(); j++)
						annuity += mTermMat[i][j - 1] * (curve.getDF(mGridMat[i][j]) * mSpreadMat_DF[i][j]);

					if (annuity == 0.0)
					{
						mIndexArray[i] = 0.0;
					}
					else
					{
						double floatPV = 0.0;
						for (unsigned int k = 1;  k < mGridMat[i].size(); k++)
						{
							const double rate = inter.value(mGridMat[i][k - 1]);
							const double d_df = curve.getDF(mGridMat[i][k]) * mSpreadMat_DF[i][k];
							floatPV += rate * d_df * mFloatTermMat[i][k - 1];
						}
						mIndexArray[i] = floatPV / annuity;
						mIndexArray[i] += getConvexityAdjust(mIndexArray[i], mTimeGrid[i], i);
					}
				}

				//mDFMat
				for (unsigned int j = 0; j < mGridMat[i].size(); j++)
				{
					if (mSpreadMat_DF.empty())
					{
						mDFMat[i][j] = curve.getDF(mGridMat[i][j]);
					}
					else
					{	
						mDFMat[i][j] = curve.getDF(mGridMat[i][j]) * mSpreadMat_DF[i][j];
					}
				}
			}
		}
		else
		{
			if (mSpreadMat.empty())
			{
				for (unsigned int i = 0; i < mTimeGrid.size(); i++)
				{
					double ret = 0.0;
					for (unsigned int j = 1; j < mGridMat[i].size(); j++)
					{
						mDFMat[i][j] = curve.getDF(mGridMat[i][j]);
						ret += mTermMat[i][j - 1] * mDFMat[i][j];
					}
					
					if (ret == 0.0)
					{
						mIndexArray[i] = 0.0;						
					}
					else
					{
						mDFMat[i][0] = curve.getDF(mGridMat[i][0]);
						mDFMat[i].back() = curve.getDF(mGridMat[i].back());
						mIndexArray[i] = (mDFMat[i][0] - mDFMat[i].back()) / ret;
						mIndexArray[i] += getConvexityAdjust(mIndexArray[i], mTimeGrid[i], i);
					}
				}
			}
			else
			{
				const LAMathYieldCurve& curve = dynamic_cast<const LAMathYieldCurve &>(mpVanilla->getIRCurves().get(mSDEPos).get());
				//calc index reflect basis spread
				for (unsigned int i = 0; i < mTimeGrid.size(); i++)
				{
					double annuity = 0.0;
					if (mGridMat[i].size() <= 2)
					{
						for (unsigned int j = 1; j < mGridMat[i].size(); j++)
							annuity += mTermMat[i][j - 1] * (curve.getDF(mGridMat[i][j]) * mSpreadMat[i][j]);

						if (annuity == 0.0)
						{
							mIndexArray[i] = 0.0;
						}
						else
						{
							mIndexArray[i] = ((curve.getDF(mGridMat[i][0]) * mSpreadMat[i][0]) - (curve.getDF(mGridMat[i].back()) * mSpreadMat[i].back())) / annuity;
						}
					}
					else
					{
						for (unsigned int j = 1; j < mGridMat[i].size(); j++)
							annuity += mTermMat[i][j - 1] * (curve.getDF(mGridMat[i][j]) * mSpreadMat_DF[i][j]);

						if (annuity == 0.0)
						{
							mIndexArray[i] = 0.0;
						}
						else
						{
							double floatPV = 0.0;
							for (unsigned int k = 1;  k < mGridMat[i].size(); k++)
							{
								const double df1 = curve.getDF(mGridMat[i][k - 1]) * mSpreadMat[i][k - 1];
								const double df2 = curve.getDF(mGridMat[i][k]) * mSpreadMat[i][k];
								const double rate_tau = (df1 - df2) / df2;
								const double d_df = curve.getDF(mGridMat[i][k]) * mSpreadMat_DF[i][k];
								floatPV += rate_tau * d_df;
							}
							mIndexArray[i] = floatPV / annuity;
							mIndexArray[i] += getConvexityAdjust(mIndexArray[i], mTimeGrid[i], i);
						}
					}
				}
			}
		}
	}
	else if(mSDEType == FX)
	{
		for (unsigned int i = 0; i < mIndexArray.size(); i++)
			mIndexArray[i] = mpFX->getForwardRate(mFromCurrency, mToCurrency, mTimeGrid[i], mForwardTimes[i], mpVanilla->getDayCount().getDayCount());
	}

	mpIndexArray = &mIndexArray;
}

/*!
    @brief Make copy(clone) of this Index Object object.
    @return pointer of Index Object object.
*/
LAObject* 
LAMathIndexEntity::clone() const
{
    try 
	{
    	LAMathIndexEntity* pIndexEntiy = new LAMathIndexEntity(*this);
    	return pIndexEntiy;
    }
    catch (bad_alloc & e)
	{
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }
}



/*!
	@brief remove dataValues except for certain data
	@param[in] dataName data name
*/
void                
LAMathIndexEntity::remove(
	const LAString& dataName)
{
	if(dataName == CALIBRATION_DATA_NAME
		|| dataName == IR_MODEL_DATA_INDEXTYPE
		|| dataName == IR_MODEL_DATA_ACCESSORY
		|| dataName == IR_MODEL_DATA_CURRENCY
		|| dataName == IR_MODEL_DATA_DAYCOUNT
		|| dataName == IR_MODEL_DATA_FREQUENCY
		|| dataName == CALIBRATION_DATA_CALENDAR
		|| dataName == CALIBRATION_DATA_SLIDINGRULE
		|| dataName == IR_MODEL_DATA_PATHENTITY
		|| dataName == IR_MODEL_DATA_FXRATE
		|| dataName == IR_MODEL_DATA_CACHESIZE)
	{
		return; 
	}
	LAObject::remove(dataName);
}

/*!
    @brief Initialize this Object
*/
void               
LAMathIndexEntity::reset(void)
{
	clear();
	mpName			= &add(CALIBRATION_DATA_NAME);
	mpIndexType		= &add(IR_MODEL_DATA_INDEXTYPE);
	mpAccessory		= &add(IR_MODEL_DATA_ACCESSORY);
	mpCurrency 		= &add(IR_MODEL_DATA_CURRENCY);
	mpDC			= &add(IR_MODEL_DATA_DAYCOUNT);
	mpFrequency		= &add(IR_MODEL_DATA_FREQUENCY);
	mpCalendar		= &add(CALIBRATION_DATA_CALENDAR);
	mpSlidingRule	= &add(CALIBRATION_DATA_SLIDINGRULE);
	mpPathEntity	= &add(IR_MODEL_DATA_PATHENTITY);
	mpFXEntity		= &add(IR_MODEL_DATA_FXRATE);
	mpCacheSize = &add(IR_MODEL_DATA_CACHESIZE);
	
	clearCache();
	clearGrid();
//	mCacheSize = 0; 
	mCachePos = -1;
	mAntiCachePos = -1;
	mPos = 0;
	mIndexArray.clear();
	mpIndexArray = &mIndexArray;
	mIsAntithetic = false; 
	mSDEPos = -1;
	mpPath = 0;	
	mpFX = 0;		
	mIndexVersion = 0;
	mGridMat.clear();
	mDFMat.clear();
	mTermMat.clear();
	mFloatTermMat.clear();
	mSpreadMat.clear();
	mSpreadMat_DF.clear();
	mFromCurrency = "";
	mToCurrency = "";
	mForwardTimes.clear();
	mIsSameFwds = true;
	mpVanilla = 0;
	mSpotLag.clear();
	mDatesForGenerate.clear();

}

/*!
    @brief called when updating Data and the version number gets increased.
	
    @param[in] type Data(TYPE_NORMAL)
*/
void                
LAMathIndexEntity::update(const unsigned int type) 
{
	LAObject::update(type);
    if ((type & TYPE_CACHESIZE_CHANGE) != 0x0000
		|| (type & TYPE_ANTITHETICFLAG_CHANGE) != 0x0000)
		mIndexVersion = getModel();

}

/////////////// PROTECTED METHODS /////////////////////
/*!
	@brief copy LAMathIndexEntity
	@param[in] e copy source
	@return reference to this object
*/
LAObject&
LAMathIndexEntity::copy(
	const LAObject& e)
{
	if (this == &e) return *this;

	LAObject::copy(e);
	if (!e.isTypeOf(ENTITY_INDEX))
	{
		LAString err = "Assignement error for LAMathIndexEntity : from ";
		err += LAString(e.getType());
		throw LACoreInvalidData(err.getCString(), __FILE__, __LINE__);
	}

	mpName			= &getData(CALIBRATION_DATA_NAME);
	mpIndexType		= &getData(IR_MODEL_DATA_INDEXTYPE);
	mpAccessory		= &getData(IR_MODEL_DATA_ACCESSORY);
	mpCurrency 		= &getData(IR_MODEL_DATA_CURRENCY);
	mpDC			= &getData(IR_MODEL_DATA_DAYCOUNT);
	mpFrequency		= &getData(IR_MODEL_DATA_FREQUENCY);
	mpCalendar		= &getData(CALIBRATION_DATA_CALENDAR);
	mpSlidingRule	= &getData(CALIBRATION_DATA_SLIDINGRULE);
	mpPathEntity	= &getData(IR_MODEL_DATA_PATHENTITY);
	mpFXEntity		= &getData(IR_MODEL_DATA_FXRATE);
	mpCacheSize		= &getData(IR_MODEL_DATA_CACHESIZE);

//	mCacheSize = dynamic_cast<const LAMathIndexEntity&>(e).mCacheSize; 
	mCache = dynamic_cast<const LAMathIndexEntity&>(e).mCache; 
	mAntiCache = dynamic_cast<const LAMathIndexEntity&>(e).mAntiCache; 
	mCachePos =  dynamic_cast<const LAMathIndexEntity&>(e).mCachePos;
	mAntiCachePos =  dynamic_cast<const LAMathIndexEntity&>(e).mAntiCachePos;
	mPos =  dynamic_cast<const LAMathIndexEntity&>(e).mPos;
	mDateGrid = dynamic_cast<const LAMathIndexEntity&>(e).mDateGrid; 
	mTimeGrid = dynamic_cast<const LAMathIndexEntity&>(e).mTimeGrid; 
	mIndexArray = dynamic_cast<const LAMathIndexEntity&>(e).mIndexArray; 
	mpIndexArray = &mIndexArray;
	mIsAntithetic = dynamic_cast<const LAMathIndexEntity&>(e).mIsAntithetic; 
	mSDEPos = dynamic_cast<const LAMathIndexEntity&>(e).mSDEPos;
	mpPath = dynamic_cast<const LAMathIndexEntity&>(e).mpPath;	
	mpFX = dynamic_cast<const LAMathIndexEntity&>(e).mpFX;		
	mIndexVersion = dynamic_cast<const LAMathIndexEntity&>(e).mIndexVersion;	
	mSDEType = dynamic_cast<const LAMathIndexEntity&>(e).mSDEType; 
	mGridMat = dynamic_cast<const LAMathIndexEntity&>(e).mGridMat; 
	mDFMat = dynamic_cast<const LAMathIndexEntity&>(e).mDFMat; 
	mTermMat = dynamic_cast<const LAMathIndexEntity&>(e).mTermMat; 
	mFloatTermMat = dynamic_cast<const LAMathIndexEntity&>(e).mFloatTermMat; 
	mForwardTimes = dynamic_cast<const LAMathIndexEntity&>(e).mForwardTimes; 
	mFromCurrency = dynamic_cast<const LAMathIndexEntity&>(e).mFromCurrency; 
	mToCurrency = dynamic_cast<const LAMathIndexEntity&>(e).mToCurrency; 
	mDateGrid_old = dynamic_cast<const LAMathIndexEntity&>(e).mDateGrid_old;

	mIsSameFwds = dynamic_cast<const LAMathIndexEntity&>(e).mIsSameFwds;
	mpVanilla = dynamic_cast<const LAMathIndexEntity&>(e).mpVanilla;	
	mSpotLag = dynamic_cast<const LAMathIndexEntity&>(e).mSpotLag;
	mDatesForGenerate = dynamic_cast<const LAMathIndexEntity&>(e).mDatesForGenerate;



	return *this;
}
/*!
	@brief add certain data
	@param[in] name name of certain data
	@return reference to holder class 
*/
LADataHolder&
LAMathIndexEntity::add(const LAString& name)
{
	LADataInstance* dataInstance = getDataInstance();
	LAPriceDataManager& dm = dataInstance->getDataMaster();
	const LADataHolder& dh = dm.getData(name);
	return LAObject::add(name, dh);
}

/*!
	@brief set up this class for index calculation
	
	@note this method is called from setforMC method
*/
void
LAMathIndexEntity::setUp(void)
{
	//SDE Type (IR or FX) correspond to this index
	LAString indextype = getIndexType();
	indextype.toUpper();
	if (indextype == LIBOR || indextype == CMS || indextype == CMT || indextype == OIS)
		mSDEType = IR;
	else if (indextype == FX_RATE)
		mSDEType = FX;
	else
	{
		//error
		LAString msg = "IndexTpye: ";
		msg += indextype;
		msg += " is not support";
		throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}

	
	// set up member variables for index calculation
	if (mSDEType == IR)
		setUpforIR();
	else //FX 
		setUpforFX();

}

/*!
	@brief set up this class for index calculation
	
	@note this method is called from setforMC method
*/
void
LAMathIndexEntity::setUpforPlainVanilla(void)
{

	//path
	mpVanilla = &dynamic_cast<LAMathPlainVanillaEntity&>(getPathEntity().get().get());

	//mIndex Resize
	mIndexArray.resize(mTimeGrid.size());
	mConvexityAdjust.resize(mTimeGrid.size());
	mConvexityAdjustVol.resize(mTimeGrid.size());

	//SDE Type (IR or FX) correspond to this index
	LAString indextype = getIndexType();
	indextype.toUpper();
	if (indextype == LIBOR || indextype == CMS || indextype == CMT || indextype == OIS)
		mSDEType = IR;
	else if (indextype == FX_RATE)
		mSDEType = FX;
	else
	{
		//error
		LAString msg = "IndexTpye: ";
		msg += indextype;
		msg += " is not support";
		throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}

	
	// set up member variables for index calculation
	if (mSDEType == IR)
		setUpforIRforPlainVanilla();
	else //FX 
		setUpforFXforPlainVanilla();


	//calc index
	//calcIndexforPlainVanilla();
	//mpIndexArray = &mIndexArray;

}


/*!
	@brief set up this class for ir index calculation
	
	@note this method is called from setUp method
*/
void
LAMathIndexEntity::setUpforIR(void)
{
	// sde position number correspond to this index
	LAStringVector sde_attrnames = mpPath->getSimulationSDEAttrNames().get();
	if (sde_attrnames.size() == 0)
		sde_attrnames = mpPath->getSDEAttrNames().get();

	unsigned int i;
	for (i = 0; i < sde_attrnames.size(); i++)
	{
		const LAMathAttrSDE& attrsde = 
			dynamic_cast<const LAMathAttrSDE&>(mpPath->getData(sde_attrnames[i], ISNOTNULL).get());
		if (attrsde.getSDEPathType() == mSDEType && attrsde.getCurrency() == getCurrency().get())
		{
			mSDEPos = i;
			break;
		}		
	}
	if (i == sde_attrnames.size())
	{
		//error
		throw LACoreInvalidData("There is not sde correspond to this index", __FILE__, __LINE__);		
	}

	
	LAString freq = getFrequency();
	freq.toUpper();
	//const LAString& accessory = getAccessory().get();
	const LAString& tmpaccessory = getAccessory().get();
	LAString accessory(tmpaccessory);
	bool iscoterm = (accessory.findString("Co-Term") != -1); 
	if(iscoterm)
	{
		accessory.remove(accessory.findString("Co-Term"),7);
		accessory.trimLeft();
	}
	
	int y, m, d, w;
	LAMathDateCalculations::termStrtoYMDW(accessory, y, m, d, w);
	if (freq != SIMPLE && d != 0)
	{
		throw LACoreInvalidData("d != 0 is not support", __FILE__, __LINE__);	
	}
	if (freq != SIMPLE && 0 == y && 0 == m)
	{
		throw LACoreInvalidData("This Accessory input supports only y,m,d or (Co-term) y,m,d",__FILE__,__LINE__);
	}
	const LAPriceDataCalendar& cal = getCalendar();
	const LAPriceDataSlidingRule& srule = getSlidingRule();
	const LAPriceDataDayCount& daycount_index = getDayCount();
	const LAPriceDataDayCount& daycount_path = mpPath->getDayCount();

	const LADate& asof = mpPath->getAsOfDate().get();

	mGridMat.resize(mDateGrid.size());
	mTermMat.resize(mDateGrid.size());
	unsigned int size = 0;
	unsigned int addmonth = 0;
	unsigned int addday = 0;
	if (freq == SIMPLE)
	{
		size = 2;
		addmonth = y * 12 + m;
		addday = d;
	}
	else if (freq == MONTHLY)
	{
		size = y * 12 + m + 1;
		addmonth = 1;
	}
	else if (freq == QUARTERLY)
	{
		if (m % 3 != 0)
		{
			throw LACoreInvalidData("Frequency and Accessory are not consistent", __FILE__, __LINE__);	
		}
		size = y * 4 + m / 3 + 1;
		addmonth = 3;

	}
	else if (freq == SEMIANNUAL)
	{
		if (m % 6 != 0)
		{
			throw LACoreInvalidData("Frequency and Accessory are not consistent", __FILE__, __LINE__);	
		}	
		size = y * 2 + m / 6 + 1;
		addmonth = 6;

	}
	else if (freq == ANNUAL)
	{
		if (m % 12 != 0)
		{
			throw LACoreInvalidData("Frequency and Accessory are not consistent", __FILE__, __LINE__);
		}
		size = y + m / 12 + 1;
		addmonth = 12;
	}
	else
	{
		//error
		LAString err = "Frequency: ";
		err += freq;
		err += " is not support";
		throw LACoreInvalidData(err.getCString(), __FILE__, __LINE__);
	}

	mSpreadMat.clear();
	mSpreadMat_DF.clear();
	LADate date, date_nonadjust, olddate;
	if(iscoterm)
	{
		LADate terminal_nonadj = cal.getBusinessDay(asof,SPOTLAG);
		terminal_nonadj.addMonths(addmonth * (size-1));
		LADate terminal = srule.getDate(terminal_nonadj,cal);
		for (i = 0; i < mGridMat.size(); i++)
		{
			mGridMat[i].clear();
			mTermMat[i].clear();
			date = cal.getBusinessDay(mDateGrid[i], SPOTLAG);
			date_nonadjust = date;
			double grid0 = daycount_path.getTerm(asof, date);
			mGridMat[i].push_back(grid0);//spot date

			if(terminal <= date)
			{
				//temporary term;
				mTermMat[i].push_back(1.0);
				mGridMat[i].push_back(grid0);
				continue;
			}
			bool chk = false;
			for(unsigned int j = 1; j < size && chk == false; j++)
			{
				olddate = date;
				date_nonadjust.addMonths(addmonth);
				date = srule.getDate(date_nonadjust, cal);
				chk = (terminal <= date);
				if(chk)
				{
					date_nonadjust = terminal_nonadj;
					date = terminal;
					mGridMat[i].push_back(daycount_path.getTerm(asof, date));
					mTermMat[i].push_back(daycount_index.getTerm(olddate, date, false));
					continue;
				}
				else
				{
					mGridMat[i].push_back(daycount_path.getTerm(asof, date));
					mTermMat[i].push_back(daycount_index.getTerm(olddate, date, false));
				
				}
			}
		}
		if (mpBasis->isDefined() && !mpBasis->isNull())
		{
			const LAStringVector &sde_attrnames_ = mpPath->getSDEAttrNames().get();
			for (i = 0; i < sde_attrnames_.size(); i++)
			{
				const LAMathAttrSDE& attrsde = 
					dynamic_cast<const LAMathAttrSDE&>(mpPath->getData(sde_attrnames_[i], ISNOTNULL).get());
				if (attrsde.getSDEPathType() == mSDEType && attrsde.getCurrency() == getCurrency().get())
				{
					const LAMathYieldCurve &curve = dynamic_cast<LAMathYieldCurve &>(mpPath->getInitialValues().get(i).get());
					calcSpreadMat(curve, mGridMat);
					break;
				}		
			}
		}
		return;
	}

	for (i = 0; i < mGridMat.size(); i++)
	{
		mGridMat[i].resize(size);
		mTermMat[i].resize(size - 1);
		date = cal.getBusinessDay(mDateGrid[i], SPOTLAG);
		date_nonadjust = date;
		mGridMat[i][0] = daycount_path.getTerm(asof, date);//spot date
	
		for (unsigned int j = 1; j < size; j++)
		{
			olddate = date;
			date_nonadjust.addMonths(addmonth);
			date_nonadjust.addDays(addday);
			date = srule.getDate(date_nonadjust, cal);
			mGridMat[i][j] = daycount_path.getTerm(asof, date);		
			mTermMat[i][j - 1] = daycount_index.getTerm(olddate, date, false);
		}
	}
	if (mpBasis->isDefined() && !mpBasis->isNull())
	{
		const LAStringVector &sde_attrnames_ = mpPath->getSDEAttrNames().get();
		for (i = 0; i < sde_attrnames_.size(); i++)
		{
			const LAMathAttrSDE& attrsde = 
				dynamic_cast<const LAMathAttrSDE&>(mpPath->getData(sde_attrnames_[i], ISNOTNULL).get());
			if (attrsde.getSDEPathType() == mSDEType && attrsde.getCurrency() == getCurrency().get())
			{
				const LAMathYieldCurve &curve = dynamic_cast<LAMathYieldCurve &>(mpPath->getInitialValues().get(i).get());
				calcSpreadMat(curve, mGridMat);
				break;
			}		
		}
	}
	if (mpIsFWDInter->isDefined() && !mpIsFWDInter->isNull() && dynamic_cast<const LADataBool &>(mpIsFWDInter->get()).get())
	{
		LAString indexType = getIndexType();
		indexType.toUpper();
		if (indexType == LIBOR)
		{
			LAString curveType = STD;
			if (mpBasis->isDefined() && !mpBasis->isNull())
			{
				curveType = dynamic_cast<const LADataString &>(mpBasis->get()).get();
			}
			LAMathYieldCurve& curve =  dynamic_cast<LAMathYieldCurve &>(mpPath->getInitialValues().get(mSDEPos).get());

			LAString freq_;
			LAPriceDataSlidingRule srule_;
			LAPriceDataDayCount dc_;
			LAPriceDataCalendar cal_;
			LAString accessory_;
			//getYieldCurvePro().getForwardConvention(curveType, dc_, srule_, cal_, accessory_);
			curve.getCurveConvention(freq_, cal_, srule_, dc_, accessory_, curveType);
			if (LAMathDateCalculations::getDate(asof, accessory_, true) == LAMathDateCalculations::getDate(asof, getAccessory().get(), true))
			{
				mIsSameFwds = true;
				DoubleArray taus;
				curve.getBaseForwardRate(curveType, mFwdsStartGrid, mFwdsGridMat, taus, mFwds);

				if (mFwdsStartGrid.empty())
				{
					throw LACoreInvalidData("Forward rate grid is empty", __FILE__, __LINE__);
				}
				DateVector startDates;
				LAMathDateCalculations::convertToDateGrid(asof, mFwdsStartGrid, startDates);
				
				mFwdsGrid.resize(mFwdsStartGrid.size());
				mFwdsTermMat.resize(mFwdsStartGrid.size());
				for (i = 0; i < mFwdsGridMat.size(); i++)
				{
					mFwdsTermMat[i].push_back(taus[i]);
					// calc fixing
					date = startDates[i];
					LADate fixingDate = cal_.getBusinessDay(startDates[i], -SPOTLAG);
					mFwdsGrid[i] = daycount_path.getTerm(asof, fixingDate);
				}
				const double maxTerm = mpPath->getSDETimeGrid().get().back();

				unsigned int pos = mFwdsGrid.size();
				while (maxTerm < mFwdsGrid[pos - 1])
				{
					--pos;
				}

				mFwdsGrid.erase(mFwdsGrid.begin() + pos, mFwdsGrid.end());
				mFwdsStartGrid.erase(mFwdsStartGrid.begin() + pos, mFwdsStartGrid.end());
				mFwdsGridMat.erase(mFwdsGridMat.begin() + pos, mFwdsGridMat.end());
				mFwdsTermMat.erase(mFwdsTermMat.begin() + pos, mFwdsTermMat.end());

				calcSpreadMat(curve, mFwdsGridMat);
			}
			else
			{
				mIsSameFwds = false;
			}

			//dynamic_cast<LAPriceDataInterpolation &>(mpFWDInter->get()).set(terms, rates);
		}
	}
}

/*!
    @brief calculate basis spread

	@param[in] spreadSet

*/
void
LAMathIndexEntity::calcSpreadMat(const LAMathYieldCurve &curve, const DoubleMatrix &gridMat)
{	
	// get basis df
	const LAString &curveName_b = dynamic_cast<const LADataString &>(mpBasis->get()).get();
	const LAInterpolationBase &dfInter_b = curve.getDFInterpolation(&curveName_b);
	const LAInterpolationBase &dfInter = curve.getDFInterpolation();

	mSpreadMat.resize(gridMat.size());
	for (unsigned i = 0; i < gridMat.size(); i++)
	{
		mSpreadMat[i].resize(gridMat[i].size(), 0.0);
		for (unsigned int j = 0; j < gridMat[i].size(); ++j)
		{
			if (gridMat[i][j] == 0.0)
			{
				mSpreadMat[i][j] = 1.0;
			}
			else
			{
				mSpreadMat[i][j] = dfInter_b.value(gridMat[i][j]) / dfInter.value(gridMat[i][j]);
			}			
		}
	}

	const LAInterpolationBase *pInter_df;
	if (mpDiscountCurve->isDefined() && !mpDiscountCurve->isNull())
	{
		const LAString &curveName_df = dynamic_cast<const LADataString &>(mpDiscountCurve->get()).get();
		pInter_df = &curve.getDFInterpolation(&curveName_df);
	}
	else
	{
		pInter_df = &curve.getBasisDFInterpolation();
	}
	mSpreadMat_DF.resize(gridMat.size());
	for (unsigned i = 0; i < gridMat.size(); i++)
	{
		mSpreadMat_DF[i].resize(gridMat[i].size(), 0.0);
		for (unsigned int j = 0; j < gridMat[i].size(); ++j)
		{
			if (gridMat[i][j] == 0.0)
			{
				mSpreadMat_DF[i][j] = 1.0;
			}
			else
			{
				mSpreadMat_DF[i][j] = pInter_df->value(gridMat[i][j]) / dfInter.value(gridMat[i][j]);
			}			
		}
	}
}

/*!
	@brief set up this class for ir index calculation
	
	@note this method is called from setUp method
*/
void
LAMathIndexEntity::setUpforIRforPlainVanilla(void)
{
	LAStringVector sde_ircurs = mpVanilla->getIRCurrencys().get();
	unsigned int i;
	for (i = 0; i < sde_ircurs.size(); i++)
	{
		if (sde_ircurs[i] == getCurrency().get())
		{
			mSDEPos = i;
			break;
		}		
	}
	if (i == sde_ircurs.size())
	{
		//error
		throw LACoreInvalidData("There is not sde correspond to this index", __FILE__, __LINE__);		
	}

	LAString freq = getFrequency();
	freq.toUpper();
	//const LAString& accessory = getAccessory().get();
	const LAString& tmpaccessory = getAccessory().get();
	LAString accessory(tmpaccessory);
	bool iscoterm = (accessory.findString("Co-Term") != -1); 
	if(iscoterm)
	{
		accessory.remove(accessory.findString("Co-Term"),7);
		accessory.trimLeft();
	}
	
	int y, m, d, w;
	LAMathDateCalculations::termStrtoYMDW(accessory, y, m, d, w);
	if (freq != SIMPLE && d != 0)
	{
		throw LACoreInvalidData("d != 0 is not support", __FILE__, __LINE__);	
	}
	if (freq != SIMPLE && 0 == y && 0 == m)
	{
		throw LACoreInvalidData("This Accessory input supports only y,m,d or (Co-term) y,m,d",__FILE__,__LINE__);
	}
	const LAPriceDataCalendar& cal = getCalendar();
	const LAPriceDataSlidingRule& srule = getSlidingRule();
	const LAPriceDataDayCount& daycount_index = getDayCount();
	const LAPriceDataDayCount& daycount_path = mpVanilla->getDayCount();

	const LADate& asof = mpVanilla->getAsOfDate().get();

	mGridMat.resize(mDateGrid.size());
	mTermMat.resize(mDateGrid.size());
	unsigned int size = 0;
	unsigned int addmonth = 0;
	unsigned int addday = 0;
	if (freq == SIMPLE)
	{
		size = 2;
		addmonth = y * 12 + m;
		addday = d;
	}
	else if (freq == MONTHLY)
	{
		size = y * 12 + m + 1;
		addmonth = 1;
	}
	else if (freq == QUARTERLY)
	{
		if (m % 3 != 0)
		{
			throw LACoreInvalidData("Frequency and Accessory are not consistent", __FILE__, __LINE__);	
		}
		size = y * 4 + m / 3 + 1;
		addmonth = 3;

	}
	else if (freq == SEMIANNUAL)
	{
		if (m % 6 != 0)
		{
			throw LACoreInvalidData("Frequency and Accessory are not consistent", __FILE__, __LINE__);	
		}	
		size = y * 2 + m / 6 + 1;
		addmonth = 6;

	}
	else if (freq == ANNUAL)
	{
		if (m % 12 != 0)
		{
			throw LACoreInvalidData("Frequency and Accessory are not consistent", __FILE__, __LINE__);
		}
		size = y + m / 12 + 1;
		addmonth = 12;
	}
	else
	{
		//error
		LAString err = "Frequency: ";
		err += freq;
		err += " is not support";
		throw LACoreInvalidData(err.getCString(), __FILE__, __LINE__);
	}

	unsigned int spotlag = 0;
	map<LAString, unsigned int>::const_iterator it = mSpotLag.find(getCurrency().get());
	if (it != mSpotLag.end())
		spotlag = it->second;
	else
		throw LACoreInvalidData("Index Spot Lag Error",__FILE__,__LINE__);

	int rollday = 0;
	LADataHolder* dh = &getData(PRICING_DATA_ROLLDAYFORINDEXGENERATE);
	if (dh->isDefined() && !dh->isNull())
		rollday = dynamic_cast<LADataInt &>(dh->get()).get();

	bool isgdatesexist = (!mDatesForGenerate.empty());
	
	mSpreadMat.clear();
	mSpreadMat_DF.clear();
	LADate date, date_nonadjust, olddate;
	if(iscoterm)
	{
		LADate terminal_nonadj = cal.getBusinessDay(asof,spotlag);
		terminal_nonadj.addMonths(addmonth * (size-1));
		if (rollday != 0)
		{
			//roll set
			if (rollday > LOOKUP_TABLE_NUMBER_OF_DAYS_IN_A_MONTH_OR_YEAR[IS_LEAP_YEAR(terminal_nonadj.yearOfEra())][0][terminal_nonadj.monthOfYear() - 1])		
				terminal_nonadj.setDay(LOOKUP_TABLE_NUMBER_OF_DAYS_IN_A_MONTH_OR_YEAR[IS_LEAP_YEAR(terminal_nonadj.yearOfEra())][0][terminal_nonadj.monthOfYear() - 1]);
			else terminal_nonadj.setDay(rollday);
		}
		
		LADate terminal = srule.getDate(terminal_nonadj,cal);
		for (i = 0; i < mGridMat.size(); i++)
		{
			mGridMat[i].clear();
			mTermMat[i].clear();
			date = cal.getBusinessDay(mDateGrid[i], spotlag);
			date_nonadjust = date;
			double grid0 = daycount_path.getTerm(asof, date);
			mGridMat[i].push_back(grid0);//spot date

			if(terminal <= date)
			{
				//temporary term;
				mTermMat[i].push_back(1.0);
				mGridMat[i].push_back(grid0);
				continue;
			}
			bool chk = false;
			for(unsigned int j = 1; j < size && chk == false; j++)
			{
				olddate = date;
				date_nonadjust.addMonths(addmonth);
				if (rollday != 0)
				{
					//roll set
					if (rollday > LOOKUP_TABLE_NUMBER_OF_DAYS_IN_A_MONTH_OR_YEAR[IS_LEAP_YEAR(date_nonadjust.yearOfEra())][0][date_nonadjust.monthOfYear() - 1])		
						date_nonadjust.setDay(LOOKUP_TABLE_NUMBER_OF_DAYS_IN_A_MONTH_OR_YEAR[IS_LEAP_YEAR(date_nonadjust.yearOfEra())][0][date_nonadjust.monthOfYear() - 1]);
					else date_nonadjust.setDay(rollday);
				}
				date = srule.getDate(date_nonadjust, cal);
				chk = (terminal <= date);
				if(chk)
				{
					date_nonadjust = terminal_nonadj;
					date = terminal;
					mGridMat[i].push_back(daycount_path.getTerm(asof, date));
					mTermMat[i].push_back(daycount_index.getTerm(olddate, date, false));
					continue;
				}
				else
				{
					mGridMat[i].push_back(daycount_path.getTerm(asof, date));
					mTermMat[i].push_back(daycount_index.getTerm(olddate, date, false));
				
				}
			}
		}
		//if (mpBasis->isDefined() && !mpBasis->isNull())
		//{
		//	LAMathYieldCurve& curve = dynamic_cast<LAMathYieldCurve &>(mpVanilla->getIRCurves().get(mSDEPos).get());
		//	calcSpreadMat(curve, mGridMat);
		//	//calcSpreadMat(spreadSet);
		//}
		return;
	}

	//set payment lag
	const LAString* pPaymentLag = NULL;
	dh = &(getData(PRICING_DATA_PAYMENTLAG, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		pPaymentLag = &dynamic_cast<const LADataString &>(dh->get()).get();
	}

	for (i = 0; i < mGridMat.size(); i++)
	{
		
		if (isgdatesexist)
		{
			std::map<LADate,DateVector>::const_iterator it = mDatesForGenerate.find(mDateGrid[i]);
			if (it != mDatesForGenerate.end())
			{
				//check size
				if (it->second.size() != size)
					throw LACoreInvalidData("Generate Dates is not consisitent with GridMat",__FILE__,__LINE__);
				mGridMat[i].resize(size);
				mTermMat[i].resize(size - 1);
				mGridMat[i][0] = daycount_path.getTerm(asof,it->second[0]);
				for (unsigned int j = 1; j < size; j++)
				{
					mGridMat[i][j] = daycount_path.getTerm(asof,it->second[j]);
					mTermMat[i][j - 1] = daycount_index.getTerm(it->second[j-1],it->second[j], false);
				}
				
				continue;
			}
		}


		mGridMat[i].resize(size);
		mTermMat[i].resize(size - 1);
		LADate spotDate = cal.getBusinessDay(mDateGrid[i], spotlag);
		mGridMat[i][0] = daycount_path.getTerm(asof, spotDate);//spot date
		if (pPaymentLag)
		{
			date = LAMathDateCalculations::getDate(spotDate, *pPaymentLag, false);
			date = srule.getDate(date, cal);
			//check
			LADate date_first_payment = date;
			date_first_payment.addMonths(addmonth);
			if (spotDate >= date_first_payment)
				throw LACoreInvalidData("Payment lag is bigger than the index frequency!", __FILE__, __LINE__);
		}
		else
		{
			date = spotDate;
		}
		olddate = spotDate;
		date_nonadjust = date;
	
		for (unsigned int j = 1; j < size; j++)
		{
			date_nonadjust.addMonths(addmonth);
			date_nonadjust.addDays(addday);
			if (rollday != 0)
			{
				//roll set
				if (rollday > LOOKUP_TABLE_NUMBER_OF_DAYS_IN_A_MONTH_OR_YEAR[IS_LEAP_YEAR(date_nonadjust.yearOfEra())][0][date_nonadjust.monthOfYear() - 1])		
					date_nonadjust.setDay(LOOKUP_TABLE_NUMBER_OF_DAYS_IN_A_MONTH_OR_YEAR[IS_LEAP_YEAR(date_nonadjust.yearOfEra())][0][date_nonadjust.monthOfYear() - 1]);
				else date_nonadjust.setDay(rollday);
			}
			date = srule.getDate(date_nonadjust, cal);
			mGridMat[i][j] = daycount_path.getTerm(asof, date);		
			mTermMat[i][j - 1] = daycount_index.getTerm(olddate, date, false);
			olddate = date;
		}
	}
	//if (mpBasis->isDefined() && !mpBasis->isNull())
	//{
	//	LAMathYieldCurve& curve = dynamic_cast<LAMathYieldCurve &>(mpVanilla->getIRCurves().get(mSDEPos).get());
	//	calcSpreadMat(curve, mGridMat);
	//	//calcSpreadMat(spreadSet);
	//}

	LAString indexType = getIndexType();
	indexType.toUpper();
	if (mpIsFWDInter->isDefined() && !mpIsFWDInter->isNull() && dynamic_cast<const LADataBool &>(mpIsFWDInter->get()).get())
	{
		if (mpBasis->isDefined() && !mpBasis->isNull())
		{
			mCurveType = dynamic_cast<const LADataString &>(mpBasis->get()).get();
		}
		// to save mFwdsStartEndDate
		LAMathYieldCurve& curve = dynamic_cast<LAMathYieldCurve &>(mpVanilla->getIRCurves().get(mSDEPos).get());
		DoubleArray taus;
		curve.getBaseForwardRate(mCurveType, mFwdsStartGrid, mFwdsGridMat, taus, mFwds, &mFwdsStartEndDate);

		if (indexType == LIBOR)
		{
			LAString freq_fwd;
			LAPriceDataSlidingRule sld_fwd;
			LAPriceDataDayCount dc_fwd;
			LAPriceDataCalendar cal_fwd;
			LAString accessory;
			//getYieldCurvePro().getForwardConvention(curveType, dc, sld, cal, accessory);
			curve.getCurveConvention(freq_fwd, cal_fwd, sld_fwd, dc_fwd, accessory, mCurveType);
			if (LAMathDateCalculations::getDate(asof, accessory, true) == LAMathDateCalculations::getDate(asof, getAccessory().get(), true))
			{
				mIsSameFwds = true;
				if (dc_fwd.getDayCount() != getDayCount().getDayCount())
				{
					mIsDaycountAdj = true;
				}
			}
			else
			{
				mIsSameFwds = false;
			}
		}
		else if (indexType == CMS)
		{
			LAString freq_fwd;
			LAPriceDataSlidingRule sld_fwd;
			LAPriceDataDayCount dc_fwd;
			LAPriceDataCalendar cal_fwd;
			LAString accessory;
			//getYieldCurvePro().getForwardConvention(curveType, dc, sld, cal, accessory);
			curve.getCurveConvention(freq_fwd, cal_fwd, sld_fwd, dc_fwd, accessory, mCurveType);
			int span = LAMathDateCalculations::getPeriodFrequencyInMonths(getFrequency().get());
			LAString termOfOnePeriod = LAString(span) + "M";
			if (LAMathDateCalculations::getDate(asof, accessory, true) == LAMathDateCalculations::getDate(asof, termOfOnePeriod, true))
			{
				mIsSameFwds = true;
				//dynamic_cast<LAPriceDataInterpolation &>(mpFWDInter->get()).set(mFwdsStartGrid, mFwds);
				//calculate taus of forward rates
				mFloatTermMat.resize(mGridMat.size());
				for (i = 0; i < mGridMat.size(); i++)
				{
					if (isgdatesexist)
					{
						std::map<LADate,DateVector>::const_iterator it = mDatesForGenerate.find(mDateGrid[i]);
						if (it != mDatesForGenerate.end())
						{
							//check size
							if (it->second.size() != size)
								throw LACoreInvalidData("Generate Dates is not consisitent with GridMat",__FILE__,__LINE__);

							for (unsigned int j = 1; j < size; j++)
							{
								mFloatTermMat[i][j - 1] = dc_fwd.getTerm(it->second[j-1],it->second[j], false);
							}
							
							continue;
						}
					}

					mFloatTermMat[i].resize(size - 1);
					LADate spotDate = cal.getBusinessDay(mDateGrid[i], spotlag);
					if (pPaymentLag)
					{
						date = LAMathDateCalculations::getDate(spotDate, *pPaymentLag, false);
						date = srule.getDate(date, cal);
						LADate date_first_payment = date;
						date_first_payment.addMonths(addmonth);
						if (mDateGrid[i] >= date_first_payment)
							throw LACoreInvalidData("Payment lag is bigger than the index frequency!", __FILE__, __LINE__);
					}
					else
					{
						date = spotDate;
					}
					olddate = spotDate;
					date_nonadjust = date;
				
					for (unsigned int j = 1; j < size; j++)
					{
						date_nonadjust.addMonths(addmonth);
						date_nonadjust.addDays(addday);
						if (rollday != 0)
						{
							//roll set
							if (rollday > LOOKUP_TABLE_NUMBER_OF_DAYS_IN_A_MONTH_OR_YEAR[IS_LEAP_YEAR(date_nonadjust.yearOfEra())][0][date_nonadjust.monthOfYear() - 1])		
								date_nonadjust.setDay(LOOKUP_TABLE_NUMBER_OF_DAYS_IN_A_MONTH_OR_YEAR[IS_LEAP_YEAR(date_nonadjust.yearOfEra())][0][date_nonadjust.monthOfYear() - 1]);
							else date_nonadjust.setDay(rollday);
						}
						date = srule.getDate(date_nonadjust, cal);
						mFloatTermMat[i][j - 1] = dc_fwd.getTerm(olddate, date, false);
						olddate = date;
					}
				}
			}
			else
			{
				mIsSameFwds = false;
			}
		}
	}
	
	// set up information of convexity adjust
	if (indexType == LIBOR)
	{
		dh = &getData(PRICING_DATA_CAMODEL, NOCHECK);
		if (dh->isDefined() && !dh->isNull())
		{
			mConvexityAdjustModel = dynamic_cast<const LADataString&> (dh->get()).get();
			if (isDelayedConvexityAdjusted())
			{
				mConvexityAdjustThreshold = dynamic_cast<const LADataDouble&> ((getData(PRICING_DATA_CATHRESHOLD, ISNOTNULL)).get()).get();
			}
		}
	}
	else if (indexType == CMS)
	{
		mConvexityAdjustModel = dynamic_cast<const LADataString&> ((getData(PRICING_DATA_CAMODEL, ISNOTNULL)).get()).get();
		if (mConvexityAdjustModel.findString(CMS_CA_SABR) >= 0)
		{
			mConvexityAdjustThreshold = dynamic_cast<const LADataDouble&> ((getData(PRICING_DATA_CATHRESHOLD, ISNOTNULL)).get()).get();
			// calculate mCouponTerm
			LAString termOfOnePeriod;
			LAPriceDataDayCount& dc_cf = dynamic_cast<LAPriceDataDayCount&> ((getData(PRICING_DATA_CFDAYCOUNT, ISNOTNULL)).get());
			const LAString& freq_cf = dynamic_cast<const LADataString&> ((getData(PRICING_DATA_CFFREQUENCY, ISNOTNULL)).get());
			if (freq_cf == BUSINESS_DAYS || freq_cf == DAILY )
			{
				termOfOnePeriod = "1D";
			}
			else

			{
				int span = LAMathDateCalculations::getPeriodFrequencyInMonths(freq_cf);
				termOfOnePeriod = LAString(span) + "M";
			}

			std::vector<LADate> startDate;
			std::vector<LADate> endDate;
			startDate = dynamic_cast<const LADataDates &>((getData(PRICING_DATA_CFCALCSTARTDATES, ISNOTNULL)).get()).get();
			endDate = dynamic_cast<const LADataDates &>((getData(PRICING_DATA_CFCALCENDDATES, ISNOTNULL)).get()).get();
			if (dc_cf.getDayCount() == ACT_ACT_ICMA)
			{
				int span = LAMathDateCalculations::getPeriodFrequencyInMonths(freq_cf);
				dc_cf.setCouponStartDates(startDate);
				dc_cf.setCouponEndDates(endDate);
				dc_cf.setCouponsInYear(12/span);
			}
			for (i = 0; i < mDateGrid.size(); ++i)
			{
				mCouponTerm.push_back( dc_cf.getTerm(startDate[i], endDate[i]) );
			}

			mConvexityAdjustIC = dynamic_cast<const LADataDoubles&> ((getData(PRICING_DATA_CAINTEGRALCONDITION, ISNOTNULL)).get()).get();
			mConvexityAdjustPremIntegralStep = 20;
			dh = &getData(PRICING_DATA_CAPREMIUMINTEGRALSTEP, NOCHECK);
			if (dh->isDefined() && !dh->isNull())
			{
				mConvexityAdjustPremIntegralStep = dynamic_cast<const LADataInt &>(dh->get()).get();
			}
		}
	}
	//for analytic
	mDFMat.resize(mGridMat.size());
	for (i = 0; i < mGridMat.size(); i++)
		mDFMat[i].resize(mGridMat[i].size());
}
/*!
	@brief set up this class for fx index calculation
	
	@note this method is called from setUp method
*/
void
LAMathIndexEntity::setUpforFX(void)
{
	mpFX = &dynamic_cast<LAMathFXEntity&>(getFXEntity().get().get());
	
	const LAString& currency = getCurrency().get();
    const LAStringVector& curs = LAMathFXUtility::getCurrencyPair(currency);
	mToCurrency = curs[0];
	mFromCurrency = curs[1];

	const LADate& asof = mpPath->getAsOfDate();
	const LAPriceDataDayCount& daycount_path = mpPath->getDayCount();
	
	LAString accessory = getAccessory().get();
	accessory.toUpper();
	mForwardTimes.resize(mTimeGrid.size());
	for (unsigned int i = 0; i < mForwardTimes.size(); i++)
	{
		const LADate& spotdate = mpFX->getSpotDate(mFromCurrency, mToCurrency, mDateGrid[i]);
		if (accessory == SPOT)
			mForwardTimes[i] = daycount_path.getTerm(asof, spotdate);
		else
		{
			const LAPriceDataCalendar& cal = getCalendar();
			const LAPriceDataSlidingRule& srule = getSlidingRule();
			//const LADate& forwarddate = mpFX->getForwardDate(mFromCurrency, mToCurrency, spotdate, accessory);
			const LADate& forwarddate = LAMathDateCalculations::getDate(spotdate, accessory, srule, &cal, true);
			mForwardTimes[i] = daycount_path.getTerm(asof, forwarddate);
		}
	}
}

/*!
	@brief set up this class for fx index calculation
	
	@note this method is called from setUp method
*/
void
LAMathIndexEntity::setUpforFXforPlainVanilla(void)
{
	mpFX = &dynamic_cast<LAMathFXEntity&>(getFXEntity().get().get());
	
	const LAString& currency = getCurrency().get();
    const LAStringVector& curs = LAMathFXUtility::getCurrencyPair(currency);
	mToCurrency = curs[0];
	mFromCurrency = curs[1];

	const LADate& asof = mpVanilla->getAsOfDate();
	const LAPriceDataDayCount& daycount_path = mpVanilla->getDayCount();
	
	LAString accessory = getAccessory().get();
	accessory.toUpper();
	mForwardTimes.resize(mTimeGrid.size());
	for (unsigned int i = 0; i < mForwardTimes.size(); i++)
	{
		const LADate& spotdate = mpFX->getSpotDate(mFromCurrency, mToCurrency, mDateGrid[i]);
		if (accessory == SPOT)
			mForwardTimes[i] = daycount_path.getTerm(asof, spotdate);
		else
		{
			const LAPriceDataCalendar& cal = getCalendar();
			const LAPriceDataSlidingRule& srule = getSlidingRule();
			//const LADate& forwarddate = mpFX->getForwardDate(mFromCurrency, mToCurrency, spotdate, accessory);
			const LADate& forwarddate = LAMathDateCalculations::getDate(spotdate, accessory, srule, &cal, true);
			mForwardTimes[i] = daycount_path.getTerm(asof, forwarddate);
		}
	}
}

// set spot lag
void 
LAMathIndexEntity::setSpotLag(const LAString &ccy, unsigned int lag)
{
	mSpotLag.insert(pair<LAString, unsigned int>(ccy, lag));
}

// set indexgeneratedates
void 
LAMathIndexEntity::setDatesForIndexGenerates(const std::map<LADate, DateVector>& datesmap)
{
	
	if (mDatesForGenerate.size() == 0)
		mDatesForGenerate = datesmap;
	else
	{

		std::map<LADate, DateVector>::const_iterator it = datesmap.begin();
		for (it = datesmap.begin(); it != datesmap.end(); ++it)
		{
			mDatesForGenerate[it->first] = it->second;
			//check is exist fixing date
			//std::map<LADate, DateVector>::const_iterator it2 = mDatesForGenerate.find(it->first);
			//if (it2 == mDatesForGenerate.end())
			//	mDatesForGenerate.insert(std::make_pair(it->first,it->second));
			//else
			//{
			//	if (it->second != it2->second)
			//		throw LACoreInvalidData("Indexdate must be 1 to 1 for Dates for Index Generate",__FILE__,__LINE__);
			//	else
			//		return;
			//}
		}
	}
}

bool
LAMathIndexEntity::isDelayedConvexityAdjusted() const
{
	const LAString& indextype = getIndexType();

	return (indextype == LAString(LIBOR)) && LAMathIndexEntity::isDelayedConvexityAdjustModel(mConvexityAdjustModel);
}

bool
LAMathIndexEntity::isDelayedConvexityAdjustModel(const LAString& caModel)
{
	LAString tmpCaModel = caModel;
	tmpCaModel.toUpper();
	return (tmpCaModel == DELAYED_CA);
}

// @brief get(calc) the value of the convexity adjustment for the LIBOR rate
double
LAMathIndexEntity::getLIBORConvexityAdjust(double forward, double fixingterm, double timingterm, double dfEnd, double dfPayment, unsigned int curpos)
{
//because vol model is not set
#ifndef VISUAL_STUDIO_2010_ANALYTICS

	mConvexityAdjust[curpos] = 0.;
	mConvexityAdjustVol[curpos] = 0.;

	if (!isDelayedConvexityAdjusted())
	{
		return 0.0;
	}

	LADataMultiReference* pirvols = &(mpVanilla->getIRVolatilitys());
	if (pirvols == 0 || pirvols->getSize() == 0)
	{
		//throw LACoreInvalidData("IR Calibration Model For Convexity Adjust is not set", __FILE__, __LINE__);
		return 0.0;
	}

	LAString currency = getCurrency();
	const LAMathVolFuncIRSABR* method = dynamic_cast<const LAMathVolFuncIRSABR*>(mpVanilla->getIRVolFunc(currency));
	if (!method->isTypeOf(FN_VOLFUNCIRSABR))
	{
		//throw LACoreInvalidData("Convexity Adjust needs only SABR model now", __FILE__, __LINE__);
		return 0.0;
	}

	LAString accessory = getAccessory();
	int y,m,d, w;
	LAMathDateCalculations::termStrtoYMDW(accessory, y, m, d, w);
	double mtenorval = static_cast<double > (y) + static_cast<double > (m) / 12.;

	double alpha = method->getSABRParam(SABR_ALPHA, fixingterm, mtenorval);
	double beta  = method->getSABRParam(SABR_BETA,  fixingterm, mtenorval);
	double nu    = method->getSABRParam(SABR_NU,    fixingterm, mtenorval);
	double rho   = method->getSABRParam(SABR_RHO,   fixingterm, mtenorval);
	double shift = method->getForwardShiftValue();

	double F = (forward + shift);
	double vol = LAMathSABR_Hagan(alpha, beta, nu, rho).getSABRVol(fixingterm, LAMath::max(F, MIN_RATE), LAMath::max(F, MIN_RATE));
	vol = LAMath::min(vol, mConvexityAdjustThreshold);

	double ret = (dfEnd / dfPayment) * timingterm * F * F * (LAMath::exp(fixingterm*vol*vol) - 1.0);

	mConvexityAdjust   [curpos] = ret;
	mConvexityAdjustVol[curpos] = vol;

	return ret;
#else
	return 0.0;
#endif
}

// get 
double 
LAMathIndexEntity::getConvexityAdjust(double forward, double optionterm, unsigned int curpos)
{
//because vol model is not set
#ifndef VISUAL_STUDIO_2010_ANALYTICS
	double ret = 0.0;

	LAString indextype = getIndexType();
	indextype.toUpper();
	if (indextype != "CMS" || optionterm == 0.)
		return 0.0;

	LAString currency = getCurrency();
	LADataMultiReference* pirvols = &(mpVanilla->getIRVolatilitys());
	if (pirvols == 0  || pirvols->getSize() == 0)
		throw LACoreInvalidData("IR Calibration Model For Convexity Adjust is not set", __FILE__,__LINE__);

	const LAMathVolFuncIRSABR* method = dynamic_cast<const LAMathVolFuncIRSABR*>(mpVanilla->getIRVolFunc(currency));
	if (!method->isTypeOf(FN_VOLFUNCIRSABR))
		throw LACoreInvalidData("Convexity Adjust needs only SABR model now", __FILE__,__LINE__);

	const LAString &underlying = getVolUnderlying().get();
	method->setUnderlying(underlying);

	LAString accessory = getAccessory();
	int y,m,d, w;
	LAMathDateCalculations::termStrtoYMDW(accessory, y, m, d, w);
	double mtenorval = static_cast<double > (y) + static_cast<double > (m) / 12.;

	if (mConvexityAdjustModel == CMS_CA_BLACK)
	{
		DoubleVector volvec(4,0.0);
		volvec[0] = optionterm;
		volvec[1] = mtenorval;
#ifdef ZEROFLOOR
		volvec[2] = LAMath::max(forward, MIN_RATE);
		volvec[3] = LAMath::max(forward, MIN_RATE);
#else
		volvec[2] = forward;
		volvec[3] = forward;
#endif
		
		double vol = method->operator ()(volvec);
		mConvexityAdjustVol[curpos] = vol;
		double conv = 0.0;
		double pvbp = 0.0;

		if (mSpreadMat.empty())
		{
			const LAMathYieldCurve& curve = dynamic_cast<const LAMathYieldCurve &>(mpVanilla->getIRCurves().get(mSDEPos).get());
		
			//calc index
			double T0 = mGridMat[curpos][0];
			for (unsigned int j = 1; j < mGridMat[curpos].size(); j++)
			{
				double Ti = mGridMat[curpos][j];
				double Ti_last = mGridMat[curpos][j-1];
				pvbp += forward * mTermMat[curpos][j - 1] * curve.getDF(Ti_last) * (Ti - T0) /curve.getDF(T0);
				conv += forward * mTermMat[curpos][j - 1] * curve.getDF(Ti) * (Ti - T0) * (Ti - T0 + 1.) /curve.getDF(T0);

			}

			double Tn = mGridMat[curpos].back();
			double Tn_last = mGridMat[curpos][mGridMat[curpos].size() - 2];
			pvbp += curve.getDF(Tn_last) * (Tn - T0) / curve.getDF(T0);
			conv += curve.getDF(Tn) * (Tn - T0) * (Tn - T0 + 1.)/ curve.getDF(T0);
		}
		else
		{
			const LAMathYieldCurve& curve = dynamic_cast<const LAMathYieldCurve &>(mpVanilla->getIRCurves().get(mSDEPos).get());

			//calc index
			double T0 = mGridMat[curpos][0];
			for (unsigned int j = 1; j < mGridMat[curpos].size(); j++)
			{
				double Ti = mGridMat[curpos][j];
				double Ti_last = mGridMat[curpos][j-1];
				pvbp += forward * mTermMat[curpos][j - 1] * curve.getDF(Ti_last) * mSpreadMat[curpos][j-1] * (Ti - T0) / (curve.getDF(T0) * mSpreadMat[curpos][0]);
				conv += forward * mTermMat[curpos][j - 1] * curve.getDF(Ti) * mSpreadMat[curpos][j] * (Ti - T0) * (Ti - T0 + 1.) / ( curve.getDF(T0) * mSpreadMat[curpos][0]);
			}

			double Tn = mGridMat[curpos].back();
			double Tn_last = mGridMat[curpos][mGridMat[curpos].size() - 2];
			pvbp += curve.getDF(Tn_last) * mSpreadMat[curpos][mGridMat[curpos].size() - 2] * (Tn - T0) / (curve.getDF(T0) * mSpreadMat[curpos][0]);
			conv += curve.getDF(Tn) * mSpreadMat[curpos].back() * (Tn - T0) * (Tn - T0 + 1.)/ (curve.getDF(T0) * mSpreadMat[curpos][0]);
		}

		if (0.0==pvbp)
			throw LACoreInvalidData("Convexity Error",__FILE__,__LINE__);
		ret = + 0.5 * forward * forward * vol * vol * optionterm * conv / pvbp;
	}
	else if (mConvexityAdjustModel.findString(CMS_CA_SABR) >= 0)
	{
		// calculate tau by frequency
		LAString freq = getFrequency();
		freq.toUpper();
		double tau = 0.;
		if (freq == MONTHLY)
		{
			tau = 1. / 12.;
		}
		else if (freq == QUARTERLY)
		{
			tau = 1. / 4.;
		}
		else if (freq == SEMIANNUAL)
		{
			tau = 1. / 2.;
		}
		else if (freq == ANNUAL)
		{
			tau = 1.;
		}
		else
		{
			//error
			LAString err = "Frequency: ";
			err += freq;
			err += " is not support in SABR convexity adjust.";
			throw LACoreInvalidData(err.getCString(), __FILE__, __LINE__);
		}
		// create sabr parameter arguement
		DoubleArray sabr_params;
		sabr_params.push_back(method->getSABRParam(SABR_ALPHA, optionterm, mtenorval));
		sabr_params.push_back(method->getSABRParam(SABR_BETA, optionterm, mtenorval));
		sabr_params.push_back(method->getSABRParam(SABR_NU, optionterm, mtenorval));
		sabr_params.push_back(method->getSABRParam(SABR_RHO, optionterm, mtenorval));
		if (mConvexityAdjustModel == CMS_CA_SABR || mConvexityAdjustModel == CMS_CA_SABR_HAGAN)
		{
			// calculate SABR convexity adjustment
			ret = LAMathIRVanillaFuncUtility::getSABRConvexityAdjust(	LAMath::max(forward, MIN_RATE),
																	optionterm,
																	tau,
																	mCouponTerm[curpos],
																	mTermMat[curpos].size(),
																	sabr_params,
																	mConvexityAdjustThreshold,
																	mConvexityAdjustIC);
			// calculate SABR vol
			LAMathSABR_Hagan sabr(sabr_params[0], sabr_params[1], sabr_params[2], sabr_params[3]);
			mConvexityAdjustVol[curpos] = sabr.getSABRVol(optionterm, LAMath::max(forward, MIN_RATE), LAMath::max(forward, MIN_RATE));
		}
		else if (mConvexityAdjustModel == CMS_CA_SABR_ANTONOV)
		{
			const double int_regime = 0.;
			// calculate SABR Antonov convexity adjustment
			ret = LAMathIRVanillaFuncUtility::getSABRConvexityAdjust2(LAMath::max(forward, MIN_RATE),
																	optionterm,
																	tau,
																	mCouponTerm[curpos],
																	mTermMat[curpos].size(),
																	sabr_params,
																	int_regime,
																	mConvexityAdjustIC,
																	mConvexityAdjustPremIntegralStep);
			// calculate SABR vol
			LAMathSABR_Antonov sabr(sabr_params[0], sabr_params[1], sabr_params[2], sabr_params[3]);
			mConvexityAdjustVol[curpos] = sabr.getSABRVol(optionterm, LAMath::max(forward, MIN_RATE), LAMath::max(forward, MIN_RATE));

		}
		else
		{
			LAString err = mConvexityAdjustModel + " is not supported in convexity adjust!";
			throw LACoreInvalidData(err.getCString(), __FILE__, __LINE__);
		}
	}
	else if (mConvexityAdjustModel == CMS_CA_ZERO)
	{
		ret = 0.;
		mConvexityAdjustVol[curpos] = 0.;
	}
	else
	{
		LAString err = mConvexityAdjustModel + " is not supported in convexity adjust!";
		throw LACoreInvalidData(err.getCString(), __FILE__, __LINE__);
	}

	mConvexityAdjust[curpos] = ret;
	return ret;
#else
	return 0.0;
#endif
}

///*!
//    @brief get yield curve pro
//
//	@return LAMathYieldCurvePro
//*/
//const LAMathYieldCurvePro& 
//LAMathIndexEntity::getYieldCurvePro(void) const 
//{
//	if (mpVanilla)
//	{
//		return dynamic_cast<const LAMathYieldCurvePro &>(mpVanilla->getIRCurvePros().get(mSDEPos).get());
//	}
//	else if (mpPath)
//	{
//		return dynamic_cast<const LAMathYieldCurvePro &>(mpPath->getIRCurvePros().get(mSDEPos).get());
//	}
//	else
//	{
//		throw LACoreInvalidData("Both mpVanilla and mpPath are NULL.", __FILE__, __LINE__);
//	}
//}