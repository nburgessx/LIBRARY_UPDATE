/*! @file
    @brief Implementation for bivariate Normal distribution.
*/



#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "LABivariateNormalDistribution.h"
#include "LANormalDistribution.h"

//================ BivariateNormalDistribution ===================================
LABivariateNormalDistribution::LABivariateNormalDistribution(double rho_, string cdfMethod_)
: LAProductDistribution(rho_)
{
    Set(cdfMethod_);
}

LABivariateNormalDistribution::LABivariateNormalDistribution(double meanX_, double meanY_, double stDevX_, double stDevY_,
                                                             double rho_, string cdfMethod_)
: LAProductDistribution(meanX_, meanY_, stDevX_, stDevY_, rho_)
{
    Set(cdfMethod_);
}

LABivariateNormalDistribution::~LABivariateNormalDistribution()
{
    delete mQlDistribution; mQlDistribution = 0;
}

double LABivariateNormalDistribution::CumulativeByQuantLib(double x, double y)
{
    return mQlDistribution->operator()(x, y);
}

double LABivariateNormalDistribution::TrueBivariateCumulative(double x, double y)
{
    if (mCdfMethod == QuantLibCDF)
        return CumulativeByQuantLib(x, y);
    else
        return CumulativeByIntegration(x, y);
}

LAFunction* LABivariateNormalDistribution::BivariateIntegrand(double x)
{
    return new LABiNormalIntegrand(mRho, x);
}

void LABivariateNormalDistribution::Set(string method)
{
    mQlDistribution = 0;
    mUnivariateX = new LANormalDistribution();
    mUnivariateY = new LANormalDistribution();
    if (method == "QuantLib")
    {
        mCdfMethod = QuantLibCDF;
        mQlDistribution = new QuantLib::BivariateCumulativeNormalDistributionWe04DP(mRho);
    }
    else if (method == "Integration")
        mCdfMethod = IntegratedCDF;
    else
        throw LACoreInvalidData(("Unknown CDF method for BiVariate Normal: " + method).c_str(), __FILE__, __LINE__);
}

//================ BiNormalIntegrand =================================================
LABiNormalIntegrand::LABiNormalIntegrand(double rho_, double y_)
{
    mRho = rho_;
    mY = y_;
    mOneMsqrtRho2 = sqrt(1.0 - pow(mRho, 2));
    mUniDist = new LANormalDistribution();
}

LABiNormalIntegrand::~LABiNormalIntegrand()
{
    delete mUniDist; mUniDist = 0;
}

double LABiNormalIntegrand::operator()(double x) const
{
    double nArg = (mY - mRho * x) / mOneMsqrtRho2;
    return mUniDist->Density(x) * mUniDist->Cumulative(nArg);
}
