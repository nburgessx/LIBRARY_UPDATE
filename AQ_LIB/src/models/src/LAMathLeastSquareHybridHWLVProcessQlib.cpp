// Cost function for least-square problems
/*! Implements a cost function using the interface provided by
    the LeastSquareProblem class.
*/

#include "LAMathLeastSquareHybridHWLVProcessQlib.h"
#include "LAMathHybridHWLVUtil.h"
#include <algorithm>
using namespace QuantLib;
using namespace std;



/*!
    @brief constructor

	@param[in] fx0
	@param[in] processHelper
	@param[in] t2f
	@param[in] mode_

*/
LAMathLeastSquareHybridHWLVProcessQlib::LAMathLeastSquareHybridHWLVProcessQlib(const DoubleArray& fx0,
                                                                   vector<LAMathHybridHWLVProcessHelper*>& processHelper,
                                                                   LAMathTarget2fit2& t2f,
                                                                   mode mode_
                                                                  )
:
mFX0(fx0),
mpT2f(&t2f),
mMode(mode_),
//mProcessHelper(processHelper),
mCloned(false)
{
    //
    size_t n = mpT2f->mX.size();
    size_t m = mpT2f->mY[0].size();

    //
    mProcessHelper.resize(processHelper.size());
    for(size_t i = 0; i < processHelper.size(); ++i)
    {
        mProcessHelper[i] = processHelper[i];
    }
    
    if(mProcessHelper.size() != n)
    {
        throw LACoreInvalidData("mProcessHelper.size() != mpT2f->mX.size(): LAMathLeastSquareHybridHWLVProcessQlib::LAMathLeastSquareHybridHWLVProcessQlib", __FILE__, __LINE__);
    }

    //
    mTarget2fit.resize(n * m);
    mFct2fit.resize(n * m);
    mWeight.resize(n * m);

    //
    getTarget2fit();
}

/*!
    @brief set up

	@param[in] fx0
	@param[in] processHelper
	@param[in] t2f
	@param[in] mode_

*/
void LAMathLeastSquareHybridHWLVProcessQlib::setUp(const DoubleArray& fx0,
                                             vector<LAMathHybridHWLVProcessHelper*>& processHelper,
                                             LAMathTarget2fit2& t2f,
                                             mode mode_
                                            )
{
    //
    mFX0 = fx0;

    //
    mProcessHelper.resize(processHelper.size());
    for(size_t i = 0; i < processHelper.size(); ++i)
    {
        mProcessHelper[i] = processHelper[i];
    }

    //
    mpT2f = &t2f;
    mMode = mode_;
    mCloned = false;

    //
    size_t n = mpT2f->mX.size();
    size_t m = mpT2f->mY[0].size();

    //
    mTarget2fit.resize(n * m);
    mFct2fit.resize(n * m);
    mWeight.resize(n * m);

    //
    getTarget2fit();
}

/*!
    @brief copy constructor
	@param[in] rhs copy source 
*/
LAMathLeastSquareHybridHWLVProcessQlib::LAMathLeastSquareHybridHWLVProcessQlib( const LAMathLeastSquareHybridHWLVProcessQlib& rhs )
:
mFX0(rhs.mFX0),
mpT2f(rhs.mpT2f),
mMode(rhs.mMode),
mTarget2fit(rhs.mTarget2fit),
mFct2fit(rhs.mFct2fit),
mCloned(true)
{
    mProcessHelper.resize( rhs.mProcessHelper.size() );
    for(size_t i = 0; i < mProcessHelper.size(); ++i)
    {
		mProcessHelper[i] = rhs.mProcessHelper[i] != 0 ? rhs.mProcessHelper[i]->clone() : 0;
    }
}

/*!
    @brief destructor

*/
LAMathLeastSquareHybridHWLVProcessQlib::~LAMathLeastSquareHybridHWLVProcessQlib()
{
    if(mCloned)
    {
        for(size_t i = 0; i < mProcessHelper.size(); ++i)
        {
            delete mProcessHelper[i];
            mProcessHelper[i] = 0;
        }
    }
}

/*!
    @brief value
	@param[in] x
*/
Real LAMathLeastSquareHybridHWLVProcessQlib::value(const Array& x) const
{
    getFct2fit(x);

    double diff_SQ = 0.0;
    double diff;
    for(size_t i = 0; i < mFct2fit.size(); ++i)
    {
        diff = mFct2fit[i] - mTarget2fit[i];
        diff_SQ += diff * diff * mWeight[i];
    }

    return  diff_SQ;
}

