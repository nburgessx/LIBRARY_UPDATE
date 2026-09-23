// aqMathSwaption.cpp

/*
 * @brief			Swig interface for the aqMathEuropeanIRSwaption* pricing and Greeks functions
 */

#include "aqMathSwaption.h"
#include "TypeUtilities.h"
#include "tryAqMathEuropeanIRSwaption.h"
#include "APISetUp.h"               // AQ_API_START and AQ_API_END Macros

double aqMathEuropeanIRSwaptionPrice( const std::string& payerReceiver,
                                       const double annuity,
                                       const double swapRate,
                                       const double strike,
                                       const double vol,
                                       const double time,
                                       const double shift,
                                       const std::string& volatilityType )
{
    AQ_API_START

    double result = validation::tryAqMathEuropeanIRSwaptionPrice(
        payerReceiver, annuity, swapRate, strike, vol, time, shift, volatilityType );
    return result;

    AQ_API_END
}

std::vector<double> aqMathEuropeanIRSwaptionPrices( const std::vector<std::string>& payerReceiver,
                                                     const std::vector<double>& annuity,
                                                     const std::vector<double>& swapRate,
                                                     const std::vector<double>& strike,
                                                     const std::vector<double>& vol,
                                                     const std::vector<double>& time,
                                                     const std::vector<double>& shift,
                                                     const std::vector<std::string>& volatilityType,
                                                     const bool optimize )
{
    AQ_API_START

    std::vector<double> result = validation::tryAqMathEuropeanIRSwaptionPrices(
        payerReceiver, annuity, swapRate, strike, vol, time, shift, volatilityType, optimize );
    return result;

    AQ_API_END
}

double aqMathEuropeanIRSwaptionImpliedVol( const double price,
                                            const std::string& payerReceiver,
                                            const double annuity,
                                            const double swapRate,
                                            const double strike,
                                            const double time,
                                            const double shift,
                                            const std::string& volatilityType )
{
    AQ_API_START

    double result = validation::tryAqMathEuropeanIRSwaptionImpliedVol(
        price, payerReceiver, annuity, swapRate, strike, time, shift, volatilityType );
    return result;

    AQ_API_END
}

std::vector<double> aqMathEuropeanIRSwaptionImpliedVols( const std::vector<double>& price,
                                                          const std::vector<std::string>& payerReceiver,
                                                          const std::vector<double>& annuity,
                                                          const std::vector<double>& swapRate,
                                                          const std::vector<double>& strike,
                                                          const std::vector<double>& time,
                                                          const std::vector<double>& shift,
                                                          const std::vector<std::string>& volatilityType,
                                                          const bool optimize )
{
    AQ_API_START

    std::vector<double> result = validation::tryAqMathEuropeanIRSwaptionImpliedVols(
        price, payerReceiver, annuity, swapRate, strike, time, shift, volatilityType, optimize );
    return result;

    AQ_API_END
}

double aqMathEuropeanIRSwaptionCashAnnuity( const double notional,
                                             const double swapRate,
                                             const unsigned int nCouponsPerYear,
                                             const double tenorInYears,
                                             const std::string& stubType )
{
    AQ_API_START

    double result = validation::tryAqMathEuropeanIRSwaptionCashAnnuity(
        notional, swapRate, nCouponsPerYear, tenorInYears, stubType );
    return result;

    AQ_API_END
}

double aqMathEuropeanIRSwaptionDelta( const std::string& payerReceiver,
                                       const double annuity,
                                       const double swapRate,
                                       const double strike,
                                       const double vol,
                                       const double time,
                                       const double shift,
                                       const std::string& volatilityType )
{
    AQ_API_START

    double result = validation::tryAqMathEuropeanIRSwaptionDelta(
        payerReceiver, annuity, swapRate, strike, vol, time, shift, volatilityType );
    return result;

    AQ_API_END
}

double aqMathEuropeanIRSwaptionGamma( const std::string& payerReceiver,
                                       const double annuity,
                                       const double swapRate,
                                       const double strike,
                                       const double vol,
                                       const double time,
                                       const double shift,
                                       const std::string& volatilityType )
{
    AQ_API_START

    double result = validation::tryAqMathEuropeanIRSwaptionGamma(
        payerReceiver, annuity, swapRate, strike, vol, time, shift, volatilityType );
    return result;

    AQ_API_END
}

double aqMathEuropeanIRSwaptionVega( const std::string& payerReceiver,
                                      const double annuity,
                                      const double swapRate,
                                      const double strike,
                                      const double vol,
                                      const double time,
                                      const double shift,
                                      const std::string& volatilityType )
{
    AQ_API_START

    double result = validation::tryAqMathEuropeanIRSwaptionVega(
        payerReceiver, annuity, swapRate, strike, vol, time, shift, volatilityType );
    return result;

    AQ_API_END
}

double aqMathEuropeanIRSwaptionTheta( const std::string& payerReceiver,
                                       const double annuity,
                                       const double swapRate,
                                       const double strike,
                                       const double vol,
                                       const double time,
                                       const double shift,
                                       const std::string& volatilityType )
{
    AQ_API_START

    double result = validation::tryAqMathEuropeanIRSwaptionTheta(
        payerReceiver, annuity, swapRate, strike, vol, time, shift, volatilityType );
    return result;

    AQ_API_END
}
