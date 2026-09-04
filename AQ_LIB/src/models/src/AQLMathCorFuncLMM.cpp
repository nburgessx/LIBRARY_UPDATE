/*! @file
    @brief Source code of class to represent LMM correlation function

	This class derives from AQLFunctionBase
*/


#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLMathCorFuncLMM.h"
#include "AQLBasic.h"

using namespace std;

//================ AQLMathCorFuncLMM ===================================
/*!
	@brief constructor
	@param[in] TMax      tenor max
	@param[in] x         calibparam x
	@param[in] y         calibparam y
*/

AQLMathCorFuncLMM::AQLMathCorFuncLMM(double TMax, double x, double y)
: AQLFunctionBase(), mTMax(TMax), mx(x), my(y)
{
}

/*!
	@brief destructor
*/
AQLMathCorFuncLMM::~AQLMathCorFuncLMM(void)
{
}


/*!
	@brief copy constructor
*/
AQLMathCorFuncLMM::AQLMathCorFuncLMM(const AQLMathCorFuncLMM &rhs) 
: AQLFunctionBase(), mTMax(rhs.mTMax), mx(rhs.mx), my(rhs.my)
{
	mX = rhs.mX;
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*	
AQLMathCorFuncLMM::clone() const
{
    try 
	{
		return new AQLMathCorFuncLMM(*this);
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
AQLMathCorFuncLMM::isTypeOf(function_t id) const
{
	return (id == FN_CORFUNCLMM ? true : AQLFunctionBase::isTypeOf(id));
}

/*!
    @brief return this function type
    @return function type
*/
function_t
AQLMathCorFuncLMM::getType() const
{
	return FN_CORFUNCLMM;
}


/*!
    @brief return function value
	@param[in] x point
    @return function value
*/
double
AQLMathCorFuncLMM::operator()(const DoubleArray& x) const
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
AQLMathCorFuncLMM::operator()(double t) const
{
	(void)t;
	if (mX.size() < 3)
	{
		throw AQLCoreInvalidData("default argument mX's size must be >=  3 !", __FILE__, __LINE__);
	}
	return AQLMath::exp(-AQLMath::abs(mX[1] - mX[2]) * (mx + my * (1.0 - (mX[1] + mX[2]) / (2.0 * mTMax))));
}