/*!
    @brief values
	@param[in] x
*/
Array LAMathLeastSquareHybridHWLVProcessQlib::values(const Array& x) const
{
    getFct2fit(x);

    Array diff_SQs( x.size() );
    double diff;
    
    size_t k = 0;
    for(size_t i = 0; i < mFct2fit.size(); ++i)
    {
        diff = mFct2fit[i] - mTarget2fit[i];
        diff_SQs[k++] = diff * sqrt( mWeight[i] );
    }
    return diff_SQs;
}
/*!
    @brief assignment operator
	@param[in] rhs copy source
*/
LAMathLeastSquareHybridHWLVProcessQlib& LAMathLeastSquareHybridHWLVProcessQlib::operator =(const LAMathLeastSquareHybridHWLVProcessQlib& rhs)
{
    if ( this != &rhs )
    {
        mCloned = true;

        mFX0 = rhs.mFX0;

        mpT2f = rhs.mpT2f;
        mTarget2fit = rhs.mTarget2fit;
        mFct2fit = rhs.mFct2fit;

        for(size_t i = 0; i < rhs.mProcessHelper.size(); ++i)
        {
            mProcessHelper[i] = rhs.mProcessHelper[i] != 0 ? rhs.mProcessHelper[i]->clone() : 0;
        }

        mWeight = rhs.mWeight;
        mMode = rhs.mMode;
    }
    return *this;
}

/*!
    @brief calc target to fit
*/
void LAMathLeastSquareHybridHWLVProcessQlib::getTarget2fit() const
{
    //
    size_t n = mpT2f->mX.size();
    size_t m = mpT2f->mY[0].size();

    //
    size_t k = 0;
    for(size_t i = 0; i < n; ++i)
    {
        for(size_t j = 0; j < m; ++j)
        {
            mTarget2fit[k] = mpT2f->mTarget2fit[i][j];
            mWeight[k++] = mpT2f->mWeight[i][j];
        }
    }
}


/*!
    @brief constructor

	@param[in] fx0
	@param[in] processHelper
	@param[in] t2f
	@param[in] mode_

*/
LAMathLeastSquareHybridHWDDProcessQlib::LAMathLeastSquareHybridHWDDProcessQlib(const DoubleArray& fx0,
                                                                   vector<LAMathHybridHWDDProcessHelper*>& processHelper,
                                                                   LAMathTarget2fit2& t2f,
                                                                   mode mode_
                                                                  )
:
LAMathLeastSquareHybridHWLVProcessQlib(),
//mProcessHelper(processHelper),
mCloned(false)
{
    mProcessHelper.resize(processHelper.size());
    vector<LAMathHybridHWLVProcessHelper*> tmp(mProcessHelper.size());
    
    for(size_t i = 0; i < processHelper.size(); ++i)
    {
        tmp[i] = dynamic_cast<LAMathHybridHWLVProcessHelper*>(mProcessHelper[i]);
        mProcessHelper[i] = processHelper[i];
    }

    LAMathLeastSquareHybridHWLVProcessQlib::setUp(fx0, tmp, t2f, mode_);
}

/*!
    @brief copy constructor
	@param[in] rhs copy source 
*/
LAMathLeastSquareHybridHWDDProcessQlib::LAMathLeastSquareHybridHWDDProcessQlib( const LAMathLeastSquareHybridHWDDProcessQlib& rhs )
:
LAMathLeastSquareHybridHWLVProcessQlib(rhs),
mCloned(true)
{
    mProcessHelper.resize( rhs.mProcessHelper.size() );
    for(size_t i = 0; i < mProcessHelper.size(); ++i)
    {
        mProcessHelper[i] = rhs.mProcessHelper[i] != 0 ? rhs.mProcessHelper[i]->clone() : 0;
    }
}

/*!
    @brief destructor

*/
LAMathLeastSquareHybridHWDDProcessQlib::~LAMathLeastSquareHybridHWDDProcessQlib()
{
    if(mCloned)
    {
        for(size_t i = 0; i < mProcessHelper.size(); ++i)
        {
            delete mProcessHelper[i];
            mProcessHelper[i] = 0;
        }
    }
}

