#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include "LAPriceCopulaCMSSpread.h"
#include "LABivariateGaussianCopula.h"
#include "LABivariateStudentTCopula.h"
#include "LABivariatePowerCopula.h"
#include "LAGaussLegendre.h"
#include "LANormalDistribution.h"
#include "LAMathAnalyticalFormula.h"

//================ CopulaCMSSpread ===================================
LAPriceCopulaCMSSpread::LAPriceCopulaCMSSpread(double confidence_)
{
    mConfidence = confidence_;
}

double LAPriceCopulaCMSSpread::Price(double t, double K, bool isCall, double alpha, double beta,
                                double cmsFwd1, double cmsATMCall1, double cmsFwd2, double cmsATMCall2,
                                string marginalType, string copulaType, const vector<double>& copulaPars)
{
    // Calibrate marginals
    LAUnivariateDistribution* marginal1 = 0;
    LAUnivariateDistribution* marginal2 = 0;

    if (marginalType == "Gaussian")
    {
        marginal1 = Calibrate(t, cmsFwd1, cmsATMCall1, marginalType);
        marginal2 = Calibrate(t, cmsFwd2, cmsATMCall2, marginalType);
    }
    else
        throw LACoreInvalidData(("Unknown marginal type: " + marginalType).c_str(), __FILE__, __LINE__);

    double price = Price(alpha, beta, K, isCall, marginal1, marginal2, copulaType, copulaPars);

    delete marginal1;
    delete marginal2;
    return price;
}

double LAPriceCopulaCMSSpread::Price(double t, double K, bool isCall, double alpha, double beta,
                                double cmsFwd1, double cmsFwd2, double vol1, double vol2,
                                string copulaType, const vector<double>& copulaPars)
{
    // Create Gaussian marginals
    double sqrtt = sqrt(t);
    LAUnivariateDistribution* marginal1 = new LANormalDistribution(cmsFwd1, vol1 * sqrtt);
    LAUnivariateDistribution* marginal2 = new LANormalDistribution(cmsFwd2, vol2 * sqrtt);

    double price = Price(alpha, beta, K, isCall, marginal1, marginal2, copulaType, copulaPars);

    delete marginal1;
    delete marginal2;
    return price;
}

double LAPriceCopulaCMSSpread::Price(double alpha, double beta, double K, bool isCall,
                                LAUnivariateDistribution* marginal1, LAUnivariateDistribution* marginal2,
                                string copulaType, const vector<double>& copulaPars)
{
    double price;
    if (isCall)
        price = Call(alpha, beta, K, marginal1, marginal2, copulaType, copulaPars);
    else
        price = Call(-alpha, -beta, -K, marginal1, marginal2, copulaType, copulaPars);

    return price;
}

double LAPriceCopulaCMSSpread::Call(double alpha, double beta, double K,
                               LAUnivariateDistribution* marginal1, LAUnivariateDistribution* marginal2,
                               string copulaType, const vector<double>& copulaPars)
{
    LABivariateCopula* copula = Copula(copulaType, copulaPars);

    // Pricing
    int nPoints = 16, nSplits = 10;
    LAGaussLegendre integrator(nPoints);
    double lwBound, upBound;
    // V1
    LAFunction* v1Integrand;
    if (beta > 0.0)
        v1Integrand = new MVVIntegrand(marginal1, marginal2, copula, alpha, beta, K, v1Right);
    else
        v1Integrand = new MVVIntegrand(marginal1, marginal2, copula, alpha, beta, K, v1Left);

    lwBound = marginal1->Bound(mConfidence, Lower);
    upBound = marginal1->Bound(mConfidence, Upper);
    double V1 = integrator.IntegrateByEqualIntervals(*v1Integrand, lwBound, upBound, nSplits);
    // V2
    LAFunction* v2Integrand;
    if (alpha > 0.0)
        v2Integrand = new MVVIntegrand(marginal1, marginal2, copula, alpha, beta, K, v2Right);
    else
        v2Integrand = new MVVIntegrand(marginal1, marginal2, copula, alpha, beta, K, v2Left);

    lwBound = marginal2->Bound(mConfidence, Lower);
    upBound = marginal2->Bound(mConfidence, Upper);
    double V2 = integrator.IntegrateByEqualIntervals(*v2Integrand, lwBound, upBound, nSplits);

    double callPrice = V1 - beta * V2;

    delete copula;
    delete v1Integrand;
    delete v2Integrand;

    return callPrice;
}

LAUnivariateDistribution* LAPriceCopulaCMSSpread::Calibrate(double t, double cmsFwd, double atmCall, string marginalType)
{
    LAUnivariateDistribution* marginal;
    if (marginalType == "Gaussian")
    {
        double mean = cmsFwd;
        LAMathAnalyticalFormula formula;
        double normalStDev = formula.BlackImplVolDD(atmCall, cmsFwd, cmsFwd, 1, 0);
        marginal = new LANormalDistribution(mean, normalStDev);
    }
    else
        throw LACoreInvalidData(("Unknown marginal type: " + marginalType).c_str(), __FILE__, __LINE__);

    return marginal;
}

