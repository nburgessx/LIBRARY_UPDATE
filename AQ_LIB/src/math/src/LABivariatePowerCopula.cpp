/*! @file
    @brief Implementation for bivariate Power copulas.
*/



#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "LABivariatePowerCopula.h"
#include <cmath>
#include "LABivariateGaussianCopula.h"
#include "LABivariateStudentTCopula.h"
#include "LANumericalConstants.h"

//================ BiVariatePowerCopula ===================================
LABivariatePowerCopula::LABivariatePowerCopula(double theta1_, double theta2_, LABivariateCopula* bCopula_)
{
    mTheta1 = theta1_;
    mTheta2 = theta2_;
    mBaseCopula = bCopula_;
}

LABivariatePowerCopula::~LABivariatePowerCopula()
{
    delete mBaseCopula;
}

double LABivariatePowerCopula::Value(double x, double y)
{
    double xt1 = pow(x, mTheta1), yt2 = pow(y, mTheta2);
    return (x / xt1) * (y / yt2) * mBaseCopula->Value(xt1, yt2);
}

double LABivariatePowerCopula::Diff(double x, double y, DiffDimension d)
{
    double xt1 = pow(x, mTheta1), yt2 = pow(y, mTheta2), term1, term2;
    double eps = MLIB_MACHINE_EPSILON * 100.0;
    if (xt1 < eps || yt2 < eps)
    {
        term1 = term2 = 0.0;
    }
    else
    {
        if (d == First)
        {
            term1 = (1.0 - mTheta1) / xt1 * (y / yt2) * mBaseCopula->Value(xt1, yt2);
            term2 = mTheta1 * (y / yt2) * mBaseCopula->Diff(xt1, yt2, First);
        }
        else
        {
            term1 = (1.0 - mTheta2) / yt2 * (x / xt1) * mBaseCopula->Value(xt1, yt2);
            term2 = mTheta2 * (x / xt1) * mBaseCopula->Diff(xt1, yt2, Second);
        }
    }

    return term1 + term2;
}

//================ BiVariatePowerGaussianCopula ===================================
LABivariatePowerGaussianCopula::LABivariatePowerGaussianCopula(double theta1_, double theta2_, double rho_)
: LABivariatePowerCopula(theta1_, theta2_, new LABivariateGaussianCopula(rho_))
{
}

//================ BiVariatePowerStudentTCopula ===================================
LABivariatePowerStudentTCopula::LABivariatePowerStudentTCopula(double theta1_, double theta2_, double rho_, double nu_)
: LABivariatePowerCopula(theta1_, theta2_, new LABivariateStudentTCopula(rho_, nu_))
{
}
