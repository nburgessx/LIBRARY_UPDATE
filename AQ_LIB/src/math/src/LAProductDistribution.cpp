/*! @file
    @brief Implementation of base class for product distributions (bivariate).
*/



#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "LAProductDistribution.h"
#include "LANumericalConstants.h"
#include "LAGaussLegendre.h"
#include "LAGaussLobattointegral.h"
#include <vector>

using namespace std;

LAProductDistribution::LAProductDistribution(double rho_)
: LABivariateDistribution()
{
    mRho = rho_;
    mUnivariateX = 0;
    mUnivariateY = 0;
}

LAProductDistribution::LAProductDistribution(double meanX_, double meanY_, double stDevX_, double stDevY_, double rho_)
: LABivariateDistribution(meanX_, meanY_, stDevX_, stDevY_)
{
    mRho = rho_;
    mUnivariateX = 0;
    mUnivariateY = 0;
}

LAProductDistribution::~LAProductDistribution()
{
    delete mUnivariateX; mUnivariateX = 0;
    delete mUnivariateY; mUnivariateY = 0;
}

double LAProductDistribution::Cumulative(double x, double y)
{
    double x_ = (mStandard ? x : Standardize(x, mMeanX, mStDevX));
    double y_ = (mStandard ? y : Standardize(y, mMeanY, mStDevY));
    //// Edge cases ////
    if (x_ <= -MLIB_MACHINE_MAX || y_ <= -MLIB_MACHINE_MAX)
        return 0.0;
    // Reduces to univariate
    if (x_ >= MLIB_MACHINE_MAX)
    {
        if (mUnivariateY == 0)
            throw LACoreInvalidData("Univariate sub-distribution not defined in product distribution", __FILE__, __LINE__);
        else
            return mUnivariateY->Cumulative(y_);
    }
    if (y_ >= MLIB_MACHINE_MAX)
    {
        if (mUnivariateX == 0)
            throw LACoreInvalidData("Univariate sub-distribution not defined in product distribution", __FILE__, __LINE__);
        else
            return mUnivariateX->Cumulative(x_);
    }

    // True bivariate
    return TrueBivariateCumulative(x_, y_);
}

double LAProductDistribution::TrueBivariateCumulative(double x, double y)
{
    return CumulativeByIntegration(x, y); // By default we do the 1D numerical integral
}

double LAProductDistribution::CumulativeByIntegration(double x, double y)
{
    double lwBound = -7.0;
    if (y < lwBound)
        return 0.0;
    else
    {
        LAFunction* f = BivariateIntegrand(x);
        bool useLobatto = true;
        double integral;
        if (useLobatto)
        {
            // Adaptive integration using Gauss-Lobatto
            GaussLobattoIntegral integrator(1000, 1.0e-6);
            integral = integrator(*f, lwBound, y);
        }
        else
        {
            // Gauss-Legendre with adaptive intervals
            LAGaussLegendre integrator(32);
            vector<double> samples;
            samples.push_back(lwBound);
            double s = lwBound;
            s += 3.0; if (s < y) samples.push_back(s); // -4.0
            s += 2.5; if (s < y) samples.push_back(s); // -1.5
            s += 1.0; if (s < y) samples.push_back(s); // -0.5
            s += 0.5; if (s < y) samples.push_back(s); //  0.0
            s += 0.5; if (s < y) samples.push_back(s); //  0.5
            s += 1.0; if (s < y) samples.push_back(s); //  1.5
            s += 2.5; if (s < y) samples.push_back(s); //  4.0
            s += 3.0; if (s < y) samples.push_back(s); //  7.0
            samples.push_back(y);
            integral = integrator.IntegrateBySampling(*f, samples);
        }

        delete f;
        return integral;
    }
}
