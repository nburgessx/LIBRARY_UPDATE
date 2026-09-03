// Cost function for least-square problems
/*! Implements a cost function using the interface provided by
    the LeastSquareProblem class.
*/

#include "LAMathLeastSquareBlackDDQlib.h"
#include "LABasic.h"
using namespace QuantLib;

/*!
    @brief constructor

	@param[in] s0
	@param[in] curve0_d
	@param[in] curve0_f
	@param[in] t
	@param[in] strike
	@param[in] prem
	@param[in] weight
	@param[in] callPutFlg
	@param[in] isCalibVol
	@param[in] isCalibSkew

*/
LAMathLeastSquareBlackDDQlib::LAMathLeastSquareBlackDDQlib(double s0,
                                         LARatesPathElementCurve& curve0_d,
                                         LARatesPathElementCurve& curve0_f,
                                         const DoubleArray& t,
                                         const DoubleMatrix& strike,
                                         const DoubleMatrix& prem,
                                         const DoubleMatrix& weight,
                                         const vector<IntVector>& callPutFlg,
                                         bool isCalibVol,
                                         bool isCalibSkew
                                        )
:
mCurve0_d(&curve0_d),
mT(t),
mStrike(strike),
mPrem(prem),
mCallPutFlg(callPutFlg),
mIsCalibVol(isCalibVol),
mIsCalibSkew(isCalibSkew),
mInit(true),
mCloned(false)
{
    size_t n = mT.size();
    size_t m = mPrem[0].size();
    
    if( n != mPrem.size() )
    {
        throw LACoreInvalidData("mT.size() != mPrem.size() :: LAMathLeastSquareBlackDDQlib::LAMathLeastSquareBlackDDQlib", __FILE__, __LINE__);
    }

    if( n != mStrike.size() )
    {
        throw LACoreInvalidData("mPrem.size() != mStrike.size() :: LAMathLeastSquareBlackDDQlib::LAMathLeastSquareBlackDDQlib", __FILE__, __LINE__);
    }

    if( m != mStrike[0].size() )
    {
        throw LACoreInvalidData("mPrem[0].size() != mStrike[0].size() :: LAMathLeastSquareBlackDDQlib::LAMathLeastSquareBlackDDQlib", __FILE__, __LINE__);
    }
    
    if( n != mCallPutFlg.size() )
    {
        throw LACoreInvalidData("mPrem.size() != mCallPutFlg.size() :: LAMathLeastSquareBlackDDQlib::LAMathLeastSquareBlackDDQlib", __FILE__, __LINE__);
    }

    if( m != mCallPutFlg[0].size() )
    {
        throw LACoreInvalidData("mPrem[0].size() != mCallPutFlg[0].size() :: LAMathLeastSquareBlackDDQlib::LAMathLeastSquareBlackDDQlib", __FILE__, __LINE__);
    }

    mTarget2Fit.resize(n * m, 0.);
    mFct2Fit.resize(n * m, 0.);
    mWeight.resize(n * m, 0.);

    mFX0.resize(n);
    for(size_t i = 0; i < n; ++i)
    {
        mFX0[i] = s0 * curve0_f.getP(mT[i]) / mCurve0_d->getP(mT[i]);
    }

    size_t k = 0;
    for(size_t i = 0; i < n; ++i)
    {
        for(size_t j = 0; j < m; ++j)
        {
            mWeight[k++] = weight[i][j];
        }
    }

    getTarget2Fit();

    mMktSigma.resize(mT.size());
    mMktSkew.resize(mT.size());
}

/*!
    @brief copy constructor

	@param[in] LAMathLeastSquareBlackDDQlib object

*/
LAMathLeastSquareBlackDDQlib::LAMathLeastSquareBlackDDQlib(const LAMathLeastSquareBlackDDQlib& rhs)
:
mT(rhs.mT),
mPrem(rhs.mPrem),
mWeight(rhs.mWeight),
mStrike(rhs.mStrike),
mTarget2Fit(rhs.mTarget2Fit),
mFct2Fit(rhs.mFct2Fit),
mFX0(rhs.mFX0),
mCurve0_d(rhs.mCurve0_d != 0 ? dynamic_cast<LARatesPathElementCurve*>(rhs.mCurve0_d->clone()) : 0),
mCallPutFlg(rhs.mCallPutFlg),
mCloned(true)
{
}

