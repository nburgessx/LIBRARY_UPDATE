/*!
    @file AQLMathVolatilityLMMDiscModel.cpp
    @brief Implements Volatility and its related classes.
*/

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif

#include "AQLMathVolatilityLMMDiscModel.h"
#include <cmath>
#include <algorithm>
#include "AQLEigenSystems.h"
#include "AQLModelUtilities.h"

//
//---------------------------------------------------------
AQLMathVolatilityLMMDiscModel::AQLMathVolatilityLMMDiscModel(const DoubleVector& paramV_,
													   const DoubleVector& paramF_,
													   const DoubleVector& tenorG_,
													   const DoubleVector& G_,
													   std::shared_ptr<AQLInterpolationBase> interG_,
													   const DoubleVector& T_fix_special_,
													   const DoubleVector& T_fix,
													   size_t num_small_step_
		)
:
AQLMathVolatilityLMMDisc( T_fix, num_small_step_ ),
mT_fix_special( T_fix_special_),
mDividedT_fix_special( DoubleVector( mNumSmallStep * mT_fix_special.size(), 0.0 ) ),
mParamV(paramV_),
mParamF(paramF_),
mTenorG(tenorG_),
mG(G_),
mpInterG(interG_)
{
    mVol_special.clear();
    mVol_special.resize( mT_fix_special.size() );
    mCacheVolMatrix.clear();
    mCacheVolMatrix.resize(mNumSmallStep * mT_fix_special.size(), mVol_special);

    size_t k = 0;
    double intercept;
    double gradient;
    for( size_t i = 0; i < mT_fix_special.size(); ++i )
    {
        intercept = i == 0 ? 0.0 : mT_fix_special[i - 1];
        gradient = ( mT_fix_special[i] - intercept ) / static_cast<double>(mNumSmallStep);
        for(size_t j = 0; j < mNumSmallStep; ++j)
        {
            mDividedT_fix_special[k++] = gradient * static_cast<double>(j + 1) + intercept;
        }
    }

	if (mpInterG.get() != 0)
	{
		mG.clear();
		mG.resize(mT_fix_special.size());
		mpInterG->set(mTenorG, G_);
		for ( size_t i = 0; i < mT_fix_special.size(); i++ )
		{
			mG[i] = mpInterG->value(mT_fix_special[i]);
		}
	}

    for ( size_t i = 0; i < mT_fix_special.size(); i++ )
    {
        createCacheVolMatrix(i);
    }

    createCacheVolIntMatrix();
}

AQLMathVolatilityLMMDiscModel::AQLMathVolatilityLMMDiscModel( const AQLMathVolatilityLMMDiscModel& rhs )
:
AQLMathVolatilityLMMDisc(rhs),
mCacheVolMatrix(rhs.mCacheVolMatrix),
mVol_special(rhs.mVol_special),
mT_fix_special(rhs.mT_fix_special),
mDividedT_fix_special(rhs.mDividedT_fix_special),
mParamV(rhs.mParamV),
mParamF(rhs.mParamF),
mTenorG(rhs.mTenorG),
mG(rhs.mG)
{
}

double AQLMathVolatilityLMMDiscModel::get(double t, size_t i)
{
    if ( mT_fix.size() <= i ) return 0.0;
    if ( t < 0 || mT_fix[i] < t ) return 0.0;
    if ( t < mDividedT_fix.front() ) return mVol_special[i];

    size_t t_idx = --upper_bound(mDividedT_fix.begin(), mDividedT_fix.end(), t) - mDividedT_fix.begin();// i.e. -1
    if( t_idx < mDividedT_fix.size() ) t_idx = AQLModelUtilities::eq(mDividedT_fix[t_idx+1], t, 7. / 367.) ? t_idx + 1 : t_idx;

    return mCacheVolMatrix[t_idx][i];
};

