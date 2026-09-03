/*! @file
    @brief Class declaration for bivariate Student t-copula.
*/

// Improved performance & precision by AlgoQuantHub.
#ifndef LABivariateStudentTCopula_h
#define LABivariateStudentTCopula_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LABivariateStudentTCopula.h
//
//  DESCRIPTION :       Bivariate Student t-copula.
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

class LABivariateStudentTCopula : public LABivariateCopula
{
public:
    LABivariateStudentTCopula(double rho_, double nu_);
    virtual ~LABivariateStudentTCopula();
    double Value(double x, double y);
    double Diff(double x, double y, DiffDimension d);

private:
    double mRho, mNu, mSqrt1mrho2, mNup1;
    LABivariateDistribution* mBiVariate;
    LAUnivariateDistribution* mUniVariate;
    LAUnivariateDistribution* mUniVariatep1;
};

#endif
