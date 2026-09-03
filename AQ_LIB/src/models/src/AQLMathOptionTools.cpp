#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include "AQLMathOptionTools.h"
#include "AQLAnalyticFormula.h"

//////////////////// Normal distribution /////////////////////////////////////////////////////////////////////////////////////////////////
double NormalDensity(double x)
{
    return ONE_OVER_SQRT_TWO_PI * exp(-0.5 * x * x);
}

//////////////////// Black ////////////////////////////////////////////////////////////////////////////////////////////////////////
double BlackPrice(double t, double strike, bool isCall, double fwd, double vol)
{
    double stDev = vol * sqrt(t);
    return BlackPrice(strike, isCall, fwd, stDev);
}

double BlackPrice(double strike, bool isCall, double fwd, double stDev)
{
    double dp = log(fwd / strike) / stDev + 0.5 * stDev;
    double dm = dp - stDev;
    if (isCall)
        return fwd * AQLDist::normsdist(dp) - strike * AQLDist::normsdist(dm);
    else
        return strike * AQLDist::normsdist(-dm) - fwd * AQLDist::normsdist(-dp);
}

double BlackImpliedVolatility(double premium, double t, double strike, bool isCall, double fwd,
                              double lowBound, double upBound, int maxIter, double tol)
{
    BlackPriceTarget target(premium, t, strike, isCall, fwd);
    return target.SolveBR(lowBound, upBound, maxIter, tol);
}

// Target function for Black implied volatility dataInstance search
BlackPriceTarget::BlackPriceTarget(double premium_, double t, double strike_, bool isCall_, double fwd_)
: premium(premium_), sqrtT(sqrt(t)), strike(strike_), isCall(isCall_), fwd(fwd_)
{
}

double BlackPriceTarget::operator()(double vol) const
{
    return BlackPrice(strike, isCall, fwd, vol * sqrtT) - premium;
}

//////////////////// Bachelier ////////////////////////////////////////////////////////////////////////////////////////////////////////
double BachelierPrice(double t, double strike, bool isCall, double fwd, double vol)
{
    double stDev = vol * sqrt(t);
    return BachelierPrice(strike, isCall, fwd, stDev);
}

double BachelierPrice(double strike, bool isCall, double fwd, double stDev)
{
    double FK = (isCall ? fwd - strike : strike - fwd);
    double e = FK / stDev;
    return FK * AQLDist::normsdist(e) + stDev * NormalDensity(e);
}

double BachelierImpliedVolatility(double premium, double t, double strike, bool isCall, double fwd,
                              double lowBound, double upBound, int maxIter, double tol)
{
    BachelierPriceTarget target(premium, t, strike, isCall, fwd);
    return target.SolveBR(lowBound, upBound, maxIter, tol);
}

// Target function for Bachelier implied volatility dataInstance search
BachelierPriceTarget::BachelierPriceTarget(double premium_, double t, double strike_, bool isCall_, double fwd_)
: premium(premium_), sqrtT(sqrt(t)), strike(strike_), isCall(isCall_), fwd(fwd_)
{
}

double BachelierPriceTarget::operator()(double vol) const
{
    return BachelierPrice(strike, isCall, fwd, vol * sqrtT) - premium;
}

