/*! @file
    @brief Class declaration for base class of univariate distributions.
*/

// Improved performance & precision by AlgoQuantHub.
#ifndef LAUnivariateDistribution_h
#define LAUnivariateDistribution_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LAUnivariateDistribution.h
//
//  DESCRIPTION :       Base class for univariate distributions.
//  SEE ALSO    :
//  VIRSION		:
//  STATUS      :       FIX
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma interface
#else
#pragma warning( disable : 4290 )
#endif


#include "LAFunction.h"

enum BoundType { Lower, Upper };

//================ UnivariateDistribution ===================================
class LAUnivariateDistribution
{
public:
    LAUnivariateDistribution();

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
class LACumulativeDensityFunction : public LAFunction
{
public:
    LACumulativeDensityFunction(LAUnivariateDistribution* distribution_, double y_);

    double operator()(double x) const;

private:
    LAUnivariateDistribution* mDistribution;
    double mY;
};

#endif
