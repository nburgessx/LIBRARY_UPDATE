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

#include "LAPriceCouponMin.h"
using namespace std;

/*!
    @brief constructor
*/
LAPriceCouponMin::LAPriceCouponMin()
: LAPriceCouponBase()
{

}
/*!
    @brief destructor

*/
LAPriceCouponMin::~LAPriceCouponMin()
{
}

/*
    @brief Check function for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
LAPriceCouponMin::isTypeOf(function_t id) const
{
	return (id == FN_CPNSLTOPERATORMIN ? true : LAFunctionBase::isTypeOf(id));
}

/*!
	@brief select coupon
	@param[in] x coupons
	@note pure virtual method
*/
double				
LAPriceCouponMin::operator()(const DoubleArray& x) const
{
	unsigned int pos;
	return selectCoupon(x, pos);
}

/*
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
LACoreFunctionBase*
LAPriceCouponMin::clone() const
{
    try 
	{
        return new LAPriceCouponMin(*this);
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
LAPriceCouponMin::getType() const
{
	return FN_CPNSLTOPERATORMIN;
}

/*!
	@brief select coupon
	@param[in] x coupons
	@param[out] pos selected position of coupons
	@note pure virtual method
*/
double
LAPriceCouponMin::selectCoupon(const DoubleArray& x, unsigned int& pos) const
{
	double min = x.at(0);
	pos = 0;

	for (unsigned int i = 1; i < x.size(); i++)
	{
		double tmp = x[i]; 
		if (tmp < min)
		{	
			min = tmp;
			pos = i;
		}
	}

	return x[pos];
}


