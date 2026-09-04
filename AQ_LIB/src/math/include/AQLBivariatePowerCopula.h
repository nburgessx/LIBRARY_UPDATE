/*! @file
    @brief Class declaration for bivariate Power copulas.
*/

// Improved performance and precision.
#ifndef AQLBivariatePowerCopula_h
#define AQLBivariatePowerCopula_h
#ifdef __GNUG__
#pragma interface
#else
#pragma warning( disable : 4290 )
#endif


#include "AQLBivariateCopula.h"

//================ BiVariatePowerCopula ===================================
class AQLBivariatePowerCopula : public AQLBivariateCopula
{
public:
    AQLBivariatePowerCopula(double rho_, double nu_, AQLBivariateCopula* bCopula_);
    virtual ~AQLBivariatePowerCopula();
    double Value(double x, double y);
    double Diff(double x, double y, DiffDimension d);

private:
    double mTheta1, mTheta2;
    AQLBivariateCopula* mBaseCopula;
};

//================ BiVariatePowerGaussianCopula ===================================
class AQLBivariatePowerGaussianCopula : public AQLBivariatePowerCopula
{
public:
    AQLBivariatePowerGaussianCopula(double theta1_, double theta2_, double rho_);
};

//================ BiVariatePowerStudentTCopula ===================================
class AQLBivariatePowerStudentTCopula : public AQLBivariatePowerCopula
{
public:
    AQLBivariatePowerStudentTCopula(double theta1_, double theta2_, double rho_, double nu_);
};

#endif
