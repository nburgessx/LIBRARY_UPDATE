/*! @file
    @brief Implementation of base class for univariate distributions.
*/



#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "LAUnivariateDistribution.h"
//#include <iostream>
#include "LANumericalConstants.h"
#include "LACoreUtil.h"

using namespace std;

//================ UnivariateDistribution ===================================
LAUnivariateDistribution::LAUnivariateDistribution()
{
    mLocation = 0.0;
    mScale = 1.0;
}

double LAUnivariateDistribution::CumulativeInverse(double y)
{
    // Limiting cases
    if (eq(y, 0.0))
        return -MLIB_MACHINE_MAX;
    if (eq(y, 1.0))
        return MLIB_MACHINE_MAX;
    if (y < 0.0 || y > 1.0)
        throw LACoreInvalidData("Argument out of range [0, 1]", __FILE__, __LINE__);
    if (mInvMethod == 0)
    {
        double x = mLocation;
        int count = 0;

        // Iterate Newton Algorithm
        do
        {
            x -= (Cumulative(x) - y) / Density(x);
            count++;
        }
        while (fabs(Cumulative(x) - y) > mInvAccuracy && count < mInvMaxIter);

        if (count >= mInvMaxIter)
            throw LACoreInvalidData("Maximum number of iterations reached in inverse cumulative", __FILE__, __LINE__);

        return x;
    }
    else
        return SolveCumulativeInverse(y, mInvMaxIter, mInvAccuracy, mLocation + mInvLw * mScale, mLocation + mInvUp * mScale);
}

double LAUnivariateDistribution::Bound(double confidence, BoundType boundType)
{
    return mLocation + confidence * mScale * (boundType == Lower ? -1.0 : 1.0);
}

double LAUnivariateDistribution::Standardize(double x)
{
    return (x - mLocation) / mScale;
}

double LAUnivariateDistribution::StandardizeInverse(double y)
{
    return mLocation + y * mScale;
}

void LAUnivariateDistribution::SetInversionNewton(int maxIter, double accuracy)
{
    mInvMethod = 0;
    mInvMaxIter = maxIter;
    mInvAccuracy = accuracy;
}

void LAUnivariateDistribution::SetInversionZBrent(int maxIter, double accuracy, double lw, double up)
{
    mInvMethod = 1;
    mInvMaxIter = maxIter;
    mInvAccuracy = accuracy;
    mInvLw = lw;
    mInvUp = up;
}

double LAUnivariateDistribution::SolveCumulativeInverse(double y, int maxIter, double accuracy, double lw, double up)
{
    if (y < 0.0 || y > 1.0)
        throw LACoreInvalidData("Probability should be between 0 and 1 in cumulative density function", __FILE__, __LINE__);
    LACumulativeDensityFunction cumulative(this, y);
    return cumulative.SolveBR(lw, up, maxIter, accuracy);
}

//================ CumulativeDensityFunction ===================================
LACumulativeDensityFunction::LACumulativeDensityFunction(LAUnivariateDistribution* distribution_, double y_)
{
    mDistribution = distribution_;
    mY = y_;
}

double LACumulativeDensityFunction::operator()(double x) const
{
    return mDistribution->Cumulative(x) - mY;
}
