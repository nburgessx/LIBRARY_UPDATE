#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include "LAPriceCMSSpreadFormula.h"
#include "LAMathOptionTools.h"

double LAPriceCMSSpreadFormula::Price(double t, double K, bool isCall, double alpha, double beta,
                                 double S1, double S2, double vol1, double vol2, double rho)
{
    LAPriceCMSSpreadCallPricer pricer;
    if (isCall)
        pricer = LAPriceCMSSpreadCallPricer(t, K, alpha, beta, S1, S2, vol1, vol2);
    else
        pricer = LAPriceCMSSpreadCallPricer(t, -K, -alpha, -beta, S1, S2, vol1, vol2);

    return pricer.Price(rho);
}

double LAPriceCMSSpreadFormula::ImpliedCorrelation(double fwdPrice, double t, double K, bool isCall, double alpha, double beta,
                                              double S1, double S2, double vol1, double vol2)
{
    LAPriceCMSSpreadCallPricer pricer;
    if (isCall)
        pricer = LAPriceCMSSpreadCallPricer(t, K, alpha, beta, S1, S2, vol1, vol2);
    else
        pricer = LAPriceCMSSpreadCallPricer(t, -K, -alpha, -beta, S1, S2, vol1, vol2);

    pricer.SetPrice(fwdPrice);
    double lwBound = -0.9999, upBound = 0.9999, tol = 1e-8;
    size_t maxIter = 1000;
    return pricer.SolveBR(lwBound, upBound, maxIter, tol);
}

LAPriceCMSSpreadCallPricer::LAPriceCMSSpreadCallPricer(double t_, double K_, double alpha_, double beta_, double S1_, double S2_,
                                             double vol1_, double vol2_)
{
    mT = t_; mK = K_; mAlpha = alpha_; mBeta = beta_; mS1 = S1_; mS2 = S2_; mVol1 = vol1_; mVol2 = vol2_;
    double sqrtt = sqrt(mT);
    double stDev1 = mAlpha * mVol1 * sqrtt;
    double stDev2 = mBeta * mVol2 * sqrtt;
    mCrossV = stDev1 * stDev2;
    mZeroCorrV = pow(stDev1, 2) + pow(stDev2, 2);
    mFwd = mAlpha * mS1 - mBeta * mS2;
}

double LAPriceCMSSpreadCallPricer::operator()(double x) const
{
    return Price(x) - mFwdPrice;
}

double LAPriceCMSSpreadCallPricer::Price(double rho) const
{
    double sigma = sqrt(mZeroCorrV - 2.0 * rho * mCrossV);
    return BachelierPrice(mK, true, mFwd, sigma);
    //double d = mPayoff / sigma;
    //double term1 = d * AQLDist::normsdist(d);
    //double term2 = exp(-0.5 * pow(d, 2)) / Maths.Constant.C_SQRT2PI;
    //return sigma * (term1 + term2);
}
