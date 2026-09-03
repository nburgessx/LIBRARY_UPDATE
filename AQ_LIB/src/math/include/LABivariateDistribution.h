/*! @file
    @brief Class declaration for bivariate distribution functions.
*/

// Improved performance & precision by AlgoQuantHub.
#ifndef LABivariateDistribution_h
#define LABivariateDistribution_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LABivariateDistribution.h
//
//  DESCRIPTION :       Base class for bivariate distributions.
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       FIX
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma interface
#else
#pragma warning( disable : 4290 )
#endif

class LABivariateDistribution
{
public:
    LABivariateDistribution();
    LABivariateDistribution(double meanX_, double meanY_, double stDevX_, double stDevY_);
    virtual ~LABivariateDistribution();

    virtual double Cumulative(double x, double y) = 0;

protected:
    double Standardize(double x, double mu, double sigma);

    double mMeanX, mMeanY, mStDevX, mStDevY;
    bool mStandard;
};

#endif
