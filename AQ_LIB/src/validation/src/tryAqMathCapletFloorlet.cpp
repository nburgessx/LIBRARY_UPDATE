#include "tryAqMathCapletFloorlet.h"
#include "CapletFloorlet.h"

// Includes for Structured Exception Handling and Input/Output Logging
#include "CreateDataFile.h"
#include "StructuredExceptionHandler.h"
#include "CommonConstants.h"
#include "RecordMacros.h"
#include <omp.h>

// Namespaces for Input/Output Logging
using etrading::CreateDataFile;
using etrading::decorateFilename;

// CapletFloorlet Namespace
using etrading::CapletFloorlet;
using etrading::BlackScholes;

// Use typedef for etrading enumerator types
typedef etrading::CapletFloorletEnum CapletFloorletEnum;
typedef etrading::VolatilityTypeEnum VolatilityTypeEnum;

namespace validation
{

    // Calculate the Price
    const double tryAqMathCapletFloorletPrice( const std::string& capletOrFloorlet,
                                               const double & annuityFactor,
                                               const double & liborRate,
										       const double & strike,
										       const double & vol,
										       const double & time,
										       const double & shift,
                                               const std::string & volatilityType )
    {
        VALID_EXCEPTION_START

        // Record Inputs for logs, tests and playback
        AQ_RECORD_INPUTS( capletOrFloorlet, annuityFactor, liborRate, strike, vol, time, shift, volatilityType );

        CapletFloorletEnum capletOrFloorletEnum = etrading::toCapletFloorletEnum( capletOrFloorlet );
        VolatilityTypeEnum volatilityTypeEnum = etrading::toVolatilityTypeEnum( volatilityType );
        
        // Calculation
        CapletFloorlet capletFloorlet( capletOrFloorletEnum, annuityFactor, liborRate, strike, vol, time, shift, volatilityTypeEnum );
        const double result = capletFloorlet.price();

        // Record Outputs AND Return the Result for logs, tests and playback
        AQ_RECORD_OUTPUTS_AND_RETURN_RESULT( result );

        VALID_EXCEPTION_END
    }

    // Calculate the Price - optimize = true uses OMP threading
    const std::vector<double> tryAqMathCapletFloorletPrices( const std::vector<std::string> & capletOrFloorlet,
													         const std::vector<double> & annuityFactor, 
													         const std::vector<double> & liborRate,
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
        AQ_REQUIRE( capletOrFloorlet.size() == annuityFactor.size(),       "Invalid dimensions: Number of capletOrFloorlet parameters must equal the number of annuityFactor parameters." );
		AQ_REQUIRE( capletOrFloorlet.size() == liborRate.size(),			 "Invalid dimensions: Number of capletOrFloorlet parameters must equal the number of liborRate parameters.");
		AQ_REQUIRE( capletOrFloorlet.size() == strike.size(),              "Invalid dimensions: Number of capletOrFloorlet parameters must equal the number of strike parameters." );
        AQ_REQUIRE( capletOrFloorlet.size() == vol.size(),                 "Invalid dimensions: Number of capletOrFloorlet parameters must equal the number of vol parameters." );
        AQ_REQUIRE( capletOrFloorlet.size() == time.size(),                "Invalid dimensions: Number of capletOrFloorlet parameters must equal the number of time parameters." );
        AQ_REQUIRE( noShift || capletOrFloorlet.size() == shift.size(),    "Invalid dimensions: Number of capletOrFloorlet parameters must equal the number of shift parameters." );
        AQ_REQUIRE( noVolType || capletOrFloorlet.size() == volatilityType.size(), "Invalid dimensions: Number of capletOrFloorlet parameters must equal the number of volatilityType parameters." );

        // Declare Results Vector
        std::vector<double> results(capletOrFloorlet.size(), 0.0 );

        // Optimize uses OMP threading
        if ( optimize )
        {
            // Find the maximum number of hardware threads (independent cores) on this machine
            const int nThreads = omp_get_max_threads();

            // Calculation
            #pragma omp parallel for num_threads( nThreads )
            for ( int i = 0; i < ( int )( capletOrFloorlet.size() ); ++i ) // OMP uses int loops, not size_t
            {
                CapletFloorletEnum capletOrFloorletEnum = etrading::toCapletFloorletEnum( capletOrFloorlet[i] );
                VolatilityTypeEnum volatilityTypeEnum =  noVolType ? etrading::LOGNORMAL_VOLATILITY : etrading::toVolatilityTypeEnum( volatilityType[i] );

				CapletFloorlet capFloorOption( capletOrFloorletEnum, annuityFactor[i], liborRate[i], strike[i], vol[i], time[i], noShift ? 0.0 : shift[i], volatilityTypeEnum );
                results[i] = capFloorOption.price();
            }
        }
        else
        {
            for ( size_t i = 0; i < capletOrFloorlet.size(); ++i )
            {
                CapletFloorletEnum capletOrFloorletEnum = etrading::toCapletFloorletEnum( capletOrFloorlet[i] );
                VolatilityTypeEnum volatilityTypeEnum =  noVolType ? etrading::LOGNORMAL_VOLATILITY : etrading::toVolatilityTypeEnum( volatilityType[i] );

				CapletFloorlet capFloorOption( capletOrFloorletEnum, annuityFactor[i], liborRate[i], strike[i], vol[i], time[i], noShift ? 0.0 : shift[i], volatilityTypeEnum );
                results[i] = capFloorOption.price();
            }
        }

        return results;

        VALID_EXCEPTION_END
    }

