#ifndef LAPriceCMSSpreadFormula_h
#define LAPriceCMSSpreadFormula_h

#ifdef __GNUG__
#pragma interface
#endif

#include "LAFunction.h"

class LAPriceCMSSpreadFormula
{
public:
    static double Price(double t, double K, bool isCall, double alpha, double beta,
                        double S1, double S2, double vol1, double vol2, double rho);

    static double ImpliedCorrelation(double fwdPrice, double t, double K, bool isCall, double alpha, double beta,
                                     double S1, double S2, double vol1, double vol2);
};

class LAPriceCMSSpreadCallPricer : public LAFunction
{
public:
    LAPriceCMSSpreadCallPricer() {}
    LAPriceCMSSpreadCallPricer(double t_, double K_, double alpha_, double beta_, double S1_, double S2_, double vol1_, double vol2_);

    double Price(double rho) const;
    double operator()(double rho) const;
    void SetPrice(double fwdPrice) { mFwdPrice = fwdPrice; }

private:
    double mT, mK, mAlpha, mBeta, mS1, mS2, mVol1, mVol2;
    double mZeroCorrV, mCrossV, mFwd, mFwdPrice;
};

#endif 
