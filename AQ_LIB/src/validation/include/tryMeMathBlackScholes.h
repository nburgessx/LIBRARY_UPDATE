#pragma once
#include "CoreEnumerations.h"

// Use typedef for etrading enumerator type(s)
typedef etrading::CallOrPutEnum CallOrPutEnum;

namespace validation_api
{

    // Calculate the tryMeMathBlackScholesPrice
    const double tryMeMathBlackScholesPrice( const CallOrPutEnum & callOrPut,
                                             const double & spot,
                                             const double & strike,
                                             const double & vol,
                                             const double & time,
                                             const double & rate,
                                             const double & carry,
                                             const double & shift = 0.0 );

    // Calculate the tryMeMathBlackScholesPrices - optimize = true uses OMP threading
    const std::vector<double> tryMeMathBlackScholesPrices( const std::vector<etrading::CallOrPutEnum> & callOrPut,
                                                           const std::vector<double> & spot,
                                                           const std::vector<double> & strike,
                                                           const std::vector<double> & vol,
                                                           const std::vector<double> & time,
                                                           const std::vector<double> & rate,
                                                           const std::vector<double> & carry,
                                                           const std::vector<double> & shift = std::vector<double>(),
                                                           const bool optimize = false );

    // Calculate the tryMeMathBlackScholesImpliedVol
    const double tryMeMathBlackScholesImpliedVol( const double & price,
                                                  const CallOrPutEnum & callOrPut,
                                                  const double & spot,
                                                  const double & strike,
                                                  const double & time,
                                                  const double & rate,
                                                  const double & carry,
                                                  const double & shift = 0.0 );

    // Calculate the tryMeMathBlackScholesPrices - optimize = true uses OMP threading
    const std::vector<double> tryMeMathBlackScholesImpliedVols( const std::vector<double> & price,
                                                                const std::vector<etrading::CallOrPutEnum> & callOrPut,
                                                                const std::vector<double> & spot,
                                                                const std::vector<double> & strike,
                                                                const std::vector<double> & time,
                                                                const std::vector<double> & rate,
                                                                const std::vector<double> & carry,
                                                                const std::vector<double> & shift = std::vector<double>(),
                                                                const bool optimize = false );

    // Calculate the tryMeMathBlackScholesForwardDelta
    const double tryMeMathBlackScholesDeltaForward( const CallOrPutEnum & callOrPut,
                                                    const double & spot,
                                                    const double & strike,
                                                    const double & vol,
                                                    const double & time,
                                                    const double & rate,
                                                    const double & carry,
                                                    const double & shift = 0.0 );

    // Calculate the tryMeMathBlackScholesSpotDelta
    const double tryMeMathBlackScholesDeltaSpot( const CallOrPutEnum & callOrPut,
                                                 const double & spot,
                                                 const double & strike,
                                                 const double & vol,
                                                 const double & time,
                                                 const double & rate,
                                                 const double & carry,
                                                 const double & shift = 0.0 );

    // Calculate the tryMeMathBlackScholesGamma
    const double tryMeMathBlackScholesGamma( const CallOrPutEnum & callOrPut,
                                             const double & spot,
                                             const double & strike,
                                             const double & vol,
                                             const double & time,
                                             const double & rate,
                                             const double & carry,
                                             const double & shift = 0.0 );

    // Calculate the tryMeMathBlackScholesVega
    const double tryMeMathBlackScholesVega(  const CallOrPutEnum & callOrPut,
                                             const double & spot,
                                             const double & strike,
                                             const double & vol,
                                             const double & time,
                                             const double & rate,
                                             const double & carry,
                                             const double & shift = 0.0 );

    // Calculate the tryMeMathBlackScholesTheta
    const double tryMeMathBlackScholesTheta( const CallOrPutEnum & callOrPut,
                                             const double & spot,
                                             const double & strike,
                                             const double & vol,
                                             const double & time,
                                             const double & rate,
                                             const double & carry,
                                             const double & shift = 0.0 );

    // Calculate the tryMeMathBlackScholesRho
    const double tryMeMathBlackScholesRho(   const CallOrPutEnum & callOrPut,
                                             const double & spot,
                                             const double & strike,
                                             const double & vol,
                                             const double & time,
                                             const double & rate,
                                             const double & carry,
                                             const double & shift = 0.0 );


}