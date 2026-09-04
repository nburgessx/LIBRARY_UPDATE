// tryAqMathEuropeanIRSwaption.cpp

/*
* @brief			validation interface for European IR Swaption method(s)
* @Created:		11 August 2017
* @Author:			Nicholas Burgess
* @Department:		ISD Front Office Development
*
* The copyright to the computer program(s) herein is the property of AlgoQuantHub.
*/

#include "tryAqMathEuropeanIRSwaption.h"
#include "EuropeanIRSwaption.h"
#include "BlackScholes.h"

// Includes for Structured Exception Handling and Input/Output Logging
#include "CreateDataFile.h"
#include "StructuredExceptionHandler.h"
#include "CommonConstants.h"
#include "RecordMacros.h"
#include <omp.h>

// Namespaces for Input/Output Logging
using etrading::CreateDataFile;
using etrading::decorateFilename;

// EuropeanIRSwaption Namespace
using etrading::EuropeanIRSwaption;

// Use typedef for etrading enumerator types
typedef etrading::PayerReceiverSwaptionEnum PayerReceiverSwaptionEnum;
typedef etrading::VolatilityTypeEnum VolatilityTypeEnum;
typedef etrading::StubTypeEnum StubTypeEnum;

namespace validation
{

    // Calculate the Price
    const double tryAqMathEuropeanIRSwaptionPrice( const std::string& payerReceiver,
                                                   const double & annuity,
                                                   const double & swapRate,
                                                   const double & strike,
                                                   const double & vol,
                                                   const double & time,
                                                   const double & shift,
                                                   const std::string & volatilityType )
    {
        VALID_EXCEPTION_START

        // Record Inputs for logs, tests and playback
        RECORD_INPUTS( payerReceiver, annuity, swapRate, strike, vol, time, shift, volatilityType );

        // Calculation
        PayerReceiverSwaptionEnum payerReceiverEnum = etrading::toPayerReceiverSwaptionEnum( payerReceiver );
        VolatilityTypeEnum VolatilityTypeEnum = etrading::toVolatilityTypeEnum( volatilityType );
        const double result = EuropeanIRSwaption::price( payerReceiverEnum, annuity, swapRate, strike, vol, time, shift, VolatilityTypeEnum );

        // Record Outputs AND Return the Result for logs, tests and playback
        RECORD_OUTPUTS_AND_RETURN_RESULT( result );

        VALID_EXCEPTION_END
    }

    // Calculate the Price - optimize = true uses OMP threading
    const std::vector<double> tryAqMathEuropeanIRSwaptionPrices( const std::vector<std::string> & payerReceiver,
                                                                 const std::vector<double> & annuity,
                                                                 const std::vector<double> & swapRate,
                                                                 const std::vector<double> & strike,
                                                                 const std::vector<double> & vol,
                                                                 const std::vector<double> & time,
                                                                 const std::vector<double> & shift,
                                                                 const std::vector<std::string> & volatilityType,
                                                                 const bool optimize )
    {
        VALID_EXCEPTION_START

        // The Volatility Type and Shift Parameters are optional
        const bool noVolType = ( volatilityType.size() == 0 ) ? true : false;
        const bool noShift = ( shift.size() == 0 ) ? true : false;
        
        // Dimension Validation
        AQ_REQUIRE( payerReceiver.size() == swapRate.size(),              "Invalid dimensions: Number of payerReceiver parameters must equal the number of swapRate parameters." );
        AQ_REQUIRE( payerReceiver.size() == annuity.size(),               "Invalid dimensions: Number of payerReceiver parameters must equal the number of annuity parameters." );
        AQ_REQUIRE( payerReceiver.size() == strike.size(),                "Invalid dimensions: Number of payerReceiver parameters must equal the number of strike parameters." );
        AQ_REQUIRE( payerReceiver.size() == vol.size(),                   "Invalid dimensions: Number of payerReceiver parameters must equal the number of vol parameters." );
        AQ_REQUIRE( payerReceiver.size() == time.size(),                  "Invalid dimensions: Number of payerReceiver parameters must equal the number of time parameters." );
        AQ_REQUIRE( noVolType || payerReceiver.size() == volatilityType.size(), "Invalid dimensions: Number of payerReceiver parameters must equal the number of volatilityType parameters." );
        AQ_REQUIRE( noShift || payerReceiver.size() == shift.size(),      "Invalid dimensions: Number of payerReceiver parameters must equal the number of shift parameters." );
        
        // Declare Results Vector
        std::vector<double> results( payerReceiver.size(), 0.0 );

        // Optimize uses OMP threading
        if ( optimize )
        {
            // Find the maximum number of hardware threads (independent cores) on this machine
            const int nThreads = omp_get_max_threads();

            // Calculation
            #pragma omp parallel for num_threads( nThreads )
            for ( int i = 0; i < ( int )( payerReceiver.size() ); ++i ) // OMP uses int loops, not size_t
            {
                PayerReceiverSwaptionEnum payerReceiverEnum = etrading::toPayerReceiverSwaptionEnum( payerReceiver[i] );
                VolatilityTypeEnum volatilityTypeEnum =  noVolType ? etrading::LOGNORMAL_VOLATILITY : etrading::toVolatilityTypeEnum( volatilityType[i] );
                std::shared_ptr<EuropeanIRSwaption> euroSwaption( new EuropeanIRSwaption( payerReceiverEnum, annuity[i], swapRate[i], strike[i], vol[i], time[i], noShift ? 0.0 : shift[i], volatilityTypeEnum ) );
                results[i] = euroSwaption->price();
            }
        }
        else
        {
            for ( size_t i = 0; i < payerReceiver.size(); ++i )
            {
                PayerReceiverSwaptionEnum payerReceiverEnum = etrading::toPayerReceiverSwaptionEnum( payerReceiver[i] );
                VolatilityTypeEnum volatilityTypeEnum =  noVolType ? etrading::LOGNORMAL_VOLATILITY : etrading::toVolatilityTypeEnum( volatilityType[i] );
                std::shared_ptr<EuropeanIRSwaption> euroSwaption( new EuropeanIRSwaption( payerReceiverEnum, annuity[i], swapRate[i], strike[i], vol[i], time[i], noShift ? 0.0 : shift[i], volatilityTypeEnum ) );
                results[i] = euroSwaption->price();
            }
        }

        return results;

        VALID_EXCEPTION_END
    }

