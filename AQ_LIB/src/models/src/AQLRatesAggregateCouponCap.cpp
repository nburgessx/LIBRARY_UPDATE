/*! @file
    @brief Source code of class to represent Coupon Cap function

    This class derives from AQLFunctionBase

*/

#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLRatesAggregateCouponCap.h"
#include <algorithm>

using namespace std;
//================ AQLRatesAggregateCouponCap ===================================
/*!
	@brief default constructor
*/
AQLRatesAggregateCouponCap::AQLRatesAggregateCouponCap() 
: AQLRatesAccruedCouponFuncBase()
{

}

/*!
	@brief default constructor
*/
AQLRatesAggregateCouponCap::AQLRatesAggregateCouponCap(const DoubleArray& x) 
: AQLRatesAccruedCouponFuncBase()
{
	setParam(x);
}

/*!
	@brief destructor
*/
AQLRatesAggregateCouponCap::~AQLRatesAggregateCouponCap() 
{
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*	
AQLRatesAggregateCouponCap::clone() const
{
    try 
	{
		return new AQLRatesAggregateCouponCap(*this);
    }
    catch (bad_alloc & e)
	{
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

/*!
    @brief Check function for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
AQLRatesAggregateCouponCap::isTypeOf(function_t id) const
{
	return (id == FN_AGGREGATECOUPONCAP ? true : AQLRatesAccruedCouponFuncBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
AQLRatesAggregateCouponCap::getType() const
{
	return FN_AGGREGATECOUPONCAP;
}

/*!
    @brief Return function value
	@param[in] x point
    @return function value
*/
double
AQLRatesAggregateCouponCap::operator()(const DoubleArray& x) const
{
	if (mParam.size() != 2)
		throw AQLCoreInvalidData("parameter size must be two; the first should be TargetValue and the last should be leverage of AggregateCoupon", __FILE__, __LINE__);
	
	double ret = 0.0;
	int size = x.size();
	for (int i = 0; i < size - 1; i++)
		ret += x[i];
	return mParam[0] + mParam[1] * ret;
}


