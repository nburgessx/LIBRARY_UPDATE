/*! @file LAMathLeastSquareCorrelationLMMQlib.cpp
  	@brief Cost function for least-square problems with QuantLib
		Implements a cost function using the interface provided by
		the LeastSquareProblem class.
 */

#include "LAMathLeastSquareCorrelationLMMQlib.h"

LAMathLeastSquareCorrelationLMMQlib::LAMathLeastSquareCorrelationLMMQlib(LAMathCorrelationLMMDiscAngle& cor_)
:
mCorTarget(cor_.getFullRankCorr(0.0)),
mn(mCorTarget.size()),
mCor(&cor_),
mNoOfFactors(mCor->getNoFactors()),
mfct2fit(mn, DoubleVector(mn, 0.0)),
mCloned(false)
{
    if( mn != mCorTarget[0].size() ) throw LACoreInvalidData("Target Cor is not correct! : LAMathLeastSquareCorrelationLMMQlib::targetAndValue", __FILE__, __LINE__);
}

LAMathLeastSquareCorrelationLMMQlib::LAMathLeastSquareCorrelationLMMQlib( const LAMathLeastSquareCorrelationLMMQlib& rhs )
:
mCorTarget(rhs.mCorTarget),
mn(rhs.mn),
mCor(rhs.mCor != 0 ? dynamic_cast<LAMathCorrelationLMMDiscAngle*>(rhs.mCor->clone()) : 0),
mNoOfFactors(rhs.mNoOfFactors),
mfct2fit(rhs.mfct2fit),
mCloned(true)
{
}

LAMathLeastSquareCorrelationLMMQlib::~LAMathLeastSquareCorrelationLMMQlib()
{
    if(mCloned)
    {
        delete mCor;
        mCor = 0;
    }
}

double LAMathLeastSquareCorrelationLMMQlib::value(const QuantLib::Array& x) const
{
    getfct2fit(x);

    double diff_SQ = 0.0;
    double diff;
    for(size_t i = 0; i < mn; ++i)
    {
        for(size_t j = 0; j < mn; ++j)
        {
            diff = mCorTarget[i][j] - mfct2fit[i][j];
            diff_SQ += diff * diff;
        }
    }

    return  diff_SQ;

}

QuantLib::Array LAMathLeastSquareCorrelationLMMQlib::values(const QuantLib::Array& x) const
{

    getfct2fit(x);

    size_t k = 0;
    QuantLib::Array diff_SQs( mn * mn );
    double diff;
    for(size_t i = 0; i < mn; ++i)
    {
        for(size_t j = 0; j < mn; ++j)
        {
            diff = mfct2fit[i][j] - mCorTarget[i][j];
//            diff_SQs[k++] = diff * diff;
            diff_SQs[k++] = diff;
        }
    }

    return diff_SQs;
}

// compute the target vector and the values of the function to fit
void LAMathLeastSquareCorrelationLMMQlib::getfct2fit(const QuantLib::Array& x) const
{
    DoubleMatrix theta_( mn, DoubleVector(mNoOfFactors - 1, 0.0) );
    size_t k = 0;
    for(size_t i = 0; i < mn; ++i )
    {
        for(size_t j = 0; j < mNoOfFactors - 1; ++j )
        {
            theta_[i][j] = x[k++];
        }
    }
    mCor->setTheta(theta_);

    for(size_t i = 0; i < mn; ++i)
    {            
        for(size_t j = 0; j < mn; ++j)
        {
            mfct2fit[i][j] = mCor->get(0.0, i, j);
        }
    }
}

void LAMathLeastSquareCorrelationLMMQlib::gradient(QuantLib::Array& grad_f, const QuantLib::Array& x) const
{
    //
    DoubleMatrix grad_fct2fit( mn, DoubleVector(mNoOfFactors - 1, 0.0) );
      
    //
    DoubleMatrix theta_( mn, DoubleVector(mNoOfFactors - 1, 0.0) );
    size_t k = 0;
    for(size_t i = 0; i < mn; ++i )
    {
        for(size_t j = 0; j < mNoOfFactors - 1; ++j )
        {
            theta_[i][j] = x[k++];
        }
    }
    mCor->setTheta(theta_);

    //
    DoubleMatrix factor_loading = mCor->getFactorLoading();
   
    for(size_t p = 0; p < mn; ++p)
    {
        for(size_t q = 0; q < mNoOfFactors - 1; ++q)
        {
            grad_fct2fit[p][q] = 0.0;
            for(size_t i = 1; i < mn; ++i)
            {
                for(size_t j = 0; j < i; ++j)
                {
                    if(i != p && j != p) continue;
                    double tmp = 0.0;
                    for(size_t k = q; k < mNoOfFactors; ++k)
                    {
                        double d_Bpk = 1.0;
                        for(size_t l = 0; l < k; ++l)
                        {
                            if(l != q) d_Bpk *= sin(theta_[p][l]);
                        }    
                        
                        if(k == q) 
                        {
                            d_Bpk *= -sin(theta_[p][k]);
                        }
                        else
                        {
                            d_Bpk *= cos(theta_[p][q]);
                            if(k < mNoOfFactors - 1) d_Bpk *= cos(theta_[p][k]);
                        }
                        tmp += d_Bpk * ( i == p ? factor_loading[j][k] : factor_loading[i][k]  );
                    }
                    grad_fct2fit[p][q] += -4.0 * tmp * ( mCorTarget[i][j] - mCor->get(0.0, i, j) );
                }
            }
        }
    }

    k = 0;
    for(size_t i = 0; i < mn; ++i)
    {
        for(size_t j = 0; j < mNoOfFactors - 1; ++j)
        {
            grad_f[k++] = grad_fct2fit[i][j];
        }
    }

}

LAMathLeastSquareCorrelationLMMQlib& LAMathLeastSquareCorrelationLMMQlib::operator =(const LAMathLeastSquareCorrelationLMMQlib& rhs)
{
    if ( this != &rhs )
    {
        delete mCor;
        mCor = rhs.mCor != 0 ? dynamic_cast<LAMathCorrelationLMMDiscAngle*>(rhs.mCor->clone()) : 0;
        mCloned = true;
        
        mCorTarget = rhs.mCorTarget;
        mn = rhs.mn;
        mNoOfFactors = rhs.mNoOfFactors;

        mfct2fit = rhs.mfct2fit;
    }
    return *this;
}

