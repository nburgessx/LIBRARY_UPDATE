/*! @file
    @brief Class declaration to provide distribution functions.
*/

// Improved performance & precision by Mizuho International London.
#ifndef LAProductDistribution_h
#define LAProductDistribution_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LAProductDistribution.h
//
//  DESCRIPTION :       Base class for product distributions (bivariate).
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       FIX
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma interface
#else
#pragma warning( disable : 4290 )
#endif


#include "LABivariateDistribution.h"
#include "LAUnivariateDistribution.h"

class LAProductDistribution : public LABivariateDistribution
{
public:
    LAProductDistribution(double rho_);
    LAProductDistribution(double meanX_, double meanY_, double stDevX_, double stDevY_, double rho_);
    virtual ~LAProductDistribution();

    double Cumulative(double x, double y);

protected:
    virtual double TrueBivariateCumulative(double x, double y);
    double CumulativeByIntegration(double x, double y);
    virtual LAFunction* BivariateIntegrand(double x) = 0;

    double mRho;
    LAUnivariateDistribution* mUnivariateX;
    LAUnivariateDistribution* mUnivariateY;
};

#endif
