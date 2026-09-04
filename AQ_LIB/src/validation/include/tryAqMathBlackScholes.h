#pragma once
#include "CoreEnumerations.h"

// Use typedef for etrading enumerator type(s)
typedef etrading::CallOrPutEnum CallOrPutEnum;

namespace validation
{

    // Calculate the tryAqMathBlackScholesPrice
    const double tryAqMathBlackScholesPrice( const CallOrPutEnum & callOrPut,
                                             const double & spot,
                                             const double & strike,
                                             const double & vol,
                                             const double & time,
                                             const double & rate,
                                             const double & carry,
                                             const double & shift = 0.0 );

    // Calculate the tryAqMathBlackScholesPrices - optimize = true uses OMP threading
    const std::vector<double> tryAqMathBlackScholesPrices( const std::vector<etrading::CallOrPutEnum> & callOrPut,
                                                           const std::vector<double> & spot,
                                                           const std::vector<double> & strike,
                                                           const std::vector<double> & vol,
                                                           const std::vector<double> & time,
                                                           const std::vector<double> & rate,
                                                           const std::vector<double> & carry,
                                                           const std::vector<double> & shift = std::vector<double>(),
                                                           const bool optimize = false );

    // Calculate the tryAqMathBlackScholesImpliedVol
    const double tryAqMathBlackScholesImpliedVol( const double & price,
                                                  const CallOrPutEnum & callOrPut,
                                                  const double & spot,
                                                  const double & strike,
                                                  const double & time,
                                                  const double & rate,
                                                  const double & carry,
                                                  const double & shift = 0.0 );

    // Calculate the tryAqMathBlackScholesPrices - optimize = true uses OMP threading
    const std::vector<double> tryAqMathBlackScholesImpliedVols( const std::vector<double> & price,
                                                                const std::vector<etrading::CallOrPutEnum> & callOrPut,
                                                                const std::vector<double> & spot,
                                                                const std::vector<double> & strike,
                                                                const std::vector<double> & time,
                                                                const std::vector<double> & rate,
                                                                const std::vector<double> & carry,
                                                                const std::vector<double> & shift = std::vector<double>(),
                                                                const bool optimize = false );

    // Calculate the tryAqMathBlackScholesForwardDelta
    const double tryAqMathBlackScholesDeltaForward( const CallOrPutEnum & callOrPut,
                                                    const double & spot,
                                                    const double & strike,
                                                    const double & vol,
                                                    const double & time,
                                                    const double & rate,
                                                    const double & carry,
                                                    const double & shift = 0.0 );

    // Calculate the tryAqMathBlackScholesSpotDelta
    const double tryAqMathBlackScholesDeltaSpot( const CallOrPutEnum & callOrPut,
                                                 const double & spot,
                                                 const double & strike,
                                                 const double & vol,
                                                 const double & time,
                                                 const double & rate,
                                                 const double & carry,
                                                 const double & shift = 0.0 );

    // Calculate the tryAqMathBlackScholesGamma
    const double tryAqMathBlackScholesGamma( const CallOrPutEnum & callOrPut,
                                             const double & spot,
                                             const double & strike,
                                             const double & vol,
                                             const double & time,
                                             const double & rate,
                                             const double & carry,
                                             const double & shift = 0.0 );

    // Calculate the tryAqMathBlackScholesVega
    const double tryAqMathBlackScholesVega(  const CallOrPutEnum & callOrPut,
                                             const double & spot,
                                             const double & strike,
                                             const double & vol,
                                             const double & time,
                                             const double & rate,
                                             const double & carry,
                                             const double & shift = 0.0 );

    // Calculate the tryAqMathBlackScholesTheta
    const double tryAqMathBlackScholesTheta( const CallOrPutEnum & callOrPut,
                                             const double & spot,
                                             const double & strike,
                                             const double & vol,
                                             const double & time,
                                             const double & rate,
                                             const double & carry,
                                             const double & shift = 0.0 );

    // Calculate the tryAqMathBlackScholesRho
    const double tryAqMathBlackScholesRho(   const CallOrPutEnum & callOrPut,
                                             const double & spot,
                                             const double & strike,
                                             const double & vol,
                                             const double & time,
                                             const double & rate,
                                             const double & carry,
                                             const double & shift = 0.0 );


}