LABivariateCopula* LAPriceCopulaCMSSpread::Copula(string type, const vector<double>& parameters)
{
    LABivariateCopula* copula;
    if (type == "Gaussian")
        copula = new LABivariateGaussianCopula(parameters[0]);
    else if (type == "StudentT")
        copula = new LABivariateStudentTCopula(parameters[0], parameters[1]);
    else if (type == "PowerGaussian")
        copula = new LABivariatePowerGaussianCopula(parameters[0], parameters[1], parameters[2]);
    else if (type == "PowerStudentT")
        copula = new LABivariatePowerStudentTCopula(parameters[0], parameters[1], parameters[2], parameters[3]);
    else
        throw LACoreInvalidData(("Unknown copula type: " + type).c_str(), __FILE__, __LINE__);

    return copula;
}

//================ Non-member helpers ===================================
double LAPriceCopulaSpreadUtility::V1Right(double S, LAUnivariateDistribution* marginal1, LAUnivariateDistribution* marginal2,
                                      LABivariateCopula* copula, double alpha, double beta, double K)
{
    double term1 = alpha * S - K;
    double term2 = marginal1->Density(S);
    double term3 = FirstDiff(S, marginal1, marginal2, copula, alpha, beta, K);
    return term1 * term2 * term3;
}

double LAPriceCopulaSpreadUtility::V1Left(double S, LAUnivariateDistribution* marginal1, LAUnivariateDistribution* marginal2,
                                     LABivariateCopula* copula, double alpha, double beta, double K)
{
    double term1 = alpha * S - K;
    double term2 = marginal1->Density(S);
    double term3 = 1.0 - FirstDiff(S, marginal1, marginal2, copula, alpha, beta, K);
    return term1 * term2 * term3;
}

double LAPriceCopulaSpreadUtility::V2Right(double S, LAUnivariateDistribution* marginal1, LAUnivariateDistribution* marginal2,
                                      LABivariateCopula* copula, double alpha, double beta, double K)
{
    double term1 = S;
    double term2 = marginal2->Density(S);
    double term3 = (1.0 - SecondDiff(S, marginal1, marginal2, copula, alpha, beta, K));
    return term1 * term2 * term3;
}

double LAPriceCopulaSpreadUtility::V2Left(double S, LAUnivariateDistribution* marginal1, LAUnivariateDistribution* marginal2,
                                     LABivariateCopula* copula, double alpha, double beta, double K)
{
    double term1 = S;
    double term2 = marginal2->Density(S);
    double term3 = SecondDiff(S, marginal1, marginal2, copula, alpha, beta, K);
    return term1 * term2 * term3;
}

double LAPriceCopulaSpreadUtility::FirstDiff(double S, LAUnivariateDistribution* marginal1, LAUnivariateDistribution* marginal2,
                                        LABivariateCopula* copula, double alpha, double beta, double K)
{
    double f1 = marginal1->Cumulative(S);
    double f2 = marginal2->Cumulative((alpha * S - K) / beta);
    return copula->Diff(f1, f2, First);
}

double LAPriceCopulaSpreadUtility::SecondDiff(double S, LAUnivariateDistribution* marginal1, LAUnivariateDistribution* marginal2,
                                         LABivariateCopula* copula, double alpha, double beta, double K)
{
    double f1 = marginal1->Cumulative((beta * S + K) / alpha);
    double f2 = marginal2->Cumulative(S);
    return copula->Diff(f1, f2, Second);
}

//================ VIntegrand ===================================
MVVIntegrand::MVVIntegrand(LAUnivariateDistribution* marginal1_, LAUnivariateDistribution* marginal2_,
                           LABivariateCopula* copula_, double alpha_, double beta_, double K_, VType vType_)
{
    mMarginal1 = marginal1_;
    mMarginal2 = marginal2_;
    mCopula = copula_;
    mAlpha = alpha_;
    mBeta = beta_;
    mK = K_;
    mVType = vType_;
}

double MVVIntegrand::operator()(double x) const
{
    double v;
    if (mVType == v1Right)
        v = LAPriceCopulaSpreadUtility::V1Right(x, mMarginal1, mMarginal2, mCopula, mAlpha, mBeta, mK);
    else if (mVType == v1Left)
        v = LAPriceCopulaSpreadUtility::V1Left(x, mMarginal1, mMarginal2, mCopula, mAlpha, mBeta, mK);
    else if (mVType == v2Right)
        v = LAPriceCopulaSpreadUtility::V2Right(x, mMarginal1, mMarginal2, mCopula, mAlpha, mBeta, mK);
    else if (mVType == v2Left)
        v = LAPriceCopulaSpreadUtility::V2Left(x, mMarginal1, mMarginal2, mCopula, mAlpha, mBeta, mK);
    else
        throw LACoreInvalidData("Unknown integrand type in CMS Spread copula", __FILE__, __LINE__);

    return v;
}
