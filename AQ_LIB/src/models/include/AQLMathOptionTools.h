#ifndef AQLMathOptionTools_h
#define AQLMathOptionTools_h

#ifdef __GNUG__
#pragma interface
#endif
//////////
#include "AQLFunction.h"
//////////////////////////

//using namespace std;

//////////////////// Normal distribution /////////////////////////////////////////////////////////////////////////////////////////////////
double NormalDensity(double x);

//////////////////// Black ////////////////////////////////////////////////////////////////////////////////////////////////////////
// Black price from time and volatility
double BlackPrice(double t, double strike, bool isCall, double fwd, double vol);

// Black price from standard deviation (save time in Black implied volatility)
double BlackPrice(double strike, bool isCall, double fwd, double stDev);

// Black implied volatility, i.e. the volatility that solves the input (forward) premium.
double BlackImpliedVolatility(double premium, double t, double strike, bool isCall, double fwd,
                              double lowBound = 0.000001, double upBound = 10.0, int maxIter = 1000000, double tol = 0.0000001);

// Target function to solve in order to find the Black-implied volatility.
class BlackPriceTarget : public AQLFunction
{
public:
    BlackPriceTarget(double premium_, double t, double strike_, bool isCall_, double fwd_);
    double operator()(double x) const;

private:
    double sqrtT, premium, fwd, strike;
    bool isCall;
};

//////////////////// Bachelier ////////////////////////////////////////////////////////////////////////////////////////////////////////
// Bachelier price from time and volatility
double BachelierPrice(double t, double strike, bool isCall, double fwd, double vol);

// Bachelier price from standard deviation (save time in Black implied volatility)
double BachelierPrice(double strike, bool isCall, double fwd, double stDev);

// Bachelier implied volatility, i.e. the volatility that solves the input (forward) premium.
double BachelierImpliedVolatility(double premium, double t, double strike, bool isCall, double fwd,
                                  double lowBound = 0.00000001, double upBound = 5.0, int maxIter = 1000000, double tol = 0.000000001);

// Target function to solve in order to find the Bachelier-implied volatility.
class BachelierPriceTarget : public AQLFunction
{
public:
    BachelierPriceTarget(double premium_, double t, double strike_, bool isCall_, double fwd_);
    double operator()(double x) const;

private:
    double sqrtT, premium, fwd, strike;
    bool isCall;
};

#endif 
