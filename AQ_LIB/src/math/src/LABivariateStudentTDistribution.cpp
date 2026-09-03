/*! @file
    @brief Implementation for bivariate Student t-distribution.
*/



#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "LABivariateStudentTDistribution.h"
#include "LAStudentTDistribution.h"

//================ BivariateStudentTDistribution ===================================
LABivariateStudentTDistribution::LABivariateStudentTDistribution(double rho_, double nu_)
: LAProductDistribution(rho_)
{
    mNu = nu_;
    Set();
}

LABivariateStudentTDistribution::LABivariateStudentTDistribution(double meanX_, double meanY_, double stDevX_, double stDevY_,
                                                                 double rho_, double nu_)
: LAProductDistribution(meanX_, meanY_, stDevX_, stDevY_, rho_)
{
    mNu = nu_;
    Set();
}

LAFunction* LABivariateStudentTDistribution::BivariateIntegrand(double x)
{
    return new LABiStudentTIntegrand(mRho, mNu, x);
}

void LABivariateStudentTDistribution::Set()
{
    mUnivariateX = new LAStudentTDistribution(mNu);
    mUnivariateY = new LAStudentTDistribution(mNu);
}

//================ BiStudentTIntegrand =================================================
LABiStudentTIntegrand::LABiStudentTIntegrand(double rho_, double nu_, double y_)
{
    mRho = rho_;
    mNu = nu_;
    mY = y_;
    mOneMsqrtRho2 = sqrt(1.0 - pow(mRho, 2));
    mNup1 = mNu + 1.0;
    mDensityDist = new LAStudentTDistribution(mNu);
    mCumulDist = new LAStudentTDistribution(mNup1);
}

LABiStudentTIntegrand::~LABiStudentTIntegrand()
{
    delete mDensityDist; mDensityDist = 0;
    delete mCumulDist; mCumulDist = 0;
}

double LABiStudentTIntegrand::operator()(double x) const
{
    double nArg = (mY - mRho * x) / mOneMsqrtRho2;
    double nupx2 = mNu + pow(x, 2);
    double coeff = sqrt(mNup1 / nupx2);
    return mDensityDist->Density(x) * mCumulDist->Cumulative(coeff * nArg);
}