/*!
    @brief assignment operator
	@param[in] rhs copy source
*/
LAMathLeastSquareHybridHWDDProcessQlib& LAMathLeastSquareHybridHWDDProcessQlib::operator =(const LAMathLeastSquareHybridHWDDProcessQlib& rhs)
{
    if ( this != &rhs )
    {
        (*this).LAMathLeastSquareHybridHWDDProcessQlib::operator =(rhs);

        for(size_t i = 0; i < rhs.mProcessHelper.size(); ++i)
        {
            mProcessHelper[i] = rhs.mProcessHelper[i] != 0 ? rhs.mProcessHelper[i]->clone() : 0;
        }        
        mCloned = true;
    }
    return *this;
}

/*!
    @brief constructor

	@param[in] fx0
	@param[in] processHelper
	@param[in] vol2fit
	@param[in] skew2fit
	@param[in] weightVolSkew
	@param[in] mode_

*/
LAMathLeastSquareHybridHWDDProcess1Qlib::LAMathLeastSquareHybridHWDDProcess1Qlib(const DoubleArray& fx0,
                                                                     vector<LAMathHybridHWDDProcessHelper*>& processHelper,
                                                                     LAMathTarget2fit2& vol2fit,
                                                                     LAMathTarget2fit2& skew2fit,
                                                                     DoubleArray weightVolSkew, //{vol_weight, skew_weight}
                                                                     mode mode_
                                                                     )
:
LAMathLeastSquareHybridHWDDProcessQlib(fx0, processHelper, skew2fit, mode_),
//mProcessHelper(processHelper),
mpT2f2(&vol2fit),
mWeightVolSkew(weightVolSkew),
mTarget2fit2(DoubleArray(vol2fit.mX.size())),
mWeight2(DoubleArray(vol2fit.mX.size())),
mFct2fit2(DoubleArray(vol2fit.mX.size())),
mCloned(false)
{
    if(mpT2f2->mX.size() != mpT2f->mX.size())
    {
        throw LACoreInvalidData("vol2fit->mX.size() != skew2fit->mX.size() : LAMathLeastSquareHybridHWDDProcess1Qlib::LAMathLeastSquareHybridHWDDProcess1Qlib", __FILE__, __LINE__);
    }

    if(mpT2f2->mTarget2fit.size() != 1)
    {
        throw LACoreInvalidData("mpT2f2->mTarget2fit.size() != 1 : LAMathLeastSquareHybridHWDDProcess1Qlib::LAMathLeastSquareHybridHWDDProcess1Qlib", __FILE__, __LINE__);
    }

    if(mWeightVolSkew.size() != 2)
    {
        throw LACoreInvalidData("mWeightVolSkew.size() != 2 : LAMathLeastSquareHybridHWDDProcess1Qlib::LAMathLeastSquareHybridHWDDProcess1Qlib", __FILE__, __LINE__);
    }

}

/*!
    @brief calc target to fit
*/
void LAMathLeastSquareHybridHWDDProcess1Qlib::getTarget2fit() const
{
    // Target to fit
    LAMathLeastSquareHybridHWLVProcessQlib::getTarget2fit();

    //
    for(size_t i = 0; i < mpT2f2->mX.size(); ++i)
    {
        mTarget2fit2[i] = mpT2f2->mTarget2fit[0][i];
        mWeight2[i] = mpT2f2->mWeight[0][i];
    }
}

/*!
    @brief copy constructor
	@param[in] rhs copy source 
*/
LAMathLeastSquareHybridHWDDProcess1Qlib::LAMathLeastSquareHybridHWDDProcess1Qlib( const LAMathLeastSquareHybridHWDDProcess1Qlib& rhs )
:
LAMathLeastSquareHybridHWDDProcessQlib(rhs),
mpT2f2(rhs.mpT2f2),
mTarget2fit2(rhs.mTarget2fit2),
mFct2fit2(rhs.mFct2fit2),
mWeight2(rhs.mWeight2),
mCloned(true)
{
}

/*!
    @brief destructor
*/
LAMathLeastSquareHybridHWDDProcess1Qlib::~LAMathLeastSquareHybridHWDDProcess1Qlib()
{
}

