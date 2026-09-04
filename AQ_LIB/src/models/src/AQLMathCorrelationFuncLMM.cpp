/*! @file
    @brief Source code of class to represent LMM correlation function

	This class derives from AQLFunctionBase
*/


#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLMathCorrelationFuncLMM.h"
#include "AQLBasic.h"

using namespace std;

//================ AQLMathCorrelationFuncLMM ===================================
/*!
	@brief constructor
*/

AQLMathCorrelationFuncLMM::AQLMathCorrelationFuncLMM(void)
: AQLFunctionBase()
{
}

/*!
	@brief destructor
*/
AQLMathCorrelationFuncLMM::~AQLMathCorrelationFuncLMM(void)
{
}


/*!
	@brief copy constructor
*/
AQLMathCorrelationFuncLMM::AQLMathCorrelationFuncLMM(const AQLMathCorrelationFuncLMM &rhs) 
: AQLFunctionBase()
{
	mX = rhs.mX;
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*	
AQLMathCorrelationFuncLMM::clone() const
{
    try 
	{
		return new AQLMathCorrelationFuncLMM(*this);
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
AQLMathCorrelationFuncLMM::isTypeOf(function_t id) const
{
	return (id == FN_CORRELATIONFUNCLMM ? true : AQLFunctionBase::isTypeOf(id));
}

/*!
    @brief return this function type
    @return function type
*/
function_t
AQLMathCorrelationFuncLMM::getType() const
{
	return FN_CORRELATIONFUNCLMM;
}


/*!
    @brief return function value
	@param[in] x point
    @return function value
*/
double
AQLMathCorrelationFuncLMM::operator()(const DoubleArray& x) const
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
AQLMathCorrelationFuncLMM::operator()(double t) const
{
	(void)t;
	if (mX.size() < 3)
	{
		throw AQLCoreInvalidData("default argument mX's size must be >=  3 !", __FILE__, __LINE__);
	}
//	return AQLMath::exp(-AQLMath::abs(mX[1] - mX[2]) * (mx + my * (1.0 - (mX[1] + mX[2]) / (2.0 * mTMax))));
	return get(t, mX[1], mX[2]);
}

//
DoubleMatrix AQLMathCorrelationFuncLMM::getCorrMat( double t, const DoubleVector& T_fix )
{
	size_t corr_s = T_fix.size();
	DoubleMatrix corr_mat(corr_s, DoubleVector(corr_s, 1));
	for( size_t i = 0; i < corr_s; i++ )
	{
		for( size_t j = i + 1; j < corr_s; j++ )
		{
			corr_mat[i][j] = get(t, T_fix[i], T_fix[j]);
			corr_mat[j][i] = corr_mat[i][j];
		}
	}
	return corr_mat;
}

