/*! @file
    @brief Source code of class to represent FX volatility function

	This class derives from LAFunctionBase

*/
//  2007, AlgoQuantHub.

////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LAMathVolFuncFX.h
//
//  SYNOPSIS    :       LAMathVolFuncFX
//  DESCRIPTION :       Source code of class  to represent volatility of FX
//						This class derives from LAFunctionBase
//                      
//  VERSION		:
////X///////////////////X///////////////////////////////X///////////////////

#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "LAMathVolFuncFX.h"
#include "LABasic.h"
#include "LAStepInterpolation.h"
#include "LACombinationFunc.h"

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
										const DoubleArray &fx0, const DoubleArray &beta, const LAString &currency, int integrate_n_)
: LAFunctionBase(), mTimeGrid(timeGrid), mSigma(sigma), mFX0(fx0), mBeta(beta), mCurrency(currency), mGL(integrate_n_)
{
	const unsigned int size = timeGrid.size();
	if ((size != sigma.size()) || (size != fx0.size()) || (size != beta.size()))
	{
		// error
		throw LACoreInvalidData(" Data size must be same !!", __FILE__, __LINE__);
	}
	LAStepInterpolation inter;
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
: LAFunctionBase(), mTimeGrid(rhs.mTimeGrid), mSigma(rhs.mSigma), mFX0(rhs.mFX0), mBeta(rhs.mBeta), mCurrency(rhs.mCurrency), mGL(rhs.mGL)
{
	LAStepInterpolation inter;
	mSigmaFunc.set(mTimeGrid, mSigma);
	mSigmaFunc.setInterpolation(inter);
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
LACoreFunctionBase*	
LAMathVolFuncFX::clone() const
{
    try 
	{
		return new LAMathVolFuncFX(*this);
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
LAMathVolFuncFX::isTypeOf(function_t id) const
{
	return (id == FN_VOLFUNCFX ? true : LAFunctionBase::isTypeOf(id));
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
		throw LACoreInvalidData("No data is set or argument size is less than two ", __FILE__, __LINE__);
	}

	unsigned int pos = searchIndex(x[0]);

	return mSigma[pos] * LAMath::pow((x[1] / mFX0[pos]), mBeta[pos] - 1);	
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
	if (LAAlgorithm::find<DoubleArray, double>(mTimeGrid, t, 0, mTimeGrid.size() - 1, pos))
	{
		return pos;
	}
	else
	{
		LAAlgorithm::locate<DoubleArray, double>(mTimeGrid, t, mTimeGrid.size(), pos);

		if (pos > 0)
		{
			return pos - 1;
		}
		else
		{
			LAString msg = LAString("Argument t is wrong t = ") + LAString(t);
			throw LACoreInvalidData(msg.getCString() , __FILE__, __LINE__);
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

	return LAMath::sqrt(ret2 - ret1);
		
}