/*!
    @brief destructor

*/
LAMathLeastSquareBlackDDQlib::~LAMathLeastSquareBlackDDQlib()
{
    if(mCloned)
    {
        delete mCurve0_d;
        mCurve0_d = 0;
    }
}

/*!
    @brief value

	@param[in] x
	@return Real

*/
Real LAMathLeastSquareBlackDDQlib::value(const Array& x) const
{
    getFct2Fit(x);

    double diffSQ = 0.0;
    for(size_t i = 0; i < mFct2Fit.size(); ++i)
    {
        const double diff = mFct2Fit[i] - mTarget2Fit[i];
        diffSQ += diff * diff * mWeight[i];
		//diffSQ += diff * diff * mWeight[i] * mWeight[i];
    }
	//double diff_ = mFct2Fit[2] - mTarget2Fit[2];
	//diffSQ += 20.0 * diff_ * diff_;

	//return LAMath::sqrt(diffSQ);
    return diffSQ;
}

/*!
    @brief values

	@param[in] x
	@return Array

*/
Array LAMathLeastSquareBlackDDQlib::values(const Array& x) const
{
    getFct2Fit(x);

    size_t k = 0;
    //Array diffSQs( x.size() );
	Array diffSQs( mFct2Fit.size() );
    //for(size_t i = 0; i < x.size(); ++i)
	for(size_t i = 0; i < mFct2Fit.size(); ++i)
    {
        const double diff = mFct2Fit[i] - mTarget2Fit[i];
		diffSQs[k++] = diff * LAMath::sqrt(mWeight[i]);
    }
    return diffSQs;
}


/*!
    @brief  compute the target vector and the values of the function to fit

*/
void LAMathLeastSquareBlackDDQlib::getTarget2Fit() const
{
    size_t k = 0;
    for(size_t i = 0; i < mPrem.size(); ++i)
    {
        for(size_t j = 0; j < mPrem[0].size(); ++j)
        {
            mTarget2Fit[k++] = mPrem[i][j];
        }
    }
}

/*!
    @brief  compute the target vector and the values of the function to fit
	
	@param[in] x
*/
void LAMathLeastSquareBlackDDQlib::getFct2Fit(const Array& x) const
{
    if( x.size() != 2 * mT.size() )
    {
        throw LACoreInvalidData("x.size() != 2 * mT.size() : LAMathLeastSquareBlackDDQlib::getFct2Fit", __FILE__, __LINE__);        
    }

    // volatility
    size_t k = 0;
    if (mIsCalibVol || mInit)
    {
        for(size_t i = 0; i < mT.size(); ++i)
        {
            mMktSigma[k++] = x[i];
        }
    }

    k = 0;
    // skew
    if (mIsCalibSkew || mInit)
    {
        for(size_t i = mT.size(); i < x.size(); ++i)
        {
            mMktSkew[k++] = x[i];
        }
    }

    if (!mIsCalibSkew)
    {
        for(size_t i = mT.size(); i < x.size(); ++i)
        {
            mMktSkew[k++] = 1.;
        }
    }

	mInit = false;
    
    k = 0;
    for(size_t i = 0; i < mPrem.size(); ++i)
    {
        double p = mCurve0_d->getP(mT[i]);
        for(size_t j = 0; j < mPrem[0].size(); ++j)
        {
			mFct2Fit[k++] =  p * LAMathAnalyticalFormula::BlackFormulaDD(mFX0[i], mMktSigma[i] * LAMath::sqrt(mT[i]), mStrike[i][j], mCallPutFlg[i][j], mMktSkew[i]);
        }
    }
}