    // Calculate the tryAqMathCapletFloorletImpliedVol
    const double tryAqMathCapletFloorletImpliedVol( const double & price,
											        const std::string& capletOrFloorlet,
											        const double & annuityFactor,
											        const double & liborRate,
											        const double & strike,
											        const double & time,
											        const double & shift,
                                                    const std::string & volatilityType )
    {
        VALID_EXCEPTION_START

        // Record Inputs for logs, tests and playback
        AQ_RECORD_INPUTS( price, capletOrFloorlet, annuityFactor, liborRate, strike, time, shift );

        CapletFloorletEnum capletOrFloorletEnum = etrading::toCapletFloorletEnum( capletOrFloorlet );
        VolatilityTypeEnum volatilityTypeEnum = etrading::toVolatilityTypeEnum( volatilityType );

        // Manaster and Koehler Seed Value
		// Set a lower bound for the volatility estimate of 10% for LOGNORMAL, 1% for NORMAL.
		const double lowerBoundOnVolatilityEstimate = volatilityTypeEnum == etrading::LOGNORMAL_VOLATILITY ? 0.1 : 0.01;
        const double initialGuessForVol = BlackScholes::initialGuessForImpliedVol(liborRate, strike, time, 0.0, shift, lowerBoundOnVolatilityEstimate );

        // Calculation
		CapletFloorlet capFloorOption( capletOrFloorletEnum, annuityFactor, liborRate, strike, initialGuessForVol, time, shift, volatilityTypeEnum );
        const double result = capFloorOption.calculateImpliedVol( price );

        // Record Outputs AND Return the Result for logs, tests and playback
        AQ_RECORD_OUTPUTS_AND_RETURN_RESULT( result );

        VALID_EXCEPTION_END
    }

    // Calculate the tryAqMathCapletFloorletImpliedVols - optimize = true uses OMP threading
    const std::vector<double> tryAqMathCapletFloorletImpliedVols( const std::vector<double> & price,
														          const std::vector<std::string> & capletOrFloorlet,
														          const std::vector<double> & annuityFactor, 
														          const std::vector<double> & liborRate,
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
        AQ_REQUIRE( capletOrFloorlet.size() == price.size(),                 "Invalid dimensions: Number of capletOrFloorlet parameters must equal the number of price parameters." );
		AQ_REQUIRE( capletOrFloorlet.size() == annuityFactor.size(),		   "Invalid dimensions: Number of capletOrFloorlet parameters must equal the number of annuityFactor parameters.");
		AQ_REQUIRE( capletOrFloorlet.size() == liborRate.size(),             "Invalid dimensions: Number of capletOrFloorlet parameters must equal the number of liborRate parameters." );
        AQ_REQUIRE( capletOrFloorlet.size() == strike.size(),                "Invalid dimensions: Number of capletOrFloorlet parameters must equal the number of strike parameters." );
        AQ_REQUIRE( capletOrFloorlet.size() == time.size(),                  "Invalid dimensions: Number of capletOrFloorlet parameters must equal the number of time parameters." );
		AQ_REQUIRE( noShift || capletOrFloorlet.size() == shift.size(),      "Invalid dimensions: Number of capletOrFloorlet parameters must equal the number of shift parameters." );
        AQ_REQUIRE( noVolType || capletOrFloorlet.size() == volatilityType.size(), "Invalid dimensions: Number of capletOrFloorlet parameters must equal the number of volatilityType parameters." );

        // Declare Results Vector
        std::vector<double> results(capletOrFloorlet.size(), 0.0 );
		const double lowerBoundOnVolatilityEstimate = 0.1;

        // Optimize uses OMP threading
        if ( optimize )
        {
            // Find the maximum number of hardware threads (independent cores) on this machine
            const int nThreads = omp_get_max_threads();

            // Calculation
            #pragma omp parallel for num_threads( nThreads )
            for ( int i = 0; i < ( int )( capletOrFloorlet.size() ); ++i ) // OMP uses int loops, not size_t
            {
                CapletFloorletEnum capletOrFloorletEnum = etrading::toCapletFloorletEnum( capletOrFloorlet[i] );
                VolatilityTypeEnum volatilityTypeEnum =  noVolType ? etrading::LOGNORMAL_VOLATILITY : etrading::toVolatilityTypeEnum( volatilityType[i] );

                // Manaster and Koehler Seed Value
                const double initialGuessForVol = BlackScholes::initialGuessForImpliedVol(liborRate[i], strike[i], time[i], 0.0, noShift ? 0.0 : shift[i], lowerBoundOnVolatilityEstimate ); // rate = 0.0

				CapletFloorlet capFloorOption( capletOrFloorletEnum, annuityFactor[i], liborRate[i], strike[i], initialGuessForVol , time[i], noShift ? 0.0 : shift[i], volatilityTypeEnum );
                results[i] = capFloorOption.calculateImpliedVol( price[i] );
            }
        }
        else
        {
            for ( size_t i = 0; i < capletOrFloorlet.size(); ++i )
            {
                CapletFloorletEnum capletOrFloorletEnum = etrading::toCapletFloorletEnum( capletOrFloorlet[i] );
                VolatilityTypeEnum volatilityTypeEnum =  noVolType ? etrading::LOGNORMAL_VOLATILITY : etrading::toVolatilityTypeEnum( volatilityType[i] );

                // Manaster and Koehler Seed Value
                const double initialGuessForVol = BlackScholes::initialGuessForImpliedVol(liborRate[i], strike[i], time[i], 0.0, noShift ? 0.0 : shift[i], lowerBoundOnVolatilityEstimate );

				CapletFloorlet capFloorOption( capletOrFloorletEnum, annuityFactor[i], liborRate[i], strike[i], initialGuessForVol, time[i], noShift ? 0.0 : shift[i], volatilityTypeEnum );
                results[i] = capFloorOption.calculateImpliedVol( price[i] );
            }
        }

        return results;

        VALID_EXCEPTION_END
    }



}