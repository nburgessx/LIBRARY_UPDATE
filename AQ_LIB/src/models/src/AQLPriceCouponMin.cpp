/*! @file
    @brief source code of class of coupon rainbow type select operator.

*/
// 
//////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////
#ifdef __GNUG__
#pragma implementation
#endif
#ifdef _MSC_VER
#pragma warning(disable:4786)
#endif

#include "AQLPriceCouponMin.h"
using namespace std;

/*!
    @brief constructor
*/
AQLPriceCouponMin::AQLPriceCouponMin()
: AQLPriceCouponBase()
{

}
/*!
    @brief destructor

*/
AQLPriceCouponMin::~AQLPriceCouponMin()
{
}

/*
    @brief Check function for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
AQLPriceCouponMin::isTypeOf(function_t id) const
{
	return (id == FN_CPNSLTOPERATORMIN ? true : AQLFunctionBase::isTypeOf(id));
}

/*!
	@brief select coupon
	@param[in] x coupons
	@note pure virtual method
*/
double				
AQLPriceCouponMin::operator()(const DoubleArray& x) const
{
	unsigned int pos;
	return selectCoupon(x, pos);
}

/*
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*
AQLPriceCouponMin::clone() const
{
    try 
	{
        return new AQLPriceCouponMin(*this);
    }
    catch (bad_alloc & e)
	{
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }	
}


/*!
    @brief Return this class type
    @return function type5
*/
function_t			
AQLPriceCouponMin::getType() const
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
AQLPriceCouponMin::selectCoupon(const DoubleArray& x, unsigned int& pos) const
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


