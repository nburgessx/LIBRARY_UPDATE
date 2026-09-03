#include "tryMeMathBlackScholes.h"
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

// Black-Scholes Namespace
using etrading::BlackScholes;

namespace validation
{

    // Calculate the tryMeMathBlackScholesPrice
    const double tryMeMathBlackScholesPrice( const CallOrPutEnum & callOrPut,
                                             const double & spot,
                                             const double & strike,
                                             const double & vol,
                                             const double & time,
                                             const double & rate,
                                             const double & carry,
                                             const double & shift )
    {
        VALID_EXCEPTION_START

        // Record Inputs for logs, tests and playback
        RECORD_INPUTS( callOrPut, spot, strike, vol, time, rate, carry, shift );

        // Calculation
        BlackScholes bs( callOrPut, spot, strike, vol, time, rate, carry, shift );
        const double result = bs.price();

        // Record Outputs AND Return the Result for logs, tests and playback
        RECORD_OUTPUTS_AND_RETURN_RESULT( result );

		VALID_EXCEPTION_END
    }


    // Calculate the tryMeMathBlackScholesPrice
    const std::vector<double> tryMeMathBlackScholesPrices( const std::vector<etrading::CallOrPutEnum> & callOrPut,
                                                           const std::vector<double> & spot,
                                                           const std::vector<double> & strike,
                                                           const std::vector<double> & vol,
                                                           const std::vector<double> & time,
                                                           const std::vector<double> & rate,
                                                           const std::vector<double> & carry,
                                                           const std::vector<double> & shift,
                                                           const bool optimize )
    {
        VALID_EXCEPTION_START
        
        // The Shift Parameter is optional
        const bool noShift = ( shift.size() == 0 ) ? true : false;

        // Dimension Validation
        AQ_REQUIRE( callOrPut.size() == spot.size(),                  "Invalid dimensions: Number of callOrPut parameters must equal the number of spot parameters." );
        AQ_REQUIRE( callOrPut.size() == strike.size(),                "Invalid dimensions: Number of callOrPut parameters must equal the number of strike parameters." );
        AQ_REQUIRE( callOrPut.size() == vol.size(),                   "Invalid dimensions: Number of callOrPut parameters must equal the number of vol parameters." );
        AQ_REQUIRE( callOrPut.size() == time.size(),                  "Invalid dimensions: Number of callOrPut parameters must equal the number of time parameters." );
        AQ_REQUIRE( callOrPut.size() == rate.size(),                  "Invalid dimensions: Number of callOrPut parameters must equal the number of rate parameters." );
        AQ_REQUIRE( callOrPut.size() == carry.size(),                 "Invalid dimensions: Number of callOrPut parameters must equal the number of carry parameters." );
        AQ_REQUIRE( noShift || callOrPut.size() == shift.size(),      "Invalid dimensions: Number of callOrPut parameters must equal the number of shift parameters." );

        // Declare Results Vector
        std::vector<double> results( callOrPut.size(), 0.0 );

        // Optimize uses OMP threading
        if ( optimize )
        {
            // Find the maximum number of hardware threads (independent cores) on this machine
            const int nThreads = omp_get_max_threads();

            // Calculation
            #pragma omp parallel for num_threads( nThreads )
            for( int i = 0; i < (int)(callOrPut.size()); ++i ) // OMP uses int loops, not size_t
            {
                BlackScholes bs( callOrPut[i], spot[i], strike[i], vol[i], time[i], rate[i], carry[i], noShift ? 0.0 : shift[i] );
                results[i] = bs.price();
            }
        }
        else
        {
            for( size_t i = 0; i < callOrPut.size(); ++i )
            {
                BlackScholes bs( callOrPut[i], spot[i], strike[i], vol[i], time[i], rate[i], carry[i], noShift ? 0.0 : shift[i] );
                results[i] = bs.price();
            }
        }

        return results;

		VALID_EXCEPTION_END
    }


    // Calculate the tryMeMathBlackScholesImpliedVol
    const double tryMeMathBlackScholesImpliedVol( const double & price,
                                                  const CallOrPutEnum & callOrPut,
                                                  const double & spot,
                                                  const double & strike,
                                                  const double & time,
                                                  const double & rate,
                                                  const double & carry,
                                                  const double & shift )
    {
        VALID_EXCEPTION_START

        // Record Inputs for logs, tests and playback
        RECORD_INPUTS( price, callOrPut, spot, strike, time, rate, carry, shift );

        // Manaster and Koehler Seed Value
		const double lowerBoundOnVolatilityEstimate = 0.1;
        const double initialGuessForVol = BlackScholes::initialGuessForImpliedVol( spot, strike, time, rate, shift, lowerBoundOnVolatilityEstimate );

        // Calculation
        BlackScholes bs( callOrPut, spot, strike, initialGuessForVol, time, rate, carry, shift );
        const double result = bs.calculateImpliedVol( price );

        // Record Outputs AND Return the Result for logs, tests and playback
        RECORD_OUTPUTS_AND_RETURN_RESULT( result );

		VALID_EXCEPTION_END
    }

