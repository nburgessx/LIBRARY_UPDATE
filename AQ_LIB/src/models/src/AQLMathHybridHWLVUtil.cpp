//
#include <algorithm>
#include "AQLMathHybridHWLVUtil.h"

using namespace std;
//using namespace Hybrid_HW_LV_Process_Util;

///////////////////////////////////////////////////////////////////////
/*!
    @brief constructor

	@param[in] T
	@param[in] fx0
	@param[in] hwParams_d
	@param[in] hwParams_f
	@param[in] corParams
	@param[in] integralTimeSteps
	@param[in] numInt

*/
AQLMathHybridHWLVProcessHelper::AQLMathHybridHWLVProcessHelper(double T,
                                                         double fx0,
                                                         AQLMathHullWhiteParams& hwParams_d,
                                                         AQLMathHullWhiteParams& hwParams_f,
                                                         AQLMathCorrelationParams& corParams,
                                                         const DoubleArray& integralTimeSteps,
                                                         size_t numInt
                                                        )
:
mT(T),
mFX0(fx0),
mpHWParams_d(&hwParams_d),
mpHWParams_f(&hwParams_f),
mpCorParams(&corParams),
mIntegralTimeSteps(integralTimeSteps),
mNumInt(numInt)
{
    mIntInvE_d.SetFunc( *this, &AQLMathHybridHWLVProcessHelper::calcInvE_d );
    mIntInvE_f.SetFunc( *this, &AQLMathHybridHWLVProcessHelper::calcInvE_f );

	mIntLamSQ.SetFunc(*this, &AQLMathHybridHWLVProcessHelper::calcLambdaSQ);
}

/*!
    @brief copy constructor
	@param[in] rhs copy source 
*/
AQLMathHybridHWLVProcessHelper::AQLMathHybridHWLVProcessHelper( const AQLMathHybridHWLVProcessHelper& rhs )
:
mT(rhs.mT),
mFX0(rhs.mFX0),
mNumInt(rhs.mNumInt),
mIntegralTimeSteps(rhs.mIntegralTimeSteps),
mpCorParams(rhs.mpCorParams),
mpHWParams_d(rhs.mpHWParams_d),
mpHWParams_f(rhs.mpHWParams_f)
{
    mIntInvE_d.SetFunc( *this, &AQLMathHybridHWLVProcessHelper::calcInvE_d );
    mIntInvE_f.SetFunc( *this, &AQLMathHybridHWLVProcessHelper::calcInvE_f );

	mIntLamSQ.SetFunc(*this, &AQLMathHybridHWLVProcessHelper::calcLambdaSQ);
}

/*!
    @brief calc E_d
	@param[in] t
*/
double AQLMathHybridHWLVProcessHelper::calcE_d(double t) const
{ 
    if(!mIsCacheE_d[t])
    {
	    //
	    //size_t m = upper_bound(mIntegralTimeSteps.begin(), mIntegralTimeSteps.end(), t) - mIntegralTimeSteps.begin();
        //if( m != 0 && m < mIntegralTimeSteps.size() ) m = eq(t, mIntegralTimeSteps[m], 1. / 367. ) ? m : --m;
        size_t m = searchIdx(t, mIntegralTimeSteps);
	
		if( m == 0 )
    	{
            return AQLMath::exp( mpHWParams_d->mA[0] * t );
	    }

	    //
	    double tmp = 0;
	    size_t i;
	    for(i = 1; i <= m; ++i)
	    {
		    tmp += mpHWParams_d->mA[i-1] * (mIntegralTimeSteps[i] - mIntegralTimeSteps[i-1]);
	    }
	    if( t > mIntegralTimeSteps[m] )
	    {
		    if(m < mIntegralTimeSteps.size())
		    {
			    tmp += mpHWParams_d->mA[m] * ( t - mIntegralTimeSteps[m] );
		    }

	    }
		
	    mIsCacheE_d[t] = true;
	    mCacheE_d[t] = AQLMath::exp(tmp);
    }
    return mCacheE_d[t];
}

/*!
    @brief calc inverse of E_d
	@param[in] t
*/
inline double AQLMathHybridHWLVProcessHelper::calcInvE_d(double t) const
{ 
    return 1. / calcE_d(t);
}


