/*!
    @brief Implements Correlation and its related classes.
*/

#include "AQLMathCorrelationLMMDisc.h"
#include "AQLMathCorrelationFuncLMM.h"
#include "AQLModelUtilities.h"
#include "AQLEigenSystems.h"

AQLMathCorrelationLMMDisc::AQLMathCorrelationLMMDisc()
: AQLMathCorrelationLMM()
{
}

AQLMathCorrelationLMMDisc::AQLMathCorrelationLMMDisc(AQLMathCorrelationFuncLMM*	corr,
											   const DoubleVector&		T_fix,
											   size_t					no_factors
		)
: AQLMathCorrelationLMM( corr, T_fix, no_factors ),
mFactorLoading( AQLEigenSystems::corre2factorloading( corr->getCorrMat( 0, T_fix ), no_factors ) ),
mCorrMat( AQLEigenSystems::factorloading2corre( mFactorLoading ) )
{
}

AQLMathCorrelationLMMDisc::AQLMathCorrelationLMMDisc(const DoubleMatrix&	corr_mat_,
											   const DoubleVector&	T_fix,
											   size_t				no_factors,
											   bool					full_mat
		)
: AQLMathCorrelationLMM( T_fix, no_factors )
{
    if (full_mat)
    {
        if (T_fix.size() != corr_mat_.size())
        {
            throw AQLCoreInvalidData("full-length corr_mat is incompatible with T_fix : AQLMathCorrelationLMMDisc::AQLMathCorrelationLMMDisc", __FILE__, __LINE__);
        }
        AQLModelUtilities::takeColRow(corr_mat_, mCorrMat, -static_cast<int>(T_fix.size()) + 1, -static_cast<int>(T_fix.size()) + 1 );
		mFactorLoading = AQLEigenSystems::corre2factorloading( mCorrMat, no_factors);
        mCorrMat = AQLEigenSystems::factorloading2corre( mFactorLoading );

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
            throw AQLCoreInvalidData("reduced-length corr_mat is incompatible with T_fix : AQLMathCorrelationLMMDisc::AQLMathCorrelationLMMDisc", __FILE__, __LINE__);
        }
        mFactorLoading = AQLEigenSystems::corre2factorloading( corr_mat_, no_factors );
        mCorrMat = AQLEigenSystems::factorloading2corre( mFactorLoading );
        for (size_t i = 0; i < mCorrMat.size(); i++ )
        {
            mCorrMat[i].insert(mCorrMat[i].begin(),0);
        }
        mCorrMat.insert(mCorrMat.begin(), DoubleVector(mCorrMat[0].size(), 0) );
        mCorrMat[0][0] = 1;
        mFactorLoading.insert(mFactorLoading.begin(), DoubleVector(no_factors, 0) );
    }
}

AQLMathCorrelationLMMDisc::AQLMathCorrelationLMMDisc(const DoubleMatrix&	factor_loading_,
											   const DoubleVector&	T_fix,
											   bool					full_mat
		)
: AQLMathCorrelationLMM( T_fix, factor_loading_[0].size() ),
mFactorLoading( factor_loading_ )
{
    if (full_mat)
    {
        if (T_fix.size() != mFactorLoading.size())
        {
            throw AQLCoreInvalidData("full-length factor loading is incompatible with T_fix : AQLMathCorrelationLMMDisc::AQLMathCorrelationLMMDisc", __FILE__, __LINE__);
        } 
        DoubleMatrix factor_loading_temp( mFactorLoading.begin() + 1, mFactorLoading.end() );
        mCorrMat = AQLEigenSystems::factorloading2corre( factor_loading_temp );
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
            throw AQLCoreInvalidData("reduced-length factors loading is incompatible with T_fix : AQLMathCorrelationLMMDisc::AQLMathCorrelationLMMDisc", __FILE__, __LINE__);
        }
        mCorrMat = AQLEigenSystems::factorloading2corre( mFactorLoading );
        for (size_t i = 0; i < mCorrMat.size(); i++ )
        {
            mCorrMat[i].insert(mCorrMat[i].begin(),0);
        }
        mCorrMat.insert(mCorrMat.begin(), DoubleVector(mCorrMat[0].size(), 0) );
        mCorrMat[0][0] = 1;
        mFactorLoading.insert(mFactorLoading.begin(), DoubleVector(mNoFactors, 0) );
    }
}

AQLMathCorrelationLMMDisc::AQLMathCorrelationLMMDisc(const DoubleMatrix& factor_loading_,
											   const DoubleVector& T_fix
                             )
: AQLMathCorrelationLMM( T_fix, factor_loading_[0].size() ),
mFactorLoading( factor_loading_ )
{
    if (T_fix.size() != mFactorLoading.size())
    {
        throw AQLCoreInvalidData("full-length factor loading is incompatible with T_fix : AQLMathCorrelationLMMDisc::AQLMathCorrelationLMMDisc", __FILE__, __LINE__);
    } 
    mCorrMat = AQLEigenSystems::factorloading2corre( mFactorLoading );
}

AQLMathCorrelationLMMDisc::AQLMathCorrelationLMMDisc(	const AQLMathCorrelationLMMDisc& rhs )
: AQLMathCorrelationLMM( rhs ),
mFactorLoading( rhs.mFactorLoading ),
mCorrMat( rhs.mCorrMat )
{
}

double AQLMathCorrelationLMMDisc::get(double t, size_t p, size_t q)
{
    if ( mNoFactors == 1 || p == q) return 1;

    return get( AQLModelUtilities::getIndex(t, mT_fix), p, q );
}

DoubleVector AQLMathCorrelationLMMDisc::getFactors(double t, size_t p)
{
    if ( mNoFactors == 1 ) return mUnity;

    return getFactors( AQLModelUtilities::getIndex(t, mT_fix), p );
}

double AQLMathCorrelationLMMDisc::get(size_t s, size_t p, size_t q)
{
    if ( mNoFactors == 1 || p == q) return 1;

    if( p < s )
    {
        throw AQLCoreInvalidData("p < s : AQLMathCorrelationLMMDisc::get", __FILE__, __LINE__);
    }
    if( q < s )
    {
        throw AQLCoreInvalidData("q < s : AQLMathCorrelationLMMDisc::get", __FILE__, __LINE__);
    }
    return mCorrMat[p - s][q - s];
}

DoubleVector AQLMathCorrelationLMMDisc::getFactors(size_t s, size_t p)
{
    if ( mNoFactors == 1 ) return mUnity;
    if( p < s )
    {
		AQLString msg = "p < s : ( p = " + AQLString(AQLModelUtilities::n2s(p).c_str()) + ", s = " + AQLString(AQLModelUtilities::n2s(s).c_str()) + " ) : AQLMathCorrelationLMMDisc::getFactors";
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
    }
    return mFactorLoading[p - s];
}