    // Calculate the tryMeMathBlackScholesPrice
    const std::vector<double> tryMeMathBlackScholesImpliedVols( const std::vector<double> & price,
                                                                const std::vector<etrading::CallOrPutEnum> & callOrPut,
                                                                const std::vector<double> & spot,
                                                                const std::vector<double> & strike,
                                                                const std::vector<double> & time,
                                                                const std::vector<double> & rate,
                                                                const std::vector<double> & carry,
                                                                const std::vector<double> & shift,
                                                                const bool optimize )
    {
        VALID_EXCEPTION_START

        // The Shift Parameter is optional
        const bool noShift = ( shift.size() == 0 ) ? true : false;

        // Dimension Validation
        AQ_REQUIRE( callOrPut.size() == price.size(),                 "Invalid dimensions: Number of callOrPut parameters must equal the number of price parameters." );
        AQ_REQUIRE( callOrPut.size() == spot.size(),                  "Invalid dimensions: Number of callOrPut parameters must equal the number of spot parameters." );
        AQ_REQUIRE( callOrPut.size() == strike.size(),                "Invalid dimensions: Number of callOrPut parameters must equal the number of strike parameters." );
        AQ_REQUIRE( callOrPut.size() == time.size(),                  "Invalid dimensions: Number of callOrPut parameters must equal the number of time parameters." );
        AQ_REQUIRE( callOrPut.size() == rate.size(),                  "Invalid dimensions: Number of callOrPut parameters must equal the number of rate parameters." );
        AQ_REQUIRE( callOrPut.size() == carry.size(),                 "Invalid dimensions: Number of callOrPut parameters must equal the number of carry parameters." );
        AQ_REQUIRE( noShift || callOrPut.size() == shift.size(),      "Invalid dimensions: Number of callOrPut parameters must equal the number of shift parameters." );

        // Declare Results Vector
        std::vector<double> results( callOrPut.size(), 0.0 );
		const double lowerBoundOnVolatilityEstimate = 0.1;

        // Optimize uses OMP threading
        if ( optimize )
        {
            // Find the maximum number of hardware threads (independent cores) on this machine
            const int nThreads = omp_get_max_threads();

            // Calculation
            #pragma omp parallel for num_threads( nThreads )
            for( int i = 0; i < (int)(callOrPut.size()); ++i ) // OMP uses int loops, not size_t
            {
                // Manaster and Koehler Seed Value
                const double initialGuessForVol = BlackScholes::initialGuessForImpliedVol( spot[i], strike[i], time[i], rate[i], noShift ? 0.0 : shift[i], lowerBoundOnVolatilityEstimate );

                BlackScholes bs( callOrPut[i], spot[i], strike[i], initialGuessForVol, time[i], rate[i], carry[i], noShift ? 0.0 : shift[i] );
                results[i] = bs.calculateImpliedVol( price[i] );
            }
        }
        else
        {
            for( size_t i = 0; i < callOrPut.size(); ++i )
            {
                // Manaster and Koehler Seed Value
                const double initialGuessForVol = BlackScholes::initialGuessForImpliedVol( spot[i], strike[i], time[i], rate[i], noShift ? 0.0 : shift[i], lowerBoundOnVolatilityEstimate );

                BlackScholes bs( callOrPut[i], spot[i], strike[i], initialGuessForVol, time[i], rate[i], carry[i], noShift ? 0.0 : shift[i] );
                results[i] = bs.calculateImpliedVol( price[i] );
            }
        }

        return results;

		VALID_EXCEPTION_END
    }


    // Calculate the tryMeMathBlackScholesForwardDelta
    const double tryMeMathBlackScholesDeltaForward( const CallOrPutEnum & callOrPut,
                                                    const double & spot,
                                                    const double & strike,
                                                    const double & vol,
                                                    const double & time,
                                                    const double & rate,
                                                    const double & carry,
                                                    const double & shift )
    {
        VALID_EXCEPTION_START

        // Record Inputs for logs, tests and playback
        RECORD_INPUTS( callOrPut, spot, strike, vol, time, rate, carry, shift );

        // Calculation
        BlackScholes bs( callOrPut, spot, strike, vol, time, rate, carry, shift );
        const double result = bs.deltaForward();
        
        // Record Outputs AND Return the Result for logs, tests and playback
        RECORD_OUTPUTS_AND_RETURN_RESULT( result );

		VALID_EXCEPTION_END
    }


