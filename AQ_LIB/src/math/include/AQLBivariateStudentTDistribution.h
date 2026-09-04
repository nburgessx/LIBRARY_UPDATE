/*! @file
    @brief Class declaration for bivariate Student t-distribution.
*/

// Improved performance and precision.
#ifndef AQLBivariateStudentTDistribution_h
#define AQLBivariateStudentTDistribution_h
#ifdef __GNUG__
#pragma interface
#else
#pragma warning( disable : 4290 )
#endif


#include "AQLProductDistribution.h"

//================ BivariateStudentTDistribution ===================================
class AQLBivariateStudentTDistribution : public AQLProductDistribution
{
public:
    AQLBivariateStudentTDistribution(double rho_, double nu_);
    AQLBivariateStudentTDistribution(double meanX_, double meanY_, double stDevX_, double stDevY_, double rho_,
                                    double nu_);

protected:
    AQLFunction* BivariateIntegrand(double x);

private:
    void Set();

    double mNu;
};

//================ BiStudentTIntegrand =================================================
class AQLBiStudentTIntegrand : public AQLFunction
{
public:
    AQLBiStudentTIntegrand(double rho_, double nu_, double y_);
    virtual ~AQLBiStudentTIntegrand();

    double operator()(double x) const;

private:
    double mRho, mNu, mY, mOneMsqrtRho2, mNup1;
    AQLUnivariateDistribution* mDensityDist;
    AQLUnivariateDistribution* mCumulDist;
};

#endif