/*!
    @brief calc Vol_Pd
	@param[in] t
	@param[in] T
*/
double AQLMathHybridHWLVProcessHelper::calcVol_Pd(double t, double T) const
{ 
    if(T == 0)
    {
	    return 0.;
    }

	//size_t p = upper_bound(mIntegralTimeSteps.begin(), mIntegralTimeSteps.end(), t) - mIntegralTimeSteps.begin();
    //if( p != 0 && p < mIntegralTimeSteps.size() ) p = eq(t, mIntegralTimeSteps[p], 1. / 367. ) ? p : --p;
    size_t p  = searchIdx(t, mIntegralTimeSteps);

    //size_t q = upper_bound(mIntegralTimeSteps.begin(), mIntegralTimeSteps.end(), T) - mIntegralTimeSteps.begin();
    //if( q != 0 && q < mIntegralTimeSteps.size() ) q = eq(t, mIntegralTimeSteps[q], 1. / 367. ) ? q : --q;
    size_t q  = searchIdx(T, mIntegralTimeSteps);

    if(!mIsCacheVol_Pd[T][t])
	{
		double ts = mIntegralTimeSteps[q]; double te = T;
    	double tmp = 0.;
	    if(ts < te)
	    {
            tmp = mIntInvE_d.IntegrateGLegendre(ts, te, mNumInt);
            mCacheVol_Pd[T][ts] = tmp;
            mIsCacheVol_Pd[T][ts] = true;
        }
    	
	    for(size_t i = q; p < i; --i)
	    {
		    ts = mIntegralTimeSteps[i-1]; te = mIntegralTimeSteps[i];
		    if(!mIsCacheVol_Pd[T][ts])
		    {
			    tmp += mIntInvE_d.IntegrateGLegendre(ts, te, mNumInt);
			    mIsCacheVol_Pd[T][ts] = true;
			    mCacheVol_Pd[T][ts] = tmp;
			}
    		else
	    	{
		    	tmp = mCacheVol_Pd[T][ts];
		    }
	    }
	    ts = t; te = mIntegralTimeSteps[p];
	    mCacheVol_Pd[T][ts] = mCacheVol_Pd[T][te] + mIntInvE_d.IntegrateGLegendre(ts, te, mNumInt);
	    mIsCacheVol_Pd[T][ts] = true;
    }
	return  mpHWParams_d->mSigma[p] * calcE_d(t) * mCacheVol_Pd[T][t];
}

/*!
    @brief calc E_f
	@param[in] t
*/
double AQLMathHybridHWLVProcessHelper::calcE_f(double t) const
{ 
	if(!mIsCacheE_f[t])
	{
		//
		//size_t m = upper_bound(mIntegralTimeSteps.begin(), mIntegralTimeSteps.end(), t) - mIntegralTimeSteps.begin();
        //if( m != 0 && m < mIntegralTimeSteps.size() ) m = eq(t, mIntegralTimeSteps[m], 1. / 367. ) ? m : --m;
        size_t m  = searchIdx(t, mIntegralTimeSteps);

		
		if( m == 0 )
		{
			return AQLMath::exp( mpHWParams_f->mA[0] * t );
		}

		//
		double tmp = 0;
		size_t i;
		for(i = 1; i <= m; ++i)
		{
			tmp += mpHWParams_f->mA[i-1] * (mIntegralTimeSteps[i] - mIntegralTimeSteps[i-1]);
		}
		if( t > mIntegralTimeSteps[m] )
		{
			if(m < mIntegralTimeSteps.size())
			{
				tmp += mpHWParams_f->mA[m] * ( t - mIntegralTimeSteps[m] );
			}

		}
		
		mIsCacheE_f[t] = true;
		mCacheE_f[t] = AQLMath::exp(tmp);
	}
	return mCacheE_f[t];
}

/*!
    @brief calc inverse of E_f
	@param[in] t
*/
double AQLMathHybridHWLVProcessHelper::calcInvE_f(double t) const
{ 
	return 1. / calcE_f(t);
}

/*!
    @brief calc Vol_Pf
	@param[in] t
	@param[in] T
*/
double AQLMathHybridHWLVProcessHelper::calcVol_Pf(double t, double T) const
{ 
    if(T == 0)
    {
	    return 0.;
    }

    //size_t p = upper_bound(mIntegralTimeSteps.begin(), mIntegralTimeSteps.end(), t) - mIntegralTimeSteps.begin();
    //if( p != 0 && p < mIntegralTimeSteps.size() ) p = eq(t, mIntegralTimeSteps[p], 1. / 367. ) ? p : --p;
    size_t p  = searchIdx(t, mIntegralTimeSteps);
    
    //size_t q = upper_bound(mIntegralTimeSteps.begin(), mIntegralTimeSteps.end(), T) - mIntegralTimeSteps.begin();
    //if( q != 0 && q < mIntegralTimeSteps.size() ) q = eq(t, mIntegralTimeSteps[q], 1. / 367. ) ? q : --q;
    size_t q  = searchIdx(T, mIntegralTimeSteps);

    if(!mIsCacheVol_Pf[T][t])
    {
	    double ts = mIntegralTimeSteps[q]; double te = T;
	    double tmp = 0.;
	    if(ts < te)
	    {
		    tmp = mIntInvE_f.IntegrateGLegendre(ts, te, mNumInt);
            mCacheVol_Pf[T][ts] = tmp;
            mIsCacheVol_Pf[T][ts] = true;
        }

	    for(size_t i = q; p < i; --i)
	    {
		    ts = mIntegralTimeSteps[i-1]; te = mIntegralTimeSteps[i];
		    if(!mIsCacheVol_Pf[T][ts])
		    {
			    tmp += mIntInvE_f.IntegrateGLegendre(ts, te, mNumInt);
			    mIsCacheVol_Pf[T][ts] = true;
			    mCacheVol_Pf[T][ts] = tmp;
		    }
		    else
		    {
			    tmp = mCacheVol_Pf[T][ts];
		    }
	    }
	    ts = t; te = mIntegralTimeSteps[p];
	    mCacheVol_Pf[T][ts] = mCacheVol_Pf[T][te] + mIntInvE_f.IntegrateGLegendre(ts, te, mNumInt);
	    mIsCacheVol_Pf[T][ts] = true;
    }
    return  mpHWParams_f->mSigma[p] * calcE_f(t) * mCacheVol_Pf[T][t];
}

