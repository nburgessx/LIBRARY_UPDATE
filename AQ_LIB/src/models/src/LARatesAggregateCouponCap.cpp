/*! @file
    @brief Source code of class to represent Coupon Cap function

    This class derives from LAFunctionBase

*/
//  2007,Mizuho International London.

////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LARatesAggregateCouponCap.cpp
//
//  SYNOPSIS    :       LARatesAggregateCouponCap
//  DESCRIPTION :       Source code of class to represent Coupon Cap function
//                      
//  SEE ALSO    :       
//  TYME        :       CLASS
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "LARatesAggregateCouponCap.h"
#include <algorithm>

using namespace std;
//================ LARatesAggregateCouponCap ===================================
/*!
	@brief default constructor
*/
LARatesAggregateCouponCap::LARatesAggregateCouponCap() 
: LARatesAccruedCouponFuncBase()
{

}

/*!
	@brief default constructor
*/
LARatesAggregateCouponCap::LARatesAggregateCouponCap(const DoubleArray& x) 
: LARatesAccruedCouponFuncBase()
{
	setParam(x);
}

/*!
	@brief destructor
*/
LARatesAggregateCouponCap::~LARatesAggregateCouponCap() 
{
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
LACoreFunctionBase*	
LARatesAggregateCouponCap::clone() const
{
    try 
	{
		return new LARatesAggregateCouponCap(*this);
    }
    catch (bad_alloc & e)
	{
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

/*!
    @brief Check function for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
LARatesAggregateCouponCap::isTypeOf(function_t id) const
{
	return (id == FN_AGGREGATECOUPONCAP ? true : LARatesAccruedCouponFuncBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
LARatesAggregateCouponCap::getType() const
{
	return FN_AGGREGATECOUPONCAP;
}

/*!
    @brief Return function value
	@param[in] x point
    @return function value
*/
double
LARatesAggregateCouponCap::operator()(const DoubleArray& x) const
{
	if (mParam.size() != 2)
		throw LACoreInvalidData("parameter size must be two; the first should be TargetValue and the last should be leverage of AggregateCoupon", __FILE__, __LINE__);
	
	double ret = 0.0;
	int size = x.size();
	for (int i = 0; i < size - 1; i++)
		ret += x[i];
	return mParam[0] + mParam[1] * ret;
}