    // Calculate the tryAqMathEuropeanIRSwaptionImpliedVol
    const double tryAqMathEuropeanIRSwaptionImpliedVol( const double & price,
                                                        const std::string& payerReceiver,
                                                        const double & annuity,
                                                        const double & swapRate,
                                                        const double & strike,
                                                        const double & time,
                                                        const double & shift,
                                                        const std::string & volatilityType )
    {
        VALID_EXCEPTION_START

        // Record Inputs for logs, tests and playback
        RECORD_INPUTS( price, payerReceiver, annuity, swapRate, strike, time, shift, volatilityType );

        // Manaster and Koehler Seed Value
		// Set a lower bound for the volatility estimate of 10% for LOGNORMAL, 1% for NORMAL.
		VolatilityTypeEnum volatilityTypeEnum = etrading::toVolatilityTypeEnum( volatilityType );
		const double lowerBoundOnVolatilityEstimate = volatilityTypeEnum == etrading::LOGNORMAL_VOLATILITY ? 0.1 : 0.01;
        const double initialGuessForVol = etrading::BlackScholes::initialGuessForImpliedVol( swapRate, strike, time, 0.0, shift, lowerBoundOnVolatilityEstimate );  // rate = 0.0

        // Calculation
        PayerReceiverSwaptionEnum payerReceiverEnum = etrading::toPayerReceiverSwaptionEnum( payerReceiver );
        std::shared_ptr<EuropeanIRSwaption> euroSwaption( new EuropeanIRSwaption( payerReceiverEnum, annuity, swapRate, strike, initialGuessForVol, time, shift, volatilityTypeEnum ) );
        const double result = euroSwaption->calculateImpliedVol( price );

        // Record Outputs AND Return the Result for logs, tests and playback
        RECORD_OUTPUTS_AND_RETURN_RESULT( result );

        VALID_EXCEPTION_END
    }

