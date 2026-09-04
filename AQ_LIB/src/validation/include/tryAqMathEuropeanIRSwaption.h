// tryAqMathEuropeanIRSwaption.h

/*
* @brief			validation interface for European IR Swaption method(s)
*/

#pragma once
#include <string>
#include <vector>


namespace validation
{
    // Calculate the Price
    const double tryAqMathEuropeanIRSwaptionPrice( const std::string & payerReceiver,
                                                   const double & annuity,
                                                   const double & swapRate,
                                                   const double & strike,
                                                   const double & vol,
                                                   const double & time,
                                                   const double & shift = 0.0,
                                                   const std::string & volatilityType = "LOGNORMAL" );
    
    // Calculate the Price - optimize = true uses OMP threading
    const std::vector<double> tryAqMathEuropeanIRSwaptionPrices( const std::vector<std::string> & payerReceiver,
                                                                 const std::vector<double> & annuity,
                                                                 const std::vector<double> & swapRate,
                                                                 const std::vector<double> & strike,
                                                                 const std::vector<double> & vol,
                                                                 const std::vector<double> & time,
                                                                 const std::vector<double> & shift = std::vector<double>(),
                                                                 const std::vector<std::string> & volatilityType = std::vector<std::string>(),
                                                                 const bool optimize = false );

    // Calculate the tryAqMathEuropeanIRSwaptionImpliedVol
    const double tryAqMathEuropeanIRSwaptionImpliedVol( const double & price,
                                                        const std::string& payerReceiver,
                                                        const double & annuity,
                                                        const double & swapRate,
                                                        const double & strike,
                                                        const double & time,
                                                        const double & shift = 0.0,
                                                        const std::string & volatilityType = "LOGNORMAL" );

    // Calculate the tryAqMathEuropeanIRSwaptionImpliedVols - optimize = true uses OMP threading
    const std::vector<double> tryAqMathEuropeanIRSwaptionImpliedVols( const std::vector<double> & price,
                                                                      const std::vector<std::string> & payerReceiver,
                                                                      const std::vector<double> & annuity,
                                                                      const std::vector<double> & swapRate,
                                                                      const std::vector<double> & strike,
                                                                      const std::vector<double> & time,
                                                                      const std::vector<double> & shift = std::vector<double>(),
                                                                      const std::vector<std::string> & volatilityType = std::vector<std::string>(),
                                                                      const bool optimize = false );

    // Calculate the Cash Annuity
    const double tryAqMathEuropeanIRSwaptionCashAnnuity( const double & notional,
                                                         const double & swapRate,
                                                         const unsigned int & nCouponsPerYear,
                                                         const double & tenorInYears,
                                                         const std::string & stubType = "SHORT_START" );

    // Calculate the Delta
    const double tryAqMathEuropeanIRSwaptionDelta( const std::string & payerReceiver,
                                                   const double & annuity,
                                                   const double & swapRate,
                                                   const double & strike,
                                                   const double & vol,
                                                   const double & time,
                                                   const double & shift = 0.0,
                                                   const std::string & volatilityType = "LOGNORMAL" );

    // Calculate the Gamma
    const double tryAqMathEuropeanIRSwaptionGamma( const std::string & payerReceiver,
                                                   const double & annuity,
                                                   const double & swapRate,
                                                   const double & strike,
                                                   const double & vol,
                                                   const double & time,
                                                   const double & shift = 0.0,
                                                   const std::string & volatilityType = "LOGNORMAL" );

    // Calculate the Vega
    const double tryAqMathEuropeanIRSwaptionVega( const std::string & payerReceiver,
                                                   const double & annuity,
                                                   const double & swapRate,
                                                   const double & strike,
                                                   const double & vol,
                                                   const double & time,
                                                   const double & shift = 0.0,
                                                   const std::string & volatilityType = "LOGNORMAL" );

    // Calculate the Theta
    const double tryAqMathEuropeanIRSwaptionTheta( const std::string & payerReceiver,
                                                   const double & annuity,
                                                   const double & swapRate,
                                                   const double & strike,
                                                   const double & vol,
                                                   const double & time,
                                                   const double & shift = 0.0,
                                                   const std::string & volatilityType = "LOGNORMAL" );

}
