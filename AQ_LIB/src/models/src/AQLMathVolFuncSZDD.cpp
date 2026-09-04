/*! @file
    @brief Source code of class to represent ScobelZhu & DD version of FX volatility function

	This class derives from AQLFunctionBase
*/


#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLMathVolFuncSZDD.h"
#include "AQLAlgorithm.h"
#include "AQLBasic.h"
#include "AQLSplineInterpolation.h"
#include "AQLStepInterpolation.h"
#include "AQLCombinationFunc.h"

using namespace std;

//================ AQLMathVolFuncSZDD ===================================
/*!
	@brief constructor

	@param[in] timeGrid time grid 
	@param[in] sigma function value
	@param[in] fx0    function value
	@param[in] v0    function value
	@param[in] beta  function value
	@param[in] theta  function value
	@param[in] kappa  function value
	@param[in] epsilon  function value
	@param[in] currency  function value
	@param[in] type  sde type
	@param[in] integrate_n_  divided num of numerical integral
*/
////AQLMathVolFuncSZDD::AQLMathVolFuncSZDD(const DoubleArray &timeGrid, const DoubleArray &sigma, 
////										const DoubleArray &fx0, const DoubleArray &beta, 
////										const AQLString &currency, SDE_TYPE type, int integrate_n_)
////: AQLMathVolFuncFX(timeGrid, sigma, fx0, beta, currency, integrate_n_), mType(type)
//temp!!
AQLMathVolFuncSZDD::AQLMathVolFuncSZDD(const DoubleArray &timeGrid,const DoubleArray &fx0, 
										const DoubleArray &beta, const DoubleArray &theta, const DoubleArray &kappa, const DoubleArray &epsilon, 
										const AQLString &currency,const DoubleArray &sigma, 
										SDE_TYPE type, int integrate_n_)
										: AQLMathVolFuncFX(timeGrid, sigma, fx0, beta, currency, integrate_n_), mType(type)
										//temp!!/
										//////: mType(type),AQLFunctionBase(), mTimeGrid(timeGrid), mFX0(fx0), mV0(v0),
										//////mBeta(beta), mTheta(theta), mKappa(kappa), mEpsilon(epsilon), mCurrency(currency), mGL(integrate_n_)
{
	//AQLSplineInterpolation inter;
	if (timeGrid.empty() || timeGrid[0] != 0.0)
	{
		throw AQLCoreInvalidData("Wrong timegrid, first grid must be 0.0", __FILE__, __LINE__);
	}
	AQLStepInterpolation inter;
	//furuya
	//DoubleArray v(timeGrid.size()), s(timeGrid.size()), alpha(timeGrid.size());
	DoubleArray v(timeGrid.size()), s(timeGrid.size()), alpha(timeGrid.size()), theta_(timeGrid.size()), kappa_(timeGrid.size()), epsilon_(timeGrid.size());
	unsigned int size = timeGrid.size();
	for (unsigned int i = 0; i < size; i++)
	{
		if(0.0 == beta[i])
			throw AQLCoreInvalidData("Beta 0.0 is not allowed",__FILE__,__LINE__);

		//v[i] = beta[i] * sigma[i];
		//s[i] = (1.0 - beta[i]) / beta[i] * fx0[i];

		double fx0_ = fx0[i];
		if (i != size - 1)
		{
			double todayFX = fx0[0];
			fx0_ = todayFX * AQLMath::exp(0.5 * (AQLMath::log(fx0[i] / todayFX )+ AQLMath::log(fx0[i + 1] / todayFX)));
		}
		alpha[i] = (1.0 - beta[i]) * fx0_;

		theta_[i] = theta[i];
		kappa_[i] = kappa[i];
		epsilon_[i] = epsilon[i];

	}
	// timeGrid,beta,sigma
	//mV.set(timeGrid, v);
	//mS.set(timeGrid, s);
	mAlpha.set(timeGrid, alpha);
	//mV.setInterpolation(inter);
	//mS.setInterpolation(inter);
	mAlpha.setInterpolation(inter);
	mTheta.set(timeGrid, theta_);
	mTheta.setInterpolation(inter);
	mKappa.set(timeGrid, kappa_);
	mKappa.setInterpolation(inter);
	mEpsilon.set(timeGrid, epsilon_);
	mEpsilon.setInterpolation(inter);

}

