/*! @file
    @brief Source code of class to represent RangeCount function

    This class derives from AQLFunctionBase

*/
//  2007, AlgoQuantHub..

////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LARatesRangeCountFunc.cpp
//
//  SYNOPSIS    :       LARatesRangeCountFunc
//  DESCRIPTION :       Source code of class to represent RangeCount function
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


#include "LARatesRangeCountFunc.h"
#include <algorithm>

using namespace std;
//================ LARatesRangeCountFunc ===================================
/*!
	@brief default constructor
*/
LARatesRangeCountFunc::LARatesRangeCountFunc() 
: AQLFunctionBase()
{

}

/*!
	@brief destructor
*/
LARatesRangeCountFunc::~LARatesRangeCountFunc() 
{
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*	
LARatesRangeCountFunc::clone() const
{
    try 
	{
		return new LARatesRangeCountFunc(*this);
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
LARatesRangeCountFunc::isTypeOf(function_t id) const
{
	return (id == FN_RANGECOUNT ? true : AQLFunctionBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
LARatesRangeCountFunc::getType() const
{
	return FN_RANGECOUNT;
}

/*!
    @brief Return function value
	@param[in] x point
    @return function value
*/
double
LARatesRangeCountFunc::operator()(const DoubleArray& x) const
{
	if (mParam.size() != 3)
		throw AQLCoreInvalidData("parameter size must be three", __FILE__, __LINE__);
	
	double ret = 0.0;
	for (unsigned int i = 0; i < x.size() - 1; i++)
		if(x[i] >= mParam[0] && x[i] <= mParam[1])	ret += 1;
	
	return ret / (x.size() - 1) * mParam[2];
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
LARatesRangeCountFunc::partialDerivative(const DoubleArray& x, unsigned int pos,
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
		return 1.0;
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
LARatesRangeCountFunc::partialDerivative2(const DoubleArray& x, unsigned int posi, unsigned int posj,
												CALC_TYPE calctype, double delta) const
{
	if (calctype == NUMERICAL)
		return AQLFunctionBase::partialDerivative2(x, 0, 0, calctype, delta);
	else
	{
		return 0;
	}
}

