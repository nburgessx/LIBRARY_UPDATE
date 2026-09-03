/*! @file
    @brief Source code of class to represent Coupon Cap function

    This class derives from LAFunctionBase

*/
//  2007, Mizuho International London..

////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LARatesCpnCapFunc.cpp
//
//  SYNOPSIS    :       LARatesCpnCapFunc
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


#include "LARatesCpnCapFunc.h"
#include <algorithm>

using namespace std;
//================ LARatesCpnCapFunc ===================================
/*!
	@brief default constructor
*/
LARatesCpnCapFunc::LARatesCpnCapFunc() 
: LAFunctionBase()
{

}

/*!
	@brief destructor
*/
LARatesCpnCapFunc::~LARatesCpnCapFunc() 
{
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
LACoreFunctionBase*	
LARatesCpnCapFunc::clone() const
{
    try 
	{
		return new LARatesCpnCapFunc(*this);
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
LARatesCpnCapFunc::isTypeOf(function_t id) const
{
	return (id == FN_CPNCAP ? true : LAFunctionBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
LARatesCpnCapFunc::getType() const
{
	return FN_CPNCAP;
}

/*!
    @brief Return function value
	@param[in] x point
    @return function value
*/
double
LARatesCpnCapFunc::operator()(const DoubleArray& x) const
{
	if (mParam.size() != 1)
		throw LACoreInvalidData("parameter size must be one", __FILE__, __LINE__);
	
	double ret = 0.0;
	for (unsigned int i = 0; i < x.size(); i++)
		ret += x[i];
	return mParam[0] - ret;
}

/*!
    @brief Return partial derivative value
	@param[in] x integral point
	@param[in] pos variable location to implement derivative
	@param[in] calctype analytical or numerical integral
	@param[in] difftype both side or one side
	@param[in] delta grid width for numerical method
    @return partial deribative value
*/

double
LARatesCpnCapFunc::partialDerivative(const DoubleArray& x, unsigned int pos,
								CALC_TYPE calctype, DIFF_TYPE difftype, double delta) const
{
	if (pos >= x.size())
	{
        throw LACoreInvalidData("pos is over x size", __FILE__, __LINE__);
	}
	
	if (calctype == NUMERICAL)
		return LAFunctionBase::partialDerivative(x, pos, calctype, difftype, delta);
	else
	{
		return -1.0;
	}

}

/*!
    @brief Return second partial derivative value
	@param[in] x integral point
	@param[in] posi one variable location to implement derivative
	@param[in] posj another variable location to implement derivative
	@param[in] calctype analytical or numerical integral
	@param[in] delta grid width for numerical method
    @return second partial deribative value
*/
double
LARatesCpnCapFunc::partialDerivative2(const DoubleArray& x, unsigned int posi, unsigned int posj,
												CALC_TYPE calctype, double delta) const
{
	if (calctype == NUMERICAL)
		return LAFunctionBase::partialDerivative2(x, 0, 0, calctype, delta);
	else
	{
		return 0;
	}
}