/*!
    @brief calc Lambda
	@param[in] t
*/
double AQLMathHybridHWLVProcessHelper::calcLambda(double t) const
{
    //size_t t_idx = upper_bound(mpCorParams->mT.begin(), mpCorParams->mT.end(), t) - mpCorParams->mT.begin();
    //if( t_idx != 0 && t_idx < mpCorParams->mT.size() ) t_idx = eq(t, mpCorParams->mT[t_idx], 1. / 367. ) ? t_idx : --t_idx;
    size_t t_idx  = searchIdx(t, mpCorParams->mT);

	double a = calcVol_Pf(t) * calcVol_Pf(t) + calcVol_Pd(t) * calcVol_Pd(t) - 2. * mpCorParams->mRho[t_idx][0][1] * calcVol_Pd(t) * calcVol_Pf(t);
	double b = 2. * mpCorParams->mRho[t_idx][0][2] * calcVol_Pd(t) - 2. * mpCorParams->mRho[t_idx][1][2] * calcVol_Pf(t);

    double gamma = calcGamma(t);
	return AQLMath::sqrt(a + b * gamma + gamma * gamma);
}

/*!
    @brief calc Lambda suquared
	@param[in] t
*/
double AQLMathHybridHWLVProcessHelper::calcLambdaSQ(double t) const
{
    return calcLambda(t) * calcLambda(t);
}

/*!
    @brief calc inverse of Lambda suquared
	@param[in] t
*/
double AQLMathHybridHWLVProcessHelper::calcIntLamSQ(double t) const
{
	if(t == 0)
	{
		return 0.;
	}

	//size_t m = upper_bound(mIntegralTimeSteps.begin(), mIntegralTimeSteps.end(), t) - mIntegralTimeSteps.begin();
    //if( m != 0 && m < mIntegralTimeSteps.size() ) m = eq(t, mIntegralTimeSteps[m], 1. / 367. ) ? m : --m;
    size_t m  = searchIdx(t, mIntegralTimeSteps);
	
    if(!mIsCacheIntLamSQ[mT][t])
	{
		double tmp = 0.;
		double ts; double te;
		for(size_t i = 0; i < m; ++i)
		{
			ts = mIntegralTimeSteps[i]; te = mIntegralTimeSteps[i+1];
			if(!mIsCacheIntLamSQ[mT][te])
			{
				tmp += mIntLamSQ.IntegrateGLegendre(ts, te, mNumInt);
				mIsCacheIntLamSQ[mT][te] = true;
				mCacheIntLamSQ[mT][te] = tmp;
			}
			else
			{
				tmp = mCacheIntLamSQ[mT][te];
			}
		}
		ts = mIntegralTimeSteps[m]; te = t;
		mCacheIntLamSQ[mT][te] = mCacheIntLamSQ[mT][ts] + mIntLamSQ.IntegrateGLegendre(ts, te, mNumInt);
		mIsCacheIntLamSQ[mT][te] = true;
	}
	return mCacheIntLamSQ[mT][t];
}

/*!
    @brief set T
	@param[in] T
*/
void AQLMathHybridHWLVProcessHelper::setT(double T) const
{
        
    if( mIntegralTimeSteps.back() < T )
    {
        throw AQLCoreInvalidData("mIntegralTimeSteps.back() < T :: AQLMathHybridHWLVProcessHelper::setT", __FILE__, __LINE__);
    }

    if( mpHWParams_f->mT.back() < T )
    {
        throw AQLCoreInvalidData("mpHWParams_f->mT.back() < T :: AQLMathHybridHWLVProcessHelper::setT", __FILE__, __LINE__);
    }
    
    if( mpHWParams_d->mT.back() < T )
    {
        throw AQLCoreInvalidData("mpHWParams_d->mT.back() < T :: AQLMathHybridHWLVProcessHelper::setT", __FILE__, __LINE__);
    }

    if( mpCorParams->mT.back() < T )
    {
        throw AQLCoreInvalidData("AQLMathCorrelationParams->mT.back() < T :: AQLMathHybridHWLVProcessHelper::setT", __FILE__, __LINE__);
    }

    mT = T;
}


