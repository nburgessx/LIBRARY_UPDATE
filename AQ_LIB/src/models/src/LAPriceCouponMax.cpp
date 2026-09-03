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

#include "LAPriceCouponMax.h"
using namespace std;

/*!
    @brief constructor
*/
LAPriceCouponMax::LAPriceCouponMax()
: LAPriceCouponBase()
{

}
/*!
    @brief destructor

*/
LAPriceCouponMax::~LAPriceCouponMax()
{
}

/*
    @brief Check function for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
LAPriceCouponMax::isTypeOf(function_t id) const
{
	return (id == FN_CPNSLTOPERATORMAX ? true : LAFunctionBase::isTypeOf(id));
}

/*!
	@brief select coupon
	@param[in] x coupons
	@note pure virtual method
*/
double				
LAPriceCouponMax::operator()(const DoubleArray& x) const
{
	unsigned int pos;
	return selectCoupon(x, pos);
	
}

/*
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
LACoreFunctionBase*
LAPriceCouponMax::clone() const
{
    try 
	{
        return new LAPriceCouponMax(*this);
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
LAPriceCouponMax::getType() const
{
	return FN_CPNSLTOPERATORMAX;
}

/*!
	@brief select coupon
	@param[in] x coupons
	@param[out] pos selected position of coupons
	@note pure virtual method
*/
double
LAPriceCouponMax::selectCoupon(const DoubleArray& x, unsigned int& pos) const
{
	double max = x.at(0);
	pos = 0;

	for (unsigned int i = 1; i < x.size(); i++)
	{
		double tmp = x[i]; 
		if (tmp > max)
		{	
			max = tmp;
			pos = i;
		}
	}

	return x[pos];
}