/*!
    @brief  compute the values of black volatility
	
	@param[in] x
*/
DoubleArray LAMathLeastSquareBlackDDQlib::getBlackVol(const DoubleArray& x) const
{
    if( x.size() != 2 * mT.size() )
    {
        throw LACoreInvalidData("x.size() != 2 * mT.size() : LAMathLeastSquareBlackDDQlib::getFct2Fit", __FILE__, __LINE__);        
    }

    // volatility
    size_t k = 0;
	DoubleArray mktSigma(mMktSigma.size()), mktSkew(mMktSkew.size());
    if (mIsCalibVol)
    {
        for(size_t i = 0; i < mT.size(); ++i)
        {
            mktSigma[k++] = x[i];
        }
    }

    k = 0;
    // skew
    if (mIsCalibSkew)
    {
        for(size_t i = mT.size(); i < x.size(); ++i)
        {
            mktSkew[k++] = x[i];
        }
    }

    if (!mIsCalibSkew)
    {
        for(size_t i = mT.size(); i < x.size(); ++i)
        {
            mktSkew[k++] = 1.;
        }
    }
    
	DoubleArray ret;
    for(size_t i = 0; i < mPrem.size(); ++i)
    {
        for(size_t j = 0; j < mPrem[0].size(); ++j)
        {
			double prem = LAMathAnalyticalFormula::BlackFormulaDD(mFX0[i], mktSigma[i] * LAMath::sqrt(mT[i]), mStrike[i][j], mCallPutFlg[i][j], mktSkew[i]);
			ret.push_back( LAMathAnalyticalFormula::BlackImplVolDD( prem, mFX0[i], mStrike[i][j], mCallPutFlg[i][j], 1. ) / LAMath::sqrt(mT[i]) );
        }
    }

	return ret;
}

/*!
    @brief operator

	@param[in] LAMathLeastSquareBlackDDQlib object

*/
LAMathLeastSquareBlackDDQlib& LAMathLeastSquareBlackDDQlib::operator =(const LAMathLeastSquareBlackDDQlib& rhs)
{
    if ( this != &rhs )
    {
        mCloned = true;

        mTarget2Fit = rhs.mTarget2Fit;        
        mFct2Fit = rhs.mFct2Fit;

        mT = rhs.mT;
        mPrem = rhs.mPrem;
        mStrike = rhs.mStrike;
        mCallPutFlg = rhs.mCallPutFlg;
        mWeight = rhs.mWeight;
        
        mFX0 = rhs.mFX0;
        mCurve0_d = rhs.mCurve0_d != 0 ? dynamic_cast<LARatesPathElementCurve*>(rhs.mCurve0_d->clone()) : 0;
    }
    return *this;
}

/*!
    @brief constructor

	@param[in] s0
	@param[in] curve0_d
	@param[in] curve0_f
	@param[in] t
	@param[in] strike
	@param[in] prem
	@param[in] weight
	@param[in] callPutFlg

*/
LAMathLeastSquareBlackDDVolQlib::LAMathLeastSquareBlackDDVolQlib(double s0,
                                                 LARatesPathElementCurve& curve0_d,
                                                 LARatesPathElementCurve& curve0_f,
                                                 const DoubleArray& t,
                                                 const DoubleMatrix& strike,
                                                 const DoubleMatrix& prem,
                                                 const DoubleMatrix& weight,
                                                 const vector<IntVector>& callPutFlg
                                                )
:
LAMathLeastSquareBlackDDQlib(s0, curve0_d, curve0_f, t, strike,prem, weight, callPutFlg, true, false),
mCloned(false)
{}

/*!
    @brief copy constructor

	@param[in] LAMathLeastSquareBlackDDVolQlib object

*/
LAMathLeastSquareBlackDDVolQlib::LAMathLeastSquareBlackDDVolQlib(const LAMathLeastSquareBlackDDVolQlib& rhs)
:
LAMathLeastSquareBlackDDQlib(rhs),
mCloned(true)
{
}

/*!
    @brief destructor

*/
LAMathLeastSquareBlackDDVolQlib::~LAMathLeastSquareBlackDDVolQlib()
{
    if(mCloned)
    {
    }
}
 
/*!
    @brief compute the target vector and the values of the function to fit

	@param[in] x

*/
void LAMathLeastSquareBlackDDVolQlib::getFct2Fit(const Array& x) const
{
    if (x.size() != mT.size())
    {
        throw LACoreInvalidData("x.size() != mT.size() : LAMathLeastSquareBlackDDQlib::Get_fct2fit", __FILE__, __LINE__);        
    }

    // volatility
    size_t k = 0;
    for(size_t i = 0; i < mT.size(); ++i)
    {
        mMktSigma[k++] = x[i];
    }
    
    k = 0;
    for(size_t i = 0; i < mPrem.size(); ++i)
    {
        double p = mCurve0_d->getP(mT[i]);
        for(size_t j = 0; j < mPrem[0].size(); ++j)
        {
			mFct2Fit[k++] =  p * LAMathAnalyticalFormula::BlackFormulaDD(mFX0[i], mMktSigma[i] * LAMath::sqrt(mT[i]), mStrike[i][j], mCallPutFlg[i][j], 1.);
        }
    }
}

