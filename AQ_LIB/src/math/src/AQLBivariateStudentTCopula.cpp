/*! @file
    @brief Implementation for bivariate Student t-copula.
*/



#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLBivariateStudentTCopula.h"
#include "AQLBivariateStudentTDistribution.h"
#include "AQLStudentTDistribution.h"

AQLBivariateStudentTCopula::AQLBivariateStudentTCopula(double rho_, double nu_)
{
    mRho = rho_;
    mNu = nu_;
    mSqrt1mrho2 = sqrt(1.0 - pow(mRho, 2));
    mBiVariate = new AQLBivariateStudentTDistribution(mRho, mNu);
    mUniVariate = new AQLStudentTDistribution(mNu);
    mNup1 = mNu + 1.0;
    mUniVariatep1 = new AQLStudentTDistribution(mNup1);
}

AQLBivariateStudentTCopula::~AQLBivariateStudentTCopula()
{
    delete mBiVariate; mBiVariate = 0;
    delete mUniVariate; mUniVariate = 0;
    delete mUniVariatep1; mUniVariatep1 = 0;
}

double AQLBivariateStudentTCopula::Value(double x, double y)
{
    return mBiVariate->Cumulative(mUniVariate->CumulativeInverse(x), mUniVariate->CumulativeInverse(y));
}

double AQLBivariateStudentTCopula::Diff(double x, double y, DiffDimension d)
{
    double v1 = (d == First ? x : y);
    double v2 = (d == First ? y : x);
    double uniV1 = mUniVariate->CumulativeInverse(v1);
    double coeff = sqrt(mNup1 / (mNu + pow(uniV1, 2)));
    double arg = mUniVariate->CumulativeInverse(v2) - mRho * uniV1;
    return mUniVariatep1->Cumulative(coeff * arg / mSqrt1mrho2);
}
