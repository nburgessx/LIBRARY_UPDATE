/*! @file
    @brief Class declaration for bivariate distribution functions.
*/

// Improved performance and precision.
#ifndef AQLBivariateDistribution_h
#define AQLBivariateDistribution_h
#ifdef __GNUG__
#pragma interface
#else
#pragma warning( disable : 4290 )
#endif

class AQLBivariateDistribution
{
public:
    AQLBivariateDistribution();
    AQLBivariateDistribution(double meanX_, double meanY_, double stDevX_, double stDevY_);
    virtual ~AQLBivariateDistribution();

    virtual double Cumulative(double x, double y) = 0;

protected:
    double Standardize(double x, double mu, double sigma);

    double mMeanX, mMeanY, mStDevX, mStDevY;
    bool mStandard;
};

#endif