/*!
	@brief set forward FX

	@param[in] fx
*/
void
AQLMathVolFuncSZDD::setFwdFX(const DoubleArray &fx)
{
	const unsigned int size = mTimeGrid.size();
	if (fx.size() != size)
	{
		throw AQLCoreInvalidData("fx array size is wrong.",__FILE__,__LINE__);
	}
	DoubleArray s(size), alpha(size);
	for (unsigned int i = 0; i < size; i++)
	{
		if(0.0 == mBeta[i])
			throw AQLCoreInvalidData("Beta 0.0 is not allowed",__FILE__,__LINE__);

		s[i] = (1.0 - mBeta[i]) / mBeta[i] * fx[i];
		double fx_ = fx[i];
		if (i != size - 1)
		{
			double todayFX = fx[0];
			fx_ = todayFX * AQLMath::exp(0.5 * (AQLMath::log(fx[i] / todayFX )+ AQLMath::log(fx[i + 1] / todayFX)));
		}
		alpha[i] = (1.0 - mBeta[i]) * fx_;
	}
	mS.set(mTimeGrid, s);
	mAlpha.set(mTimeGrid, alpha);
}

/*!
	@brief destructor
*/
AQLMathVolFuncSZDD::~AQLMathVolFuncSZDD(void)
{
}


/*!
	@brief copy constructor
*/
AQLMathVolFuncSZDD::AQLMathVolFuncSZDD(const AQLMathVolFuncSZDD &rhs) 
	: AQLMathVolFuncFX(rhs), mType(rhs.mType), mV(rhs.mV), mS(rhs.mS), mAlpha(rhs.mAlpha),
	mKappa(rhs.mKappa), mTheta(rhs.mTheta), mEpsilon(rhs.mEpsilon)
, integrate_cache(rhs.integrate_cache), is_cache(rhs.is_cache)
{

}



