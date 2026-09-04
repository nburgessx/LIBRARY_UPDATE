/*! @file
    @brief Source code of class to represent Coupon Cap function

    This class derives from AQLFunctionBase

*/
//  2007, AlgoQuantHub..

#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLRatesCpnCapFunc.h"
#include <algorithm>

using namespace std;
//================ AQLRatesCpnCapFunc ===================================
/*!
	@brief default constructor
*/
AQLRatesCpnCapFunc::AQLRatesCpnCapFunc() 
: AQLFunctionBase()
{

}

/*!
	@brief destructor
*/
AQLRatesCpnCapFunc::~AQLRatesCpnCapFunc() 
{
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*	
AQLRatesCpnCapFunc::clone() const
{
    try 
	{
		return new AQLRatesCpnCapFunc(*this);
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
AQLRatesCpnCapFunc::isTypeOf(function_t id) const
{
	return (id == FN_CPNCAP ? true : AQLFunctionBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
AQLRatesCpnCapFunc::getType() const
{
	return FN_CPNCAP;
}

/*!
    @brief Return function value
	@param[in] x point
    @return function value
*/
double
AQLRatesCpnCapFunc::operator()(const DoubleArray& x) const
{
	if (mParam.size() != 1)
		throw AQLCoreInvalidData("parameter size must be one", __FILE__, __LINE__);
	
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
AQLRatesCpnCapFunc::partialDerivative(const DoubleArray& x, unsigned int pos,
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
AQLRatesCpnCapFunc::partialDerivative2(const DoubleArray& x, unsigned int posi, unsigned int posj,
												CALC_TYPE calctype, double delta) const
{
	if (calctype == NUMERICAL)
		return AQLFunctionBase::partialDerivative2(x, 0, 0, calctype, delta);
	else
	{
		return 0;
	}
}
