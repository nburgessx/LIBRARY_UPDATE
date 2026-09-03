/*!
    \file LAMathVolatilityLMM.cpp
    \brief Implements Volatility and its related classes.
    \author
    \date 2008/10
    \version 1.0

     2005, AlgoQuantHub. All rights reserved.
*/

#include "LAMathVolatilityLMM.h"
#include <algorithm>

//
LAMathVolatilityLMMCont::LAMathVolatilityLMMCont( vector<double> T_fix_, size_t n_ )
:
mT_fix(T_fix_),
mn(n_)
{
    mIntegrate.SetFunc( *this, &LAMathVolatilityLMMCont::get );
    mIntegrateSQ.SetFunc( *this, &LAMathVolatilityLMMCont::getSQ );
}

//
LAMathVolatilityLMMCont::LAMathVolatilityLMMCont( const LAMathVolatilityLMMCont& rhs )
:
mT_fix( rhs.mT_fix ),
mIntegrate( rhs.mIntegrate ),
mIntegrateSQ( rhs.mIntegrateSQ ),
mn( rhs.mn ),
mi_temp( rhs.mi_temp )
{
}

//
LAMathVolatilityLMMCont& LAMathVolatilityLMMCont::operator =(const LAMathVolatilityLMMCont& rhs)
{
    if ( this != &rhs )
    {
        (*this).LAMathVolatilityLMM::operator =(rhs);

        mT_fix = rhs.mT_fix;
        mIntegrate = rhs.mIntegrate;
        mIntegrateSQ = rhs.mIntegrateSQ;
        mn = rhs.mn;
        mi_temp = rhs.mi_temp;
    }
    return *this;
}

//
double LAMathVolatilityLMMCont::integrate0(double t, size_t i)
{
    if ( t < 0.0 || LAModelUtilities::eq(t,0.0) ) return 0.0;

    mi_temp = i;
    return mIntegrate.IntegrateGLegendre(0.0, t, mn);
}

//
double LAMathVolatilityLMMCont::integrateSQ0(double t, size_t i)
{
    if ( t < 0.0 || LAModelUtilities::eq(t,0.0) ) return 0.0;

    mi_temp = i;
    return mIntegrateSQ.IntegrateGLegendre(0.0, t, mn);
}

//
double LAMathVolatilityLMMCont::get(double t)
{
    return get(t, mi_temp);
}

//
double LAMathVolatilityLMMCont::getSQ(double t)
{
    double temp = get(t);
    return temp * temp;
}

//
//----------------------------------------------------------------
//
double LAMathVolatilityLMMDisc::integrate0( double t, size_t i )
{
    if ( t < 0.0 || LAModelUtilities::eq(t,0.0)) return 0.0;

    // 0.5*t is used to make sure we get vol_special
    if (t < mDividedT_fix.front() || LAModelUtilities::eq(t,mDividedT_fix.front()) ) return 0 < mDividedT_fix.front() ? t * get(0.5 * t, i) : 0.0;
    if (mT_fix[i] < t)  return integrate0( mNumSmallStep * i, i );

    size_t t_idx = findIndex( t );

    double u = mDividedT_fix[ t_idx ];
    if ( t < u )
    {
        throw LACoreInvalidData(" Wrong t_idx : LAMathVolatilityLMMDisc::Integrate0 ",__FILE__,__LINE__);
    }
    // integration from 0 to the closest preceding canonical point
    double integral = integrate0( t_idx, i );
//    if ( t == u )
    if ( LAModelUtilities::eq(t,u) )
    {
        // the closest preceding canonical point is equal to the upper limit
        return integral;
    }
    return integral + (t - u) * get(u, i);
}

//
double LAMathVolatilityLMMDisc::integrate0(size_t t, size_t i)
{
    return mCacheVolIntMatrix[t][i];
}

//
double LAMathVolatilityLMMDisc::integrateSQ0( double t, size_t i )
{
    if ( t < 0.0 || LAModelUtilities::eq(t,0.0) ) return 0.0;

    // 0.5*t is used to make sure we get vol_special
    if (t < mDividedT_fix.front() || LAModelUtilities::eq(t,mDividedT_fix.front()) )
    {
        if ( 0.0 < mDividedT_fix.front() )
        {
            double tmp = get(0.5 * t, i);
            return t * tmp * tmp;
        }
        else
        {
            return 0.0;
        }
    }
    if (mT_fix[i] < t)  return integrateSQ0( mNumSmallStep * i, i );

    size_t t_idx = findIndex( t );

    double u = mDividedT_fix[ t_idx ];
    if ( t < u )
    {
        throw LACoreInvalidData(" Wrong t_idx : LAMathVolatilityLMMDisc::Integrate_SQ ",__FILE__,__LINE__);
    }
    // integration from 0 to the closest preceding canonical point
    double integral = integrateSQ0( t_idx, i );
//    if ( t == u )
    if ( LAModelUtilities::eq(t,u) )
    {
        // the closest preceding canonical point is equal to the upper limit
        return integral;
    }
    double tmp = get(u, i);
    return integral + (t - u) * tmp * tmp;
}

//
double LAMathVolatilityLMMDisc::integrateSQ0(size_t t, size_t i)
{
    return mCacheVolSqIntMatrix[t][i];
}


//
void LAMathVolatilityLMMDisc::createCacheVolIntMatrix()
{
    size_t n = mT_fix.size();
    mCacheVolIntMatrix.resize( mNumSmallStep * n, DoubleVector( n, 0.0 ) );
    mCacheVolSqIntMatrix = mCacheVolIntMatrix;

    for ( size_t i = 0; i < n; i++ )
    {
        for ( size_t t = 0; t < mNumSmallStep * n; t++ )
        {
            if ( t == 0.0 )
            {
                if ( 0.0 < mDividedT_fix.front() )
                {
                    //double tmp = Get(0.5 * Divided_T_fix.front(), i );
                    double tmp = get(0.5 * mT_fix.front(), i );
                    mCacheVolIntMatrix[t][i] = mDividedT_fix.front() * tmp;
                    mCacheVolSqIntMatrix[t][i] = mCacheVolIntMatrix[t][i] * tmp;
                }
                else
                {
                    mCacheVolIntMatrix[t][i] = mCacheVolSqIntMatrix[t][i] = 0.0;
                }
            }
            else
            {
                double tmp = get(mDividedT_fix[t-1], i);
                double tmp2 = (mDividedT_fix[t] - mDividedT_fix[t-1]) * tmp;
                mCacheVolIntMatrix[t][i] = mCacheVolIntMatrix[t-1][i] + tmp2;
                mCacheVolSqIntMatrix[t][i] = mCacheVolSqIntMatrix[t-1][i] + tmp2 * tmp;
            }
        }
    }
}

//
size_t LAMathVolatilityLMMDisc::findIndex( double t )
{
    size_t t_idx = --upper_bound(mDividedT_fix.begin(), mDividedT_fix.end(), t) - mDividedT_fix.begin();// i.e. -1
    return t_idx;

}