/*!
    @brief clear cache

*/
void AQLMathHybridHWLVProcessHelper::clearCache1() const
{
//    mCacheE_d.clear();
//    mIsCacheE_d.clear();
//    mCacheVol_Pd.clear();
//    mIsCacheVol_Pd.clear();

//    mIsCacheE_f.clear();
//    mIsCacheE_f.clear();
//    mCacheVol_Pf.clear();
//    mIsCacheVol_Pf.clear();

    mCacheIntLamSQ.clear();
    mIsCacheIntLamSQ.clear();
}

/*!
    @brief clear cache

*/
void AQLMathHybridHWLVProcessHelper::clearCache2() const
{
}

/*!
    @brief calc gamma
	@param[in] t
	@param[in] x
*/
double AQLMathHybridHWDDProcessHelper::calcGamma(double t, double x) const
{
     if( AQLMath::abs(x) < numeric_limits<double>::epsilon() )
     {
         x = x < 0 ? -numeric_limits<double>::epsilon() : numeric_limits<double>::epsilon();
     }

     //size_t t_idx = upper_bound(mpDDParams->mT.begin(), mpDDParams->mT.end(), t) - mpDDParams->mT.begin();
     //if( t_idx != 0 && t_idx < mpDDParams->mT.size() ) t_idx = eq(t, mpDDParams->mT[t_idx], 1. / 367. ) ? t_idx : --t_idx;
     size_t t_idx  = searchIdx(t, mpDDParams->mT);

     double sigma = mpDDParams->mSigma[t_idx];
     double beta = mpDDParams->mSigma[t_idx];
                 
     return sigma * ( x + (1. - beta) * (x - mFX0) ) / x;
}

/*!
    @brief constructor

	@param[in] T
	@param[in] fx0
	@param[in] hwParams_d
	@param[in] hwParams_f
	@param[in] ddParams
	@param[in] corParams
	@param[in] integralTimeSteps
	@param[in] numInt

*/
AQLMathHybridHWDDProcessHelper::AQLMathHybridHWDDProcessHelper(double T,
                                                         double fx0,
                                                         AQLMathHullWhiteParams& hwParams_d,
                                                         AQLMathHullWhiteParams& hwParams_f,
                                                         AQLMathDisplacedDiffusionParams& ddParams,
                                                         AQLMathCorrelationParams& corParams,
                                                         const DoubleArray& integralTimeSteps,
                                                         size_t numInt
                                                        )
:
AQLMathHybridHWLVProcessHelper(T, fx0, hwParams_d, hwParams_f, corParams, integralTimeSteps, numInt),
mpDDParams(&ddParams)
{
}

/*!
    @brief copy constructor
	@param[in] rhs copy source 
*/
AQLMathHybridHWDDProcessHelper::AQLMathHybridHWDDProcessHelper( const AQLMathHybridHWDDProcessHelper& rhs )
:
AQLMathHybridHWLVProcessHelper(rhs),
mpDDParams(rhs.mpDDParams)
{
}

/*!
    @brief calc U1helper
	@param[in] s
*/
double AQLMathHybridHWDDMPProcessHelper::calcU1helper(double s) const
{
    //
    double delta_vol_Pf = calcVol_Pf(s, mT) - calcVol_Pf(s, m_t);
    double delta_vol_Pd = calcVol_Pd(s, mT) - calcVol_Pd(s, m_t);

    //
    //size_t t_idx = upper_bound(mpCorParams->mT.begin(), mpCorParams->mT.end(), s) - mpCorParams->mT.begin();
    //if( t_idx != 0 && t_idx < mpCorParams->mT.size() ) t_idx = eq(s, mpCorParams->mT[t_idx], 1. / 367. ) ? t_idx : --t_idx;
    size_t t_idx = searchIdx(s, mpCorParams->mT);
    
    double rho_df = mpCorParams->mRho[t_idx][0][1];
    double rho_dS = mpCorParams->mRho[t_idx][0][2];
    double rho_fS = mpCorParams->mRho[t_idx][1][2];

    //
    //t_idx = upper_bound(mpDDParams->mT.begin(), mpDDParams->mT.end(), s) - mpDDParams->mT.begin();
    //if( t_idx != 0 && t_idx < mpDDParams->mT.size() ) t_idx = eq(s, mpDDParams->mT[t_idx], 1. / 367. ) ? t_idx : --t_idx;
    t_idx = searchIdx(s, mpDDParams->mT);

    double sigma = mpDDParams->mSigma[t_idx];

    //
    return   rho_df * delta_vol_Pf * calcVol_Pd(s, mT) - delta_vol_Pf * calcVol_Pf(s, mT) + rho_fS * delta_vol_Pf * sigma
           - delta_vol_Pd * calcVol_Pd(s, mT) + rho_df * delta_vol_Pd * calcVol_Pf(s, mT) - rho_dS * delta_vol_Pd * sigma;
}

