/*! @file
    @brief Implementation for bivariate Normal distribution.
*/



#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLBivariateNormalDistribution.h"
#include "AQLNormalDistribution.h"

//================ BivariateNormalDistribution ===================================
AQLBivariateNormalDistribution::AQLBivariateNormalDistribution(double rho_, string cdfMethod_)
: AQLProductDistribution(rho_)
{
    Set(cdfMethod_);
}

AQLBivariateNormalDistribution::AQLBivariateNormalDistribution(double meanX_, double meanY_, double stDevX_, double stDevY_,
                                                             double rho_, string cdfMethod_)
: AQLProductDistribution(meanX_, meanY_, stDevX_, stDevY_, rho_)
{
    Set(cdfMethod_);
}

AQLBivariateNormalDistribution::~AQLBivariateNormalDistribution()
{
    delete mQlDistribution; mQlDistribution = 0;
}

double AQLBivariateNormalDistribution::CumulativeByQuantLib(double x, double y)
{
    return mQlDistribution->operator()(x, y);
}

double AQLBivariateNormalDistribution::TrueBivariateCumulative(double x, double y)
{
    if (mCdfMethod == QuantLibCDF)
        return CumulativeByQuantLib(x, y);
    else
        return CumulativeByIntegration(x, y);
}

AQLFunction* AQLBivariateNormalDistribution::BivariateIntegrand(double x)
{
    return new AQLBiNormalIntegrand(mRho, x);
}

void AQLBivariateNormalDistribution::Set(string method)
{
    mQlDistribution = 0;
    mUnivariateX = new AQLNormalDistribution();
    mUnivariateY = new AQLNormalDistribution();
    if (method == "QuantLib")
    {
        mCdfMethod = QuantLibCDF;
        mQlDistribution = new QuantLib::BivariateCumulativeNormalDistributionWe04DP(mRho);
    }
    else if (method == "Integration")
        mCdfMethod = IntegratedCDF;
    else
        throw AQLCoreInvalidData(("Unknown CDF method for BiVariate Normal: " + method).c_str(), __FILE__, __LINE__);
}

//================ BiNormalIntegrand =================================================
AQLBiNormalIntegrand::AQLBiNormalIntegrand(double rho_, double y_)
{
    mRho = rho_;
    mY = y_;
    mOneMsqrtRho2 = sqrt(1.0 - pow(mRho, 2));
    mUniDist = new AQLNormalDistribution();
}

AQLBiNormalIntegrand::~AQLBiNormalIntegrand()
{
    delete mUniDist; mUniDist = 0;
}

double AQLBiNormalIntegrand::operator()(double x) const
{
    double nArg = (mY - mRho * x) / mOneMsqrtRho2;
    return mUniDist->Density(x) * mUniDist->Cumulative(nArg);
}
