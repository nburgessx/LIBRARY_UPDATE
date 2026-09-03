/*! @file
    @brief Implementation of Student t-distribution.
*/



#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLStudentTDistribution.h"
#include "AQLDist.h"
#include "AQLCoreUtil.h"
#include "AQLBasic.h"
#include "AQLNumericalConstants.h"

AQLStudentTDistribution::AQLStudentTDistribution(double nu_)
: AQLUnivariateDistribution()
{
    mStandard = true;
    mNu = nu_;
    Set();
}

AQLStudentTDistribution::AQLStudentTDistribution(double location_, double scale_, double nu_)
: AQLUnivariateDistribution()
{
    mLocation = location_;
    mScale = scale_;
    mStandard = (eq(mLocation, 0.0) && eq(mScale, 1.0));
    mNu = nu_;
    Set();
}

void AQLStudentTDistribution::Set()
{
    mThreshold = 1e-8;
    if (mNu < mThreshold)
        throw AQLCoreInvalidData("Invalid parameter in Student t-distribution", __FILE__, __LINE__);

    mP = 0.5 * (mNu + 1.0);
    mHalfNu = 0.5 * mNu;
    double g1 = exp(AQLMath::gammln(mP));
    double g2 = exp(AQLMath::gammln(mHalfNu));
    mC = g1 / g2 / sqrt(AQ_PI * mNu) / mScale;
    double max = 1e20;
    SetInversionZBrent(1000, 1e-8, -max, max);
}

double AQLStudentTDistribution::Density(double x)
{
    double z = (mStandard ? x : Standardize(x));
    return mC * pow(1.0 + z * z / mNu, -mP);
}

double AQLStudentTDistribution::Cumulative(double x)
{
    double z = (mStandard ? x : Standardize(x));
    double xx = 1.0 / (1.0 + z * z / mNu);
    double sig = (z > 0.0 ? 1.0 : -1.0);
    return 0.5 + 0.5 * sig * (1.0 - AQLMath::betai(mHalfNu, 0.5, xx));
    //return 0.5 * (1.0 + sig * AQLDist::tdist(z, nu));
}