/*!
    @brief assignment operator
	@param[in] rhs copy source
*/
LAMathLeastSquareHybridHWDDProcess1Qlib& LAMathLeastSquareHybridHWDDProcess1Qlib::operator =(const LAMathLeastSquareHybridHWDDProcess1Qlib& rhs)
{
    if ( this != &rhs )
    {
        (*this).LAMathLeastSquareHybridHWDDProcessQlib::operator =(rhs);

        mpT2f2 = rhs.mpT2f2;
        mTarget2fit2 = rhs.mTarget2fit2;
        mWeight2 = rhs.mWeight2;
        
        mCloned = true;
    }
    return *this;
}

/*!
    @brief calc function to fit
	@param[in] x
*/
void LAMathLeastSquareHybridHWDDProcess1Qlib::getFct2fit(const Array& x) const
{
    // Number of Canonical Time Grid
    size_t n = mpT2f->mX.size();
    
    // Error check
    if( 2 * n != x.size() )
    {
        throw LACoreInvalidData("mpT2f->x_.size() != x.size() : LAMathLeastSquareHybridHWDDProcessQlib::Get_fct2fit_SkewAndPrice", __FILE__, __LINE__);
    }

    // Set sigma and beta
    DoubleArray sigma(n);
    DoubleArray beta(n);
    for(size_t i = 0; i < n; ++i)
    {
        sigma[i] = x[i];
        beta[i] = x[n + i];
    }

    // Attension! DD params are Picewise Constant.
    double t_prev = 0.;
    size_t idx_s, idx_e;
    size_t m = mpT2f->mY[0].size();
    DoubleArray t_ddParams;
    for(size_t i = 0; i < n; ++i)
    {
        double t = mpT2f->mX[i];

        t_ddParams = mProcessHelper[i]->getDDTimeStep();        
        idx_s = lower_bound(t_ddParams.begin(), t_ddParams.end(), t_prev) - t_ddParams.begin();
        //if( idx_s != 0 && idx_s < t_ddParams.size() && t_ddParams[idx_s] + 1. / 367. < t_prev ) ++idx_s;
        //idx_s = searchIdx(t_prev, t_ddParams);

        idx_e = lower_bound(t_ddParams.begin(), t_ddParams.end(), t) - t_ddParams.begin();
        //if( idx_e != 0 && idx_e < t_ddParams.size() && t_ddParams[idx_e] + 1. / 367. < t ) ++idx_e;
        //idx_e = searchIdx(t, t_ddParams);

         for(size_t j = idx_s; j < idx_e; ++j)
        {
            if( j == idx_s && idx_s != 0 ) continue;

            setSigma(j, sigma[i]);
            setBeta(j, beta[i]);
        }

        t_prev = t;
        size_t k = 0;
        mFct2fit[k] = mProcessHelper[i]->calcSkew();   //under_construction!!
        mFct2fit2[k] = mProcessHelper[i]->calcSigma(); //under_construction!!
    }

    //

}

/*!
    @brief value
	@param[in] x
*/
Real LAMathLeastSquareHybridHWDDProcess1Qlib::value(const Array& x) const
{
    getFct2fit(x);

    // Skew
    double diff1;
    double diff_SQ1 = 0.0;

    for(size_t i = 0; i < mFct2fit.size(); ++i)
    {
        diff1 = mFct2fit[i] - mTarget2fit[i];
        diff_SQ1 += diff1 * diff1 * mWeight[i];
    }

    // Vol
    double diff2;
    double diff_SQ2 = 0.0;

    for(size_t i = 0; i < mFct2fit2.size(); ++i)
    {
        diff2 = mFct2fit2[i] - mTarget2fit2[i];
        diff_SQ2 += diff2 * diff2 * mWeight2[i];
    }

    return  mWeightVolSkew[0] * diff_SQ2 + mWeightVolSkew[1] * diff_SQ2;
}

/*!
    @brief values
	@param[in] x
*/
Array LAMathLeastSquareHybridHWDDProcess1Qlib::values(const Array& x) const
{
    getFct2fit(x);
    
    Array diff_SQs( x.size() );
    double diff1;
    double diff2;

    size_t k = 0;
    for(size_t i = 0; i < mFct2fit.size(); ++i)
    {
        diff1 = mFct2fit[i] - mTarget2fit[i];
        diff2 = mFct2fit2[i] - mTarget2fit2[i];

        diff_SQs[k++] = diff1 * sqrt(mWeight[i]) + diff2 * sqrt(mWeight[i]);
    }

    // Vol
    return  diff_SQs;
}

