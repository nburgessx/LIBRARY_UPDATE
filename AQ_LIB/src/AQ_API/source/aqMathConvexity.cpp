// aqMathConvexity.cpp

/*
 * @brief			Swig interface for the aqMathLibor* convexity-adjustment functions
 */

#include "aqMathConvexity.h"
#include "CoreEnumerations.h"        // etrading::toVolatilityTypeEnum
#include "TypeUtilities.h"
#include "tryAqMathConvexity.h"
#include "APISetUp.h"               // AQ_API_START and AQ_API_END Macros

double aqMathLiborConvexityAdjustmentInArrears( const double liborRate,
                                                 const double couponYearFraction,
                                                 const double timeToMaturity,
                                                 const double volatility,
                                                 const std::string& volatilityType,
                                                 const double volatilityShift,
                                                 const bool useHullApproximation )
{
    AQ_API_START

    double result = validation::tryAqMathLiborConvexityAdjustmentInArrears(
        liborRate, couponYearFraction, timeToMaturity, volatility,
        etrading::toVolatilityTypeEnum( volatilityType ), volatilityShift, useHullApproximation );
    return result;

    AQ_API_END
}

double aqMathLiborRateInArrears( const double liborRate,
                                  const double couponYearFraction,
                                  const double timeToMaturity,
                                  const double volatility,
                                  const std::string& volatilityType,
                                  const double volatilityShift,
                                  const bool useHullApproximation )
{
    AQ_API_START

    double result = validation::tryAqMathLiborRateInArrears(
        liborRate, couponYearFraction, timeToMaturity, volatility,
        etrading::toVolatilityTypeEnum( volatilityType ), volatilityShift, useHullApproximation );
    return result;

    AQ_API_END
}

double aqMathLiborConvexityAdjustmentForArbitraryFixingDate( const double liborRate,
                                                               const double couponYearFraction,
                                                               const double timeToMaturity,
                                                               const double fixingDateYearFraction,
                                                               const double startDiscountFactor,
                                                               const double endDiscountFactor,
                                                               const double volatility,
                                                               const std::string& volatilityType,
                                                               const double volatilityShift,
                                                               const bool useHullApproximation )
{
    AQ_API_START

    double result = validation::tryAqMathLiborConvexityAdjustmentForArbitraryFixingDate(
        liborRate, couponYearFraction, timeToMaturity, fixingDateYearFraction,
        startDiscountFactor, endDiscountFactor, volatility,
        etrading::toVolatilityTypeEnum( volatilityType ), volatilityShift, useHullApproximation );
    return result;

    AQ_API_END
}

double aqMathLiborRateForArbitraryFixingDate( const double liborRate,
                                               const double couponYearFraction,
                                               const double timeToMaturity,
                                               const double fixingDateYearFraction,
                                               const double startDiscountFactor,
                                               const double endDiscountFactor,
                                               const double volatility,
                                               const std::string& volatilityType,
                                               const double volatilityShift,
                                               const bool useHullApproximation )
{
    AQ_API_START

    double result = validation::tryAqMathLiborRateForArbitraryFixingDate(
        liborRate, couponYearFraction, timeToMaturity, fixingDateYearFraction,
        startDiscountFactor, endDiscountFactor, volatility,
        etrading::toVolatilityTypeEnum( volatilityType ), volatilityShift, useHullApproximation );
    return result;

    AQ_API_END
}
