/*! @file
    @brief Source code of class to represent LMM correlation function

	This class derives from AQLMathCorrelationFuncLMM

*/
//  2007, AlgoQuantHub.

////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       AQLMathCorrelationFuncLMMFnB.cpp
//
//  SYNOPSIS    :       AQLMathCorrelationFuncLMMFnB
//  DESCRIPTION :       Source code of class  to represent correlation of LMM
//						This class derives from AQLMathCorrelationFuncLMM
//                      
//  VERSION		:
////X///////////////////X///////////////////////////////X///////////////////

#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLMathCorrelationFuncLMMFnB.h"
#include "AQLBasic.h"

using namespace std;

//================ AQLMathCorrelationFuncLMMFnB ================================
/*!
	@brief constructor
	@param[in] TMax      tenor max
	@param[in] x         calibparam x
	@param[in] y         calibparam y
*/

AQLMathCorrelationFuncLMMFnB::AQLMathCorrelationFuncLMMFnB(double TMax, double x, double y)
: AQLMathCorrelationFuncLMM(), mTMax(TMax), mx(x), my(y)
{
}

/*!
	@brief destructor
*/
AQLMathCorrelationFuncLMMFnB::~AQLMathCorrelationFuncLMMFnB(void)
{
}


/*!
	@brief copy constructor
*/
AQLMathCorrelationFuncLMMFnB::AQLMathCorrelationFuncLMMFnB(const AQLMathCorrelationFuncLMMFnB &rhs) 
: AQLMathCorrelationFuncLMM(rhs), mTMax(rhs.mTMax), mx(rhs.mx), my(rhs.my)
{
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*	
AQLMathCorrelationFuncLMMFnB::clone() const
{
    try 
	{
		return new AQLMathCorrelationFuncLMMFnB(*this);
    }
    catch (bad_alloc &e)
	{
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

/*!
    @brief check function for this class ID
    @param[in] id ID to check
    @return true or false
*/
bool
AQLMathCorrelationFuncLMMFnB::isTypeOf(function_t id) const
{
	return (id == FN_CORRELATIONFUNCLMMFNB ? true : AQLFunctionBase::isTypeOf(id));
}

/*!
    @brief return this function type
    @return function type
*/
function_t
AQLMathCorrelationFuncLMMFnB::getType() const
{
	return FN_CORRELATIONFUNCLMMFNB;
}


/*!
    @brief return function value
	@param[in] x point
    @return function value
*/
double
AQLMathCorrelationFuncLMMFnB::operator()(const DoubleArray& x) const
{
	(void)x;
	// set dmy value
	return operator()(0.0);
}


/*!
    @brief return function value
	@param[in] t term
    @return function value
*/
double
AQLMathCorrelationFuncLMMFnB::operator()(double t) const
{
	(void)t;
	if (mX.size() < 3)
	{
		throw AQLCoreInvalidData("default argument mX's size must be >=  3 !", __FILE__, __LINE__);
	}
	return get(t, mX[1], mX[2]);
}
