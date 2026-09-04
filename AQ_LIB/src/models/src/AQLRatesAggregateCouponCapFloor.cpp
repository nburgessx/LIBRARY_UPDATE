/*! @file
    @brief Source code of class to represent Coupon Cap function

    This class derives from AQLFunctionBase

*/
//  2007,AlgoQuantHub.

#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLRatesAggregateCouponCapFloor.h"
#include <algorithm>

using namespace std;
//================ AQLRatesAggregateCouponCapFloor ===================================
/*!
	@brief default constructor
*/
AQLRatesAggregateCouponCapFloor::AQLRatesAggregateCouponCapFloor() 
: AQLRatesAccruedCouponFuncBase()
{

}

/*!
	@brief default constructor
*/
AQLRatesAggregateCouponCapFloor::AQLRatesAggregateCouponCapFloor(const DoubleArray& x) 
: AQLRatesAccruedCouponFuncBase()
{
	setParam(x);
}

/*!
	@brief destructor
*/
AQLRatesAggregateCouponCapFloor::~AQLRatesAggregateCouponCapFloor() 
{
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*	
AQLRatesAggregateCouponCapFloor::clone() const
{
    try 
	{
		return new AQLRatesAggregateCouponCapFloor(*this);
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
AQLRatesAggregateCouponCapFloor::isTypeOf(function_t id) const
{
	return (id == FN_AGGREGATECOUPONCAPFLOOR ? true : AQLRatesAccruedCouponFuncBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
AQLRatesAggregateCouponCapFloor::getType() const
{
	return FN_AGGREGATECOUPONCAPFLOOR;
}

/*!
    @brief Return function value
	@param[in] x point
    @return function value
*/
double
AQLRatesAggregateCouponCapFloor::operator()(const DoubleArray& x) const
{
	if (mParam.size() != 2)
		throw AQLCoreInvalidData("parameter size must be two; the first should be TargetValue and the last should be leverage of AggregateCoupon", __FILE__, __LINE__);
	
	double ret = 0.0;
	int size = x.size();
	for (int i = 0; i < size - 1; i++)
		ret += x[i];
	return mParam[0] + mParam[1] * ret;
}


