/*! @file
    @brief Source code of class to represent LMM correlation function

	This class derives from LAFunctionBase

*/
//  2007, Mizuho International London.

////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LAMathCorFuncLMM.h
//
//  SYNOPSIS    :       LAMathCorFuncLMM
//  DESCRIPTION :       Source code of class  to represent correlation of LMM
//						This class derives from LAFunctionBase
//                      
//  VERSION		:
////X///////////////////X///////////////////////////////X///////////////////

#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "LAMathCorFuncLMM.h"
#include "LABasic.h"

using namespace std;

//================ LAMathCorFuncLMM ===================================
/*!
	@brief constructor
	@param[in] TMax      tenor max
	@param[in] x         calibparam x
	@param[in] y         calibparam y
*/

LAMathCorFuncLMM::LAMathCorFuncLMM(double TMax, double x, double y)
: LAFunctionBase(), mTMax(TMax), mx(x), my(y)
{
}

/*!
	@brief destructor
*/
LAMathCorFuncLMM::~LAMathCorFuncLMM(void)
{
}


/*!
	@brief copy constructor
*/
LAMathCorFuncLMM::LAMathCorFuncLMM(const LAMathCorFuncLMM &rhs) 
: LAFunctionBase(), mTMax(rhs.mTMax), mx(rhs.mx), my(rhs.my)
{
	mX = rhs.mX;
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
LACoreFunctionBase*	
LAMathCorFuncLMM::clone() const
{
    try 
	{
		return new LAMathCorFuncLMM(*this);
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
LAMathCorFuncLMM::isTypeOf(function_t id) const
{
	return (id == FN_CORFUNCLMM ? true : LAFunctionBase::isTypeOf(id));
}

/*!
    @brief return this function type
    @return function type
*/
function_t
LAMathCorFuncLMM::getType() const
{
	return FN_CORFUNCLMM;
}


/*!
    @brief return function value
	@param[in] x point
    @return function value
*/
double
LAMathCorFuncLMM::operator()(const DoubleArray& x) const
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
LAMathCorFuncLMM::operator()(double t) const
{
	(void)t;
	if (mX.size() < 3)
	{
		throw LACoreInvalidData("default argument mX's size must be >=  3 !", __FILE__, __LINE__);
	}
	return LAMath::exp(-LAMath::abs(mX[1] - mX[2]) * (mx + my * (1.0 - (mX[1] + mX[2]) / (2.0 * mTMax))));
}