    // Calculate the tryAqMathEuropeanIRSwaptionImpliedVols - optimize = true uses OMP threading
    const std::vector<double> tryAqMathEuropeanIRSwaptionImpliedVols( const std::vector<double> & price,
                                                                      const std::vector<std::string> & payerReceiver,
                                                                      const std::vector<double> & annuity,
                                                                      const std::vector<double> & swapRate,
                                                                      const std::vector<double> & strike,
                                                                      const std::vector<double> & time,
                                                                      const std::vector<double> & shift,
                                                                      const std::vector<std::string> & volatilityType,
                                                                      const bool optimize )
    {
        VALID_EXCEPTION_START
        
         // The Volatility Type and Shift Parameters are optional
        const bool noVolType = ( volatilityType.size() == 0 ) ? true : false;
        const bool noShift = ( shift.size() == 0 ) ? true : false;

        // Dimension Validation
        AQ_REQUIRE( payerReceiver.size() == price.size(),                 "Invalid dimensions: Number of payerReceiver parameters must equal the number of price parameters." );
        AQ_REQUIRE( payerReceiver.size() == annuity.size(),               "Invalid dimensions: Number of payerReceiver parameters must equal the number of annuity parameters." );
        AQ_REQUIRE( payerReceiver.size() == swapRate.size(),              "Invalid dimensions: Number of payerReceiver parameters must equal the number of swapRate parameters." );
        AQ_REQUIRE( payerReceiver.size() == strike.size(),                "Invalid dimensions: Number of payerReceiver parameters must equal the number of strike parameters." );
        AQ_REQUIRE( payerReceiver.size() == time.size(),                  "Invalid dimensions: Number of payerReceiver parameters must equal the number of time parameters." );
        AQ_REQUIRE( noVolType || payerReceiver.size() == volatilityType.size(), "Invalid dimensions: Number of payerReceiver parameters must equal the number of volatilityType parameters." );
        AQ_REQUIRE( noShift || payerReceiver.size() == shift.size(),      "Invalid dimensions: Number of payerReceiver parameters must equal the number of shift parameters." );
        
        // Declare Results Vector
        std::vector<double> results( payerReceiver.size(), 0.0 );

        // Optimize uses OMP threading
        if ( optimize )
        {
            // Find the maximum number of hardware threads (independent cores) on this machine
            const int nThreads = omp_get_max_threads();

            // Calculation
            #pragma omp parallel for num_threads( nThreads )
            for ( int i = 0; i < ( int )( payerReceiver.size() ); ++i ) // OMP uses int loops, not size_t
            {
                // Manaster and Koehler Seed Value
				// Set a lower bound for the volatility estimate of 10% for LOGNORMAL, 1% for NORMAL.
				VolatilityTypeEnum volatilityTypeEnum =  noVolType ? etrading::LOGNORMAL_VOLATILITY : etrading::toVolatilityTypeEnum( volatilityType[i] );
				const double lowerBoundOnVolatilityEstimate = volatilityTypeEnum == etrading::LOGNORMAL_VOLATILITY ? 0.1 : 0.01;
                const double initialGuessForVol = etrading::BlackScholes::initialGuessForImpliedVol( swapRate[i], strike[i], time[i], 0.0, noShift ? 0.0 : shift[i], lowerBoundOnVolatilityEstimate ); // rate = 0.0

                PayerReceiverSwaptionEnum payerReceiverEnum = etrading::toPayerReceiverSwaptionEnum( payerReceiver[i] );
                std::shared_ptr<EuropeanIRSwaption> euroSwaption( new EuropeanIRSwaption( payerReceiverEnum, annuity[i], swapRate[i], strike[i], initialGuessForVol , time[i], noShift ? 0.0 : shift[i], volatilityTypeEnum ) );
                results[i] = euroSwaption->calculateImpliedVol( price[i] );
            }
        }
        else
        {
            for ( size_t i = 0; i < payerReceiver.size(); ++i )
            {
                // Manaster and Koehler Seed Value
				// Set a lower bound for the volatility estimate of 10% for LOGNORMAL, 1% for NORMAL.
				VolatilityTypeEnum volatilityTypeEnum =  noVolType ? etrading::LOGNORMAL_VOLATILITY : etrading::toVolatilityTypeEnum( volatilityType[i] );
				const double lowerBoundOnVolatilityEstimate = volatilityTypeEnum == etrading::LOGNORMAL_VOLATILITY ? 0.1 : 0.01;
                const double initialGuessForVol = etrading::BlackScholes::initialGuessForImpliedVol( swapRate[i], strike[i], time[i], 0.0, noShift ? 0.0 : shift[i], lowerBoundOnVolatilityEstimate ); // rate = 0.0

                PayerReceiverSwaptionEnum payerReceiverEnum = etrading::toPayerReceiverSwaptionEnum( payerReceiver[i] );
                std::shared_ptr<EuropeanIRSwaption> euroSwaption( new EuropeanIRSwaption( payerReceiverEnum, annuity[i], swapRate[i], strike[i], initialGuessForVol, time[i], noShift ? 0.0 : shift[i], volatilityTypeEnum ) );
                results[i] = euroSwaption->calculateImpliedVol( price[i] );
            }
        }

        return results;

        VALID_EXCEPTION_END
    }

    // Calculate the Cash Annuity
    const double tryAqMathEuropeanIRSwaptionCashAnnuity( const double & notional,
                                                         const double & swapRate,
                                                         const unsigned int & nCouponsPerYear,
                                                         const double & tenorInYears,
                                                         const std::string & stubType )
    {
        VALID_EXCEPTION_START
        const etrading::StubTypeEnum stubTypeEnum = etrading::toStubTypeEnum( stubType );
        const double result = etrading::EuropeanIRSwaption::cashAnnuity( notional, swapRate, nCouponsPerYear, tenorInYears, stubTypeEnum );
        return result;
        VALID_EXCEPTION_END
    }

