/*! @file
    @brief Class declaration for bivariate Student t-copula.
*/

// Improved performance and precision.
#ifndef AQLBivariateStudentTCopula_h
#define AQLBivariateStudentTCopula_h
#ifdef __GNUG__
#pragma interface
#else
#pragma warning( disable : 4290 )
#endif


#include "AQLBivariateCopula.h"
#include "AQLBivariateDistribution.h"
#include "AQLUnivariateDistribution.h"

class AQLBivariateStudentTCopula : public AQLBivariateCopula
{
public:
    AQLBivariateStudentTCopula(double rho_, double nu_);
    virtual ~AQLBivariateStudentTCopula();
    double Value(double x, double y);
    double Diff(double x, double y, DiffDimension d);

private:
    double mRho, mNu, mSqrt1mrho2, mNup1;
    AQLBivariateDistribution* mBiVariate;
    AQLUnivariateDistribution* mUniVariate;
    AQLUnivariateDistribution* mUniVariatep1;
};

#endif