/*!
    @brief constructor

	@param[in] fx0
	@param[in] processHelper
	@param[in] vol2fit
	@param[in] ts
	@param[in] mode_

*/
LAMathLeastSquareHybridHWDDProcess2Qlib::LAMathLeastSquareHybridHWDDProcess2Qlib(const DoubleArray& fx0,
                                                                     vector<LAMathHybridHWDDProcessHelper*>& processHelper,
                                                                     LAMathTarget2fit2& vol2fit,
                                                                     double ts,
                                                                     mode mode_
                                                                     )
:
LAMathLeastSquareHybridHWDDProcessQlib(fx0, processHelper, vol2fit, mode_),
mTs(ts),
mCloned(false)
{
    if(mpT2f->mTarget2fit.size() != 1)
    {
        throw LACoreInvalidData("mpT2f2->mTarget2fit.size() != 1 : LAMathLeastSquareHybridHWDDProcess1Qlib::LAMathLeastSquareHybridHWDDProcess1Qlib", __FILE__, __LINE__);
    }

}

/*!
    @brief copy constructor
	@param[in] rhs copy source 
*/
LAMathLeastSquareHybridHWDDProcess2Qlib::LAMathLeastSquareHybridHWDDProcess2Qlib( const LAMathLeastSquareHybridHWDDProcess2Qlib& rhs )
:
LAMathLeastSquareHybridHWDDProcessQlib(rhs),
mTs(rhs.mTs),
mCloned(true)
{
}

/*!
    @brief destructor
*/
LAMathLeastSquareHybridHWDDProcess2Qlib::~LAMathLeastSquareHybridHWDDProcess2Qlib()
{
}

/*!
    @brief assignment operator
	@param[in] rhs copy source
*/
LAMathLeastSquareHybridHWDDProcess2Qlib& LAMathLeastSquareHybridHWDDProcess2Qlib::operator =(const LAMathLeastSquareHybridHWDDProcess2Qlib& rhs)
{
    if ( this != &rhs )
    {
        (*this).LAMathLeastSquareHybridHWDDProcessQlib::operator =(rhs);

        mTs = rhs.mTs;        
        mCloned = true;
    }
    return *this;
}

/*!
    @brief calc function to fit
	@param[in] x
*/
void LAMathLeastSquareHybridHWDDProcess2Qlib::getFct2fit(const Array& x) const
{
    // Number of Canonical Time Grid
    size_t n = mpT2f->mX.size();
    
    // Error check
    if( n != x.size() )
    {
        throw LACoreInvalidData("mpT2f->x_.size() != x.size() : LAMathLeastSquareHybridHWDDProcess2Qlib::Get_fct2fit_SkewAndPrice", __FILE__, __LINE__);
    }

    // Set sigma and beta
    DoubleArray sigma(n);
    for(size_t i = 0; i < n; ++i)
    {
        sigma[i] = x[i];
    }

    // Attension! DD params are Picewise Constant.
    double t_prev = mTs;
    size_t idx_s, idx_e;
    for(size_t i = 0; i < n; ++i)
    {
        DoubleArray t_ddParams = mProcessHelper[i]->getDDTimeStep();
    
        double t = mpT2f->mX[i];

        idx_s = lower_bound(t_ddParams.begin(), t_ddParams.end(), t_prev) - t_ddParams.begin();
        //if( idx_s != 0 && idx_s < t_ddParams.size() && t_ddParams[idx_s] + 1. / 367. < t_prev ) ++idx_s;
        //idx_s = searchIdx(t_prev, t_ddParams);

        idx_e = lower_bound(t_ddParams.begin(), t_ddParams.end(), t) - t_ddParams.begin();
        //if( idx_e != 0 && idx_e < t_ddParams.size() && t_ddParams[idx_e] + 1. / 367. < t ) ++idx_e;
        //idx_e = searchIdx(t, t_ddParams);

        if(idx_s == 0 && idx_e == 0) setSigma(0, sigma[i]);
    
        for(size_t j = idx_s; j < idx_e; ++j)
        {
            //if( j == idx_s && idx_s != 0 ) continue;
            setSigma(j, sigma[i]);
        }

        t_prev = t;

        //
        mFct2fit[i] = mProcessHelper[i]->calcSigma();
    }
}

