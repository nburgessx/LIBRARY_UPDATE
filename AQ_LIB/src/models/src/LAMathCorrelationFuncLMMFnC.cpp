/*! @file
    @brief Source code of class to represent LMM correlation function

	This class derives from LAMathCorrelationFuncLMM

*/
//  2007, Mizuho International London.

////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LAMathCorrelationFuncLMMFnC.cpp
//
//  SYNOPSIS    :       LAMathCorrelationFuncLMMFnC
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


#include "LAMathCorrelationFuncLMMFnC.h"
#include "LABasic.h"

using namespace std;

//================ LAMathCorrelationFuncLMMFnC ================================
/*!
	@brief constructor
	@param[in] x         calibparam x
	@param[in] y         calibparam y
*/

LAMathCorrelationFuncLMMFnC::LAMathCorrelationFuncLMMFnC(double x, double y)
: LAMathCorrelationFuncLMM(), mx(x), my(y)
{
}

/*!
	@brief destructor
*/
LAMathCorrelationFuncLMMFnC::~LAMathCorrelationFuncLMMFnC(void)
{
}


/*!
	@brief copy constructor
*/
LAMathCorrelationFuncLMMFnC::LAMathCorrelationFuncLMMFnC(const LAMathCorrelationFuncLMMFnC &rhs) 
: LAMathCorrelationFuncLMM(), mx(rhs.mx), my(rhs.my)
{
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
LACoreFunctionBase*	
LAMathCorrelationFuncLMMFnC::clone() const
{
    try 
	{
		return new LAMathCorrelationFuncLMMFnC(*this);
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
LAMathCorrelationFuncLMMFnC::isTypeOf(function_t id) const
{
	return (id == FN_CORRELATIONFUNCLMMFNC ? true : LAFunctionBase::isTypeOf(id));
}

/*!
    @brief return this function type
    @return function type
*/
function_t
LAMathCorrelationFuncLMMFnC::getType() const
{
	return FN_CORRELATIONFUNCLMMFNC;
}


/*!
    @brief return function value
	@param[in] x point
    @return function value
*/
double
LAMathCorrelationFuncLMMFnC::operator()(const DoubleArray& x) const
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
LAMathCorrelationFuncLMMFnC::operator()(double t) const
{
	(void)t;
	if (mX.size() < 3)
	{
		throw LACoreInvalidData("default argument mX's size must be >=  3 !", __FILE__, __LINE__);
	}
	return get(t, mX[1], mX[2]);
}
