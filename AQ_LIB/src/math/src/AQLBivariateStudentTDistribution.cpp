/*! @file
    @brief Implementation for bivariate Student t-distribution.
*/



#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLBivariateStudentTDistribution.h"
#include "AQLStudentTDistribution.h"

//================ BivariateStudentTDistribution ===================================
AQLBivariateStudentTDistribution::AQLBivariateStudentTDistribution(double rho_, double nu_)
: AQLProductDistribution(rho_)
{
    mNu = nu_;
    Set();
}

AQLBivariateStudentTDistribution::AQLBivariateStudentTDistribution(double meanX_, double meanY_, double stDevX_, double stDevY_,
                                                                 double rho_, double nu_)
: AQLProductDistribution(meanX_, meanY_, stDevX_, stDevY_, rho_)
{
    mNu = nu_;
    Set();
}

AQLFunction* AQLBivariateStudentTDistribution::BivariateIntegrand(double x)
{
    return new AQLBiStudentTIntegrand(mRho, mNu, x);
}

void AQLBivariateStudentTDistribution::Set()
{
    mUnivariateX = new AQLStudentTDistribution(mNu);
    mUnivariateY = new AQLStudentTDistribution(mNu);
}

//================ BiStudentTIntegrand =================================================
AQLBiStudentTIntegrand::AQLBiStudentTIntegrand(double rho_, double nu_, double y_)
{
    mRho = rho_;
    mNu = nu_;
    mY = y_;
    mOneMsqrtRho2 = sqrt(1.0 - pow(mRho, 2));
    mNup1 = mNu + 1.0;
    mDensityDist = new AQLStudentTDistribution(mNu);
    mCumulDist = new AQLStudentTDistribution(mNup1);
}

AQLBiStudentTIntegrand::~AQLBiStudentTIntegrand()
{
    delete mDensityDist; mDensityDist = 0;
    delete mCumulDist; mCumulDist = 0;
}

double AQLBiStudentTIntegrand::operator()(double x) const
{
    double nArg = (mY - mRho * x) / mOneMsqrtRho2;
    double nupx2 = mNu + pow(x, 2);
    double coeff = sqrt(mNup1 / nupx2);
    return mDensityDist->Density(x) * mCumulDist->Cumulative(coeff * nArg);
}
