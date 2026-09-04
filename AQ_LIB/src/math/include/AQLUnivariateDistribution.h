/*! @file
    @brief Class declaration for base class of univariate distributions.
*/

// Improved performance and precision.
#ifndef AQLUnivariateDistribution_h
#define AQLUnivariateDistribution_h
#ifdef __GNUG__
#pragma interface
#else
#pragma warning( disable : 4290 )
#endif


#include "AQLFunction.h"

enum BoundType { Lower, Upper };

//================ UnivariateDistribution ===================================
class AQLUnivariateDistribution
{
public:
    AQLUnivariateDistribution();

    virtual double Density(double x) = 0;
    virtual double Cumulative(double x) = 0;
    virtual double CumulativeInverse(double y);
    virtual double Bound(double confidence, BoundType boundType);

protected:
    double Standardize(double x);
    double StandardizeInverse(double y);
    void SetInversionNewton(int maxIter = 100, double accuracy = 1e-9);
    void SetInversionZBrent(int maxIter = 100, double accuracy = 1e-9, double lw = -100.0, double up = 100.0);

    double mLocation, mScale, mInvAccuracy, mInvLw, mInvUp;
    int mInvMethod, mInvMaxIter;

private:
    double SolveCumulativeInverse(double y, int maxIter, double accuracy, double lw, double up);
};

//================ CumulativeDensityFunction ===================================
class AQLCumulativeDensityFunction : public AQLFunction
{
public:
    AQLCumulativeDensityFunction(AQLUnivariateDistribution* distribution_, double y_);

    double operator()(double x) const;

private:
    AQLUnivariateDistribution* mDistribution;
    double mY;
};

#endif