/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*	
AQLMathVolFuncSZDD::clone() const
{
    try 
	{
		return new AQLMathVolFuncSZDD(*this);
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
AQLMathVolFuncSZDD::isTypeOf(function_t id) const
{
	return (id == FN_VOLFUNCSZDD ? true : AQLFunctionBase::isTypeOf(id));
}

/*!
    @brief return this function type
    @return function type
*/
function_t
AQLMathVolFuncSZDD::getType() const
{
	return FN_VOLFUNCSZDD;
}


/*!
    @brief return function value
	@param[in] x point
    @return function value

	x[1] is fx value
*/
double
AQLMathVolFuncSZDD::operator()(const DoubleArray& x) const
{
	if (mTimeGrid.size() == 0 || x.size() < 2)
	{
		throw AQLCoreInvalidData("No data is set or argument size is less than two ", __FILE__, __LINE__);
	}

//	unsigned int pos = searchIndex(x[0]);

//	if (mType == dX)
//		return mSigma[pos] *  (mBeta[pos] * x[1] + (1.0 - mBeta[pos]) * mFX0[pos]);	
//	else
//		return mSigma[pos] *  (mBeta[pos] + (1.0 - mBeta[pos]) * mFX0[pos] / x[1]);	

	if (mType == dX)
		return mV(x[0]) *  (x[1] + mS(x[0]));	
	else
		return mV(x[0]) *  (1.0 + mS(x[0]) / x[1]);	
}

/*!
    @brief Return integral result
	@param[in] x integral region
    @return integral result
*/
double
AQLMathVolFuncSZDD::integral(const vector<pair<double,double> >& x) const
{
	if (mTimeGrid.size() == 0 || x.size() < 2)
	{
		throw AQLCoreInvalidData("No data is set or argument size is less than two ", __FILE__, __LINE__);
	}

	unsigned int pos = searchIndex(x[0].first);

	if (mType == dX)
		return AQLMath::sqrt(x[0].second - x[0].first) * 
				mSigma[pos] *  (mBeta[pos] * x[1].first + (1.0 - mBeta[pos]) * mFX0[pos]);	
	else
		return AQLMath::sqrt(x[0].second - x[0].first) * 
				mSigma[pos] *  (mBeta[pos] + (1.0 - mBeta[pos]) * mFX0[pos] / x[1].first);		

}


	                            //==========================================
	                            // Return integral of square of v(=beta*sigma)	
double
AQLMathVolFuncSZDD::getIntegralofV(double ts, double te) const
{
	double ret1 = 0.0, ret2 = 0.0;
	if ( !is_cache[ts] ) 
	{
		DoubleArray integral(3, 0.0);
		if (ts != 0.0)
		{
			ret1 = (mV * mV).integral(0.0, ts, &mGL);
			integral[0] = ret1;
			integral[1] = (mS * mV * mV).integral(0.0, ts, &mGL);
			integral[2] = (mS * mS * mV * mV).integral(0.0, ts, &mGL);
		}
		integrate_cache[ts] = integral;
		is_cache[ts] = true;		
	}
	else
		ret1 = integrate_cache[ts][0];

	if ( !is_cache[te] ) 
	{
		ret2 = ret1 + (mV * mV).integral(ts, te, &mGL);
		DoubleArray integral = integrate_cache[ts];
		integral[0] = ret2;
        integral[1] += (mS * mV * mV).integral(ts, te, &mGL);
		integral[2] += (mS * mS * mV * mV).integral(ts, te, &mGL);
		integrate_cache[te] = integral;
		is_cache[te] = true;
	}
	else
		ret2 = integrate_cache[te][0];


	if (getIntegralofSVV(ts, te) < 0 && getBeta(ts) < 0.5)
		return -AQLMath::sqrt(ret2 - ret1);
	else
		return AQLMath::sqrt(ret2 - ret1);
}
	                            //==========================================
	                            // Return integral of s * square of v(=beta*sigma)		
double
AQLMathVolFuncSZDD::getIntegralofSVV(double ts, double te) const
{
	double ret1 = 0.0, ret2 = 0.0;
	if ( !is_cache[ts] ) 
	{
		DoubleArray integral(3, 0.0);
		if (ts != 0.0)
		{
			ret1 = (mS * mV * mV).integral(0.0, ts, &mGL);		
			integral[0] = (mV * mV).integral(0.0, ts, &mGL);
			integral[1] = ret1;
			integral[2] = (mS * mS * mV * mV).integral(0.0, ts, &mGL);
		}
		integrate_cache[ts] = integral;
		is_cache[ts] = true;
	}
	else
		ret1 = integrate_cache[ts][1];

	if ( !is_cache[te] ) 
	{
		ret2 = ret1 + (mS * mV * mV).integral(ts, te, &mGL);
		DoubleArray integral = integrate_cache[ts];
		integral[0] += (mV * mV).integral(ts, te, &mGL);
		integral[1] = ret2;
		integral[2] += (mS * mS * mV * mV).integral(ts, te, &mGL);
		integrate_cache[te] = integral; 
		is_cache[te] = true;
	}
	else
		ret2 = integrate_cache[te][1];

	return ret2 - ret1;
}
	                            //==========================================
	                            // Return integral of square of s * v(=beta*sigma)	
double
AQLMathVolFuncSZDD::getIntegralofSV(double ts, double te) const
{
	double ret1 = 0.0, ret2 = 0.0;
	if ( !is_cache[ts] ) 
	{
		DoubleArray integral(3, 0.0);
		if (ts != 0.0)
		{
			ret1 = (mS * mS * mV * mV).integral(0.0, ts, &mGL);		
			integral[0] = (mV * mV).integral(0.0, ts, &mGL);
			integral[1] = (mS * mV * mV).integral(0.0, ts, &mGL);
			integral[2] = ret1;
		}
		integrate_cache[ts] = integral; 
		is_cache[ts] = true;
	}
	else
		ret1 = integrate_cache[ts][2];

	if ( !is_cache[te] ) 
	{
		ret2 = ret1 + (mS * mS * mV * mV).integral(ts, te, &mGL);
		DoubleArray integral = integrate_cache[ts];
		integral[0] += (mV * mV).integral(ts, te, &mGL);
		integral[1] += (mS * mV * mV).integral(ts, te, &mGL);
		integral[2] = ret2;
		integrate_cache[te] = integral;
		is_cache[te] = true;
	}
	else
		ret2 = integrate_cache[te][2];

	if (getIntegralofSVV(ts, te) < 0 && getBeta(ts) >= 0.5)
		return -AQLMath::sqrt(ret2 - ret1);
	else
		return AQLMath::sqrt(ret2 - ret1);

}



