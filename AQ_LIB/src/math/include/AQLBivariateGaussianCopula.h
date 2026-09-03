/*! @file
    @brief Class declaration for bivariate Gaussian copula.
*/

// Improved performance & precision by AlgoQuantHub.
#ifndef AQLBivariateGaussianCopula_h
#define AQLBivariateGaussianCopula_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       AQLBivariateGaussianCopula.h
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


#include "AQLBivariateCopula.h"
#include "AQLBivariateDistribution.h"
#include "AQLUnivariateDistribution.h"

class AQLBivariateGaussianCopula : public AQLBivariateCopula
{
public:
    AQLBivariateGaussianCopula(double rho_);
    virtual ~AQLBivariateGaussianCopula();
    double Value(double x, double y);
    double Diff(double x, double y, DiffDimension d);

private:
    double mRho, mSqrt1mrho2;
    AQLBivariateDistribution* mBiVariate;
    AQLUnivariateDistribution* mUniVariate;
};

#endif