/*!
    @brief operator

	@param[in] LAMathLeastSquareBlackDDVolQlib object

*/
LAMathLeastSquareBlackDDVolQlib& LAMathLeastSquareBlackDDVolQlib::operator =(const LAMathLeastSquareBlackDDVolQlib& rhs)
{
    if ( this != &rhs )
    {
        mCloned = true;
        (*this).LAMathLeastSquareBlackDDQlib::operator =(rhs);
    }
    return *this;
}

/*!
    @brief constructor

	@param[in] s0
	@param[in] curve0_d
	@param[in] curve0_f
	@param[in] t
	@param[in] strike
	@param[in] prem
	@param[in] weight
	@param[in] impVol
	@param[in] callPutFlg

*/
LAMathLeastSquareBlackDDSkewQlib::LAMathLeastSquareBlackDDSkewQlib(double s0,
                                                   LARatesPathElementCurve& curve0_d,
                                                   LARatesPathElementCurve& curve0_f,
                                                   const DoubleArray& t,
                                                   const DoubleMatrix& strike,
                                                   const DoubleMatrix& prem,
                                                   const DoubleMatrix& weight,
                                                   const DoubleArray& impVol,
                                                   const vector<IntVector>& callPutFlg
                                                  )
:
LAMathLeastSquareBlackDDQlib(s0,curve0_d, curve0_f, t, strike,prem, weight, callPutFlg, false, true),
mCloned(false)
{
    if(impVol.size() != t.size())
    {
        throw LACoreInvalidData("impVol.size() != t.size() :  LAMathLeastSquareBlackDDSkewQlib::LAMathLeastSquareBlackDDSkewQlib", __FILE__, __LINE__);
    }
    mMktSigma = impVol;
}

/*!
    @brief copy constructor

	@param[in] LAMathLeastSquareBlackDDSkewQlib object

*/
LAMathLeastSquareBlackDDSkewQlib::LAMathLeastSquareBlackDDSkewQlib(const LAMathLeastSquareBlackDDSkewQlib& rhs)
:
LAMathLeastSquareBlackDDQlib(rhs),
mCloned(true)
{
}

/*!
    @brief destructor

*/
LAMathLeastSquareBlackDDSkewQlib::~LAMathLeastSquareBlackDDSkewQlib()
{
    if(mCloned)
    {
    }
}


/*!
    @brief compute the target vector and the values of the function to fit

	@param[in] x

*/
void LAMathLeastSquareBlackDDSkewQlib::getFct2Fit(const Array& x) const
{
    if (x.size() != mT.size())
    {
        throw LACoreInvalidData("x.size() != mT.size()  : LAMathLeastSquareBlackDDQlib::Get_fct2fit", __FILE__, __LINE__);        
    }

    // volatility
    size_t k = 0;
    for(size_t i = 0; i < mT.size(); ++i)
    {
        mMktSkew[k++] = x[i];
    }

    k = 0;
    for(size_t i = 0; i < mPrem.size(); ++i)
    {
        double p = mCurve0_d->getP(mT[i]);
        for(size_t j = 0; j < mPrem[0].size(); ++j)
        {
			mFct2Fit[k++] =  p * LAMathAnalyticalFormula::BlackFormulaDD(mFX0[i], mMktSigma[i] * LAMath::sqrt(mT[i]), mStrike[i][j], mCallPutFlg[i][j], mMktSkew[i]);
        }
    }
}

/*!
    @brief set implied volatility

	@param[in] impVol

*/
void LAMathLeastSquareBlackDDSkewQlib::setImpVol(const DoubleArray& impVol)
{
    if( mT.size() != impVol.size() )
    {
        throw LACoreInvalidData("mT.size() != impVol.size() : LAMathLeastSquareBlackDDSkewQlib::setImpVol", __FILE__, __LINE__);
    }

    mMktSigma = impVol;
}

/*!
    @brief operator

	@param[in] LAMathLeastSquareBlackDDSkewQlib object

*/
LAMathLeastSquareBlackDDSkewQlib& LAMathLeastSquareBlackDDSkewQlib::operator =(const LAMathLeastSquareBlackDDSkewQlib& rhs)
{
    if ( this != &rhs )
    {
        mCloned = true;

        (*this).LAMathLeastSquareBlackDDQlib::operator =(rhs);
    }
    return *this;
}