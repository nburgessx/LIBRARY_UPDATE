/*! @file
    @brief Class declaration for bivariate Gaussian copula.
*/

// Improved performance & precision by AlgoQuantHub.
#ifndef LABivariateGaussianCopula_h
#define LABivariateGaussianCopula_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LABivariateGaussianCopula.h
//
//  DESCRIPTION :       Bivariate Gaussian copula.
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       FIX
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma interface
#else
#pragma warning( disable : 4290 )
#endif


#include "LABivariateCopula.h"
#include "LABivariateDistribution.h"
#include "LAUnivariateDistribution.h"

class LABivariateGaussianCopula : public LABivariateCopula
{
public:
    LABivariateGaussianCopula(double rho_);
    virtual ~LABivariateGaussianCopula();
    double Value(double x, double y);
    double Diff(double x, double y, DiffDimension d);

private:
    double mRho, mSqrt1mrho2;
    LABivariateDistribution* mBiVariate;
    LAUnivariateDistribution* mUniVariate;
};

#endif
