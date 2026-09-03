/*! @file
    @brief Implementation of base class for bivariate distributions.
*/



#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLBivariateDistribution.h"
#include "AQLCoreUtil.h"

AQLBivariateDistribution::AQLBivariateDistribution()
{
    mMeanX = mMeanY = 0.0;
    mStDevX = mStDevY = 1.0;
    mStandard = true;
}

AQLBivariateDistribution::AQLBivariateDistribution(double meanX_, double meanY_, double stDevX_, double stDevY_)
{
    mMeanX = meanX_; mMeanY = meanY_;
    mStDevX = stDevX_; mStDevY = stDevY_;
    mStandard = (eq(mMeanX, 0.0) && eq(mMeanY, 0.0) && eq(mStDevX, 1.0) && eq(mStDevY, 1.0));
}

AQLBivariateDistribution::~AQLBivariateDistribution()
{
}

double AQLBivariateDistribution::Standardize(double x, double mu, double sigma)
{
    return (x - mu) / sigma;
}
