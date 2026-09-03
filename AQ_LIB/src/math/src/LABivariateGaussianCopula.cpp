/*! @file
    @brief Implementation for bivariate Gaussian copula.
*/



#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "LABivariateGaussianCopula.h"
#include "LABivariateNormalDistribution.h"
#include "LANormalDistribution.h"

LABivariateGaussianCopula::LABivariateGaussianCopula(double rho_)
{
    mRho = rho_;
    mSqrt1mrho2 = sqrt(1.0 - pow(mRho, 2));
    mBiVariate = new LABivariateNormalDistribution(mRho);
    mUniVariate = new LANormalDistribution();
}

LABivariateGaussianCopula::~LABivariateGaussianCopula()
{
    delete mBiVariate; mBiVariate = 0;
    delete mUniVariate; mUniVariate = 0;
}

double LABivariateGaussianCopula::Value(double x, double y)
{
    return mBiVariate->Cumulative(mUniVariate->CumulativeInverse(x), mUniVariate->CumulativeInverse(y));
}

double LABivariateGaussianCopula::Diff(double x, double y, DiffDimension d)
{
    double v1 = (d == First ? x : y);
    double v2 = (d == First ? y : x);
    double arg = mUniVariate->CumulativeInverse(v2) - mRho * mUniVariate->CumulativeInverse(v1);
    return mUniVariate->Cumulative(arg / mSqrt1mrho2);
}
