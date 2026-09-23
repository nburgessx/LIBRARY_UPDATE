// aqMathCapletFloorlet.cpp

/*
 * @brief			Swig interface for the aqMathCapletFloorlet* pricing functions
 */

#include "aqMathCapletFloorlet.h"
#include "TypeUtilities.h"
#include "tryAqMathCapletFloorlet.h"
#include "APISetUp.h"               // AQ_API_START and AQ_API_END Macros

double aqMathCapletFloorletPrice( const std::string& capletOrFloorlet,
                                   const double annuityFactor,
                                   const double liborRate,
                                   const double strike,
                                   const double vol,
                                   const double time,
                                   const double shift,
                                   const std::string& volatilityType )
{
    AQ_API_START

    double result = validation::tryAqMathCapletFloorletPrice(
        capletOrFloorlet, annuityFactor, liborRate, strike, vol, time, shift, volatilityType );
    return result;

    AQ_API_END
}

std::vector<double> aqMathCapletFloorletPrices( const std::vector<std::string>& capletOrFloorlet,
                                                 const std::vector<double>& annuityFactor,
                                                 const std::vector<double>& liborRate,
                                                 const std::vector<double>& strike,
                                                 const std::vector<double>& vol,
                                                 const std::vector<double>& time,
                                                 const std::vector<double>& shift,
                                                 const std::vector<std::string>& volatilityType,
                                                 const bool optimize )
{
    AQ_API_START

    std::vector<double> result = validation::tryAqMathCapletFloorletPrices(
        capletOrFloorlet, annuityFactor, liborRate, strike, vol, time, shift, volatilityType, optimize );
    return result;

    AQ_API_END
}

double aqMathCapletFloorletImpliedVol( const double price,
                                        const std::string& capletOrFloorlet,
                                        const double annuityFactor,
                                        const double liborRate,
                                        const double strike,
                                        const double time,
                                        const double shift,
                                        const std::string& volatilityType )
{
    AQ_API_START

    double result = validation::tryAqMathCapletFloorletImpliedVol(
        price, capletOrFloorlet, annuityFactor, liborRate, strike, time, shift, volatilityType );
    return result;

    AQ_API_END
}

std::vector<double> aqMathCapletFloorletImpliedVols( const std::vector<double>& price,
                                                      const std::vector<std::string>& capletOrFloorlet,
                                                      const std::vector<double>& annuityFactor,
                                                      const std::vector<double>& liborRate,
                                                      const std::vector<double>& strike,
                                                      const std::vector<double>& time,
                                                      const std::vector<double>& shift,
                                                      const std::vector<std::string>& volatilityType,
                                                      const bool optimize )
{
    AQ_API_START

    std::vector<double> result = validation::tryAqMathCapletFloorletImpliedVols(
        price, capletOrFloorlet, annuityFactor, liborRate, strike, time, shift, volatilityType, optimize );
    return result;

    AQ_API_END
}
