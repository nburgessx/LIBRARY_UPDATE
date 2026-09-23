// aqMathVolatilityConversion.cpp

/*
 * @brief			Swig interface for the aqMathVolatilityTo* volatility-type conversion functions
 */

#include "aqMathVolatilityConversion.h"
#include "TypeUtilities.h"
#include "tryAqMathConvexity.h"
#include "APISetUp.h"               // AQ_API_START and AQ_API_END Macros

double aqMathVolatilityToNormalFromLognormal( const double lognormalVol, const double underlyingRate )
{
    AQ_API_START

    double result = validation::tryAqMathVolatilityToNormalFromLognormal( lognormalVol, underlyingRate );
    return result;

    AQ_API_END
}

double aqMathVolatilityToLognormalFromNormal( const double normalVol, const double underlyingRate )
{
    AQ_API_START

    double result = validation::tryAqMathVolatilityToLognormalFromNormal( normalVol, underlyingRate );
    return result;

    AQ_API_END
}

double aqMathVolatilityToShiftedLognormalFromLognormal( const double lognormalVol, const double underlyingRate, const double shiftSize )
{
    AQ_API_START

    double result = validation::tryAqMathVolatilityToShiftedLognormalFromLognormal( lognormalVol, underlyingRate, shiftSize );
    return result;

    AQ_API_END
}

double aqMathVolatilityToLognormalFromShiftedLognormal( const double shiftedLognormalVol, const double underlyingRate, const double shiftSize )
{
    AQ_API_START

    double result = validation::tryAqMathVolatilityToLognormalFromShiftedLognormal( shiftedLognormalVol, underlyingRate, shiftSize );
    return result;

    AQ_API_END
}

double aqMathVolatilityToShiftedLognormalFromNormal( const double normalVol, const double underlyingRate, const double shiftSize )
{
    AQ_API_START

    double result = validation::tryAqMathVolatilityToShiftedLognormalFromNormal( normalVol, underlyingRate, shiftSize );
    return result;

    AQ_API_END
}

double aqMathVolatilityToNormalFromShiftedLognormal( const double shiftedLognormalVol, const double underlyingRate, const double shiftSize )
{
    AQ_API_START

    double result = validation::tryAqMathVolatilityToNormalFromShiftedLognormal( shiftedLognormalVol, underlyingRate, shiftSize );
    return result;

    AQ_API_END
}
