/*! @file
    @brief Class declaration to provide distribution functions.
*/

// Improved performance & precision by AlgoQuantHub.
#ifndef AQLProductDistribution_h
#define AQLProductDistribution_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       AQLProductDistribution.h
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


#include "AQLBivariateDistribution.h"
#include "AQLUnivariateDistribution.h"

class AQLProductDistribution : public AQLBivariateDistribution
{
public:
    AQLProductDistribution(double rho_);
    AQLProductDistribution(double meanX_, double meanY_, double stDevX_, double stDevY_, double rho_);
    virtual ~AQLProductDistribution();

    double Cumulative(double x, double y);

protected:
    virtual double TrueBivariateCumulative(double x, double y);
    double CumulativeByIntegration(double x, double y);
    virtual AQLFunction* BivariateIntegrand(double x) = 0;

    double mRho;
    AQLUnivariateDistribution* mUnivariateX;
    AQLUnivariateDistribution* mUnivariateY;
};

#endif
