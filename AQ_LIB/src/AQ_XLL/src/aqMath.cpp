#include <aqMain.h>
#include "aqXllTools.h"

// include
#include "CoreEnumerations.h"
#include "tryAqMathNormalDistribution.h"
#include "tryAqMathBlackScholes.h"


// Black-Scholes Price Method
XLO_FUNC_START(aqMathBlackScholesPrice(
    const ExcelObj& callOrPut,
    const ExcelObj& spot,
    const ExcelObj& strike,
    const ExcelObj& vol,
    const ExcelObj& time,
    const ExcelObj& rate,
    const ExcelObj& carry,
    const ExcelObj& shift))
{
    // Carry and Shift are optional and default to zero when omitted
    const double carryArg = carry.isMissing() ? 0.0 : carry.get<double>();
    const double shiftArg = shift.isMissing() ? 0.0 : shift.get<double>();

    CallOrPutEnum cp = etrading::toCallOrPutEnum(aq_xll::toNarrowString(callOrPut));

    double result = validation::tryAqMathBlackScholesPrice(
        cp,
        spot.get<double>(),
        strike.get<double>(),
        vol.get<double>(),
        time.get<double>(),
        rate.get<double>(),
        carryArg,
        shiftArg);

    return returnValue(result);
}
XLO_FUNC_END(aqMathBlackScholesPrice)
.help(L"Function to calculate the Black-Scholes Price")
.arg(L"CallOrPut", L"Call or Put")
.arg(L"Spot", L"Spot")
.arg(L"Strike", L"Strike")
.arg(L"Vol", L"Volatility (%)")
.arg(L"Time", L"Time to Expiry in Years")
.arg(L"Rate", L"Interest Rate (%)")
.arg(L"Carry", L"Cost of Carry (%) - Defaults to Zero")
.arg(L"Shift", L"[Optional] Apply a lognormal shift, defaults to zero");