/*!
    @brief calc weight
	@param[in] t
*/
double AQLMathHybridHWDDMPProcessHelper::calcWeight(double t) const
{
    //
    size_t t_idx = searchIdx(t, mpDDParams->mT);
    
    double sigma = mpDDParams->mSigma[t_idx];

    //
    double b = 2. * mpCorParams->mRho[t_idx][0][2] * calcVol_Pd(t) - 2. * mpCorParams->mRho[t_idx][1][2] * calcVol_Pf(t);

    //
    double u1_t = calcU1(t);
    double u2_t = calcU2(t);

    double u2_T = calcU2(mT);

    return ( 2. * sigma * sigma + b * sigma ) * ( u1_t + u2_t ) / u2_T / u2_T;
}

/*!
    @brief calc U1
	@param[in] t
*/
double AQLMathHybridHWDDMPProcessHelper::calcU1(double t) const
{
    set_t(t);

    if(!mIsCacheU1[t])
    {
	    //
	    //size_t m = upper_bound(mIntegralTimeSteps.begin(), mIntegralTimeSteps.end(), t) - mIntegralTimeSteps.begin();
        //if( m != 0 && m < mIntegralTimeSteps.size() ) m = eq(t, mIntegralTimeSteps[m], 1. / 367. ) ? m : --m;
        size_t m = searchIdx(t, mIntegralTimeSteps);

        double tmp = 0;
		if( m == 0 )
    	{
            tmp = mIntU1Helper.IntegrateGLegendre(0, t, mNumInt);
            mIsCacheU1[t] = true;
	        mCacheU1[t] = tmp;

            return mCacheU1[t];
	    }

	    //
	    size_t i;
        double ts, te;
	    for(i = 1; i <= m; ++i)
	    {
            ts = mIntegralTimeSteps[i-1]; te = mIntegralTimeSteps[i];
            if(!mIsCacheU1[te])
            {
                tmp += mIntU1Helper.IntegrateGLegendre(ts, te, mNumInt);
                mIsCacheU1[te] = true;
	            mCacheU1[te] = tmp;
            }
            else
            {
    	        tmp = mCacheU1[te];
            }
            
	    }

	    if( t > mIntegralTimeSteps[m] )
	    {
		    if(m < mIntegralTimeSteps.size())
		    {
			    tmp += mIntU1Helper.IntegrateGLegendre(te, t, mNumInt);
		    }
	    }

	    mIsCacheU1[t] = true;
	    mCacheU1[t] = tmp;
    }
    return mCacheU1[t];
}

/*!
    @brief calc U2
	@param[in] t
*/
double AQLMathHybridHWDDMPProcessHelper::calcU2(double t) const
{
    return calcIntLamSQ(t);
}

/*!
    @brief set t
	@param[in] t
*/
void AQLMathHybridHWDDMPProcessHelper::set_t(double t) const
{
        
    if( mIntegralTimeSteps.back() < t )
    {
        throw AQLCoreInvalidData("mIntegralTimeSteps.back() < t :: AQLMathHybridHWLVProcessHelper::set_t", __FILE__, __LINE__);
    }

    if( mpHWParams_f->mT.back() < t )
    {
        throw AQLCoreInvalidData("mpHWParams_f->mT.back() < t :: AQLMathHybridHWLVProcessHelper::set_t", __FILE__, __LINE__);
    }
    
    if( mpHWParams_d->mT.back() < t )
    {
        throw AQLCoreInvalidData("mpHWParams_d->mT.back() < t :: AQLMathHybridHWLVProcessHelper::set_t", __FILE__, __LINE__);
    }

    if( mpCorParams->mT.back() < t )
    {
        throw AQLCoreInvalidData("AQLMathCorrelationParams->mT.back() < t :: AQLMathHybridHWLVProcessHelper::set_t", __FILE__, __LINE__);
    }

    if(m_t != t)
    {
        m_t = t;
//clear_cache();
        mIsCacheU1.clear();
        mCacheU1.clear();
    }    
}

