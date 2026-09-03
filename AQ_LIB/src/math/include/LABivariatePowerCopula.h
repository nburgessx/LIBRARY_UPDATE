/*! @file
    @brief Class declaration for bivariate Power copulas.
*/

// Improved performance & precision by Mizuho International London.
#ifndef LABivariatePowerCopula_h
#define LABivariatePowerCopula_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LABivariatePowerCopula.h
//
//  DESCRIPTION :       Bivariate Power copulas.
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

//================ BiVariatePowerCopula ===================================
class LABivariatePowerCopula : public LABivariateCopula
{
public:
    LABivariatePowerCopula(double rho_, double nu_, LABivariateCopula* bCopula_);
    virtual ~LABivariatePowerCopula();
    double Value(double x, double y);
    double Diff(double x, double y, DiffDimension d);

private:
    double mTheta1, mTheta2;
    LABivariateCopula* mBaseCopula;
};

//================ BiVariatePowerGaussianCopula ===================================
class LABivariatePowerGaussianCopula : public LABivariatePowerCopula
{
public:
    LABivariatePowerGaussianCopula(double theta1_, double theta2_, double rho_);
};

//================ BiVariatePowerStudentTCopula ===================================
class LABivariatePowerStudentTCopula : public LABivariatePowerCopula
{
public:
    LABivariatePowerStudentTCopula(double theta1_, double theta2_, double rho_, double nu_);
};

#endif
