/*! @file
    @brief Source code of class to represent Coupon Cap function

    This class derives from AQLFunctionBase

*/
//  2007, AlgoQuantHub..

////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       AQLRatesCpnCapFuncForTARN.cpp
//
//  SYNOPSIS    :       AQLRatesCpnCapFuncForTARN
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


#include "AQLRatesCpnCapFuncForTARN.h"
#include <algorithm>

using namespace std;
//================ AQLRatesCpnCapFuncForTARN ===================================
/*!
	@brief default constructor
*/
AQLRatesCpnCapFuncForTARN::AQLRatesCpnCapFuncForTARN() 
: AQLFunctionBase()
{

}

/*!
	@brief default constructor
*/
AQLRatesCpnCapFuncForTARN::AQLRatesCpnCapFuncForTARN(const DoubleArray& x) 
: AQLFunctionBase()
{
	(void)x;
}

/*!
	@brief destructor
*/
AQLRatesCpnCapFuncForTARN::~AQLRatesCpnCapFuncForTARN() 
{
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*	
AQLRatesCpnCapFuncForTARN::clone() const
{
    try 
	{
		return new AQLRatesCpnCapFuncForTARN(*this);
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
AQLRatesCpnCapFuncForTARN::isTypeOf(function_t id) const
{
	return (id == FN_CPNCAPFORTARN ? true : AQLFunctionBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
AQLRatesCpnCapFuncForTARN::getType() const
{
	return FN_CPNCAPFORTARN;
}

/*!
    @brief Return function value
	@param[in] x point
    @return function value
*/
double
AQLRatesCpnCapFuncForTARN::operator()(const DoubleArray& x) const
{
	if (mParam.size() != x.size() + 3)
		throw AQLCoreInvalidData("parameter size must be indexInfos + 3; FloorValue, CapValue, indexcoefficient(1),...,constant", __FILE__, __LINE__);
	
	double ret = 0.0;
	unsigned int N = x.size();
	for (unsigned int i = 0; i < N; i++)
		ret += mParam[i+2] * x[i];
	ret += mParam.back();

	//floor 
	ret = ret > mParam[0] ? ret : mParam[0];
	//cap
	ret = ret < mParam[1] ? ret : mParam[1];

	return ret;
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
AQLRatesCpnCapFuncForTARN::partialDerivative(const DoubleArray& x, unsigned int pos,
								CALC_TYPE calctype, DIFF_TYPE difftype, double delta) const
{
	if (pos >= x.size())
	{
        throw AQLCoreInvalidData("pos is over x size", __FILE__, __LINE__);
	}
	
	if (calctype == NUMERICAL)
		return AQLFunctionBase::partialDerivative(x, pos, calctype, difftype, delta);
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
AQLRatesCpnCapFuncForTARN::partialDerivative2(const DoubleArray& x, unsigned int posi, unsigned int posj,
												CALC_TYPE calctype, double delta) const
{
	if (calctype == NUMERICAL)
		return AQLFunctionBase::partialDerivative2(x, 0, 0, calctype, delta);
	else
	{
		return 0;
	}
}
