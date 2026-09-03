/*!
    @file LAMathCorrelationLMMDisc.cpp
    @brief Implements Correlation and its related classes.
*/

#include "LAMathCorrelationLMMDisc.h"
#include "LAMathCorrelationFuncLMM.h"
#include "LAModelUtilities.h"
#include "LAEigenSystems.h"

LAMathCorrelationLMMDisc::LAMathCorrelationLMMDisc()
: LAMathCorrelationLMM()
{
}

//
LAMathCorrelationLMMDisc::LAMathCorrelationLMMDisc(LAMathCorrelationFuncLMM*	corr,
											   const DoubleVector&		T_fix,
											   size_t					no_factors
		)
: LAMathCorrelationLMM( corr, T_fix, no_factors ),
mFactorLoading( LAEigenSystems::corre2factorloading( corr->getCorrMat( 0, T_fix ), no_factors ) ),
mCorrMat( LAEigenSystems::factorloading2corre( mFactorLoading ) )
{
}

//
LAMathCorrelationLMMDisc::LAMathCorrelationLMMDisc(const DoubleMatrix&	corr_mat_,
											   const DoubleVector&	T_fix,
											   size_t				no_factors,
											   bool					full_mat
		)
: LAMathCorrelationLMM( T_fix, no_factors )
{
    if (full_mat)
    {
        if (T_fix.size() != corr_mat_.size())
        {
            throw AQLCoreInvalidData("full-length corr_mat is incompatible with T_fix : LAMathCorrelationLMMDisc::LAMathCorrelationLMMDisc", __FILE__, __LINE__);
        }
        LAModelUtilities::takeColRow(corr_mat_, mCorrMat, -static_cast<int>(T_fix.size()) + 1, -static_cast<int>(T_fix.size()) + 1 );
		mFactorLoading = LAEigenSystems::corre2factorloading( mCorrMat, no_factors);
        mCorrMat = LAEigenSystems::factorloading2corre( mFactorLoading );

        for (size_t i = 0; i < mCorrMat.size(); i++ )
        {
            mCorrMat[i].insert(mCorrMat[i].begin(),0);
        }
        mCorrMat.insert(mCorrMat.begin(), corr_mat_.front() );
        mFactorLoading.insert(mFactorLoading.begin(), DoubleVector(no_factors, 0));
    }
    else
    {
        if (T_fix.size() != corr_mat_.size() + 1)
        {
            throw AQLCoreInvalidData("reduced-length corr_mat is incompatible with T_fix : LAMathCorrelationLMMDisc::LAMathCorrelationLMMDisc", __FILE__, __LINE__);
        }
        mFactorLoading = LAEigenSystems::corre2factorloading( corr_mat_, no_factors );
        mCorrMat = LAEigenSystems::factorloading2corre( mFactorLoading );
        for (size_t i = 0; i < mCorrMat.size(); i++ )
        {
            mCorrMat[i].insert(mCorrMat[i].begin(),0);
        }
        mCorrMat.insert(mCorrMat.begin(), DoubleVector(mCorrMat[0].size(), 0) );
        mCorrMat[0][0] = 1;
        mFactorLoading.insert(mFactorLoading.begin(), DoubleVector(no_factors, 0) );
    }
}

//
LAMathCorrelationLMMDisc::LAMathCorrelationLMMDisc(const DoubleMatrix&	factor_loading_,
											   const DoubleVector&	T_fix,
											   bool					full_mat
		)
: LAMathCorrelationLMM( T_fix, factor_loading_[0].size() ),
mFactorLoading( factor_loading_ )
{
    if (full_mat)
    {
        if (T_fix.size() != mFactorLoading.size())
        {
            throw AQLCoreInvalidData("full-length factor loading is incompatible with T_fix : LAMathCorrelationLMMDisc::LAMathCorrelationLMMDisc", __FILE__, __LINE__);
        } 
        DoubleMatrix factor_loading_temp( mFactorLoading.begin() + 1, mFactorLoading.end() );
        mCorrMat = LAEigenSystems::factorloading2corre( factor_loading_temp );
        for (size_t i = 0; i < mCorrMat.size(); i++ )
        {
            mCorrMat[i].insert(mCorrMat[i].begin(),0);
        }
        mCorrMat.insert(mCorrMat.begin(), DoubleVector(mCorrMat[0].size(), 0) );
        mCorrMat[0][0] = 1;
    }
    else
    {
        if (T_fix.size() != mFactorLoading.size() + 1)
        {
            throw AQLCoreInvalidData("reduced-length factors loading is incompatible with T_fix : LAMathCorrelationLMMDisc::LAMathCorrelationLMMDisc", __FILE__, __LINE__);
        }
        mCorrMat = LAEigenSystems::factorloading2corre( mFactorLoading );
        for (size_t i = 0; i < mCorrMat.size(); i++ )
        {
            mCorrMat[i].insert(mCorrMat[i].begin(),0);
        }
        mCorrMat.insert(mCorrMat.begin(), DoubleVector(mCorrMat[0].size(), 0) );
        mCorrMat[0][0] = 1;
        mFactorLoading.insert(mFactorLoading.begin(), DoubleVector(mNoFactors, 0) );
    }
}

//
LAMathCorrelationLMMDisc::LAMathCorrelationLMMDisc(const DoubleMatrix& factor_loading_,
											   const DoubleVector& T_fix
                             )
: LAMathCorrelationLMM( T_fix, factor_loading_[0].size() ),
mFactorLoading( factor_loading_ )
{
    if (T_fix.size() != mFactorLoading.size())
    {
        throw AQLCoreInvalidData("full-length factor loading is incompatible with T_fix : LAMathCorrelationLMMDisc::LAMathCorrelationLMMDisc", __FILE__, __LINE__);
    } 
    mCorrMat = LAEigenSystems::factorloading2corre( mFactorLoading );
}

//
LAMathCorrelationLMMDisc::LAMathCorrelationLMMDisc(	const LAMathCorrelationLMMDisc& rhs )
: LAMathCorrelationLMM( rhs ),
mFactorLoading( rhs.mFactorLoading ),
mCorrMat( rhs.mCorrMat )
{
}

//
double LAMathCorrelationLMMDisc::get(double t, size_t p, size_t q)
{
    if ( mNoFactors == 1 || p == q) return 1;

    return get( LAModelUtilities::getIndex(t, mT_fix), p, q );
}

//
DoubleVector LAMathCorrelationLMMDisc::getFactors(double t, size_t p)
{
    if ( mNoFactors == 1 ) return mUnity;

    return getFactors( LAModelUtilities::getIndex(t, mT_fix), p );
}

//
double LAMathCorrelationLMMDisc::get(size_t s, size_t p, size_t q)
{
    if ( mNoFactors == 1 || p == q) return 1;

    if( p < s )
    {
        throw AQLCoreInvalidData("p < s : LAMathCorrelationLMMDisc::get", __FILE__, __LINE__);
    }
    if( q < s )
    {
        throw AQLCoreInvalidData("q < s : LAMathCorrelationLMMDisc::get", __FILE__, __LINE__);
    }
    return mCorrMat[p - s][q - s];
}

//
DoubleVector LAMathCorrelationLMMDisc::getFactors(size_t s, size_t p)
{
    if ( mNoFactors == 1 ) return mUnity;
    if( p < s )
    {
		AQLString msg = "p < s : ( p = " + AQLString(LAModelUtilities::n2s(p).c_str()) + ", s = " + AQLString(LAModelUtilities::n2s(s).c_str()) + " ) : LAMathCorrelationLMMDisc::getFactors";
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
    }
    return mFactorLoading[p - s];
}

