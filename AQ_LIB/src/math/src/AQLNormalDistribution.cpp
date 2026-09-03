/*! @file
    @brief Implementation of Normal distribution.
*/



#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLNormalDistribution.h"
#include "AQLDist.h"
#include "AQLCoreUtil.h"
#include "AQLNumericalConstants.h"

AQLNormalDistribution::AQLNormalDistribution()
: AQLUnivariateDistribution()
{
    mStandard = true;
}

AQLNormalDistribution::AQLNormalDistribution(double mean, double stDev)
: AQLUnivariateDistribution()
{
    mLocation = mean;
    mScale = stDev;
    mStandard = (eq(mLocation, 0.0) && eq(mScale, 1.0));
}

double AQLNormalDistribution::Density(double x)
{
    double z = (mStandard ? x : Standardize(x));
    return AQLDist::normsPDF(z) / mScale;
}

double AQLNormalDistribution::Cumulative(double x)
{
    double z = (mStandard ? x : Standardize(x));
    return AQLDist::normsdist(z);
}

double AQLNormalDistribution::CumulativeInverse(double y)
{
    if (eq(y, 0.0))
        return -AQ_MACHINE_MAX;
    else if (eq(y, 1.0))
        return AQ_MACHINE_MAX;
    else if (y < 0.0 || y > 1.0)
        throw AQLCoreInvalidData("0 < y || y > 1 in Normal cumulative inverse", __FILE__, __LINE__);
    else
        return StandardizeInverse(AQLDist::invNormdist(y));
}