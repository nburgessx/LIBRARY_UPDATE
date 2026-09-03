#ifndef LAPriceCopulaCMSSpread_h
#define LAPriceCopulaCMSSpread_h

#ifdef __GNUG__
#pragma interface
#endif

#include <string>
#include <vector>
#include "LABivariateCopula.h"
#include "LAUnivariateDistribution.h"

using namespace std;

//================ CopulaCMSSpread ===================================
class LAPriceCopulaCMSSpread
{
public:
    LAPriceCopulaCMSSpread() { };

    LAPriceCopulaCMSSpread(double confidence_);

    /*! Option price knowing CMS Fwd and ATM call (calibration done inside) */
    double Price(double t, double K, bool isCall, double alpha, double beta,
                 double cmsFwd1, double cmsATMCall1, double cmsFwd2, double cmsATMCall2,
                 string marginalType, string copulaType, const vector<double>& copulaPars);

    /*! Option price knowing Gaussian marginal parameters */
    double Price(double t, double K, bool isCall, double alpha, double beta,
                 double cmsFwd1, double cmsFwd2, double vol1, double vol2,
                 string copulaType, const vector<double>& copulaPars);

private:
    /*! Option price knowing marginals */
    double Price(double alpha, double beta, double K, bool isCall,
                 LAUnivariateDistribution* marginal1, LAUnivariateDistribution* marginal2,
                 string copulaType, const vector<double>& copulaPars);

    /*! Call price knowing marginals */
    double Call(double alpha, double beta, double K,
                LAUnivariateDistribution* marginal1, LAUnivariateDistribution* marginal2,
                string copulaType, const vector<double>& copulaPars);

    LABivariateCopula* Copula(string type, const vector<double>& parameters);

    LAUnivariateDistribution* Calibrate(double t, double cmsFwd, double atmCall, string marginalType);

    double mConfidence;
};

class LAPriceCopulaSpreadUtility
{
public:

    static double V1Right(double S, LAUnivariateDistribution* marginal1, LAUnivariateDistribution* marginal2,
                          LABivariateCopula* copula, double alpha, double beta, double K);

    static double V1Left(double S, LAUnivariateDistribution* marginal1, LAUnivariateDistribution* marginal2,
                         LABivariateCopula* copula, double alpha, double beta, double K);

    static double V2Right(double S, LAUnivariateDistribution* marginal1, LAUnivariateDistribution* marginal2,
                          LABivariateCopula* copula, double alpha, double beta, double K);

    static double V2Left(double S, LAUnivariateDistribution* marginal1, LAUnivariateDistribution* marginal2,
                         LABivariateCopula* copula, double alpha, double beta, double K);

    static double FirstDiff(double S, LAUnivariateDistribution* marginal1, LAUnivariateDistribution* marginal2,
                            LABivariateCopula* copula, double alpha, double beta, double K);

    static double SecondDiff(double S, LAUnivariateDistribution* marginal1, LAUnivariateDistribution* marginal2,
                             LABivariateCopula* copula, double alpha, double beta, double K);
};

//================ VIntegrand ===================================
enum VType { v1Right, v1Left, v2Right, v2Left };

class MVVIntegrand : public LAFunction
{
public:
    MVVIntegrand(LAUnivariateDistribution* marginal1_, LAUnivariateDistribution* marginal2_,
                 LABivariateCopula* copula_, double alpha_, double beta_, double K_, VType vType_);

    double operator()(double x) const;

private:
    LAUnivariateDistribution* mMarginal1;
    LAUnivariateDistribution* mMarginal2;
    LABivariateCopula* mCopula;
    double mAlpha, mBeta, mK;
    VType mVType;
};

#endif 