/*!
    @brief calc gamma
	@param[in] t
	@param[in] x
*/
double AQLMathHybridHWDDMPProcessHelper::calcGamma(double t, double x) const
{ 
    //size_t i = upper_bound(mpDDParams->mT.begin(), mpDDParams->mT.end(), t) - mpDDParams->mT.begin();
    //if( i != 0 && i < mIntegralTimeSteps.size() ) i = eq(t, mIntegralTimeSteps[i], 1. / 367. ) ? i : --i;
    size_t i = searchIdx(t, mpDDParams->mT);
    
    double simga = mpDDParams->mSigma[i];
    double beta = mpDDParams->mBeta[i];

    if(x == mFX0)
    {
        return simga;
    }
    else
    {
        return simga * ( beta + (1. - beta) * mFX0 / x * AQLMath::exp(-calcR(t) *  log(x / mFX0) ) );
    }
}

/*!
    @brief calc skew

*/
double AQLMathHybridHWDDMPProcessHelper::calcSkew() const
{
    //size_t p = upper_bound(mIntegralTimeSteps.begin(), mIntegralTimeSteps.end(), 0) - mIntegralTimeSteps.begin();
    //if( p != 0 && p < mIntegralTimeSteps.size() ) p = eq(t, mIntegralTimeSteps[p], 1. / 367. ) ? p : --p;
    size_t p = searchIdx(0., mIntegralTimeSteps);

    //size_t q = upper_bound(mIntegralTimeSteps.begin(), mIntegralTimeSteps.end(), mT) - mIntegralTimeSteps.begin();
    //if( q != 0 && q < mIntegralTimeSteps.size() ) q = eq(t, mIntegralTimeSteps[q], 1. / 367. ) ? q : --q;
    size_t q = searchIdx(mT, mIntegralTimeSteps);

    if(!mIsCacheSkew[mT][0.])
    {
        double ts = mIntegralTimeSteps[q]; double te = mT;
        double tmp = 0.;
        
        if(ts < te)
        {
            tmp = mIntSkewHelper.IntegrateGLegendre(ts, te, mNumInt);
        
            mCacheSkew[mT][ts] = tmp;
            mIsCacheSkew[mT][ts] = true;

            if(q == 0) return mCacheSkew[mT][ts] + 1.;
        }

        for(size_t i = q; p < i; --i)
        {
            ts = mIntegralTimeSteps[i-1]; te = mIntegralTimeSteps[i];
            if(!mIsCacheSkew[mT][ts])
            {
                tmp += mIntSkewHelper.IntegrateGLegendre(ts, te, mNumInt);
                mIsCacheSkew[mT][ts] = true;
                mCacheSkew[mT][ts] = tmp;
            }
	        else
            {
                tmp = mCacheSkew[mT][ts];
            }
        }
        ts = 0.; te = mIntegralTimeSteps[p];
        mCacheSkew[mT][ts] = mCacheSkew[mT][te] + mIntSkewHelper.IntegrateGLegendre(ts, te, mNumInt);
        mIsCacheSkew[mT][ts] = true;
    }
    return  mCacheSkew[mT][0.] + 1.;
}

/*!
    @brief skew helper

	@param[in] t

*/
double AQLMathHybridHWDDMPProcessHelper::calcSkewHelper(double t) const
{
    //size_t i = upper_bound(mpDDParams->mT.begin(), mpDDParams->mT.end(), t) - mpDDParams->mT.begin();
    //if( i != 0 && i < mpDDParams->mT.size() ) i = eq(t, mpDDParams->mT[i], 1. / 367. ) ? i : --i;
    size_t i = searchIdx(t, mpDDParams->mT);

    double beta = mpDDParams->mBeta[i];

    //return (beta - 1.) * weight(t);
    if(!mIsCacheWeight[t])
    {
        mCacheWeight[t] = calcWeight(t);
        mIsCacheWeight[t] = true;
    }

    return (beta - 1.) * mCacheWeight[t];
}

/*!
    @brief calc CDF

	@param[in] ts_
	@param[in] te_
*/
double AQLMathHybridHWDDMPProcessHelper::calcCDF(double ts_, double te_) const
{
    //
    size_t p = searchIdx(ts_, mIntegralTimeSteps);
    size_t q = searchIdx(te_, mIntegralTimeSteps);

    double ts = mIntegralTimeSteps[q]; double te = te_;
    double tmp = 0.;
    
    //
    if(ts < te)
    {
        tmp = mIntCDFHelper.IntegrateGLegendre(ts, te, mNumInt);        
        if(q == 0) return tmp;
    }

    //
    for(size_t i = q; p < i; --i)
    {
        ts = mIntegralTimeSteps[i-1]; te = mIntegralTimeSteps[i];
        tmp += mIntCDFHelper.IntegrateGLegendre(ts, te, mNumInt);
    }
    
    ts = 0.; te = mIntegralTimeSteps[p];
    return tmp + mIntCDFHelper.IntegrateGLegendre(ts, te, mNumInt);
}

