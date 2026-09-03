/*! @file
    @brief Source code of class to represent wave structure volatility function

	This class derives from AQLFunctionBase

*/
//  2007, AlgoQuantHub.

////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LAMathVolFuncWave.h
//
//  SYNOPSIS    :       LAMathVolFuncWave
//  DESCRIPTION :       Class declaration to represent wave structure volatility function
//						This class derives from AQLFunctionBase
//                      
//  VERSION		:
////X///////////////////X///////////////////////////////X///////////////////

#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#ifndef _USE_MATH_DEFINES
#define  _USE_MATH_DEFINES
#endif

#include <math.h>
#include "LAMathVolFuncWave.h"


using namespace std;

//================ LAMathVolFuncWave ===================================
/*!
	@brief constructor
	@param[in] Tmax   term max
	@param[in] decay
	@param[in] amp1
	@param[in] phase1
	@param[in] amp2
	@param[in] phase2
	@param[in] amp3
	@param[in] phase3
	@param[in] shift
*/

LAMathVolFuncWave::LAMathVolFuncWave(double Tmax, double decay, double amp1, 
								 double phase1, double amp2, double phase2, double amp3, double phase3, double shift)
: AQLFunctionBase(), mAlpha(0.0), mDecay(decay), mAmp1(amp1), mPhase1(phase1), 
mAmp2(amp2), mPhase2(phase2), mAmp3(amp3), mPhase3(phase3), mShift(shift) 
{
	mAlpha = M_PI / Tmax;
}

/*!
	@brief destructor
*/
LAMathVolFuncWave::~LAMathVolFuncWave(void)
{

}


/*!
	@brief copy constructor
*/
LAMathVolFuncWave::LAMathVolFuncWave(const LAMathVolFuncWave &rhs) 
: AQLFunctionBase(), mAlpha(rhs.mAlpha), mDecay(rhs.mDecay), mAmp1(rhs.mAmp1), mPhase1(rhs.mPhase1), mAmp2(rhs.mAmp2),
 mPhase2(rhs.mPhase2), mAmp3(rhs.mAmp3), mPhase3(rhs.mPhase3), mShift(rhs.mShift)
{

}

/*!
    @brief make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*	
LAMathVolFuncWave::clone() const
{
    try 
	{
		return new LAMathVolFuncWave(*this);
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
LAMathVolFuncWave::isTypeOf(function_t id) const
{
	return (id == FN_VOLFUNCWAVE ? true : AQLFunctionBase::isTypeOf(id));
}

/*!
    @brief return this function type
    @return function type
*/
function_t
LAMathVolFuncWave::getType() const
{
	return FN_VOLFUNCWAVE;
}


/*!
    @brief return function value
	@param[in] x point
    @return function value
*/
double
LAMathVolFuncWave::operator()(const DoubleArray& x) const
{
	if (static_cast<int>(x.size()) < 1)
	{
		throw AQLCoreInvalidData("the argument DoubleArray's size must be more than zero !", __FILE__, __LINE__);
	}
	return operator()(x[0]);
}


/*!
    @brief return function value
	@param[in] t term
    @return function value
*/
double
LAMathVolFuncWave::operator()(double t) const
{
	return exp(-mDecay * t) * (mAmp1 * sin(mAlpha * t + mPhase1) + 
							mAmp2 * sin(2.0 * mAlpha * t + mPhase2) + 
							mAmp3 * sin(3.0 * mAlpha * t + mPhase3)) + mShift;
}
