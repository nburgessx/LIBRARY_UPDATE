// Cost function for least-square problems
/*! Implements a cost function using the interface provided by
    the LeastSquareProblem class.
*/

#include "AQLMathLeastSquareBlackDDQlib.h"
#include "AQLBasic.h"
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
AQLMathLeastSquareBlackDDQlib::AQLMathLeastSquareBlackDDQlib(double s0,
                                         AQLRatesPathElementCurve& curve0_d,
                                         AQLRatesPathElementCurve& curve0_f,
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
        throw AQLCoreInvalidData("mT.size() != mPrem.size() :: AQLMathLeastSquareBlackDDQlib::AQLMathLeastSquareBlackDDQlib", __FILE__, __LINE__);
    }

    if( n != mStrike.size() )
    {
        throw AQLCoreInvalidData("mPrem.size() != mStrike.size() :: AQLMathLeastSquareBlackDDQlib::AQLMathLeastSquareBlackDDQlib", __FILE__, __LINE__);
    }

    if( m != mStrike[0].size() )
    {
        throw AQLCoreInvalidData("mPrem[0].size() != mStrike[0].size() :: AQLMathLeastSquareBlackDDQlib::AQLMathLeastSquareBlackDDQlib", __FILE__, __LINE__);
    }
    
    if( n != mCallPutFlg.size() )
    {
        throw AQLCoreInvalidData("mPrem.size() != mCallPutFlg.size() :: AQLMathLeastSquareBlackDDQlib::AQLMathLeastSquareBlackDDQlib", __FILE__, __LINE__);
    }

    if( m != mCallPutFlg[0].size() )
    {
        throw AQLCoreInvalidData("mPrem[0].size() != mCallPutFlg[0].size() :: AQLMathLeastSquareBlackDDQlib::AQLMathLeastSquareBlackDDQlib", __FILE__, __LINE__);
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

	@param[in] AQLMathLeastSquareBlackDDQlib object

*/
AQLMathLeastSquareBlackDDQlib::AQLMathLeastSquareBlackDDQlib(const AQLMathLeastSquareBlackDDQlib& rhs)
:
mT(rhs.mT),
mPrem(rhs.mPrem),
mWeight(rhs.mWeight),
mStrike(rhs.mStrike),
mTarget2Fit(rhs.mTarget2Fit),
mFct2Fit(rhs.mFct2Fit),
mFX0(rhs.mFX0),
mCurve0_d(rhs.mCurve0_d != 0 ? dynamic_cast<AQLRatesPathElementCurve*>(rhs.mCurve0_d->clone()) : 0),
mCallPutFlg(rhs.mCallPutFlg),
mCloned(true)
{
}

/*!
    @brief destructor

*/
AQLMathLeastSquareBlackDDQlib::~AQLMathLeastSquareBlackDDQlib()
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
Real AQLMathLeastSquareBlackDDQlib::value(const Array& x) const
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

	//return AQLMath::sqrt(diffSQ);
    return diffSQ;
}

/*!
    @brief values

	@param[in] x
	@return Array

*/
Array AQLMathLeastSquareBlackDDQlib::values(const Array& x) const
{
    getFct2Fit(x);

    size_t k = 0;
    //Array diffSQs( x.size() );
	Array diffSQs( mFct2Fit.size() );
    //for(size_t i = 0; i < x.size(); ++i)
	for(size_t i = 0; i < mFct2Fit.size(); ++i)
    {
        const double diff = mFct2Fit[i] - mTarget2Fit[i];
		diffSQs[k++] = diff * AQLMath::sqrt(mWeight[i]);
    }
    return diffSQs;
}


/*!
    @brief  compute the target vector and the values of the function to fit

*/
void AQLMathLeastSquareBlackDDQlib::getTarget2Fit() const
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
void AQLMathLeastSquareBlackDDQlib::getFct2Fit(const Array& x) const
{
    if( x.size() != 2 * mT.size() )
    {
        throw AQLCoreInvalidData("x.size() != 2 * mT.size() : AQLMathLeastSquareBlackDDQlib::getFct2Fit", __FILE__, __LINE__);        
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
			mFct2Fit[k++] =  p * AQLMathAnalyticalFormula::BlackFormulaDD(mFX0[i], mMktSigma[i] * AQLMath::sqrt(mT[i]), mStrike[i][j], mCallPutFlg[i][j], mMktSkew[i]);
        }
    }
}

/*!
    @brief  compute the values of black volatility
	
	@param[in] x
*/
DoubleArray AQLMathLeastSquareBlackDDQlib::getBlackVol(const DoubleArray& x) const
{
    if( x.size() != 2 * mT.size() )
    {
        throw AQLCoreInvalidData("x.size() != 2 * mT.size() : AQLMathLeastSquareBlackDDQlib::getFct2Fit", __FILE__, __LINE__);        
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
			double prem = AQLMathAnalyticalFormula::BlackFormulaDD(mFX0[i], mktSigma[i] * AQLMath::sqrt(mT[i]), mStrike[i][j], mCallPutFlg[i][j], mktSkew[i]);
			ret.push_back( AQLMathAnalyticalFormula::BlackImplVolDD( prem, mFX0[i], mStrike[i][j], mCallPutFlg[i][j], 1. ) / AQLMath::sqrt(mT[i]) );
        }
    }

	return ret;
}