/*!
    @brief calc CDF Helper

	@param[in] t
*/
double AQLMathHybridHWDDMPProcessHelper::calcCDFHelper(double t) const
{

    //return (beta - 1.) * weight(t);
    if(!mIsCacheWeight[t])
    {
        mCacheWeight[t] = calcWeight(t);
        mIsCacheWeight[t] = true;
    }

    return mCacheWeight[t];
}

/*!
    @brief calc C1

	@param[in] t
*/
double AQLMathHybridHWDDMPProcessHelper2::calcC1(double t) const
{
    if(!mIsCacheC1[t])
    {
	    //
        size_t m = searchIdx(t, mIntegralTimeSteps);

        double tmp = 0;
		if( m == 0 )
    	{
            tmp = mIntC1Helper.IntegrateGLegendre(0, t, mNumInt);
            mIsCacheC1[t] = true;
	        mCacheC1[t] = tmp;

            return mCacheC1[t];
	    }

	    //
	    size_t i;
        double ts, te;
	    for(i = 1; i <= m; ++i)
	    {
            ts = mIntegralTimeSteps[i-1]; te = mIntegralTimeSteps[i];
            if(!mIsCacheC1[te])
            {
                tmp += mIntC1Helper.IntegrateGLegendre(ts, te, mNumInt);
                mIsCacheC1[te] = true;
	            mCacheC1[te] = tmp;
            }
            else
            {
    	        tmp = mCacheC1[te];
            }
            
	    }

	    if( t > mIntegralTimeSteps[m] )
	    {
		    if(m < mIntegralTimeSteps.size())
		    {
			    tmp += mIntC1Helper.IntegrateGLegendre(te, t, mNumInt);
		    }
	    }

	    mIsCacheC1[t] = true;
	    mCacheC1[t] = tmp;
    }
    return mCacheC1[t];
}

/*!
    @brief calc C1 Helper

	@param[in] t
*/
double AQLMathHybridHWDDMPProcessHelper2::calcC1Helper(double t) const
{
    //
    size_t t_idx = searchIdx(t, mpCorParams->mT);
    double rho_df = mpCorParams->mRho[t_idx][0][1];
    double rho_fS = mpCorParams->mRho[t_idx][1][2];

    //
    t_idx = searchIdx(t, mpDDParams->mT);
    double sigma = mpDDParams->mSigma[t_idx];

    //
    return rho_df * calcVol_Pd(t, mT) * calcVol_Pf(t, mT) + rho_fS * calcVol_Pf(t, mT) * sigma;
    //return rho_df * calcVol_Pd(t, mT) * calcVol_Pf(t, mT) + rho_fS * (calcVol_Pf(t, mT) - calcVol_Pf(t, m_t)) * sigma;
}

/*!
    @brief calc C3 Helper

	@param[in] s
*/
double AQLMathHybridHWDDMPProcessHelper2::calcU3Helper(double s) const
{
    //
    double delta_vol_Pf = calcVol_Pf(s, mT) - calcVol_Pf(s, m_t);
    double delta_vol_Pd = calcVol_Pd(s, mT) - calcVol_Pd(s, m_t);

    //
    size_t t_idx = searchIdx(s, mpCorParams->mT);
    double rho_df = mpCorParams->mRho[t_idx][0][1];

    //
    return  delta_vol_Pf * delta_vol_Pf - 2. * rho_df * delta_vol_Pf * delta_vol_Pd + delta_vol_Pd * delta_vol_Pd;
}

/*!
    @brief calc C3

	@param[in] t
*/
double AQLMathHybridHWDDMPProcessHelper2::calcU3(double t) const
{
    set_t(t);

    if(!mIsCacheU3[t])
    {
	    //
	    //size_t m = upper_bound(mIntegralTimeSteps.begin(), mIntegralTimeSteps.end(), t) - mIntegralTimeSteps.begin();
        //if( m != 0 && m < mIntegralTimeSteps.size() ) m = eq(t, mIntegralTimeSteps[m], 1. / 367. ) ? m : --m;
        size_t m = searchIdx(t, mIntegralTimeSteps);

        double tmp = 0;
		if( m == 0 )
    	{
            tmp = mIntU3Helper.IntegrateGLegendre(0, t, mNumInt);
            mIsCacheU3[t] = true;
	        mCacheU3[t] = tmp;

            return mCacheU3[t];
	    }

	    //
	    size_t i;
        double ts, te;
	    for(i = 1; i <= m; ++i)
	    {
            ts = mIntegralTimeSteps[i-1]; te = mIntegralTimeSteps[i];
            if(!mIsCacheU3[te])
            {
                tmp += mIntU3Helper.IntegrateGLegendre(ts, te, mNumInt);
                mIsCacheU3[te] = true;
	            mCacheU3[te] = tmp;
            }
            else
            {
    	        tmp = mCacheU3[te];
            }
            
	    }

	    if( t > mIntegralTimeSteps[m] )
	    {
		    if(m < mIntegralTimeSteps.size())
		    {
			    tmp += mIntU3Helper.IntegrateGLegendre(te, t, mNumInt);
		    }
	    }

	    mIsCacheU3[t] = true;
	    mCacheU3[t] = tmp;
    }
    return mCacheU3[t];
}

