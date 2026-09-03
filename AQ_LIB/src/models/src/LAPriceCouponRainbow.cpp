/*! @file
    @brief source code of class of coupon rainbow type select operator.

*/
// Copyright (C) 2006, AlgoQuantHub..
//////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////
#ifdef __GNUG__
#pragma implementation
#endif
#ifdef _MSC_VER
#pragma warning(disable:4786)
#endif

#include "LAPriceCouponRainbow.h"

#include "AQLObject.h"
#include "AQLObjectHolder.h"
#include "AQLDataBasics.h"
#include "AQLDataReference.h"
#include "AQLDataMultiReference.h"
#include "AQLPriceDataManager.h"

#include "AQLPriceDataDayCount.h"
#include "LAMathPathEntity.h"
#include "LAMathFXEntity.h"

using namespace std;

/*!
    @brief constructor
*/
LAPriceCouponRainbow::LAPriceCouponRainbow()
: LAPriceCouponBase(), mpFX_for_Notional(NULL)
{

}
/*!
    @brief destructor

*/
LAPriceCouponRainbow::~LAPriceCouponRainbow()
{
}

/*
    @brief Check function for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
LAPriceCouponRainbow::isTypeOf(function_t id) const
{
	return (id == FN_CPNSLTOPERATORRAINBOW ? true : AQLFunctionBase::isTypeOf(id));
}

/*
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*
LAPriceCouponRainbow::clone() const    
{
    try 
	{
        return new LAPriceCouponRainbow(*this);
    }
    catch (bad_alloc & e)
	{
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }	
}


/*!
    @brief Return this class type
    @return function type
*/
function_t			
LAPriceCouponRainbow::getType() const
{
	return FN_CPNSLTOPERATORRAINBOW;
}

/*!
	@brief register Datas that this class uses

	@param[in, out] dm Data master 
*/
void
LAPriceCouponRainbow::registerData(AQLPriceDataManager& dm) const
{
	dm.setData(PRICING_DATA_PATHENTITY,		DATA_REFERENCE);
	dm.setData(PRICING_DATA_CURRENCY,		DATA_STRING);
	dm.setData(PRICING_DATA_COUPONINFOS,	DATA_MULTIREFERENCE);
	dm.setData(PRICING_DATA_FXRATE,			DATA_REFERENCE);
	dm.setData(PRICING_DATA_PAYMENTDATE,	DATA_DATE);

}

/*!
	@brief set up this class
	@param[in] basedate basedate
	@param[in] trade trade entity
	@param[in] legNo leg number(first leg number = 0)
	@param[in] cashlet cashlet entity
	@param[in] payoff LAPricePayOff object
	@param[in] current position of payoff(first payoff position = 0)
*/
void
LAPriceCouponRainbow::setUp(const AQLDate& basedate, const AQLObject& trade, unsigned int legNo, const AQLObject& cashlet) 
{
	(void)basedate;	(void)legNo; //20070411--Nagase--gcc
	mpFX_for_Notional = NULL;
	mpFX_for_Coupons.clear();
	mCouponsCur.clear();

	//fx rate for notional
	const AQLDataHolder* ah = &(cashlet.getData(PRICING_DATA_FXRATE, NOCHECK));
	if (ah->isDefined() && !ah->isNull())
	{
		const AQLDataReference& ref = dynamic_cast<const AQLDataReference&>(ah->get());
		mpFX_for_Notional = &dynamic_cast<const LAMathFXEntity&>(ref.get().get());	
	}
	//notional currecny
	ah = &(cashlet.getData(PRICING_DATA_CURRENCY, NOCHECK));
	if (ah->isDefined() && !ah->isNull())
		mNotionalCur = dynamic_cast<const AQLDataString&>(ah->get()).get();	
	
	//coupon info
	ah = &(cashlet.getData(PRICING_DATA_COUPONINFOS, NOCHECK));
	if (!ah->isDefined() || ah->isNull()) return;//extra cf only
	const AQLDataMultiReference& couponinfos = dynamic_cast<const AQLDataMultiReference&>(ah->get());

	mpFX_for_Coupons.resize(couponinfos.getSize(), NULL);
	mCouponsCur.resize(couponinfos.getSize());
	for (unsigned int i = 0; i < couponinfos.getSize(); i++)
	{
		//coupon currecny
		ah = &(couponinfos.get(i).getData(PRICING_DATA_CURRENCY, ISNOTNULL));
		mCouponsCur[i] = dynamic_cast<const AQLDataString&>(ah->get()).get();
		//fx rate for notional
		ah = &(couponinfos.get(i).getData(PRICING_DATA_FXRATE, NOCHECK));
		if (ah->isDefined() && !ah->isNull())
		{
			const AQLDataReference& ref = dynamic_cast<const AQLDataReference&>(ah->get());
			mpFX_for_Coupons[i] = &dynamic_cast<const LAMathFXEntity&>(ref.get().get());	
		}		
	}

	//payment date
	ah = &(cashlet.getData(PRICING_DATA_PAYMENTDATE, ISNOTNULL));
	const AQLDate& paymentdate = dynamic_cast<const AQLDataDate&>(ah->get()).get();
	//path entity
	ah = &(trade.getData(PRICING_DATA_PATHENTITY, ISNOTNULL));
	const AQLDataReference& ref = dynamic_cast<const AQLDataReference&>(ah->get());
	const LAMathPathEntity& path = dynamic_cast<const LAMathPathEntity&>(ref.get().get());
	//daycount of path entity
	const AQLPriceDataDayCount& dc_path = path.getDayCount();
	//payment time
	mPaymentTime = dc_path.getTerm(path.getAsOfDate().get(), paymentdate);

}


/*!
	@brief select coupon
	@param[in] x coupons
	@note pure virtual method
*/
double				
LAPriceCouponRainbow::operator()(const DoubleArray& x) const
{
	unsigned int pos;
	return selectCoupon(x, pos);
	
}
/*!
	@brief select coupon
	@param[in] x coupons
	@param[out] pos selected position of coupons
	@note pure virtual method
*/
double
LAPriceCouponRainbow::selectCoupon(const DoubleArray& x, unsigned int& pos) const
{
	double max = x.at(0) * 
		mpFX_for_Coupons.at(0)->getRate(mCouponsCur.at(0), mNotionalCur, mPaymentTime)
		/ mpFX_for_Notional->getRate(mCouponsCur.at(0), mNotionalCur, 0);
	pos = 0;

	for (unsigned int i = 1; i < mpFX_for_Coupons.size(); i++)
	{
		double tmp = x[i] * 
		mpFX_for_Coupons[i]->getRate(mCouponsCur[i], mNotionalCur, mPaymentTime)
		/ mpFX_for_Notional->getRate(mCouponsCur[i], mNotionalCur, 0);
		if (tmp > max)
		{	
			max = tmp;
			pos = i;
		}
	}

	return x[pos];
}


