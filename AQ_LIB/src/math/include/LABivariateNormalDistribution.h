/*! @file
    @brief Class declaration for bivariate Normal distribution.
*/

// Improved performance & precision by AlgoQuantHub.
#ifndef LABivariateNormalDistribution_h
#define LABivariateNormalDistribution_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LABivariateNormalDistribution.h
//
//  DESCRIPTION :       Bivariate Normal distribution.
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       FIX
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma interface
#else
#pragma warning( disable : 4290 )
#endif


#include "LAProductDistribution.h"
#include <string>
#include <ql/math/distributions/bivariatenormaldistribution.hpp>

using namespace std;

enum CDFMethod { QuantLibCDF, IntegratedCDF };

//================ BivariateNormalDistribution ===================================
class LABivariateNormalDistribution : public LAProductDistribution
{
public:
    LABivariateNormalDistribution(double rho_, string cdfMethod_ = "QuantLib");
    LABivariateNormalDistribution(double meanX_, double meanY_, double stDevX_, double stDevY_, double rho_,
                                  string cdfMethod_ = "QuantLib");
    virtual ~LABivariateNormalDistribution();

protected:
    double TrueBivariateCumulative(double x, double y);
    LAFunction* BivariateIntegrand(double x);

private:
    double CumulativeByQuantLib(double x, double y);
    void Set(string method);

    CDFMethod mCdfMethod;
    QuantLib::BivariateCumulativeNormalDistributionWe04DP* mQlDistribution;
};

//================ BiNormalIntegrand =================================================
class LABiNormalIntegrand : public LAFunction
{
public:
    LABiNormalIntegrand(double rho_, double y_);
    virtual ~LABiNormalIntegrand();

    double operator()(double x) const;

private:
    double mRho, mY, mOneMsqrtRho2;
    LAUnivariateDistribution* mUniDist;
};

#endif
