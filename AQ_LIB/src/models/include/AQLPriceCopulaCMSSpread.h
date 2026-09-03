#ifndef AQLPriceCopulaCMSSpread_h
#define AQLPriceCopulaCMSSpread_h

#ifdef __GNUG__
#pragma interface
#endif

#include <string>
#include <vector>
#include "AQLBivariateCopula.h"
#include "AQLUnivariateDistribution.h"

using namespace std;

//================ CopulaCMSSpread ===================================
class AQLPriceCopulaCMSSpread
{
public:
    AQLPriceCopulaCMSSpread() { };

    AQLPriceCopulaCMSSpread(double confidence_);

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
                 AQLUnivariateDistribution* marginal1, AQLUnivariateDistribution* marginal2,
                 string copulaType, const vector<double>& copulaPars);

    /*! Call price knowing marginals */
    double Call(double alpha, double beta, double K,
                AQLUnivariateDistribution* marginal1, AQLUnivariateDistribution* marginal2,
                string copulaType, const vector<double>& copulaPars);

    AQLBivariateCopula* Copula(string type, const vector<double>& parameters);

    AQLUnivariateDistribution* Calibrate(double t, double cmsFwd, double atmCall, string marginalType);

    double mConfidence;
};

class AQLPriceCopulaSpreadUtility
{
public:

    static double V1Right(double S, AQLUnivariateDistribution* marginal1, AQLUnivariateDistribution* marginal2,
                          AQLBivariateCopula* copula, double alpha, double beta, double K);

    static double V1Left(double S, AQLUnivariateDistribution* marginal1, AQLUnivariateDistribution* marginal2,
                         AQLBivariateCopula* copula, double alpha, double beta, double K);

    static double V2Right(double S, AQLUnivariateDistribution* marginal1, AQLUnivariateDistribution* marginal2,
                          AQLBivariateCopula* copula, double alpha, double beta, double K);

    static double V2Left(double S, AQLUnivariateDistribution* marginal1, AQLUnivariateDistribution* marginal2,
                         AQLBivariateCopula* copula, double alpha, double beta, double K);

    static double FirstDiff(double S, AQLUnivariateDistribution* marginal1, AQLUnivariateDistribution* marginal2,
                            AQLBivariateCopula* copula, double alpha, double beta, double K);

    static double SecondDiff(double S, AQLUnivariateDistribution* marginal1, AQLUnivariateDistribution* marginal2,
                             AQLBivariateCopula* copula, double alpha, double beta, double K);
};

//================ VIntegrand ===================================
enum VType { v1Right, v1Left, v2Right, v2Left };

class AQLVIntegrand : public AQLFunction
{
public:
    AQLVIntegrand(AQLUnivariateDistribution* marginal1_, AQLUnivariateDistribution* marginal2_,
                 AQLBivariateCopula* copula_, double alpha_, double beta_, double K_, VType vType_);

    double operator()(double x) const;

private:
    AQLUnivariateDistribution* mMarginal1;
    AQLUnivariateDistribution* mMarginal2;
    AQLBivariateCopula* mCopula;
    double mAlpha, mBeta, mK;
    VType mVType;
};

#endif 
