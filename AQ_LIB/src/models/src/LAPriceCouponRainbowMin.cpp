/*! @file
    @brief source code of class of coupon rainbow type select operator.

*/
//   2007,Mizuho International London.
// 
//////////////////////////////////////////////////////////
//いくかのパターンでテスト実施ずみ
//////////////////////////////////////////////////////////
#ifdef __GNUG__
#pragma implementation
#endif
#ifdef _MSC_VER
#pragma warning(disable:4786)
#endif

#include "LAPriceCouponRainbowMin.h"
using namespace std;

/*!
    @brief constructor
*/
LAPriceCouponRainbowMin::LAPriceCouponRainbowMin()
: LAPriceCouponRainbow()
{

}
/*!
    @brief destructor

*/
LAPriceCouponRainbowMin::~LAPriceCouponRainbowMin()
{
}

/*
    @brief Check function for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
LAPriceCouponRainbowMin::isTypeOf(function_t id) const
{
	return (id == FN_CPNSLTOPERATORRAINBOWMIN ? true : LAFunctionBase::isTypeOf(id));
}

/*
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
LACoreFunctionBase*
LAPriceCouponRainbowMin::clone() const    
{
    try 
	{
        return new LAPriceCouponRainbowMin(*this);
    }
    catch (bad_alloc & e)
	{
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }	
}


/*!
    @brief Return this class type
    @return function type5
*/
function_t			
LAPriceCouponRainbowMin::getType() const
{
	return FN_CPNSLTOPERATORRAINBOWMIN;
}

/*!
	@brief select coupon
	@param[in] x coupons
	@param[out] pos selected position of coupons
	@note pure virtual method
*/
double
LAPriceCouponRainbowMin::selectCoupon(const DoubleArray& x, unsigned int& pos) const
{
	double min = x.at(0) * 
		mpFX_for_Coupons.at(0)->getRate(mCouponsCur.at(0), mNotionalCur, mPaymentTime)
		/ mpFX_for_Notional->getRate(mCouponsCur.at(0), mNotionalCur, 0);
	pos = 0;

	for (unsigned int i = 1; i < mpFX_for_Coupons.size(); i++)
	{
		double tmp = x[i] * 
		mpFX_for_Coupons[i]->getRate(mCouponsCur[i], mNotionalCur, mPaymentTime)
		/ mpFX_for_Notional->getRate(mCouponsCur[i], mNotionalCur, 0);
		if (tmp < min)
		{	
			min = tmp;
			pos = i;
		}
	}

	return x[pos];
}