/*!
    @brief operator

	@param[in] AQLMathLeastSquareBlackDDQlib object

*/
AQLMathLeastSquareBlackDDQlib& AQLMathLeastSquareBlackDDQlib::operator =(const AQLMathLeastSquareBlackDDQlib& rhs)
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
        mCurve0_d = rhs.mCurve0_d != 0 ? dynamic_cast<AQLRatesPathElementCurve*>(rhs.mCurve0_d->clone()) : 0;
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
AQLMathLeastSquareBlackDDVolQlib::AQLMathLeastSquareBlackDDVolQlib(double s0,
                                                 AQLRatesPathElementCurve& curve0_d,
                                                 AQLRatesPathElementCurve& curve0_f,
                                                 const DoubleArray& t,
                                                 const DoubleMatrix& strike,
                                                 const DoubleMatrix& prem,
                                                 const DoubleMatrix& weight,
                                                 const vector<IntVector>& callPutFlg
                                                )
:
AQLMathLeastSquareBlackDDQlib(s0, curve0_d, curve0_f, t, strike,prem, weight, callPutFlg, true, false),
mCloned(false)
{}

/*!
    @brief copy constructor

	@param[in] AQLMathLeastSquareBlackDDVolQlib object

*/
AQLMathLeastSquareBlackDDVolQlib::AQLMathLeastSquareBlackDDVolQlib(const AQLMathLeastSquareBlackDDVolQlib& rhs)
:
AQLMathLeastSquareBlackDDQlib(rhs),
mCloned(true)
{
}

/*!
    @brief destructor

*/
AQLMathLeastSquareBlackDDVolQlib::~AQLMathLeastSquareBlackDDVolQlib()
{
    if(mCloned)
    {
    }
}
 
/*!
    @brief compute the target vector and the values of the function to fit

	@param[in] x

*/
void AQLMathLeastSquareBlackDDVolQlib::getFct2Fit(const Array& x) const
{
    if (x.size() != mT.size())
    {
        throw AQLCoreInvalidData("x.size() != mT.size() : AQLMathLeastSquareBlackDDQlib::Get_fct2fit", __FILE__, __LINE__);        
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
			mFct2Fit[k++] =  p * AQLMathAnalyticalFormula::BlackFormulaDD(mFX0[i], mMktSigma[i] * AQLMath::sqrt(mT[i]), mStrike[i][j], mCallPutFlg[i][j], 1.);
        }
    }
}

/*!
    @brief operator

	@param[in] AQLMathLeastSquareBlackDDVolQlib object

*/
AQLMathLeastSquareBlackDDVolQlib& AQLMathLeastSquareBlackDDVolQlib::operator =(const AQLMathLeastSquareBlackDDVolQlib& rhs)
{
    if ( this != &rhs )
    {
        mCloned = true;
        (*this).AQLMathLeastSquareBlackDDQlib::operator =(rhs);
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
AQLMathLeastSquareBlackDDSkewQlib::AQLMathLeastSquareBlackDDSkewQlib(double s0,
                                                   AQLRatesPathElementCurve& curve0_d,
                                                   AQLRatesPathElementCurve& curve0_f,
                                                   const DoubleArray& t,
                                                   const DoubleMatrix& strike,
                                                   const DoubleMatrix& prem,
                                                   const DoubleMatrix& weight,
                                                   const DoubleArray& impVol,
                                                   const vector<IntVector>& callPutFlg
                                                  )
:
AQLMathLeastSquareBlackDDQlib(s0,curve0_d, curve0_f, t, strike,prem, weight, callPutFlg, false, true),
mCloned(false)
{
    if(impVol.size() != t.size())
    {
        throw AQLCoreInvalidData("impVol.size() != t.size() :  AQLMathLeastSquareBlackDDSkewQlib::AQLMathLeastSquareBlackDDSkewQlib", __FILE__, __LINE__);
    }
    mMktSigma = impVol;
}

/*!
    @brief copy constructor

	@param[in] AQLMathLeastSquareBlackDDSkewQlib object

*/
AQLMathLeastSquareBlackDDSkewQlib::AQLMathLeastSquareBlackDDSkewQlib(const AQLMathLeastSquareBlackDDSkewQlib& rhs)
:
AQLMathLeastSquareBlackDDQlib(rhs),
mCloned(true)
{
}

/*!
    @brief destructor

*/
AQLMathLeastSquareBlackDDSkewQlib::~AQLMathLeastSquareBlackDDSkewQlib()
{
    if(mCloned)
    {
    }
}


/*!
    @brief compute the target vector and the values of the function to fit

	@param[in] x

*/
void AQLMathLeastSquareBlackDDSkewQlib::getFct2Fit(const Array& x) const
{
    if (x.size() != mT.size())
    {
        throw AQLCoreInvalidData("x.size() != mT.size()  : AQLMathLeastSquareBlackDDQlib::Get_fct2fit", __FILE__, __LINE__);        
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
			mFct2Fit[k++] =  p * AQLMathAnalyticalFormula::BlackFormulaDD(mFX0[i], mMktSigma[i] * AQLMath::sqrt(mT[i]), mStrike[i][j], mCallPutFlg[i][j], mMktSkew[i]);
        }
    }
}

/*!
    @brief set implied volatility

	@param[in] impVol

*/
void AQLMathLeastSquareBlackDDSkewQlib::setImpVol(const DoubleArray& impVol)
{
    if( mT.size() != impVol.size() )
    {
        throw AQLCoreInvalidData("mT.size() != impVol.size() : AQLMathLeastSquareBlackDDSkewQlib::setImpVol", __FILE__, __LINE__);
    }

    mMktSigma = impVol;
}

/*!
    @brief operator

	@param[in] AQLMathLeastSquareBlackDDSkewQlib object

*/
AQLMathLeastSquareBlackDDSkewQlib& AQLMathLeastSquareBlackDDSkewQlib::operator =(const AQLMathLeastSquareBlackDDSkewQlib& rhs)
{
    if ( this != &rhs )
    {
        mCloned = true;

        (*this).AQLMathLeastSquareBlackDDQlib::operator =(rhs);
    }
    return *this;
}