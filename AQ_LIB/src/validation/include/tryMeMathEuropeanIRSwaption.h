// tryMeMathEuropeanIRSwaption.h

/*
* @brief			validation interface for European IR Swaption method(s)
* @Created:		    06 December 2017
* @Author:			Nicholas Burgess
* @Department:		ISD Front Office Development
*
* The copyright to the computer program(s) herein is the property of Mizuho International.
*/

#pragma once
#include <string>
#include <vector>


namespace validation_api
{
    // Calculate the Price
    const double tryMeMathEuropeanIRSwaptionPrice( const std::string & payerReceiver,
                                                   const double & annuity,
                                                   const double & swapRate,
                                                   const double & strike,
                                                   const double & vol,
                                                   const double & time,
                                                   const double & shift = 0.0,
                                                   const std::string & volatilityType = "LOGNORMAL" );
    
    // Calculate the Price - optimize = true uses OMP threading
    const std::vector<double> tryMeMathEuropeanIRSwaptionPrices( const std::vector<std::string> & payerReceiver,
                                                                 const std::vector<double> & annuity,
                                                                 const std::vector<double> & swapRate,
                                                                 const std::vector<double> & strike,
                                                                 const std::vector<double> & vol,
                                                                 const std::vector<double> & time,
                                                                 const std::vector<double> & shift = std::vector<double>(),
                                                                 const std::vector<std::string> & volatilityType = std::vector<std::string>(),
                                                                 const bool optimize = false );

    // Calculate the tryMeMathEuropeanIRSwaptionImpliedVol
    const double tryMeMathEuropeanIRSwaptionImpliedVol( const double & price,
                                                        const std::string& payerReceiver,
                                                        const double & annuity,
                                                        const double & swapRate,
                                                        const double & strike,
                                                        const double & time,
                                                        const double & shift = 0.0,
                                                        const std::string & volatilityType = "LOGNORMAL" );

    // Calculate the tryMeMathEuropeanIRSwaptionImpliedVols - optimize = true uses OMP threading
    const std::vector<double> tryMeMathEuropeanIRSwaptionImpliedVols( const std::vector<double> & price,
                                                                      const std::vector<std::string> & payerReceiver,
                                                                      const std::vector<double> & annuity,
                                                                      const std::vector<double> & swapRate,
                                                                      const std::vector<double> & strike,
                                                                      const std::vector<double> & time,
                                                                      const std::vector<double> & shift = std::vector<double>(),
                                                                      const std::vector<std::string> & volatilityType = std::vector<std::string>(),
                                                                      const bool optimize = false );

    // Calculate the Cash Annuity
    const double tryMeMathEuropeanIRSwaptionCashAnnuity( const double & notional,
                                                         const double & swapRate,
                                                         const unsigned int & nCouponsPerYear,
                                                         const double & tenorInYears,
                                                         const std::string & stubType = "SHORT_START" );

    // Calculate the Delta
    const double tryMeMathEuropeanIRSwaptionDelta( const std::string & payerReceiver,
                                                   const double & annuity,
                                                   const double & swapRate,
                                                   const double & strike,
                                                   const double & vol,
                                                   const double & time,
                                                   const double & shift = 0.0,
                                                   const std::string & volatilityType = "LOGNORMAL" );

    // Calculate the Gamma
    const double tryMeMathEuropeanIRSwaptionGamma( const std::string & payerReceiver,
                                                   const double & annuity,
                                                   const double & swapRate,
                                                   const double & strike,
                                                   const double & vol,
                                                   const double & time,
                                                   const double & shift = 0.0,
                                                   const std::string & volatilityType = "LOGNORMAL" );

    // Calculate the Vega
    const double tryMeMathEuropeanIRSwaptionVega( const std::string & payerReceiver,
                                                   const double & annuity,
                                                   const double & swapRate,
                                                   const double & strike,
                                                   const double & vol,
                                                   const double & time,
                                                   const double & shift = 0.0,
                                                   const std::string & volatilityType = "LOGNORMAL" );

    // Calculate the Theta
    const double tryMeMathEuropeanIRSwaptionTheta( const std::string & payerReceiver,
                                                   const double & annuity,
                                                   const double & swapRate,
                                                   const double & strike,
                                                   const double & vol,
                                                   const double & time,
                                                   const double & shift = 0.0,
                                                   const std::string & volatilityType = "LOGNORMAL" );

}
