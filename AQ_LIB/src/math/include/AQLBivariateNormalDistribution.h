/*! @file
    @brief Class declaration for bivariate Normal distribution.
*/

// Improved performance & precision by AlgoQuantHub.
#ifndef AQLBivariateNormalDistribution_h
#define AQLBivariateNormalDistribution_h
#ifdef __GNUG__
#pragma interface
#else
#pragma warning( disable : 4290 )
#endif


#include "AQLProductDistribution.h"
#include <string>
#include <ql/math/distributions/bivariatenormaldistribution.hpp>

using namespace std;

enum CDFMethod { QuantLibCDF, IntegratedCDF };

//================ BivariateNormalDistribution ===================================
class AQLBivariateNormalDistribution : public AQLProductDistribution
{
public:
    AQLBivariateNormalDistribution(double rho_, string cdfMethod_ = "QuantLib");
    AQLBivariateNormalDistribution(double meanX_, double meanY_, double stDevX_, double stDevY_, double rho_,
                                  string cdfMethod_ = "QuantLib");
    virtual ~AQLBivariateNormalDistribution();

protected:
    double TrueBivariateCumulative(double x, double y);
    AQLFunction* BivariateIntegrand(double x);

private:
    double CumulativeByQuantLib(double x, double y);
    void Set(string method);

    CDFMethod mCdfMethod;
    QuantLib::BivariateCumulativeNormalDistributionWe04DP* mQlDistribution;
};

//================ BiNormalIntegrand =================================================
class AQLBiNormalIntegrand : public AQLFunction
{
public:
    AQLBiNormalIntegrand(double rho_, double y_);
    virtual ~AQLBiNormalIntegrand();

    double operator()(double x) const;

private:
    double mRho, mY, mOneMsqrtRho2;
    AQLUnivariateDistribution* mUniDist;
};

#endif