/*!
    @brief calc weight

	@param[in] t
*/
double AQLMathHybridHWDDMPProcessHelper2::calcWeight(double t) const
{
    //
    size_t t_idx = searchIdx(t, mpDDParams->mT);
    
    double sigma = mpDDParams->mSigma[t_idx];

    //
    double b = 2. * mpCorParams->mRho[t_idx][0][2] * calcVol_Pd(t) - 2. * mpCorParams->mRho[t_idx][1][2] * calcVol_Pf(t);

    //
    double u1_t = calcU1(t);
    double u2_t = calcU2(t);

    double u2_T = calcU2(mT);
    //double u2_T = calcIntLamSQApproximated(mT);
    
    //double weight = 0.3;
    //double weight = 0.3;
    double weight = 0.;
    return ( weight + (1. - weight) * theta(t) ) * ( 2. * calcGamma(t) + b ) * sigma * ( u1_t + u2_t ) / u2_T / u2_T;
    //return ( 2. * gamma(t) + b ) * sigma * ( u1_t + u2_t ) / u2_T / u2_T;
}

/*!
    @brief calc U2

	@param[in] t
*/
double AQLMathHybridHWDDMPProcessHelper2::calcU2(double t) const
{
    return calcIntLamSQApproximated(t);
}

/*!
    @brief calc

	@param[in] t
*/
double AQLMathHybridHWDDMPProcessHelper2::calcIntLamSQApproximated(double t) const
{
	if(t == 0)
	{
		return 0.;
	}

	//size_t m = upper_bound(mIntegralTimeSteps.begin(), mIntegralTimeSteps.end(), t) - mIntegralTimeSteps.begin();
    //if( m != 0 && m < mIntegralTimeSteps.size() ) m = eq(t, mIntegralTimeSteps[m], 1. / 367. ) ? m : --m;
    size_t m  = searchIdx(t, mIntegralTimeSteps);
	
    if(!mIsCacheIntLamSQApproximated[mT][t])
	{
		double tmp = 0.;
		double ts; double te;
		for(size_t i = 0; i < m; ++i)
		{
			ts = mIntegralTimeSteps[i]; te = mIntegralTimeSteps[i+1];
			if(!mIsCacheIntLamSQApproximated[mT][te])
			{
				tmp += mIntLamSQApproximated.IntegrateGLegendre(ts, te, mNumInt);
				mIsCacheIntLamSQApproximated[mT][te] = true;
				mCacheIntLamSQApproximated[mT][te] = tmp;
			}
			else
			{
				tmp = mCacheIntLamSQApproximated[mT][te];
			}
		}
		ts = mIntegralTimeSteps[m]; te = t;
		mCacheIntLamSQApproximated[mT][te] = mCacheIntLamSQApproximated[mT][ts] + mIntLamSQApproximated.IntegrateGLegendre(ts, te, mNumInt);
		mIsCacheIntLamSQApproximated[mT][te] = true;
	}
	return mCacheIntLamSQApproximated[mT][t];
}

/*!
    @brief calc

	@param[in] t
*/
double AQLMathHybridHWDDMPProcessHelper2::calcLamSQApproximated(double t) const
{
    return calcLambdaApproximated(t) * calcLambdaApproximated(t);
}

/*!
    @brief calc

	@param[in] t
*/
double AQLMathHybridHWDDMPProcessHelper2::calcLambdaApproximated(double t) const
{
    //size_t t_idx = upper_bound(mpCorParams->mT.begin(), mpCorParams->mT.end(), t) - mpCorParams->mT.begin();
    //if( t_idx != 0 && t_idx < mpCorParams->mT.size() ) t_idx = eq(t, mpCorParams->mT[t_idx], 1. / 367. ) ? t_idx : --t_idx;
    size_t t_idx  = searchIdx(t, mpCorParams->mT);

	double a = calcVol_Pf(t) * calcVol_Pf(t) + calcVol_Pd(t) * calcVol_Pd(t) - 2. * mpCorParams->mRho[t_idx][0][1] * calcVol_Pd(t) * calcVol_Pf(t);
	double b = 2. * mpCorParams->mRho[t_idx][0][2] * calcVol_Pd(t) - 2. * mpCorParams->mRho[t_idx][1][2] * calcVol_Pf(t);

    //double gamma_ = AQLMathHybridHWDDMPProcessHelper::gamma(t, mFX0);
    double gamma = calcGamma(t, mFX0);
    return AQLMath::sqrt( a + b * gamma + gamma * gamma );
}