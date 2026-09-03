/*! @file
    @brief Implementation for bivariate Power copulas.
*/



#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLBivariatePowerCopula.h"
#include <cmath>
#include "AQLBivariateGaussianCopula.h"
#include "AQLBivariateStudentTCopula.h"
#include "AQLNumericalConstants.h"

//================ BiVariatePowerCopula ===================================
AQLBivariatePowerCopula::AQLBivariatePowerCopula(double theta1_, double theta2_, AQLBivariateCopula* bCopula_)
{
    mTheta1 = theta1_;
    mTheta2 = theta2_;
    mBaseCopula = bCopula_;
}

AQLBivariatePowerCopula::~AQLBivariatePowerCopula()
{
    delete mBaseCopula;
}

double AQLBivariatePowerCopula::Value(double x, double y)
{
    double xt1 = pow(x, mTheta1), yt2 = pow(y, mTheta2);
    return (x / xt1) * (y / yt2) * mBaseCopula->Value(xt1, yt2);
}

double AQLBivariatePowerCopula::Diff(double x, double y, DiffDimension d)
{
    double xt1 = pow(x, mTheta1), yt2 = pow(y, mTheta2), term1, term2;
    double eps = AQ_MACHINE_EPSILON * 100.0;
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
AQLBivariatePowerGaussianCopula::AQLBivariatePowerGaussianCopula(double theta1_, double theta2_, double rho_)
: AQLBivariatePowerCopula(theta1_, theta2_, new AQLBivariateGaussianCopula(rho_))
{
}

//================ BiVariatePowerStudentTCopula ===================================
AQLBivariatePowerStudentTCopula::AQLBivariatePowerStudentTCopula(double theta1_, double theta2_, double rho_, double nu_)
: AQLBivariatePowerCopula(theta1_, theta2_, new AQLBivariateStudentTCopula(rho_, nu_))
{
}