    // Calculate the Delta
    const double tryAqMathEuropeanIRSwaptionDelta( const std::string& payerReceiver,
                                                   const double & annuity,
                                                   const double & swapRate,
                                                   const double & strike,
                                                   const double & vol,
                                                   const double & time,
                                                   const double & shift,
                                                   const std::string & volatilityType )
    {
        VALID_EXCEPTION_START

        // Record Inputs for logs, tests and playback
        RECORD_INPUTS( payerReceiver, annuity, swapRate, strike, vol, time, shift, volatilityType );

        // Calculation
        PayerReceiverSwaptionEnum payerReceiverEnum = etrading::toPayerReceiverSwaptionEnum( payerReceiver );
        VolatilityTypeEnum VolatilityTypeEnum = etrading::toVolatilityTypeEnum( volatilityType );
        const double result = EuropeanIRSwaption::delta( payerReceiverEnum, annuity, swapRate, strike, vol, time, shift, VolatilityTypeEnum );

        // Record Outputs AND Return the Result for logs, tests and playback
        RECORD_OUTPUTS_AND_RETURN_RESULT( result );

        VALID_EXCEPTION_END
    }

    // Calculate the Gamma
    const double tryAqMathEuropeanIRSwaptionGamma( const std::string& payerReceiver,
                                                   const double & annuity,
                                                   const double & swapRate,
                                                   const double & strike,
                                                   const double & vol,
                                                   const double & time,
                                                   const double & shift,
                                                   const std::string & volatilityType )
    {
        VALID_EXCEPTION_START

        // Record Inputs for logs, tests and playback
        RECORD_INPUTS( payerReceiver, annuity, swapRate, strike, vol, time, shift, volatilityType );

        // Calculation
        PayerReceiverSwaptionEnum payerReceiverEnum = etrading::toPayerReceiverSwaptionEnum( payerReceiver );
        VolatilityTypeEnum VolatilityTypeEnum = etrading::toVolatilityTypeEnum( volatilityType );
        const double result = EuropeanIRSwaption::gamma( payerReceiverEnum, annuity, swapRate, strike, vol, time, shift, VolatilityTypeEnum );

        // Record Outputs AND Return the Result for logs, tests and playback
        RECORD_OUTPUTS_AND_RETURN_RESULT( result );

        VALID_EXCEPTION_END
    }

    // Calculate the Vega
    const double tryAqMathEuropeanIRSwaptionVega( const std::string& payerReceiver,
                                                   const double & annuity,
                                                   const double & swapRate,
                                                   const double & strike,
                                                   const double & vol,
                                                   const double & time,
                                                   const double & shift,
                                                   const std::string & volatilityType )
    {
        VALID_EXCEPTION_START

        // Record Inputs for logs, tests and playback
        RECORD_INPUTS( payerReceiver, annuity, swapRate, strike, vol, time, shift, volatilityType );

        // Calculation
        PayerReceiverSwaptionEnum payerReceiverEnum = etrading::toPayerReceiverSwaptionEnum( payerReceiver );
        VolatilityTypeEnum VolatilityTypeEnum = etrading::toVolatilityTypeEnum( volatilityType );
        const double result = EuropeanIRSwaption::vega( payerReceiverEnum, annuity, swapRate, strike, vol, time, shift, VolatilityTypeEnum );

        // Record Outputs AND Return the Result for logs, tests and playback
        RECORD_OUTPUTS_AND_RETURN_RESULT( result );

        VALID_EXCEPTION_END
    }

    // Calculate the Theta
    const double tryAqMathEuropeanIRSwaptionTheta( const std::string& payerReceiver,
                                                   const double & annuity,
                                                   const double & swapRate,
                                                   const double & strike,
                                                   const double & vol,
                                                   const double & time,
                                                   const double & shift,
                                                   const std::string & volatilityType )
    {
        VALID_EXCEPTION_START

        // Record Inputs for logs, tests and playback
        RECORD_INPUTS( payerReceiver, annuity, swapRate, strike, vol, time, shift, volatilityType );

        // Calculation
        PayerReceiverSwaptionEnum payerReceiverEnum = etrading::toPayerReceiverSwaptionEnum( payerReceiver );
        VolatilityTypeEnum VolatilityTypeEnum = etrading::toVolatilityTypeEnum( volatilityType );
        const double result = EuropeanIRSwaption::theta( payerReceiverEnum, annuity, swapRate, strike, vol, time, shift, VolatilityTypeEnum );

        // Record Outputs AND Return the Result for logs, tests and playback
        RECORD_OUTPUTS_AND_RETURN_RESULT( result );

        VALID_EXCEPTION_END
    }

}