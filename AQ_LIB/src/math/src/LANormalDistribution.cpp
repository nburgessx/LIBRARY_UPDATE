/*! @file
    @brief Implementation of Normal distribution.
*/



#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "LANormalDistribution.h"
#include "LADist.h"
#include "LACoreUtil.h"
#include "LANumericalConstants.h"

LANormalDistribution::LANormalDistribution()
: LAUnivariateDistribution()
{
    mStandard = true;
}

LANormalDistribution::LANormalDistribution(double mean, double stDev)
: LAUnivariateDistribution()
{
    mLocation = mean;
    mScale = stDev;
    mStandard = (eq(mLocation, 0.0) && eq(mScale, 1.0));
}

double LANormalDistribution::Density(double x)
{
    double z = (mStandard ? x : Standardize(x));
    return LADist::normsPDF(z) / mScale;
}

double LANormalDistribution::Cumulative(double x)
{
    double z = (mStandard ? x : Standardize(x));
    return LADist::normsdist(z);
}

double LANormalDistribution::CumulativeInverse(double y)
{
    if (eq(y, 0.0))
        return -AQ_MACHINE_MAX;
    else if (eq(y, 1.0))
        return AQ_MACHINE_MAX;
    else if (y < 0.0 || y > 1.0)
        throw LACoreInvalidData("0 < y || y > 1 in Normal cumulative inverse", __FILE__, __LINE__);
    else
        return StandardizeInverse(LADist::invNormdist(y));
}