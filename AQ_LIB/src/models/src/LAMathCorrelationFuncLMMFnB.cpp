/*! @file
    @brief Source code of class to represent LMM correlation function

	This class derives from LAMathCorrelationFuncLMM

*/
//  2007, Mizuho International London.

////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LAMathCorrelationFuncLMMFnB.cpp
//
//  SYNOPSIS    :       LAMathCorrelationFuncLMMFnB
//  DESCRIPTION :       Source code of class  to represent correlation of LMM
//						This class derives from LAMathCorrelationFuncLMM
//                      
//  VERSION		:
////X///////////////////X///////////////////////////////X///////////////////

#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "LAMathCorrelationFuncLMMFnB.h"
#include "LABasic.h"

using namespace std;

//================ LAMathCorrelationFuncLMMFnB ================================
/*!
	@brief constructor
	@param[in] TMax      tenor max
	@param[in] x         calibparam x
	@param[in] y         calibparam y
*/

LAMathCorrelationFuncLMMFnB::LAMathCorrelationFuncLMMFnB(double TMax, double x, double y)
: LAMathCorrelationFuncLMM(), mTMax(TMax), mx(x), my(y)
{
}

/*!
	@brief destructor
*/
LAMathCorrelationFuncLMMFnB::~LAMathCorrelationFuncLMMFnB(void)
{
}


/*!
	@brief copy constructor
*/
LAMathCorrelationFuncLMMFnB::LAMathCorrelationFuncLMMFnB(const LAMathCorrelationFuncLMMFnB &rhs) 
: LAMathCorrelationFuncLMM(rhs), mTMax(rhs.mTMax), mx(rhs.mx), my(rhs.my)
{
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
LACoreFunctionBase*	
LAMathCorrelationFuncLMMFnB::clone() const
{
    try 
	{
		return new LAMathCorrelationFuncLMMFnB(*this);
    }
    catch (bad_alloc &e)
	{
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

/*!
    @brief check function for this class ID
    @param[in] id ID to check
    @return true or false
*/
bool
LAMathCorrelationFuncLMMFnB::isTypeOf(function_t id) const
{
	return (id == FN_CORRELATIONFUNCLMMFNB ? true : LAFunctionBase::isTypeOf(id));
}

/*!
    @brief return this function type
    @return function type
*/
function_t
LAMathCorrelationFuncLMMFnB::getType() const
{
	return FN_CORRELATIONFUNCLMMFNB;
}


/*!
    @brief return function value
	@param[in] x point
    @return function value
*/
double
LAMathCorrelationFuncLMMFnB::operator()(const DoubleArray& x) const
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
LAMathCorrelationFuncLMMFnB::operator()(double t) const
{
	(void)t;
	if (mX.size() < 3)
	{
		throw LACoreInvalidData("default argument mX's size must be >=  3 !", __FILE__, __LINE__);
	}
	return get(t, mX[1], mX[2]);
}