    // Calculate the tryMeMathBlackScholesSpotDelta
    const double tryMeMathBlackScholesDeltaSpot( const CallOrPutEnum & callOrPut,
                                                 const double & spot,
                                                 const double & strike,
                                                 const double & vol,
                                                 const double & time,
                                                 const double & rate,
                                                 const double & carry,
                                                 const double & shift )
    {
        VALID_EXCEPTION_START

        // Record Inputs for logs, tests and playback
        RECORD_INPUTS( callOrPut, spot, strike, vol, time, rate, carry, shift );

        // Calculation
        BlackScholes bs( callOrPut, spot, strike, vol, time, rate, carry, shift );
        const double result = bs.deltaSpot();
        
        // Record Outputs AND Return the Result for logs, tests and playback
        RECORD_OUTPUTS_AND_RETURN_RESULT( result );

		VALID_EXCEPTION_END
    }


    // Calculate the tryMeMathBlackScholesGamma
    const double tryMeMathBlackScholesGamma( const CallOrPutEnum & callOrPut,
                                             const double & spot,
                                             const double & strike,
                                             const double & vol,
                                             const double & time,
                                             const double & rate,
                                             const double & carry,
                                             const double & shift )
    {
        VALID_EXCEPTION_START

        // Record Inputs for logs, tests and playback
        RECORD_INPUTS( callOrPut, spot, strike, vol, time, rate, carry, shift );

        // Calculation
        BlackScholes bs( callOrPut, spot, strike, vol, time, rate, carry, shift );
        const double result = bs.gamma();
        
        // Record Outputs AND Return the Result for logs, tests and playback
        RECORD_OUTPUTS_AND_RETURN_RESULT( result );

		VALID_EXCEPTION_END
    }

    // Calculate the tryMeMathBlackScholesVega
    const double tryMeMathBlackScholesVega(  const CallOrPutEnum & callOrPut,
                                             const double & spot,
                                             const double & strike,
                                             const double & vol,
                                             const double & time,
                                             const double & rate,
                                             const double & carry,
                                             const double & shift )
    {
        VALID_EXCEPTION_START

        // Record Inputs for logs, tests and playback
        RECORD_INPUTS( callOrPut, spot, strike, vol, time, rate, carry, shift );

        // Calculation
        BlackScholes bs( callOrPut, spot, strike, vol, time, rate, carry, shift );
        const double result = bs.vega();
        
        // Record Outputs AND Return the Result for logs, tests and playback
        RECORD_OUTPUTS_AND_RETURN_RESULT( result );

		VALID_EXCEPTION_END
    }

    // Calculate the tryMeMathBlackScholesTheta
    const double tryMeMathBlackScholesTheta( const CallOrPutEnum & callOrPut,
                                             const double & spot,
                                             const double & strike,
                                             const double & vol,
                                             const double & time,
                                             const double & rate,
                                             const double & carry,
                                             const double & shift )
    {
        VALID_EXCEPTION_START

        // Record Inputs for logs, tests and playback
        RECORD_INPUTS( callOrPut, spot, strike, vol, time, rate, carry, shift );

        // Calculation
        BlackScholes bs( callOrPut, spot, strike, vol, time, rate, carry, shift );
        const double result = bs.theta();
        
        // Record Outputs AND Return the Result for logs, tests and playback
        RECORD_OUTPUTS_AND_RETURN_RESULT( result );

		VALID_EXCEPTION_END;
    }

    // Calculate the tryMeMathBlackScholesRho
    const double tryMeMathBlackScholesRho(   const CallOrPutEnum & callOrPut,
                                             const double & spot,
                                             const double & strike,
                                             const double & vol,
                                             const double & time,
                                             const double & rate,
                                             const double & carry,
                                             const double & shift )
    {
        VALID_EXCEPTION_START

        // Record Inputs for logs, tests and playback
        RECORD_INPUTS( callOrPut, spot, strike, vol, time, rate, carry, shift );

        // Calculation
        BlackScholes bs( callOrPut, spot, strike, vol, time, rate, carry, shift );
        const double result = bs.rho();
        
        // Record Outputs AND Return the Result for logs, tests and playback
        RECORD_OUTPUTS_AND_RETURN_RESULT( result );

		VALID_EXCEPTION_END
    }


}