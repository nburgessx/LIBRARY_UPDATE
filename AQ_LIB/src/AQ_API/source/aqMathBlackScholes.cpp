// aqMathBlackScholes.cpp

/*
 * @brief			Swig interface for the aqMathBlackScholes* pricing and Greeks functions
 */

#include "aqMathBlackScholes.h"
#include "CoreEnumerations.h"        // etrading::toCallOrPutEnum
#include "TypeUtilities.h"
#include "tryAqMathBlackScholes.h"
#include "APISetUp.h"               // AQ_API_START and AQ_API_END Macros

namespace
{
    // Local helper - a vector of CALL/PUT strings to a vector<CallOrPutEnum>
    std::vector<etrading::CallOrPutEnum> toCallOrPutEnumVector( const std::vector<std::string>& callOrPut )
    {
        std::vector<etrading::CallOrPutEnum> result;
        result.reserve( callOrPut.size() );
        for ( const std::string& s : callOrPut )
        {
            result.push_back( etrading::toCallOrPutEnum( s ) );
        }
        return result;
    }
}

double aqMathBlackScholesPrice( const std::string& callOrPut,
                                 const double spot,
                                 const double strike,
                                 const double vol,
                                 const double time,
                                 const double rate,
                                 const double carry,
                                 const double shift )
{
    AQ_API_START

    double result = validation::tryAqMathBlackScholesPrice(
        etrading::toCallOrPutEnum( callOrPut ), spot, strike, vol, time, rate, carry, shift );
    return result;

    AQ_API_END
}

std::vector<double> aqMathBlackScholesPrices( const std::vector<std::string>& callOrPut,
                                               const std::vector<double>& spot,
                                               const std::vector<double>& strike,
                                               const std::vector<double>& vol,
                                               const std::vector<double>& time,
                                               const std::vector<double>& rate,
                                               const std::vector<double>& carry,
                                               const std::vector<double>& shift,
                                               const bool optimize )
{
    AQ_API_START

    std::vector<double> result = validation::tryAqMathBlackScholesPrices(
        toCallOrPutEnumVector( callOrPut ), spot, strike, vol, time, rate, carry, shift, optimize );
    return result;

    AQ_API_END
}

double aqMathBlackScholesImpliedVol( const double price,
                                      const std::string& callOrPut,
                                      const double spot,
                                      const double strike,
                                      const double time,
                                      const double rate,
                                      const double carry,
                                      const double shift )
{
    AQ_API_START

    double result = validation::tryAqMathBlackScholesImpliedVol(
        price, etrading::toCallOrPutEnum( callOrPut ), spot, strike, time, rate, carry, shift );
    return result;

    AQ_API_END
}

std::vector<double> aqMathBlackScholesImpliedVols( const std::vector<double>& price,
                                                    const std::vector<std::string>& callOrPut,
                                                    const std::vector<double>& spot,
                                                    const std::vector<double>& strike,
                                                    const std::vector<double>& time,
                                                    const std::vector<double>& rate,
                                                    const std::vector<double>& carry,
                                                    const std::vector<double>& shift,
                                                    const bool optimize )
{
    AQ_API_START

    std::vector<double> result = validation::tryAqMathBlackScholesImpliedVols(
        price, toCallOrPutEnumVector( callOrPut ), spot, strike, time, rate, carry, shift, optimize );
    return result;

    AQ_API_END
}

double aqMathBlackScholesDeltaForward( const std::string& callOrPut,
                                        const double spot,
                                        const double strike,
                                        const double vol,
                                        const double time,
                                        const double rate,
                                        const double carry,
                                        const double shift )
{
    AQ_API_START

    double result = validation::tryAqMathBlackScholesDeltaForward(
        etrading::toCallOrPutEnum( callOrPut ), spot, strike, vol, time, rate, carry, shift );
    return result;

    AQ_API_END
}

double aqMathBlackScholesDeltaSpot( const std::string& callOrPut,
                                     const double spot,
                                     const double strike,
                                     const double vol,
                                     const double time,
                                     const double rate,
                                     const double carry,
                                     const double shift )
{
    AQ_API_START

    double result = validation::tryAqMathBlackScholesDeltaSpot(
        etrading::toCallOrPutEnum( callOrPut ), spot, strike, vol, time, rate, carry, shift );
    return result;

    AQ_API_END
}

double aqMathBlackScholesGamma( const std::string& callOrPut,
                                 const double spot,
                                 const double strike,
                                 const double vol,
                                 const double time,
                                 const double rate,
                                 const double carry,
                                 const double shift )
{
    AQ_API_START

    double result = validation::tryAqMathBlackScholesGamma(
        etrading::toCallOrPutEnum( callOrPut ), spot, strike, vol, time, rate, carry, shift );
    return result;

    AQ_API_END
}

double aqMathBlackScholesVega( const std::string& callOrPut,
                                const double spot,
                                const double strike,
                                const double vol,
                                const double time,
                                const double rate,
                                const double carry,
                                const double shift )
{
    AQ_API_START

    double result = validation::tryAqMathBlackScholesVega(
        etrading::toCallOrPutEnum( callOrPut ), spot, strike, vol, time, rate, carry, shift );
    return result;

    AQ_API_END
}

double aqMathBlackScholesTheta( const std::string& callOrPut,
                                 const double spot,
                                 const double strike,
                                 const double vol,
                                 const double time,
                                 const double rate,
                                 const double carry,
                                 const double shift )
{
    AQ_API_START

    double result = validation::tryAqMathBlackScholesTheta(
        etrading::toCallOrPutEnum( callOrPut ), spot, strike, vol, time, rate, carry, shift );
    return result;

    AQ_API_END
}

double aqMathBlackScholesRho( const std::string& callOrPut,
                               const double spot,
                               const double strike,
                               const double vol,
                               const double time,
                               const double rate,
                               const double carry,
                               const double shift )
{
    AQ_API_START

    double result = validation::tryAqMathBlackScholesRho(
        etrading::toCallOrPutEnum( callOrPut ), spot, strike, vol, time, rate, carry, shift );
    return result;

    AQ_API_END
}
