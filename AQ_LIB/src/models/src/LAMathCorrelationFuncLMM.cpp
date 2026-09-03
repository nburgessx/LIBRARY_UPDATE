/*! @file
    @brief Source code of class to represent LMM correlation function

	This class derives from LAFunctionBase

*/
//  2007, Mizuho International London.

////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LAMathCorrelationFuncLMM.h
//
//  SYNOPSIS    :       LAMathCorrelationFuncLMM
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


#include "LAMathCorrelationFuncLMM.h"
#include "LABasic.h"

using namespace std;

//================ LAMathCorrelationFuncLMM ===================================
/*!
	@brief constructor
*/

LAMathCorrelationFuncLMM::LAMathCorrelationFuncLMM(void)
: LAFunctionBase()
{
}

/*!
	@brief destructor
*/
LAMathCorrelationFuncLMM::~LAMathCorrelationFuncLMM(void)
{
}


/*!
	@brief copy constructor
*/
LAMathCorrelationFuncLMM::LAMathCorrelationFuncLMM(const LAMathCorrelationFuncLMM &rhs) 
: LAFunctionBase()
{
	mX = rhs.mX;
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
LACoreFunctionBase*	
LAMathCorrelationFuncLMM::clone() const
{
    try 
	{
		return new LAMathCorrelationFuncLMM(*this);
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
LAMathCorrelationFuncLMM::isTypeOf(function_t id) const
{
	return (id == FN_CORRELATIONFUNCLMM ? true : LAFunctionBase::isTypeOf(id));
}

/*!
    @brief return this function type
    @return function type
*/
function_t
LAMathCorrelationFuncLMM::getType() const
{
	return FN_CORRELATIONFUNCLMM;
}


/*!
    @brief return function value
	@param[in] x point
    @return function value
*/
double
LAMathCorrelationFuncLMM::operator()(const DoubleArray& x) const
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
LAMathCorrelationFuncLMM::operator()(double t) const
{
	(void)t;
	if (mX.size() < 3)
	{
		throw LACoreInvalidData("default argument mX's size must be >=  3 !", __FILE__, __LINE__);
	}
//	return LAMath::exp(-LAMath::abs(mX[1] - mX[2]) * (mx + my * (1.0 - (mX[1] + mX[2]) / (2.0 * mTMax))));
	return get(t, mX[1], mX[2]);
}

//
DoubleMatrix LAMathCorrelationFuncLMM::getCorrMat( double t, const DoubleVector& T_fix )
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