void AQLMathVolatilityLMMDiscModel::createCacheVolMatrix(size_t i)//, const DoubleVector& T_fix_special, const DoubleVector& paramV, const DoubleVector& paramF, double G)
{
    mVol_special[i] = 0.0 < mDividedT_fix_special[0] ? AQLEigenSystems::sigma0( 0.5 * mT_fix_special[0], mT_fix_special[i], mParamV, mParamF, mG[i]) : 0.0;

    size_t j = 0;
    size_t k = 0;
    
    if(i == 0)
    {
        for( k = 0; k < mNumSmallStep; ++k)
        {
            mCacheVolMatrix[k][i] = mVol_special[i];
        }
    }
    else
    {
        while ( j < i )
        {
            for( k = mNumSmallStep * j; k < mNumSmallStep * (j + 1) ; ++k)
            {
                mCacheVolMatrix[k][i] = AQLEigenSystems::sigma0( 0.5 * (mDividedT_fix_special[k + 1] + mDividedT_fix_special[k]), mT_fix_special[i], mParamV, mParamF, mG[i]);
            }
            j++;
        }
    }

    while( k < mNumSmallStep * (j + 1) - 1 )
    {
        mCacheVolMatrix[k][i] = AQLEigenSystems::sigma0( 0.5 * (mDividedT_fix_special[k + 1] + mDividedT_fix_special[k]), mT_fix_special[i], mParamV, mParamF, mG[i]);
        ++k;
    }
    mCacheVolMatrix[k][i] = (i != 0) ? mCacheVolMatrix[k - 1][i] : 0.0;
    j++;

    while ( j < mT_fix_special.size() )
    {
        for( k = mNumSmallStep * j; k < mNumSmallStep * (j + 1) ; ++k )
        {
            mCacheVolMatrix[k][i] = 0.0;
        }
        j++;
    }
}

void AQLMathVolatilityLMMDiscModel::setParam(const DoubleVector& paramV_,
										  const DoubleVector& paramF_,
										  const DoubleVector& G_
		)
{
    mParamV = paramV_;
    mParamF = paramF_;
	if (mpInterG.get() == 0)
	{
	    mG = G_;
	}
	else
	{
		mpInterG->set(mTenorG, G_);

		mG.clear();
		mG.resize(mT_fix_special.size());
		for ( size_t i = 0; i < mT_fix_special.size(); i++ )
		{
			mG[i] = mpInterG->value(mT_fix_special[i]);
		}
	}

    mVol_special.clear();
    mVol_special.resize(mT_fix_special.size());

    mCacheVolMatrix.clear();
    mCacheVolMatrix.resize( mNumSmallStep * mT_fix_special.size(), mVol_special);
    
    for ( size_t i = 0; i < mT_fix_special.size(); i++ )
    {
        createCacheVolMatrix(i);
    }
    createCacheVolIntMatrix();

}

void AQLMathVolatilityLMMDiscModel::setParamV(const DoubleVector& paramV_)
{
    mParamV = paramV_;

    mVol_special.clear();
    mVol_special.resize(mT_fix_special.size());
    mCacheVolMatrix.clear();
    mCacheVolMatrix.resize( mNumSmallStep * mT_fix_special.size(), mVol_special);
    
    for ( size_t i = 0; i < mT_fix_special.size(); i++ )
    {
        createCacheVolMatrix(i);
    }
    createCacheVolIntMatrix();

}

void AQLMathVolatilityLMMDiscModel::setParamF(const DoubleVector& paramF_)
{
    mParamF = paramF_;
 
    mVol_special.clear();
    mVol_special.resize(mT_fix_special.size());
    mCacheVolMatrix.clear();
    mCacheVolMatrix.resize( mNumSmallStep * mT_fix_special.size(), mVol_special);
    
    for ( size_t i = 0; i < mT_fix_special.size(); i++ )
    {
        createCacheVolMatrix(i);
    }
    createCacheVolIntMatrix();

}

void AQLMathVolatilityLMMDiscModel::setParamG(const DoubleVector& G_)
{
	if (mpInterG.get() == 0)
	{
	    mG = G_;
	}
	else
	{
		mpInterG->set(mTenorG, G_);

		mG.clear();
		mG.resize(mT_fix_special.size());
		for ( size_t i = 0; i < mT_fix_special.size(); i++ )
		{
			mG[i] = mpInterG->value(mT_fix_special[i]);
		}
	}

    mVol_special.clear();
    mVol_special.resize(mT_fix_special.size());
    mCacheVolMatrix.clear();
    mCacheVolMatrix.resize( mNumSmallStep * mT_fix_special.size(), mVol_special );
    
    for ( size_t i = 0; i < mT_fix_special.size(); i++ )
    {
        createCacheVolMatrix(i);
    }
    createCacheVolIntMatrix();

}

