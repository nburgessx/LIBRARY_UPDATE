/*! @file
    @brief Class declaration for bivariate Student t-distribution.
*/

// Improved performance & precision by AlgoQuantHub.
#ifndef LABivariateStudentTDistribution_h
#define LABivariateStudentTDistribution_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LABivariateStudentTDistribution_.h
//
//  DESCRIPTION :       Bivariate Student t-distribution.
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       FIX
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma interface
#else
#pragma warning( disable : 4290 )
#endif


#include "LAProductDistribution.h"

//================ BivariateStudentTDistribution ===================================
class LABivariateStudentTDistribution : public LAProductDistribution
{
public:
    LABivariateStudentTDistribution(double rho_, double nu_);
    LABivariateStudentTDistribution(double meanX_, double meanY_, double stDevX_, double stDevY_, double rho_,
                                    double nu_);

protected:
    LAFunction* BivariateIntegrand(double x);

private:
    void Set();

    double mNu;
};

//================ BiStudentTIntegrand =================================================
class LABiStudentTIntegrand : public LAFunction
{
public:
    LABiStudentTIntegrand(double rho_, double nu_, double y_);
    virtual ~LABiStudentTIntegrand();

    double operator()(double x) const;

private:
    double mRho, mNu, mY, mOneMsqrtRho2, mNup1;
    LAUnivariateDistribution* mDensityDist;
    LAUnivariateDistribution* mCumulDist;
};

#endif
