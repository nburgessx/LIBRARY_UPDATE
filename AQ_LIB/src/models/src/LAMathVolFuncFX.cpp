/*! @file
    @brief Source code of class to represent FX volatility function

	This class derives from AQLFunctionBase

*/
//  2007, AlgoQuantHub.

////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LAMathVolFuncFX.h
//
//  SYNOPSIS    :       LAMathVolFuncFX
//  DESCRIPTION :       Source code of class  to represent volatility of FX
//						This class derives from AQLFunctionBase
//                      
//  VERSION		:
////X///////////////////X///////////////////////////////X///////////////////

#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "LAMathVolFuncFX.h"
#include "AQLBasic.h"
#include "AQLStepInterpolation.h"
#include "AQLCombinationFunc.h"

using namespace std;

//================ LAMathVolFuncFX ===================================
/*!
	@brief constructor

	@param[in] timeGrid 
	@param[in] sigma function value
	@param[in] fx0    function value
	@param[in] beta  function value
	@param[in] currency  function value

*/
LAMathVolFuncFX::LAMathVolFuncFX(const DoubleArray &timeGrid, const DoubleArray &sigma, 
										const DoubleArray &fx0, const DoubleArray &beta, const AQLString &currency, int integrate_n_)
: AQLFunctionBase(), mTimeGrid(timeGrid), mSigma(sigma), mFX0(fx0), mBeta(beta), mCurrency(currency), mGL(integrate_n_)
{
	const unsigned int size = timeGrid.size();
	if ((size != sigma.size()) || (size != fx0.size()) || (size != beta.size()))
	{
		// error
		throw AQLCoreInvalidData(" Data size must be same !!", __FILE__, __LINE__);
	}
	AQLStepInterpolation inter;
	mSigmaFunc.set(timeGrid, sigma);
	mSigmaFunc.setInterpolation(inter);

}

/*!
	@brief destructor
*/
LAMathVolFuncFX::~LAMathVolFuncFX(void)
{
}


/*!
	@brief copy constructor
*/
LAMathVolFuncFX::LAMathVolFuncFX(const LAMathVolFuncFX &rhs) 
: AQLFunctionBase(), mTimeGrid(rhs.mTimeGrid), mSigma(rhs.mSigma), mFX0(rhs.mFX0), mBeta(rhs.mBeta), mCurrency(rhs.mCurrency), mGL(rhs.mGL)
{
	AQLStepInterpolation inter;
	mSigmaFunc.set(mTimeGrid, mSigma);
	mSigmaFunc.setInterpolation(inter);
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*	
LAMathVolFuncFX::clone() const
{
    try 
	{
		return new LAMathVolFuncFX(*this);
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
LAMathVolFuncFX::isTypeOf(function_t id) const
{
	return (id == FN_VOLFUNCFX ? true : AQLFunctionBase::isTypeOf(id));
}

/*!
    @brief return this function type
    @return function type
*/
function_t
LAMathVolFuncFX::getType() const
{
	return FN_VOLFUNCFX;
}


/*!
    @brief return function value
	@param[in] x point
    @return function value

	x[1] is fx value
*/
double
LAMathVolFuncFX::operator()(const DoubleArray& x) const
{
	if (mTimeGrid.size() == 0 || x.size() < 2)
	{
		throw AQLCoreInvalidData("No data is set or argument size is less than two ", __FILE__, __LINE__);
	}

	unsigned int pos = searchIndex(x[0]);

	return mSigma[pos] * AQLMath::pow((x[1] / mFX0[pos]), mBeta[pos] - 1);	
}

/*!
    @brief get sigma

	@param[in] t
	@return double
*/
double
LAMathVolFuncFX::getSigma(double t) const
{
	unsigned int pos = searchIndex(t);
	return mSigma[pos];
}


/*!
    @brief get forward fx

	@param[in] t
	@return double
*/
double
LAMathVolFuncFX::getForwardFX0(double t) const
{
	unsigned int pos = searchIndex(t);
	return mFX0[pos];
}


/*!
    @brief get beta

	@param[in] t
	@return double
*/
double
LAMathVolFuncFX::getBeta(double t) const
{
	unsigned int pos = searchIndex(t);
	return mBeta[pos];
}


/*!
    @brief search intex

	@param[in] t
	@return unsigned int
*/
unsigned int
LAMathVolFuncFX::searchIndex(double t) const
{
	unsigned int pos;
	if (AQLAlgorithm::find<DoubleArray, double>(mTimeGrid, t, 0, mTimeGrid.size() - 1, pos))
	{
		return pos;
	}
	else
	{
		AQLAlgorithm::locate<DoubleArray, double>(mTimeGrid, t, mTimeGrid.size(), pos);

		if (pos > 0)
		{
			return pos - 1;
		}
		else
		{
			AQLString msg = AQLString("Argument t is wrong t = ") + AQLString(t);
			throw AQLCoreInvalidData(msg.getCString() , __FILE__, __LINE__);
		}
	}
}


	                            //==========================================
	                            // Return integral of square of v(=beta*sigma)	
double
LAMathVolFuncFX::getIntegralofSigma(double ts, double te) const
{
	double ret1 = 0.0, ret2 = 0.0;
	if ( !mIsCacheSigma[ts] ) 
	{
		if (ts != 0.0)
		{
			ret1 = (mSigmaFunc * mSigmaFunc).integral(0.0, ts, &mGL);
			mIntegratedCacheSigma[ts] = ret1;
		}
		mIsCacheSigma[ts] = true;		
	}
	else
		ret1 = mIntegratedCacheSigma[ts];

	if ( !mIsCacheSigma[te] ) 
	{
		ret2 = ret1 + (mSigmaFunc * mSigmaFunc).integral(ts, te, &mGL);
		mIntegratedCacheSigma[te] = ret2;
		mIsCacheSigma[te] = true;
	}
	else
		ret2 = mIntegratedCacheSigma[te];

	return AQLMath::sqrt(ret2 - ret1);
		
}