/*!
    @brief constructor

	@param[in] fx0
	@param[in] processHelper
	@param[in] skew2fit
	@param[in] ts
	@param[in] mode_

*/
LAMathLeastSquareHybridHWDDProcess3Qlib::LAMathLeastSquareHybridHWDDProcess3Qlib(const DoubleArray& fx0,
                                                                     vector<LAMathHybridHWDDProcessHelper*>& processHelper,
                                                                     LAMathTarget2fit2& skew2fit,
                                                                     double ts,
                                                                     mode mode_
                                                                     )
:
LAMathLeastSquareHybridHWDDProcessQlib(fx0, processHelper, skew2fit, mode_),
mTs(ts),
mCloned(false)
{
    if(mpT2f->mTarget2fit.size() != 1)
    {
        throw LACoreInvalidData("mpT2f2->mTarget2fit.size() != 1 : LAMathLeastSquareHybridHWDDProcess1Qlib::LAMathLeastSquareHybridHWDDProcess1Qlib", __FILE__, __LINE__);
    }
}

/*!
    @brief copy constructor
	@param[in] rhs copy source 
*/
LAMathLeastSquareHybridHWDDProcess3Qlib::LAMathLeastSquareHybridHWDDProcess3Qlib( const LAMathLeastSquareHybridHWDDProcess3Qlib& rhs )
:
LAMathLeastSquareHybridHWDDProcessQlib(rhs),
mTs(rhs.mTs),
mCloned(true)
{
}

/*!
    @brief destructor

*/
LAMathLeastSquareHybridHWDDProcess3Qlib::~LAMathLeastSquareHybridHWDDProcess3Qlib()
{
}

/*!
    @brief assignment operator
	@param[in] rhs copy source
*/
LAMathLeastSquareHybridHWDDProcess3Qlib& LAMathLeastSquareHybridHWDDProcess3Qlib::operator =(const LAMathLeastSquareHybridHWDDProcess3Qlib& rhs)
{
    if ( this != &rhs )
    {
        (*this).LAMathLeastSquareHybridHWDDProcessQlib::operator =(rhs);
        
        mTs = rhs.mTs;
        mCloned = true;
    }
    return *this;
}

/*!
    @brief calc function to fit
	@param[in] x
*/
void LAMathLeastSquareHybridHWDDProcess3Qlib::getFct2fit(const Array& x) const
{
    // Number of Canonical Time Grid
    size_t n = mpT2f->mX.size();
    
    // Error check
    if( n != x.size() )
    {
        throw LACoreInvalidData("mpT2f->x_.size() != x.size() : LAMathLeastSquareHybridHWDDProcess3Qlib::Get_fct2fit_SkewAndPrice", __FILE__, __LINE__);
    }

    // Set sigma and beta
    DoubleArray beta(n);
    for(size_t i = 0; i < n; ++i)
    {
        beta[i] = x[i];
    }

    // Attension! DD params are Picewise Constant.
    double t_prev = mTs;
    size_t idx_s, idx_e;
    for(size_t i = 0; i < n; ++i)
    {
        DoubleArray t_ddParams = mProcessHelper[i]->getDDTimeStep();

        double t = mpT2f->mX[i];

        idx_s = lower_bound(t_ddParams.begin(), t_ddParams.end(), t_prev) - t_ddParams.begin();
        //if( idx_s != 0 && idx_s < t_ddParams.size() && t_ddParams[idx_s] + 1. / 367. < t_prev ) ++idx_s;
        //idx_s = searchIdx(t_prev, t_ddParams);

        idx_e = lower_bound(t_ddParams.begin(), t_ddParams.end(), t) - t_ddParams.begin();
        //if( idx_e != 0 && idx_e < t_ddParams.size() && t_ddParams[idx_e] + 1. / 367. < t ) ++idx_e;
        //idx_e = searchIdx(t, t_ddParams);

        if(idx_s == 0 && idx_e == 0) setBeta(0, beta[i]);

        for(size_t j = idx_s; j < idx_e; ++j)
        {
            setBeta(j, beta[i]);         
        }

        t_prev = t;

        //
        mFct2fit[i] = mProcessHelper[i]->calcSkew();
    }
}
