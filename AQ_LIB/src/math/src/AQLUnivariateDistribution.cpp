/*! @file
    @brief Implementation of base class for univariate distributions.
*/



#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLUnivariateDistribution.h"
//#include <iostream>
#include "AQLNumericalConstants.h"
#include "AQLCoreUtil.h"

using namespace std;

//================ UnivariateDistribution ===================================
AQLUnivariateDistribution::AQLUnivariateDistribution()
{
    mLocation = 0.0;
    mScale = 1.0;
}

double AQLUnivariateDistribution::CumulativeInverse(double y)
{
    // Limiting cases
    if (eq(y, 0.0))
        return -AQ_MACHINE_MAX;
    if (eq(y, 1.0))
        return AQ_MACHINE_MAX;
    if (y < 0.0 || y > 1.0)
        throw AQLCoreInvalidData("Argument out of range [0, 1]", __FILE__, __LINE__);
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
            throw AQLCoreInvalidData("Maximum number of iterations reached in inverse cumulative", __FILE__, __LINE__);

        return x;
    }
    else
        return SolveCumulativeInverse(y, mInvMaxIter, mInvAccuracy, mLocation + mInvLw * mScale, mLocation + mInvUp * mScale);
}

double AQLUnivariateDistribution::Bound(double confidence, BoundType boundType)
{
    return mLocation + confidence * mScale * (boundType == Lower ? -1.0 : 1.0);
}

double AQLUnivariateDistribution::Standardize(double x)
{
    return (x - mLocation) / mScale;
}

double AQLUnivariateDistribution::StandardizeInverse(double y)
{
    return mLocation + y * mScale;
}

void AQLUnivariateDistribution::SetInversionNewton(int maxIter, double accuracy)
{
    mInvMethod = 0;
    mInvMaxIter = maxIter;
    mInvAccuracy = accuracy;
}

void AQLUnivariateDistribution::SetInversionZBrent(int maxIter, double accuracy, double lw, double up)
{
    mInvMethod = 1;
    mInvMaxIter = maxIter;
    mInvAccuracy = accuracy;
    mInvLw = lw;
    mInvUp = up;
}

double AQLUnivariateDistribution::SolveCumulativeInverse(double y, int maxIter, double accuracy, double lw, double up)
{
    if (y < 0.0 || y > 1.0)
        throw AQLCoreInvalidData("Probability should be between 0 and 1 in cumulative density function", __FILE__, __LINE__);
    AQLCumulativeDensityFunction cumulative(this, y);
    return cumulative.SolveBR(lw, up, maxIter, accuracy);
}

//================ CumulativeDensityFunction ===================================
AQLCumulativeDensityFunction::AQLCumulativeDensityFunction(AQLUnivariateDistribution* distribution_, double y_)
{
    mDistribution = distribution_;
    mY = y_;
}

double AQLCumulativeDensityFunction::operator()(double x) const
{
    return mDistribution->Cumulative(x) - mY;
}
