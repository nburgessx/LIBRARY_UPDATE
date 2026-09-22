#include <xllMain.h>
#include <xllSupport.h>

#include <string>
#include <vector>

// include
#include <CoreEnumerations.h>
#include <tryAqMathNormalDistribution.h>
#include <tryAqMathBlackScholes.h>
#include <tryAqMathInterpolation.h>   // validation::tryAqMathInterpolation
#include <tryAqMathPCA.h>             // validation::tryAqMathPCA
#include <tryAqMathCapletFloorlet.h>  // validation::tryAqMathCapletFloorlet*
#include <tryAqMathConvexity.h>       // validation::tryAqMathVolatility* / tryAqMathLibor*
#include <tryAqMathEuropeanIRSwaption.h>  // validation::tryAqMathEuropeanIRSwaption*
#include <tryAqMathIntegrate.h>       // validation::tryAqMathIntegrate*
#include <tryAqMathPolynomial.h>      // validation::tryAqMathPolynomial* / tryAqMathPoynomial*
#include <tryAqMathForwardRate.h>     // validation::tryAqMathForwardRate(s) / tryAqMathDiscountFactor(s)

using namespace aq_xll;

namespace
{
    // Optional integer worksheet argument: missing / empty / blank yields
    // defaultValue.
    int toIntOr( const xloil::ExcelObj& obj, int defaultValue )
    {
        if ( obj.isMissing() || !obj.isNonEmpty() )
        {
            return defaultValue;
        }
        return static_cast< int >( obj.get<double>() );
    }

    // Optional floating-point worksheet argument.
    double toDoubleOr( const xloil::ExcelObj& obj, double defaultValue )
    {
        if ( obj.isMissing() || !obj.isNonEmpty() )
        {
            return defaultValue;
        }
        return obj.get<double>();
    }

    // Optional string worksheet argument.
    std::string toStrOr( const xloil::ExcelObj& obj, const char* defaultValue )
    {
        if ( obj.isMissing() || !obj.isNonEmpty() )
        {
            return std::string( defaultValue );
        }
        return toNarrowString( obj );
    }

    // Required unsigned integer worksheet argument.
    unsigned int toUInt( const xloil::ExcelObj& obj )
    {
        return static_cast< unsigned int >( obj.get<double>() );
    }

    // Optional column of strings: missing / empty / blank yields an empty vector.
    std::vector<std::string> toStringVectorOr( const xloil::ExcelObj& obj )
    {
        if ( obj.isMissing() || !obj.isNonEmpty() )
        {
            return std::vector<std::string>();
        }
        return toStringVector( obj, true );
    }

    // Optional column of doubles: missing / empty / blank yields an empty vector.
    std::vector<double> toDoubleVectorOr( const xloil::ExcelObj& obj, const char* nameOfVariable )
    {
        if ( obj.isMissing() || !obj.isNonEmpty() )
        {
            return std::vector<double>();
        }
        return toDoubleVector( obj, true, nameOfVariable );
    }

    // A column of call/put strings -> a vector<CallOrPutEnum>.
    std::vector<etrading::CallOrPutEnum> toCallOrPutEnumVector( const xloil::ExcelObj& obj )
    {
        const std::vector<std::string> strings = toStringVector( obj, true );
        std::vector<etrading::CallOrPutEnum> result;
        result.reserve( strings.size() );
        for ( const std::string& s : strings )
        {
            result.push_back( etrading::toCallOrPutEnum( s ) );
        }
        return result;
    }

    // Optional matrix worksheet argument: missing / empty / blank yields a
    // genuinely empty StandardStringMatrix.
    StandardStringMatrix toStandardStringMatrixOr( const xloil::ExcelObj& obj )
    {
        if ( obj.isMissing() || !obj.isNonEmpty() )
        {
            return StandardStringMatrix();
        }
        return toStandardStringMatrix( obj );
    }
}


// Black-Scholes Price Method
#if AQ_XLL_ENABLED(aqMathBlackScholesPrice)
XLO_FUNC_START( aqMathBlackScholesPrice(
    const ExcelObj& callOrPut,
    const ExcelObj& spot,
    const ExcelObj& strike,
    const ExcelObj& vol,
    const ExcelObj& time,
    const ExcelObj& rate,
    const ExcelObj& carry,
    const ExcelObj& shift ) )
{
	// Carry and Shift are optional and default to zero when omitted
	const double carryArg =
	    carry.isMissing() ? 0.0 : carry.get<double>();

	const double shiftArg =
	    shift.isMissing() ? 0.0 : shift.get<double>();

	const CallOrPutEnum cp =
	    etrading::toCallOrPutEnum(
	        aq_xll::toNarrowString( callOrPut ) );

	const double result =
	    validation::tryAqMathBlackScholesPrice(
	        cp,
	        spot.get<double>(),
	        strike.get<double>(),
	        vol.get<double>(),
	        time.get<double>(),
	        rate.get<double>(),
	        carryArg,
	        shiftArg );

	return returnValue( result );
}
XLO_FUNC_END( aqMathBlackScholesPrice )
    .help(L"Function to calculate the Black-Scholes Price")
.arg(L"CallOrPut", L"Call or Put")
.arg(L"Spot", L"Spot")
.arg(L"Strike", L"Strike")
.arg(L"Vol", L"Volatility (%)")
.arg(L"Time", L"Time to Expiry in Years")
.arg(L"Rate", L"Interest Rate (%)")
.arg(L"Carry", L"Cost of Carry (%) - Defaults to Zero")
.arg(L"Shift", L"[Optional] Apply a lognormal shift, defaults to zero");
#endif


/*
 * Interpolate a Y value at XPoint from the (XValues, YValues) data.
 */
#if AQ_XLL_ENABLED(aqMathInterpolation)
XLO_FUNC_START( aqMathInterpolation(
    const ExcelObj& xValues,
    const ExcelObj& yValues,
    const ExcelObj& xPoint,
    const ExcelObj& interpolation,
    const ExcelObj& joinXValue ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const double result = validation::tryAqMathInterpolation(
        toDoubleVector( xValues ),
        toDoubleVector( yValues ),
        xPoint.get<double>(),
        toAQLString( interpolation ),
        toDoubleOr( joinXValue, 0.0 ) );

    return returnValue( result );
}
XLO_FUNC_END( aqMathInterpolation )
    .help( L"Interpolate a Y value at XPoint from the (XValues, YValues) data." )
    .arg( L"XValues",       L"The x-axis data" )
    .arg( L"YValues",       L"The y-axis data, aligned with XValues" )
    .arg( L"XPoint",        L"The x value to interpolate at" )
    .arg( L"Interpolation", L"Interpolation method, e.g. LINEAR, MONOTONE_CONVEX, SPLINE" )
    .arg( L"JoinXValue",    L"Optional. Join x value for piecewise schemes. Default 0" );
#endif


/*
 * Principal-component analysis of a data matrix. Returns the factor loadings as
 * a labelled matrix.
 */
#if AQ_XLL_ENABLED(aqMathPCA)
XLO_FUNC_START( aqMathPCA(
    const ExcelObj& key,
    const ExcelObj& data,
    const ExcelObj& useCorrelationMatrix,
    const ExcelObj& nFactors ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelMatrix( validation::tryAqMathPCA(
        toAQLString( key ),
        toDoubleMatrix( data ),
        toBool( useCorrelationMatrix, false ),
        toIntOr( nFactors, 0 ) ) ) );
}
XLO_FUNC_END( aqMathPCA )
    .help( L"Principal-component analysis of a data matrix. Returns the factor loadings as a labelled matrix." )
    .arg( L"Key",                  L"Row/label key for the analysis" )
    .arg( L"Data",                 L"The data matrix (observations x variables)" )
    .arg( L"UseCorrelationMatrix", L"Optional. Default FALSE. TRUE uses the correlation matrix, FALSE the covariance matrix" )
    .arg( L"NFactors",             L"Number of factors (principal components) to return" );
#endif


/* =========================================================================
 *  Black-Scholes - implied vol and Greeks
 * ====================================================================== */

// Implied volatility from a Black-Scholes price.
#if AQ_XLL_ENABLED(aqMathBlackScholesImpliedVol)
XLO_FUNC_START( aqMathBlackScholesImpliedVol(
    const ExcelObj& price,
    const ExcelObj& callOrPut,
    const ExcelObj& spot,
    const ExcelObj& strike,
    const ExcelObj& time,
    const ExcelObj& rate,
    const ExcelObj& carry,
    const ExcelObj& shift ) )
{
    AQ_XLL_GUARD

    return returnValue( validation::tryAqMathBlackScholesImpliedVol(
        price.get<double>(),
        etrading::toCallOrPutEnum( toNarrowString( callOrPut ) ),
        spot.get<double>(), strike.get<double>(), time.get<double>(),
        rate.get<double>(), carry.get<double>(), toDoubleOr( shift, 0.0 ) ) );
}
XLO_FUNC_END( aqMathBlackScholesImpliedVol )
    .help( L"Implied volatility from a Black-Scholes price." )
    .arg( L"Price",     L"The option price" )
    .arg( L"CallOrPut", L"Call or Put" )
    .arg( L"Spot",      L"Spot" )
    .arg( L"Strike",    L"Strike" )
    .arg( L"Time",      L"Time to expiry in years" )
    .arg( L"Rate",      L"Interest rate" )
    .arg( L"Carry",     L"Cost of carry" )
    .arg( L"Shift",     L"Optional. Lognormal shift. Default 0" );
#endif


// Black-Scholes forward delta.
#if AQ_XLL_ENABLED(aqMathBlackScholesDeltaForward)
XLO_FUNC_START( aqMathBlackScholesDeltaForward(
    const ExcelObj& callOrPut,
    const ExcelObj& spot,
    const ExcelObj& strike,
    const ExcelObj& vol,
    const ExcelObj& time,
    const ExcelObj& rate,
    const ExcelObj& carry,
    const ExcelObj& shift ) )
{
    AQ_XLL_GUARD

    return returnValue( validation::tryAqMathBlackScholesDeltaForward(
        etrading::toCallOrPutEnum( toNarrowString( callOrPut ) ),
        spot.get<double>(), strike.get<double>(), vol.get<double>(), time.get<double>(),
        rate.get<double>(), carry.get<double>(), toDoubleOr( shift, 0.0 ) ) );
}
XLO_FUNC_END( aqMathBlackScholesDeltaForward )
    .help( L"Black-Scholes forward delta." )
    .arg( L"CallOrPut", L"Call or Put" )
    .arg( L"Spot",      L"Spot" )
    .arg( L"Strike",    L"Strike" )
    .arg( L"Vol",       L"Volatility" )
    .arg( L"Time",      L"Time to expiry in years" )
    .arg( L"Rate",      L"Interest rate" )
    .arg( L"Carry",     L"Cost of carry" )
    .arg( L"Shift",     L"Optional. Lognormal shift. Default 0" );
#endif


// Black-Scholes spot delta.
#if AQ_XLL_ENABLED(aqMathBlackScholesDeltaSpot)
XLO_FUNC_START( aqMathBlackScholesDeltaSpot(
    const ExcelObj& callOrPut,
    const ExcelObj& spot,
    const ExcelObj& strike,
    const ExcelObj& vol,
    const ExcelObj& time,
    const ExcelObj& rate,
    const ExcelObj& carry,
    const ExcelObj& shift ) )
{
    AQ_XLL_GUARD

    return returnValue( validation::tryAqMathBlackScholesDeltaSpot(
        etrading::toCallOrPutEnum( toNarrowString( callOrPut ) ),
        spot.get<double>(), strike.get<double>(), vol.get<double>(), time.get<double>(),
        rate.get<double>(), carry.get<double>(), toDoubleOr( shift, 0.0 ) ) );
}
XLO_FUNC_END( aqMathBlackScholesDeltaSpot )
    .help( L"Black-Scholes spot delta." )
    .arg( L"CallOrPut", L"Call or Put" )
    .arg( L"Spot",      L"Spot" )
    .arg( L"Strike",    L"Strike" )
    .arg( L"Vol",       L"Volatility" )
    .arg( L"Time",      L"Time to expiry in years" )
    .arg( L"Rate",      L"Interest rate" )
    .arg( L"Carry",     L"Cost of carry" )
    .arg( L"Shift",     L"Optional. Lognormal shift. Default 0" );
#endif


// Black-Scholes gamma.
#if AQ_XLL_ENABLED(aqMathBlackScholesGamma)
XLO_FUNC_START( aqMathBlackScholesGamma(
    const ExcelObj& callOrPut,
    const ExcelObj& spot,
    const ExcelObj& strike,
    const ExcelObj& vol,
    const ExcelObj& time,
    const ExcelObj& rate,
    const ExcelObj& carry,
    const ExcelObj& shift ) )
{
    AQ_XLL_GUARD

    return returnValue( validation::tryAqMathBlackScholesGamma(
        etrading::toCallOrPutEnum( toNarrowString( callOrPut ) ),
        spot.get<double>(), strike.get<double>(), vol.get<double>(), time.get<double>(),
        rate.get<double>(), carry.get<double>(), toDoubleOr( shift, 0.0 ) ) );
}
XLO_FUNC_END( aqMathBlackScholesGamma )
    .help( L"Black-Scholes gamma." )
    .arg( L"CallOrPut", L"Call or Put" )
    .arg( L"Spot",      L"Spot" )
    .arg( L"Strike",    L"Strike" )
    .arg( L"Vol",       L"Volatility" )
    .arg( L"Time",      L"Time to expiry in years" )
    .arg( L"Rate",      L"Interest rate" )
    .arg( L"Carry",     L"Cost of carry" )
    .arg( L"Shift",     L"Optional. Lognormal shift. Default 0" );
#endif


// Black-Scholes vega.
#if AQ_XLL_ENABLED(aqMathBlackScholesVega)
XLO_FUNC_START( aqMathBlackScholesVega(
    const ExcelObj& callOrPut,
    const ExcelObj& spot,
    const ExcelObj& strike,
    const ExcelObj& vol,
    const ExcelObj& time,
    const ExcelObj& rate,
    const ExcelObj& carry,
    const ExcelObj& shift ) )
{
    AQ_XLL_GUARD

    return returnValue( validation::tryAqMathBlackScholesVega(
        etrading::toCallOrPutEnum( toNarrowString( callOrPut ) ),
        spot.get<double>(), strike.get<double>(), vol.get<double>(), time.get<double>(),
        rate.get<double>(), carry.get<double>(), toDoubleOr( shift, 0.0 ) ) );
}
XLO_FUNC_END( aqMathBlackScholesVega )
    .help( L"Black-Scholes vega." )
    .arg( L"CallOrPut", L"Call or Put" )
    .arg( L"Spot",      L"Spot" )
    .arg( L"Strike",    L"Strike" )
    .arg( L"Vol",       L"Volatility" )
    .arg( L"Time",      L"Time to expiry in years" )
    .arg( L"Rate",      L"Interest rate" )
    .arg( L"Carry",     L"Cost of carry" )
    .arg( L"Shift",     L"Optional. Lognormal shift. Default 0" );
#endif


// Black-Scholes theta.
#if AQ_XLL_ENABLED(aqMathBlackScholesTheta)
XLO_FUNC_START( aqMathBlackScholesTheta(
    const ExcelObj& callOrPut,
    const ExcelObj& spot,
    const ExcelObj& strike,
    const ExcelObj& vol,
    const ExcelObj& time,
    const ExcelObj& rate,
    const ExcelObj& carry,
    const ExcelObj& shift ) )
{
    AQ_XLL_GUARD

    return returnValue( validation::tryAqMathBlackScholesTheta(
        etrading::toCallOrPutEnum( toNarrowString( callOrPut ) ),
        spot.get<double>(), strike.get<double>(), vol.get<double>(), time.get<double>(),
        rate.get<double>(), carry.get<double>(), toDoubleOr( shift, 0.0 ) ) );
}
XLO_FUNC_END( aqMathBlackScholesTheta )
    .help( L"Black-Scholes theta." )
    .arg( L"CallOrPut", L"Call or Put" )
    .arg( L"Spot",      L"Spot" )
    .arg( L"Strike",    L"Strike" )
    .arg( L"Vol",       L"Volatility" )
    .arg( L"Time",      L"Time to expiry in years" )
    .arg( L"Rate",      L"Interest rate" )
    .arg( L"Carry",     L"Cost of carry" )
    .arg( L"Shift",     L"Optional. Lognormal shift. Default 0" );
#endif


// Black-Scholes rho.
#if AQ_XLL_ENABLED(aqMathBlackScholesRho)
XLO_FUNC_START( aqMathBlackScholesRho(
    const ExcelObj& callOrPut,
    const ExcelObj& spot,
    const ExcelObj& strike,
    const ExcelObj& vol,
    const ExcelObj& time,
    const ExcelObj& rate,
    const ExcelObj& carry,
    const ExcelObj& shift ) )
{
    AQ_XLL_GUARD

    return returnValue( validation::tryAqMathBlackScholesRho(
        etrading::toCallOrPutEnum( toNarrowString( callOrPut ) ),
        spot.get<double>(), strike.get<double>(), vol.get<double>(), time.get<double>(),
        rate.get<double>(), carry.get<double>(), toDoubleOr( shift, 0.0 ) ) );
}
XLO_FUNC_END( aqMathBlackScholesRho )
    .help( L"Black-Scholes rho." )
    .arg( L"CallOrPut", L"Call or Put" )
    .arg( L"Spot",      L"Spot" )
    .arg( L"Strike",    L"Strike" )
    .arg( L"Vol",       L"Volatility" )
    .arg( L"Time",      L"Time to expiry in years" )
    .arg( L"Rate",      L"Interest rate" )
    .arg( L"Carry",     L"Cost of carry" )
    .arg( L"Shift",     L"Optional. Lognormal shift. Default 0" );
#endif


/* =========================================================================
 *  Caplet / floorlet
 * ====================================================================== */

// Black caplet / floorlet price.
#if AQ_XLL_ENABLED(aqMathCapletFloorletPrice)
XLO_FUNC_START( aqMathCapletFloorletPrice(
    const ExcelObj& capletOrFloorlet,
    const ExcelObj& annuityFactor,
    const ExcelObj& liborRate,
    const ExcelObj& strike,
    const ExcelObj& vol,
    const ExcelObj& time,
    const ExcelObj& shift,
    const ExcelObj& volatilityType ) )
{
    AQ_XLL_GUARD

    return returnValue( validation::tryAqMathCapletFloorletPrice(
        toNarrowString( capletOrFloorlet ),
        annuityFactor.get<double>(), liborRate.get<double>(), strike.get<double>(),
        vol.get<double>(), time.get<double>(), toDoubleOr( shift, 0.0 ),
        toStrOr( volatilityType, "LOGNORMAL" ) ) );
}
XLO_FUNC_END( aqMathCapletFloorletPrice )
    .help( L"Black caplet / floorlet price." )
    .arg( L"CapletOrFloorlet", L"CAPLET or FLOORLET" )
    .arg( L"AnnuityFactor",    L"Discounted year-fraction of the period" )
    .arg( L"LiborRate",        L"Forward Libor rate for the period" )
    .arg( L"Strike",           L"Strike rate" )
    .arg( L"Vol",              L"Volatility" )
    .arg( L"Time",             L"Time to expiry in years" )
    .arg( L"Shift",            L"Optional. Lognormal shift. Default 0" )
    .arg( L"VolatilityType",   L"Optional. LOGNORMAL or NORMAL. Default LOGNORMAL" );
#endif


// Implied volatility from a caplet / floorlet price.
#if AQ_XLL_ENABLED(aqMathCapletFloorletImpliedVol)
XLO_FUNC_START( aqMathCapletFloorletImpliedVol(
    const ExcelObj& price,
    const ExcelObj& capletOrFloorlet,
    const ExcelObj& annuityFactor,
    const ExcelObj& liborRate,
    const ExcelObj& strike,
    const ExcelObj& time,
    const ExcelObj& shift,
    const ExcelObj& volatilityType ) )
{
    AQ_XLL_GUARD

    return returnValue( validation::tryAqMathCapletFloorletImpliedVol(
        price.get<double>(),
        toNarrowString( capletOrFloorlet ),
        annuityFactor.get<double>(), liborRate.get<double>(), strike.get<double>(),
        time.get<double>(), toDoubleOr( shift, 0.0 ),
        toStrOr( volatilityType, "LOGNORMAL" ) ) );
}
XLO_FUNC_END( aqMathCapletFloorletImpliedVol )
    .help( L"Implied volatility from a caplet / floorlet price." )
    .arg( L"Price",            L"The caplet / floorlet price" )
    .arg( L"CapletOrFloorlet", L"CAPLET or FLOORLET" )
    .arg( L"AnnuityFactor",    L"Discounted year-fraction of the period" )
    .arg( L"LiborRate",        L"Forward Libor rate for the period" )
    .arg( L"Strike",           L"Strike rate" )
    .arg( L"Time",             L"Time to expiry in years" )
    .arg( L"Shift",            L"Optional. Lognormal shift. Default 0" )
    .arg( L"VolatilityType",   L"Optional. LOGNORMAL or NORMAL. Default LOGNORMAL" );
#endif


/* =========================================================================
 *  Volatility conversion + Libor-in-arrears convexity
 * ====================================================================== */

#if AQ_XLL_ENABLED(aqMathVolatilityToNormalFromLognormal)
XLO_FUNC_START( aqMathVolatilityToNormalFromLognormal(
    const ExcelObj& lognormalVol,
    const ExcelObj& underlyingRate ) )
{
    AQ_XLL_GUARD
    return returnValue( validation::tryAqMathVolatilityToNormalFromLognormal(
        lognormalVol.get<double>(), underlyingRate.get<double>() ) );
}
XLO_FUNC_END( aqMathVolatilityToNormalFromLognormal )
    .help( L"Convert a lognormal volatility to a normal (basis-point) volatility." )
    .arg( L"LognormalVol",   L"The lognormal volatility" )
    .arg( L"UnderlyingRate", L"The underlying forward rate" );
#endif


#if AQ_XLL_ENABLED(aqMathVolatilityToLognormalFromNormal)
XLO_FUNC_START( aqMathVolatilityToLognormalFromNormal(
    const ExcelObj& normalVol,
    const ExcelObj& underlyingRate ) )
{
    AQ_XLL_GUARD
    return returnValue( validation::tryAqMathVolatilityToLognormalFromNormal(
        normalVol.get<double>(), underlyingRate.get<double>() ) );
}
XLO_FUNC_END( aqMathVolatilityToLognormalFromNormal )
    .help( L"Convert a normal (basis-point) volatility to a lognormal volatility." )
    .arg( L"NormalVol",      L"The normal volatility" )
    .arg( L"UnderlyingRate", L"The underlying forward rate" );
#endif


#if AQ_XLL_ENABLED(aqMathVolatilityToShiftedLognormalFromLognormal)
XLO_FUNC_START( aqMathVolatilityToShiftedLognormalFromLognormal(
    const ExcelObj& lognormalVol,
    const ExcelObj& underlyingRate,
    const ExcelObj& shiftSize ) )
{
    AQ_XLL_GUARD
    return returnValue( validation::tryAqMathVolatilityToShiftedLognormalFromLognormal(
        lognormalVol.get<double>(), underlyingRate.get<double>(), shiftSize.get<double>() ) );
}
XLO_FUNC_END( aqMathVolatilityToShiftedLognormalFromLognormal )
    .help( L"Convert a lognormal volatility to a shifted-lognormal volatility." )
    .arg( L"LognormalVol",   L"The lognormal volatility" )
    .arg( L"UnderlyingRate", L"The underlying forward rate" )
    .arg( L"ShiftSize",      L"The lognormal shift" );
#endif


#if AQ_XLL_ENABLED(aqMathVolatilityToLognormalFromShiftedLognormal)
XLO_FUNC_START( aqMathVolatilityToLognormalFromShiftedLognormal(
    const ExcelObj& shiftedLognormalVol,
    const ExcelObj& underlyingRate,
    const ExcelObj& shiftSize ) )
{
    AQ_XLL_GUARD
    return returnValue( validation::tryAqMathVolatilityToLognormalFromShiftedLognormal(
        shiftedLognormalVol.get<double>(), underlyingRate.get<double>(), shiftSize.get<double>() ) );
}
XLO_FUNC_END( aqMathVolatilityToLognormalFromShiftedLognormal )
    .help( L"Convert a shifted-lognormal volatility to a lognormal volatility." )
    .arg( L"ShiftedLognormalVol", L"The shifted-lognormal volatility" )
    .arg( L"UnderlyingRate",      L"The underlying forward rate" )
    .arg( L"ShiftSize",           L"The lognormal shift" );
#endif


#if AQ_XLL_ENABLED(aqMathVolatilityToShiftedLognormalFromNormal)
XLO_FUNC_START( aqMathVolatilityToShiftedLognormalFromNormal(
    const ExcelObj& normalVol,
    const ExcelObj& underlyingRate,
    const ExcelObj& shiftSize ) )
{
    AQ_XLL_GUARD
    return returnValue( validation::tryAqMathVolatilityToShiftedLognormalFromNormal(
        normalVol.get<double>(), underlyingRate.get<double>(), shiftSize.get<double>() ) );
}
XLO_FUNC_END( aqMathVolatilityToShiftedLognormalFromNormal )
    .help( L"Convert a normal volatility to a shifted-lognormal volatility." )
    .arg( L"NormalVol",      L"The normal volatility" )
    .arg( L"UnderlyingRate", L"The underlying forward rate" )
    .arg( L"ShiftSize",      L"The lognormal shift" );
#endif


#if AQ_XLL_ENABLED(aqMathVolatilityToNormalFromShiftedLognormal)
XLO_FUNC_START( aqMathVolatilityToNormalFromShiftedLognormal(
    const ExcelObj& shiftedLognormalVol,
    const ExcelObj& underlyingRate,
    const ExcelObj& shiftSize ) )
{
    AQ_XLL_GUARD
    return returnValue( validation::tryAqMathVolatilityToNormalFromShiftedLognormal(
        shiftedLognormalVol.get<double>(), underlyingRate.get<double>(), shiftSize.get<double>() ) );
}
XLO_FUNC_END( aqMathVolatilityToNormalFromShiftedLognormal )
    .help( L"Convert a shifted-lognormal volatility to a normal volatility." )
    .arg( L"ShiftedLognormalVol", L"The shifted-lognormal volatility" )
    .arg( L"UnderlyingRate",      L"The underlying forward rate" )
    .arg( L"ShiftSize",           L"The lognormal shift" );
#endif


#if AQ_XLL_ENABLED(aqMathLiborConvexityAdjustmentInArrears)
XLO_FUNC_START( aqMathLiborConvexityAdjustmentInArrears(
    const ExcelObj& liborRate,
    const ExcelObj& couponYearFraction,
    const ExcelObj& timeToMaturity,
    const ExcelObj& volatility,
    const ExcelObj& volatilityType,
    const ExcelObj& volatilityShift,
    const ExcelObj& useHullApproximation ) )
{
    AQ_XLL_GUARD

    return returnValue( validation::tryAqMathLiborConvexityAdjustmentInArrears(
        liborRate.get<double>(), couponYearFraction.get<double>(), timeToMaturity.get<double>(),
        volatility.get<double>(),
        etrading::toVolatilityTypeEnum( toNarrowString( volatilityType ) ),
        toDoubleOr( volatilityShift, 0.0 ), toBool( useHullApproximation, false ) ) );
}
XLO_FUNC_END( aqMathLiborConvexityAdjustmentInArrears )
    .help( L"Libor-in-arrears convexity adjustment." )
    .arg( L"LiborRate",           L"The forward Libor rate" )
    .arg( L"CouponYearFraction",  L"Year fraction of the coupon period" )
    .arg( L"TimeToMaturity",      L"Time to the fixing, in years" )
    .arg( L"Volatility",          L"Volatility of the Libor rate" )
    .arg( L"VolatilityType",      L"LOGNORMAL, NORMAL or SHIFTED_LOGNORMAL" )
    .arg( L"VolatilityShift",     L"Optional. Shift for a shifted-lognormal vol. Default 0" )
    .arg( L"UseHullApproximation", L"Optional. Default FALSE. Use Hull's approximation" );
#endif


#if AQ_XLL_ENABLED(aqMathLiborRateInArrears)
XLO_FUNC_START( aqMathLiborRateInArrears(
    const ExcelObj& liborRate,
    const ExcelObj& couponYearFraction,
    const ExcelObj& timeToMaturity,
    const ExcelObj& volatility,
    const ExcelObj& volatilityType,
    const ExcelObj& volatilityShift,
    const ExcelObj& useHullApproximation ) )
{
    AQ_XLL_GUARD

    return returnValue( validation::tryAqMathLiborRateInArrears(
        liborRate.get<double>(), couponYearFraction.get<double>(), timeToMaturity.get<double>(),
        volatility.get<double>(),
        etrading::toVolatilityTypeEnum( toNarrowString( volatilityType ) ),
        toDoubleOr( volatilityShift, 0.0 ), toBool( useHullApproximation, false ) ) );
}
XLO_FUNC_END( aqMathLiborRateInArrears )
    .help( L"Convexity-adjusted Libor-in-arrears rate." )
    .arg( L"LiborRate",           L"The forward Libor rate" )
    .arg( L"CouponYearFraction",  L"Year fraction of the coupon period" )
    .arg( L"TimeToMaturity",      L"Time to the fixing, in years" )
    .arg( L"Volatility",          L"Volatility of the Libor rate" )
    .arg( L"VolatilityType",      L"LOGNORMAL, NORMAL or SHIFTED_LOGNORMAL" )
    .arg( L"VolatilityShift",     L"Optional. Shift for a shifted-lognormal vol. Default 0" )
    .arg( L"UseHullApproximation", L"Optional. Default FALSE. Use Hull's approximation" );
#endif


#if AQ_XLL_ENABLED(aqMathLiborConvexityAdjustmentForArbitraryFixingDate)
XLO_FUNC_START( aqMathLiborConvexityAdjustmentForArbitraryFixingDate(
    const ExcelObj& liborRate,
    const ExcelObj& couponYearFraction,
    const ExcelObj& timeToMaturity,
    const ExcelObj& fixingDateYearFraction,
    const ExcelObj& startDiscountFactor,
    const ExcelObj& endDiscountFactor,
    const ExcelObj& volatility,
    const ExcelObj& volatilityType,
    const ExcelObj& volatilityShift,
    const ExcelObj& useHullApproximation ) )
{
    AQ_XLL_GUARD

    return returnValue( validation::tryAqMathLiborConvexityAdjustmentForArbitraryFixingDate(
        liborRate.get<double>(), couponYearFraction.get<double>(), timeToMaturity.get<double>(),
        fixingDateYearFraction.get<double>(), startDiscountFactor.get<double>(), endDiscountFactor.get<double>(),
        volatility.get<double>(),
        etrading::toVolatilityTypeEnum( toNarrowString( volatilityType ) ),
        toDoubleOr( volatilityShift, 0.0 ), toBool( useHullApproximation, false ) ) );
}
XLO_FUNC_END( aqMathLiborConvexityAdjustmentForArbitraryFixingDate )
    .help( L"Libor convexity adjustment for a fixing date that is not the period start." )
    .arg( L"LiborRate",              L"The forward Libor rate" )
    .arg( L"CouponYearFraction",     L"Year fraction of the coupon period" )
    .arg( L"TimeToMaturity",         L"Time to the period end, in years" )
    .arg( L"FixingDateYearFraction", L"Time to the fixing date, in years" )
    .arg( L"StartDiscountFactor",    L"Discount factor to the period start" )
    .arg( L"EndDiscountFactor",      L"Discount factor to the period end" )
    .arg( L"Volatility",             L"Volatility of the Libor rate" )
    .arg( L"VolatilityType",         L"LOGNORMAL, NORMAL or SHIFTED_LOGNORMAL" )
    .arg( L"VolatilityShift",        L"Optional. Shift for a shifted-lognormal vol. Default 0" )
    .arg( L"UseHullApproximation",   L"Optional. Default FALSE. Use Hull's approximation" );
#endif


#if AQ_XLL_ENABLED(aqMathLiborRateForArbitraryFixingDate)
XLO_FUNC_START( aqMathLiborRateForArbitraryFixingDate(
    const ExcelObj& liborRate,
    const ExcelObj& couponYearFraction,
    const ExcelObj& timeToMaturity,
    const ExcelObj& fixingDateYearFraction,
    const ExcelObj& startDiscountFactor,
    const ExcelObj& endDiscountFactor,
    const ExcelObj& volatility,
    const ExcelObj& volatilityType,
    const ExcelObj& volatilityShift,
    const ExcelObj& useHullApproximation ) )
{
    AQ_XLL_GUARD

    return returnValue( validation::tryAqMathLiborRateForArbitraryFixingDate(
        liborRate.get<double>(), couponYearFraction.get<double>(), timeToMaturity.get<double>(),
        fixingDateYearFraction.get<double>(), startDiscountFactor.get<double>(), endDiscountFactor.get<double>(),
        volatility.get<double>(),
        etrading::toVolatilityTypeEnum( toNarrowString( volatilityType ) ),
        toDoubleOr( volatilityShift, 0.0 ), toBool( useHullApproximation, false ) ) );
}
XLO_FUNC_END( aqMathLiborRateForArbitraryFixingDate )
    .help( L"Convexity-adjusted Libor rate for a fixing date that is not the period start." )
    .arg( L"LiborRate",              L"The forward Libor rate" )
    .arg( L"CouponYearFraction",     L"Year fraction of the coupon period" )
    .arg( L"TimeToMaturity",         L"Time to the period end, in years" )
    .arg( L"FixingDateYearFraction", L"Time to the fixing date, in years" )
    .arg( L"StartDiscountFactor",    L"Discount factor to the period start" )
    .arg( L"EndDiscountFactor",      L"Discount factor to the period end" )
    .arg( L"Volatility",             L"Volatility of the Libor rate" )
    .arg( L"VolatilityType",         L"LOGNORMAL, NORMAL or SHIFTED_LOGNORMAL" )
    .arg( L"VolatilityShift",        L"Optional. Shift for a shifted-lognormal vol. Default 0" )
    .arg( L"UseHullApproximation",   L"Optional. Default FALSE. Use Hull's approximation" );
#endif


/* =========================================================================
 *  European IR swaption - price, implied vol, Greeks, cash annuity
 * ====================================================================== */

#if AQ_XLL_ENABLED(aqMathEuropeanIRSwaptionPrice)
XLO_FUNC_START( aqMathEuropeanIRSwaptionPrice(
    const ExcelObj& payerReceiver,
    const ExcelObj& annuity,
    const ExcelObj& swapRate,
    const ExcelObj& strike,
    const ExcelObj& vol,
    const ExcelObj& time,
    const ExcelObj& shift,
    const ExcelObj& volatilityType ) )
{
    AQ_XLL_GUARD

    return returnValue( validation::tryAqMathEuropeanIRSwaptionPrice(
        toNarrowString( payerReceiver ),
        annuity.get<double>(), swapRate.get<double>(), strike.get<double>(),
        vol.get<double>(), time.get<double>(), toDoubleOr( shift, 0.0 ),
        toStrOr( volatilityType, "LOGNORMAL" ) ) );
}
XLO_FUNC_END( aqMathEuropeanIRSwaptionPrice )
    .help( L"European interest-rate swaption price (Black / Bachelier)." )
    .arg( L"PayerReceiver",  L"PAYER or RECEIVER" )
    .arg( L"Annuity",        L"The swap annuity (PV01 x 10000)" )
    .arg( L"SwapRate",       L"The forward swap rate" )
    .arg( L"Strike",         L"The strike rate" )
    .arg( L"Vol",            L"Volatility" )
    .arg( L"Time",           L"Time to expiry in years" )
    .arg( L"Shift",          L"Optional. Lognormal shift. Default 0" )
    .arg( L"VolatilityType", L"Optional. LOGNORMAL or NORMAL. Default LOGNORMAL" );
#endif


#if AQ_XLL_ENABLED(aqMathEuropeanIRSwaptionImpliedVol)
XLO_FUNC_START( aqMathEuropeanIRSwaptionImpliedVol(
    const ExcelObj& price,
    const ExcelObj& payerReceiver,
    const ExcelObj& annuity,
    const ExcelObj& swapRate,
    const ExcelObj& strike,
    const ExcelObj& time,
    const ExcelObj& shift,
    const ExcelObj& volatilityType ) )
{
    AQ_XLL_GUARD

    return returnValue( validation::tryAqMathEuropeanIRSwaptionImpliedVol(
        price.get<double>(),
        toNarrowString( payerReceiver ),
        annuity.get<double>(), swapRate.get<double>(), strike.get<double>(),
        time.get<double>(), toDoubleOr( shift, 0.0 ),
        toStrOr( volatilityType, "LOGNORMAL" ) ) );
}
XLO_FUNC_END( aqMathEuropeanIRSwaptionImpliedVol )
    .help( L"Implied volatility from a European interest-rate swaption price." )
    .arg( L"Price",          L"The swaption price" )
    .arg( L"PayerReceiver",  L"PAYER or RECEIVER" )
    .arg( L"Annuity",        L"The swap annuity" )
    .arg( L"SwapRate",       L"The forward swap rate" )
    .arg( L"Strike",         L"The strike rate" )
    .arg( L"Time",           L"Time to expiry in years" )
    .arg( L"Shift",          L"Optional. Lognormal shift. Default 0" )
    .arg( L"VolatilityType", L"Optional. LOGNORMAL or NORMAL. Default LOGNORMAL" );
#endif


#if AQ_XLL_ENABLED(aqMathEuropeanIRSwaptionDelta)
XLO_FUNC_START( aqMathEuropeanIRSwaptionDelta(
    const ExcelObj& payerReceiver,
    const ExcelObj& annuity,
    const ExcelObj& swapRate,
    const ExcelObj& strike,
    const ExcelObj& vol,
    const ExcelObj& time,
    const ExcelObj& shift,
    const ExcelObj& volatilityType ) )
{
    AQ_XLL_GUARD

    return returnValue( validation::tryAqMathEuropeanIRSwaptionDelta(
        toNarrowString( payerReceiver ),
        annuity.get<double>(), swapRate.get<double>(), strike.get<double>(),
        vol.get<double>(), time.get<double>(), toDoubleOr( shift, 0.0 ),
        toStrOr( volatilityType, "LOGNORMAL" ) ) );
}
XLO_FUNC_END( aqMathEuropeanIRSwaptionDelta )
    .help( L"European interest-rate swaption delta." )
    .arg( L"PayerReceiver",  L"PAYER or RECEIVER" )
    .arg( L"Annuity",        L"The swap annuity" )
    .arg( L"SwapRate",       L"The forward swap rate" )
    .arg( L"Strike",         L"The strike rate" )
    .arg( L"Vol",            L"Volatility" )
    .arg( L"Time",           L"Time to expiry in years" )
    .arg( L"Shift",          L"Optional. Lognormal shift. Default 0" )
    .arg( L"VolatilityType", L"Optional. LOGNORMAL or NORMAL. Default LOGNORMAL" );
#endif


#if AQ_XLL_ENABLED(aqMathEuropeanIRSwaptionGamma)
XLO_FUNC_START( aqMathEuropeanIRSwaptionGamma(
    const ExcelObj& payerReceiver,
    const ExcelObj& annuity,
    const ExcelObj& swapRate,
    const ExcelObj& strike,
    const ExcelObj& vol,
    const ExcelObj& time,
    const ExcelObj& shift,
    const ExcelObj& volatilityType ) )
{
    AQ_XLL_GUARD

    return returnValue( validation::tryAqMathEuropeanIRSwaptionGamma(
        toNarrowString( payerReceiver ),
        annuity.get<double>(), swapRate.get<double>(), strike.get<double>(),
        vol.get<double>(), time.get<double>(), toDoubleOr( shift, 0.0 ),
        toStrOr( volatilityType, "LOGNORMAL" ) ) );
}
XLO_FUNC_END( aqMathEuropeanIRSwaptionGamma )
    .help( L"European interest-rate swaption gamma." )
    .arg( L"PayerReceiver",  L"PAYER or RECEIVER" )
    .arg( L"Annuity",        L"The swap annuity" )
    .arg( L"SwapRate",       L"The forward swap rate" )
    .arg( L"Strike",         L"The strike rate" )
    .arg( L"Vol",            L"Volatility" )
    .arg( L"Time",           L"Time to expiry in years" )
    .arg( L"Shift",          L"Optional. Lognormal shift. Default 0" )
    .arg( L"VolatilityType", L"Optional. LOGNORMAL or NORMAL. Default LOGNORMAL" );
#endif


#if AQ_XLL_ENABLED(aqMathEuropeanIRSwaptionVega)
XLO_FUNC_START( aqMathEuropeanIRSwaptionVega(
    const ExcelObj& payerReceiver,
    const ExcelObj& annuity,
    const ExcelObj& swapRate,
    const ExcelObj& strike,
    const ExcelObj& vol,
    const ExcelObj& time,
    const ExcelObj& shift,
    const ExcelObj& volatilityType ) )
{
    AQ_XLL_GUARD

    return returnValue( validation::tryAqMathEuropeanIRSwaptionVega(
        toNarrowString( payerReceiver ),
        annuity.get<double>(), swapRate.get<double>(), strike.get<double>(),
        vol.get<double>(), time.get<double>(), toDoubleOr( shift, 0.0 ),
        toStrOr( volatilityType, "LOGNORMAL" ) ) );
}
XLO_FUNC_END( aqMathEuropeanIRSwaptionVega )
    .help( L"European interest-rate swaption vega." )
    .arg( L"PayerReceiver",  L"PAYER or RECEIVER" )
    .arg( L"Annuity",        L"The swap annuity" )
    .arg( L"SwapRate",       L"The forward swap rate" )
    .arg( L"Strike",         L"The strike rate" )
    .arg( L"Vol",            L"Volatility" )
    .arg( L"Time",           L"Time to expiry in years" )
    .arg( L"Shift",          L"Optional. Lognormal shift. Default 0" )
    .arg( L"VolatilityType", L"Optional. LOGNORMAL or NORMAL. Default LOGNORMAL" );
#endif


#if AQ_XLL_ENABLED(aqMathEuropeanIRSwaptionTheta)
XLO_FUNC_START( aqMathEuropeanIRSwaptionTheta(
    const ExcelObj& payerReceiver,
    const ExcelObj& annuity,
    const ExcelObj& swapRate,
    const ExcelObj& strike,
    const ExcelObj& vol,
    const ExcelObj& time,
    const ExcelObj& shift,
    const ExcelObj& volatilityType ) )
{
    AQ_XLL_GUARD

    return returnValue( validation::tryAqMathEuropeanIRSwaptionTheta(
        toNarrowString( payerReceiver ),
        annuity.get<double>(), swapRate.get<double>(), strike.get<double>(),
        vol.get<double>(), time.get<double>(), toDoubleOr( shift, 0.0 ),
        toStrOr( volatilityType, "LOGNORMAL" ) ) );
}
XLO_FUNC_END( aqMathEuropeanIRSwaptionTheta )
    .help( L"European interest-rate swaption theta." )
    .arg( L"PayerReceiver",  L"PAYER or RECEIVER" )
    .arg( L"Annuity",        L"The swap annuity" )
    .arg( L"SwapRate",       L"The forward swap rate" )
    .arg( L"Strike",         L"The strike rate" )
    .arg( L"Vol",            L"Volatility" )
    .arg( L"Time",           L"Time to expiry in years" )
    .arg( L"Shift",          L"Optional. Lognormal shift. Default 0" )
    .arg( L"VolatilityType", L"Optional. LOGNORMAL or NORMAL. Default LOGNORMAL" );
#endif


#if AQ_XLL_ENABLED(aqMathEuropeanIRSwaptionCashAnnuity)
XLO_FUNC_START( aqMathEuropeanIRSwaptionCashAnnuity(
    const ExcelObj& notional,
    const ExcelObj& swapRate,
    const ExcelObj& nCouponsPerYear,
    const ExcelObj& tenorInYears,
    const ExcelObj& stubType ) )
{
    AQ_XLL_GUARD

    return returnValue( validation::tryAqMathEuropeanIRSwaptionCashAnnuity(
        notional.get<double>(), swapRate.get<double>(),
        toUInt( nCouponsPerYear ), tenorInYears.get<double>(),
        toStrOr( stubType, "SHORT_START" ) ) );
}
XLO_FUNC_END( aqMathEuropeanIRSwaptionCashAnnuity )
    .help( L"Cash-settled annuity for a European interest-rate swaption." )
    .arg( L"Notional",        L"The swap notional" )
    .arg( L"SwapRate",        L"The forward swap rate" )
    .arg( L"NCouponsPerYear", L"Fixed-leg coupons per year" )
    .arg( L"TenorInYears",    L"Swap tenor in years" )
    .arg( L"StubType",        L"Optional. SHORT_START, LONG_START, SHORT_END, LONG_END. Default SHORT_START" );
#endif


/* =========================================================================
 *  Normal distribution
 * ====================================================================== */

#if AQ_XLL_ENABLED(aqMathNormalDistributionStandard)
XLO_FUNC_START( aqMathNormalDistributionStandard( const ExcelObj& z ) )
{
    AQ_XLL_GUARD
    return returnValue( validation::tryAqMathNormalDistributionStandard( z.get<double>() ) );
}
XLO_FUNC_END( aqMathNormalDistributionStandard )
    .help( L"Standard normal cumulative distribution function N(z)." )
    .arg( L"Z", L"The argument" );
#endif


#if AQ_XLL_ENABLED(aqMathNormalDistributionStandardPDF)
XLO_FUNC_START( aqMathNormalDistributionStandardPDF( const ExcelObj& z ) )
{
    AQ_XLL_GUARD
    return returnValue( validation::tryAqMathNormalDistributionStandardPDF( z.get<double>() ) );
}
XLO_FUNC_END( aqMathNormalDistributionStandardPDF )
    .help( L"Standard normal probability density function n(z)." )
    .arg( L"Z", L"The argument" );
#endif


#if AQ_XLL_ENABLED(aqMathNormalDistributionStandardInverse)
XLO_FUNC_START( aqMathNormalDistributionStandardInverse( const ExcelObj& phi ) )
{
    AQ_XLL_GUARD
    return returnValue( validation::tryAqMathNormalDistributionStandardInverse( phi.get<double>() ) );
}
XLO_FUNC_END( aqMathNormalDistributionStandardInverse )
    .help( L"Inverse standard normal CDF (probit)." )
    .arg( L"Phi", L"A probability in (0, 1)" );
#endif


#if AQ_XLL_ENABLED(aqMathNormalDistribution)
XLO_FUNC_START( aqMathNormalDistribution(
    const ExcelObj& x,
    const ExcelObj& mean,
    const ExcelObj& variance ) )
{
    AQ_XLL_GUARD
    return returnValue( validation::tryAqMathNormalDistribution(
        x.get<double>(), mean.get<double>(), variance.get<double>() ) );
}
XLO_FUNC_END( aqMathNormalDistribution )
    .help( L"Normal cumulative distribution function with a given mean and variance." )
    .arg( L"X",        L"The argument" )
    .arg( L"Mean",     L"Distribution mean" )
    .arg( L"Variance", L"Distribution variance" );
#endif


#if AQ_XLL_ENABLED(aqMathNormalDistributionPDF)
XLO_FUNC_START( aqMathNormalDistributionPDF(
    const ExcelObj& x,
    const ExcelObj& mean,
    const ExcelObj& variance ) )
{
    AQ_XLL_GUARD
    return returnValue( validation::tryAqMathNormalDistributionPDF(
        x.get<double>(), mean.get<double>(), variance.get<double>() ) );
}
XLO_FUNC_END( aqMathNormalDistributionPDF )
    .help( L"Normal probability density function with a given mean and variance." )
    .arg( L"X",        L"The argument" )
    .arg( L"Mean",     L"Distribution mean" )
    .arg( L"Variance", L"Distribution variance" );
#endif


#if AQ_XLL_ENABLED(aqMathNormalDistributionInverse)
XLO_FUNC_START( aqMathNormalDistributionInverse(
    const ExcelObj& phi,
    const ExcelObj& mean,
    const ExcelObj& variance ) )
{
    AQ_XLL_GUARD
    return returnValue( validation::tryAqMathNormalDistributionInverse(
        phi.get<double>(), mean.get<double>(), variance.get<double>() ) );
}
XLO_FUNC_END( aqMathNormalDistributionInverse )
    .help( L"Inverse normal CDF with a given mean and variance." )
    .arg( L"Phi",      L"A probability in (0, 1)" )
    .arg( L"Mean",     L"Distribution mean" )
    .arg( L"Variance", L"Distribution variance" );
#endif


/* =========================================================================
 *  Polynomial interpolation / integration
 * ====================================================================== */

#if AQ_XLL_ENABLED(aqMathPolynomialInterpolation)
XLO_FUNC_START( aqMathPolynomialInterpolation(
    const ExcelObj& xValues,
    const ExcelObj& yValues,
    const ExcelObj& degree,
    const ExcelObj& x ) )
{
    AQ_XLL_GUARD

    return returnValue( validation::tryAqMathPolynomialInterpolation(
        toDoubleVector( xValues, true, "XValues" ),
        toDoubleVector( yValues, true, "YValues" ),
        toUInt( degree ), x.get<double>() ) );
}
XLO_FUNC_END( aqMathPolynomialInterpolation )
    .help( L"Fit a polynomial of the given degree to (x, y) and evaluate it at X." )
    .arg( L"XValues", L"Column of x values" )
    .arg( L"YValues", L"Column of y values, aligned with XValues" )
    .arg( L"Degree",  L"Polynomial degree" )
    .arg( L"X",       L"The point to evaluate at" );
#endif


#if AQ_XLL_ENABLED(aqMathPolynomialInterpolations)
XLO_FUNC_START( aqMathPolynomialInterpolations(
    const ExcelObj& xValues,
    const ExcelObj& yValues,
    const ExcelObj& degree,
    const ExcelObj& x ) )
{
    AQ_XLL_GUARD

    return returnValue( toExcelDoubleColumn( validation::tryAqMathPolynomialInterpolations(
        toDoubleVector( xValues, true, "XValues" ),
        toDoubleVector( yValues, true, "YValues" ),
        toUInt( degree ),
        toDoubleVector( x, true, "X" ) ) ) );
}
XLO_FUNC_END( aqMathPolynomialInterpolations )
    .help( L"Fit a polynomial of the given degree to (x, y) and evaluate it at a column of points." )
    .arg( L"XValues", L"Column of x values" )
    .arg( L"YValues", L"Column of y values, aligned with XValues" )
    .arg( L"Degree",  L"Polynomial degree" )
    .arg( L"X",       L"Column of points to evaluate at" );
#endif


#if AQ_XLL_ENABLED(aqMathPoynomialIntegration)
XLO_FUNC_START( aqMathPoynomialIntegration(
    const ExcelObj& xValues,
    const ExcelObj& yValues,
    const ExcelObj& degree,
    const ExcelObj& lowerBound,
    const ExcelObj& upperBound ) )
{
    AQ_XLL_GUARD

    return returnValue( validation::tryAqMathPoynomialIntegration(
        toDoubleVector( xValues, true, "XValues" ),
        toDoubleVector( yValues, true, "YValues" ),
        toUInt( degree ), lowerBound.get<double>(), upperBound.get<double>() ) );
}
XLO_FUNC_END( aqMathPoynomialIntegration )
    .help( L"Fit a polynomial to (x, y) and integrate it between two bounds." )
    .arg( L"XValues",    L"Column of x values" )
    .arg( L"YValues",    L"Column of y values, aligned with XValues" )
    .arg( L"Degree",     L"Polynomial degree" )
    .arg( L"LowerBound", L"Lower integration bound" )
    .arg( L"UpperBound", L"Upper integration bound" );
#endif


#if AQ_XLL_ENABLED(aqMathPoynomialIntegrations)
XLO_FUNC_START( aqMathPoynomialIntegrations(
    const ExcelObj& xValues,
    const ExcelObj& yValues,
    const ExcelObj& degree,
    const ExcelObj& lowerBounds,
    const ExcelObj& upperBounds ) )
{
    AQ_XLL_GUARD

    return returnValue( toExcelDoubleColumn( validation::tryAqMathPoynomialIntegrations(
        toDoubleVector( xValues, true, "XValues" ),
        toDoubleVector( yValues, true, "YValues" ),
        toUInt( degree ),
        toDoubleVector( lowerBounds, true, "LowerBounds" ),
        toDoubleVector( upperBounds, true, "UpperBounds" ) ) ) );
}
XLO_FUNC_END( aqMathPoynomialIntegrations )
    .help( L"Fit a polynomial to (x, y) and integrate it over each (lower, upper) bound pair." )
    .arg( L"XValues",     L"Column of x values" )
    .arg( L"YValues",     L"Column of y values, aligned with XValues" )
    .arg( L"Degree",      L"Polynomial degree" )
    .arg( L"LowerBounds", L"Column of lower bounds" )
    .arg( L"UpperBounds", L"Column of upper bounds, aligned with LowerBounds" );
#endif


/* =========================================================================
 *  Curve integration
 * ====================================================================== */

#if AQ_XLL_ENABLED(aqMathIntegrateUsingTerms)
XLO_FUNC_START( aqMathIntegrateUsingTerms(
    const ExcelObj& terms,
    const ExcelObj& values,
    const ExcelObj& interpolationType,
    const ExcelObj& joinDateAsDouble,
    const ExcelObj& lowerBounds,
    const ExcelObj& upperBounds,
    const ExcelObj& nSteps,
    const ExcelObj& optimize ) )
{
    AQ_XLL_GUARD

    return returnValue( toExcelDoubleColumn( validation::tryAqMathIntegrateUsingTerms(
        toDoubleVector( terms, true, "Terms" ),
        toDoubleVector( values, true, "Values" ),
        toNarrowString( interpolationType ),
        joinDateAsDouble.get<double>(),
        toDoubleVector( lowerBounds, true, "LowerBounds" ),
        toDoubleVector( upperBounds, true, "UpperBounds" ),
        toUInt( nSteps ),
        toBool( optimize, false ) ) ) );
}
XLO_FUNC_END( aqMathIntegrateUsingTerms )
    .help( L"Integrate an interpolated (term, value) curve over each (lower, upper) term-bound pair." )
    .arg( L"Terms",             L"Column of terms in years" )
    .arg( L"Values",            L"Column of values, aligned with Terms" )
    .arg( L"InterpolationType", L"Interpolation method, e.g. LINEAR, MONOTONE_CONVEX" )
    .arg( L"JoinDateAsDouble",  L"Join term for a piecewise scheme (0 for none)" )
    .arg( L"LowerBounds",       L"Column of lower term bounds" )
    .arg( L"UpperBounds",       L"Column of upper term bounds, aligned with LowerBounds" )
    .arg( L"NSteps",            L"Number of integration steps" )
    .arg( L"Optimize",          L"Optional. Default FALSE" );
#endif


#if AQ_XLL_ENABLED(aqMathIntegrate)
XLO_FUNC_START( aqMathIntegrate(
    const ExcelObj& asOfDate,
    const ExcelObj& dates,
    const ExcelObj& values,
    const ExcelObj& interpolationType,
    const ExcelObj& joinDate,
    const ExcelObj& lowerBoundDates,
    const ExcelObj& upperBoundDates,
    const ExcelObj& nSteps,
    const ExcelObj& optimize ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const AQLDate joinDateArg =
        ( joinDate.isMissing() || !joinDate.isNonEmpty() ) ? AQLDate() : toAQLDate( joinDate );

    return returnValue( toExcelDoubleColumn( validation::tryAqMathIntegrate(
        toAQLDate( asOfDate ),
        toDateVector( dates, true, "Dates" ),
        toDoubleVector( values, true, "Values" ),
        toNarrowString( interpolationType ),
        joinDateArg,
        toDateVector( lowerBoundDates, true, "LowerBoundDates" ),
        toDateVector( upperBoundDates, true, "UpperBoundDates" ),
        toUInt( nSteps ),
        toBool( optimize, false ) ) ) );
}
XLO_FUNC_END( aqMathIntegrate )
    .help( L"Integrate an interpolated (date, value) curve over each (lower, upper) date-bound pair." )
    .arg( L"AsOfDate",          L"The anchor date for time measurement" )
    .arg( L"Dates",             L"Column of curve dates" )
    .arg( L"Values",            L"Column of values, aligned with Dates" )
    .arg( L"InterpolationType", L"Interpolation method, e.g. LINEAR, MONOTONE_CONVEX" )
    .arg( L"JoinDate",          L"Join date for a piecewise scheme (blank for none)" )
    .arg( L"LowerBoundDates",   L"Column of lower bound dates" )
    .arg( L"UpperBoundDates",   L"Column of upper bound dates, aligned with LowerBoundDates" )
    .arg( L"NSteps",            L"Number of integration steps" )
    .arg( L"Optimize",          L"Optional. Default FALSE" );
#endif


/* -------------------------------------------------------------------------
 *  Vector overloads (optimize=true uses OMP threading) - deferred earlier
 *  this session alongside the Curve/Bond vector forms, now ported.
 * ---------------------------------------------------------------------- */

#if AQ_XLL_ENABLED(aqMathBlackScholesPrices)
XLO_FUNC_START( aqMathBlackScholesPrices(
    const ExcelObj& callOrPut,
    const ExcelObj& spot,
    const ExcelObj& strike,
    const ExcelObj& vol,
    const ExcelObj& time,
    const ExcelObj& rate,
    const ExcelObj& carry,
    const ExcelObj& shift,
    const ExcelObj& optimize ) )
{
    AQ_XLL_GUARD

    return returnValue( toExcelDoubleColumn( validation::tryAqMathBlackScholesPrices(
        toCallOrPutEnumVector( callOrPut ),
        toDoubleVector( spot, true, "Spot" ), toDoubleVector( strike, true, "Strike" ),
        toDoubleVector( vol, true, "Vol" ), toDoubleVector( time, true, "Time" ),
        toDoubleVector( rate, true, "Rate" ), toDoubleVector( carry, true, "Carry" ),
        toDoubleVectorOr( shift, "Shift" ), toBool( optimize, false ) ) ) );
}
XLO_FUNC_END( aqMathBlackScholesPrices )
    .help( L"Black-Scholes prices for a column of options." )
    .arg( L"CallOrPut", L"Column of CALL/PUT" )
    .arg( L"Spot",      L"Column of spot prices" )
    .arg( L"Strike",    L"Column of strikes" )
    .arg( L"Vol",       L"Column of volatilities" )
    .arg( L"Time",      L"Column of times to expiry, in years" )
    .arg( L"Rate",      L"Column of risk-free rates" )
    .arg( L"Carry",     L"Column of cost-of-carry rates" )
    .arg( L"Shift",     L"Optional. Column of shifts" )
    .arg( L"Optimize",  L"Optional. Default FALSE. Use OMP threading" );
#endif


#if AQ_XLL_ENABLED(aqMathBlackScholesImpliedVols)
XLO_FUNC_START( aqMathBlackScholesImpliedVols(
    const ExcelObj& price,
    const ExcelObj& callOrPut,
    const ExcelObj& spot,
    const ExcelObj& strike,
    const ExcelObj& time,
    const ExcelObj& rate,
    const ExcelObj& carry,
    const ExcelObj& shift,
    const ExcelObj& optimize ) )
{
    AQ_XLL_GUARD

    return returnValue( toExcelDoubleColumn( validation::tryAqMathBlackScholesImpliedVols(
        toDoubleVector( price, true, "Price" ), toCallOrPutEnumVector( callOrPut ),
        toDoubleVector( spot, true, "Spot" ), toDoubleVector( strike, true, "Strike" ),
        toDoubleVector( time, true, "Time" ), toDoubleVector( rate, true, "Rate" ),
        toDoubleVector( carry, true, "Carry" ),
        toDoubleVectorOr( shift, "Shift" ), toBool( optimize, false ) ) ) );
}
XLO_FUNC_END( aqMathBlackScholesImpliedVols )
    .help( L"Black-Scholes implied volatilities for a column of options." )
    .arg( L"Price",     L"Column of option prices" )
    .arg( L"CallOrPut", L"Column of CALL/PUT" )
    .arg( L"Spot",      L"Column of spot prices" )
    .arg( L"Strike",    L"Column of strikes" )
    .arg( L"Time",      L"Column of times to expiry, in years" )
    .arg( L"Rate",      L"Column of risk-free rates" )
    .arg( L"Carry",     L"Column of cost-of-carry rates" )
    .arg( L"Shift",     L"Optional. Column of shifts" )
    .arg( L"Optimize",  L"Optional. Default FALSE. Use OMP threading" );
#endif


#if AQ_XLL_ENABLED(aqMathCapletFloorletPrices)
XLO_FUNC_START( aqMathCapletFloorletPrices(
    const ExcelObj& capletOrFloorlet,
    const ExcelObj& annuityFactor,
    const ExcelObj& liborRate,
    const ExcelObj& strike,
    const ExcelObj& vol,
    const ExcelObj& time,
    const ExcelObj& shift,
    const ExcelObj& volatilityType,
    const ExcelObj& optimize ) )
{
    AQ_XLL_GUARD

    return returnValue( toExcelDoubleColumn( validation::tryAqMathCapletFloorletPrices(
        toStringVector( capletOrFloorlet, true ),
        toDoubleVector( annuityFactor, true, "AnnuityFactor" ), toDoubleVector( liborRate, true, "LiborRate" ),
        toDoubleVector( strike, true, "Strike" ), toDoubleVector( vol, true, "Vol" ), toDoubleVector( time, true, "Time" ),
        toDoubleVectorOr( shift, "Shift" ), toStringVectorOr( volatilityType ), toBool( optimize, false ) ) ) );
}
XLO_FUNC_END( aqMathCapletFloorletPrices )
    .help( L"Caplet/floorlet prices for a column of options." )
    .arg( L"CapletOrFloorlet", L"Column of CAPLET/FLOORLET" )
    .arg( L"AnnuityFactor",    L"Column of annuity factors" )
    .arg( L"LiborRate",        L"Column of forward Libor rates" )
    .arg( L"Strike",           L"Column of strikes" )
    .arg( L"Vol",              L"Column of volatilities" )
    .arg( L"Time",             L"Column of times to expiry, in years" )
    .arg( L"Shift",            L"Optional. Column of shifts" )
    .arg( L"VolatilityType",   L"Optional. Column of LOGNORMAL/NORMAL, default LOGNORMAL" )
    .arg( L"Optimize",         L"Optional. Default FALSE. Use OMP threading" );
#endif


#if AQ_XLL_ENABLED(aqMathCapletFloorletImpliedVols)
XLO_FUNC_START( aqMathCapletFloorletImpliedVols(
    const ExcelObj& price,
    const ExcelObj& capletOrFloorlet,
    const ExcelObj& annuityFactor,
    const ExcelObj& liborRate,
    const ExcelObj& strike,
    const ExcelObj& time,
    const ExcelObj& shift,
    const ExcelObj& volatilityType,
    const ExcelObj& optimize ) )
{
    AQ_XLL_GUARD

    return returnValue( toExcelDoubleColumn( validation::tryAqMathCapletFloorletImpliedVols(
        toDoubleVector( price, true, "Price" ), toStringVector( capletOrFloorlet, true ),
        toDoubleVector( annuityFactor, true, "AnnuityFactor" ), toDoubleVector( liborRate, true, "LiborRate" ),
        toDoubleVector( strike, true, "Strike" ), toDoubleVector( time, true, "Time" ),
        toDoubleVectorOr( shift, "Shift" ), toStringVectorOr( volatilityType ), toBool( optimize, false ) ) ) );
}
XLO_FUNC_END( aqMathCapletFloorletImpliedVols )
    .help( L"Caplet/floorlet implied volatilities for a column of options." )
    .arg( L"Price",            L"Column of option prices" )
    .arg( L"CapletOrFloorlet", L"Column of CAPLET/FLOORLET" )
    .arg( L"AnnuityFactor",    L"Column of annuity factors" )
    .arg( L"LiborRate",        L"Column of forward Libor rates" )
    .arg( L"Strike",           L"Column of strikes" )
    .arg( L"Time",             L"Column of times to expiry, in years" )
    .arg( L"Shift",            L"Optional. Column of shifts" )
    .arg( L"VolatilityType",   L"Optional. Column of LOGNORMAL/NORMAL, default LOGNORMAL" )
    .arg( L"Optimize",         L"Optional. Default FALSE. Use OMP threading" );
#endif


#if AQ_XLL_ENABLED(aqMathEuropeanIRSwaptionPrices)
XLO_FUNC_START( aqMathEuropeanIRSwaptionPrices(
    const ExcelObj& payerReceiver,
    const ExcelObj& annuity,
    const ExcelObj& swapRate,
    const ExcelObj& strike,
    const ExcelObj& vol,
    const ExcelObj& time,
    const ExcelObj& shift,
    const ExcelObj& volatilityType,
    const ExcelObj& optimize ) )
{
    AQ_XLL_GUARD

    return returnValue( toExcelDoubleColumn( validation::tryAqMathEuropeanIRSwaptionPrices(
        toStringVector( payerReceiver, true ),
        toDoubleVector( annuity, true, "Annuity" ), toDoubleVector( swapRate, true, "SwapRate" ),
        toDoubleVector( strike, true, "Strike" ), toDoubleVector( vol, true, "Vol" ), toDoubleVector( time, true, "Time" ),
        toDoubleVectorOr( shift, "Shift" ), toStringVectorOr( volatilityType ), toBool( optimize, false ) ) ) );
}
XLO_FUNC_END( aqMathEuropeanIRSwaptionPrices )
    .help( L"European IR swaption prices for a column of options." )
    .arg( L"PayerReceiver",  L"Column of PAYER/RECEIVER" )
    .arg( L"Annuity",        L"Column of annuities" )
    .arg( L"SwapRate",       L"Column of forward swap rates" )
    .arg( L"Strike",         L"Column of strikes" )
    .arg( L"Vol",            L"Column of volatilities" )
    .arg( L"Time",           L"Column of times to expiry, in years" )
    .arg( L"Shift",          L"Optional. Column of shifts" )
    .arg( L"VolatilityType", L"Optional. Column of LOGNORMAL/NORMAL, default LOGNORMAL" )
    .arg( L"Optimize",       L"Optional. Default FALSE. Use OMP threading" );
#endif


#if AQ_XLL_ENABLED(aqMathEuropeanIRSwaptionImpliedVols)
XLO_FUNC_START( aqMathEuropeanIRSwaptionImpliedVols(
    const ExcelObj& price,
    const ExcelObj& payerReceiver,
    const ExcelObj& annuity,
    const ExcelObj& swapRate,
    const ExcelObj& strike,
    const ExcelObj& time,
    const ExcelObj& shift,
    const ExcelObj& volatilityType,
    const ExcelObj& optimize ) )
{
    AQ_XLL_GUARD

    return returnValue( toExcelDoubleColumn( validation::tryAqMathEuropeanIRSwaptionImpliedVols(
        toDoubleVector( price, true, "Price" ), toStringVector( payerReceiver, true ),
        toDoubleVector( annuity, true, "Annuity" ), toDoubleVector( swapRate, true, "SwapRate" ),
        toDoubleVector( strike, true, "Strike" ), toDoubleVector( time, true, "Time" ),
        toDoubleVectorOr( shift, "Shift" ), toStringVectorOr( volatilityType ), toBool( optimize, false ) ) ) );
}
XLO_FUNC_END( aqMathEuropeanIRSwaptionImpliedVols )
    .help( L"European IR swaption implied volatilities for a column of options." )
    .arg( L"Price",          L"Column of option prices" )
    .arg( L"PayerReceiver",  L"Column of PAYER/RECEIVER" )
    .arg( L"Annuity",        L"Column of annuities" )
    .arg( L"SwapRate",       L"Column of forward swap rates" )
    .arg( L"Strike",         L"Column of strikes" )
    .arg( L"Time",           L"Column of times to expiry, in years" )
    .arg( L"Shift",          L"Optional. Column of shifts" )
    .arg( L"VolatilityType", L"Optional. Column of LOGNORMAL/NORMAL, default LOGNORMAL" )
    .arg( L"Optimize",       L"Optional. Default FALSE. Use OMP threading" );
#endif


/* -------------------------------------------------------------------------
 *  Low-level forward-rate / discount-factor primitives, direct from a raw
 *  (dates, values) curve fit. Each of these validation wrappers also has a
 *  "*** LEGACY METHOD ***" overload (curveCollection/curveIndex based,
 *  explicitly marked legacy in the header) - only the non-legacy overload
 *  is ported here, per that labelling.
 * ---------------------------------------------------------------------- */

#if AQ_XLL_ENABLED(aqMathForwardRate)
XLO_FUNC_START( aqMathForwardRate(
    const ExcelObj& fixingDate,
    const ExcelObj& asOfDate,
    const ExcelObj& interpolation,
    const ExcelObj& stateVariable,
    const ExcelObj& xValues,
    const ExcelObj& yValues,
    const ExcelObj& accrualDaycount,
    const ExcelObj& curveFrequencyTenor,
    const ExcelObj& fixingBusinessDayAdjustment,
    const ExcelObj& fixingCalendar,
    const ExcelObj& joinDate,
    const ExcelObj& polynomialOrder,
    const ExcelObj& forwardAdjustmentTable,
    const ExcelObj& compoundFreq ) )
{
    AQ_XLL_GUARD

    return returnValue( validation::tryAqMathForwardRate(
        toAQLDate( fixingDate ), toAQLDate( asOfDate ),
        etrading::toInterpolationEnum( toNarrowString( interpolation ) ),
        etrading::toStateVariableEnum( toNarrowString( stateVariable ) ),
        toDateVector( xValues, true, "XValues" ), toDoubleVector( yValues, true, "YValues" ),
        etrading::toDayCountEnum( toNarrowString( accrualDaycount ) ), toNarrowString( curveFrequencyTenor ),
        etrading::toBusinessDayAdjustmentEnum( toStrOr( fixingBusinessDayAdjustment, "NONE" ) ), toNarrowString( fixingCalendar ),
        joinDate.isMissing() || !joinDate.isNonEmpty() ? AQLDate() : toAQLDate( joinDate ),
        static_cast<size_t>( toIntOr( polynomialOrder, 0 ) ),
        toStandardStringMatrixOr( forwardAdjustmentTable ),
        etrading::toCompoundingFrequencyEnum( toStrOr( compoundFreq, "SIMPLE" ) ) ) );
}
XLO_FUNC_END( aqMathForwardRate )
    .help( L"Forward rate at a fixing date, from a raw (dates, values) curve fit." )
    .arg( L"FixingDate",                   L"The fixing date" )
    .arg( L"AsOfDate",                     L"The curve as-of date" )
    .arg( L"Interpolation",                L"Interpolation method, e.g. LINEAR, MONOTONE_CONVEX" )
    .arg( L"StateVariable",                L"The interpolated state variable, e.g. FORWARD_RATE, DISCOUNT_FACTOR" )
    .arg( L"XValues",                      L"Column of curve pillar dates" )
    .arg( L"YValues",                      L"Column of curve pillar values, aligned with XValues" )
    .arg( L"AccrualDaycount",              L"Day count convention" )
    .arg( L"CurveFrequencyTenor",          L"The curve's own tenor, e.g. 3M" )
    .arg( L"FixingBusinessDayAdjustment",  L"Optional. Default NONE" )
    .arg( L"FixingCalendar",               L"Holiday centre(s) for the fixing" )
    .arg( L"JoinDate",                     L"Optional. Piecewise-scheme join date" )
    .arg( L"PolynomialOrder",              L"Optional. Default 0" )
    .arg( L"ForwardAdjustmentTable",       L"Optional. A forward-adjustment override table" )
    .arg( L"CompoundFreq",                 L"Optional. Default SIMPLE" );
#endif


#if AQ_XLL_ENABLED(aqMathForwardRates)
XLO_FUNC_START( aqMathForwardRates(
    const ExcelObj& fixingDates,
    const ExcelObj& asOfDate,
    const ExcelObj& interpolation,
    const ExcelObj& stateVariable,
    const ExcelObj& xValues,
    const ExcelObj& yValues,
    const ExcelObj& accrualDaycount,
    const ExcelObj& curveFrequencyTenor,
    const ExcelObj& fixingBusinessDayAdjustment,
    const ExcelObj& fixingCalendar,
    const ExcelObj& joinDate,
    const ExcelObj& polynomialOrder,
    const ExcelObj& forwardAdjustmentTable,
    const ExcelObj& compoundFreq ) )
{
    AQ_XLL_GUARD

    return returnValue( toExcelDoubleColumn( validation::tryAqMathForwardRates(
        toDateVector( fixingDates, true, "FixingDates" ), toAQLDate( asOfDate ),
        etrading::toInterpolationEnum( toNarrowString( interpolation ) ),
        etrading::toStateVariableEnum( toNarrowString( stateVariable ) ),
        toDateVector( xValues, true, "XValues" ), toDoubleVector( yValues, true, "YValues" ),
        etrading::toDayCountEnum( toNarrowString( accrualDaycount ) ), toNarrowString( curveFrequencyTenor ),
        etrading::toBusinessDayAdjustmentEnum( toStrOr( fixingBusinessDayAdjustment, "NONE" ) ), toNarrowString( fixingCalendar ),
        joinDate.isMissing() || !joinDate.isNonEmpty() ? AQLDate() : toAQLDate( joinDate ),
        static_cast<size_t>( toIntOr( polynomialOrder, 0 ) ),
        toStandardStringMatrixOr( forwardAdjustmentTable ),
        etrading::toCompoundingFrequencyEnum( toStrOr( compoundFreq, "SIMPLE" ) ) ) ) );
}
XLO_FUNC_END( aqMathForwardRates )
    .help( L"Forward rates at a column of fixing dates, from a raw (dates, values) curve fit." )
    .arg( L"FixingDates",                  L"Column of fixing dates" )
    .arg( L"AsOfDate",                     L"The curve as-of date" )
    .arg( L"Interpolation",                L"Interpolation method, e.g. LINEAR, MONOTONE_CONVEX" )
    .arg( L"StateVariable",                L"The interpolated state variable, e.g. FORWARD_RATE, DISCOUNT_FACTOR" )
    .arg( L"XValues",                      L"Column of curve pillar dates" )
    .arg( L"YValues",                      L"Column of curve pillar values, aligned with XValues" )
    .arg( L"AccrualDaycount",              L"Day count convention" )
    .arg( L"CurveFrequencyTenor",          L"The curve's own tenor, e.g. 3M" )
    .arg( L"FixingBusinessDayAdjustment",  L"Optional. Default NONE" )
    .arg( L"FixingCalendar",               L"Holiday centre(s) for the fixing" )
    .arg( L"JoinDate",                     L"Optional. Piecewise-scheme join date" )
    .arg( L"PolynomialOrder",              L"Optional. Default 0" )
    .arg( L"ForwardAdjustmentTable",       L"Optional. A forward-adjustment override table" )
    .arg( L"CompoundFreq",                 L"Optional. Default SIMPLE" );
#endif


#if AQ_XLL_ENABLED(aqMathDiscountFactor)
XLO_FUNC_START( aqMathDiscountFactor(
    const ExcelObj& paymentDate,
    const ExcelObj& asOfDate,
    const ExcelObj& interpolation,
    const ExcelObj& stateVariable,
    const ExcelObj& xValues,
    const ExcelObj& yValues,
    const ExcelObj& accrualDaycount,
    const ExcelObj& curveFrequencyTenor,
    const ExcelObj& fixingBusinessDayAdjustment,
    const ExcelObj& fixingCalendar,
    const ExcelObj& joinDate,
    const ExcelObj& polynomialOrder,
    const ExcelObj& forwardAdjustmentTable,
    const ExcelObj& compoundFreq ) )
{
    AQ_XLL_GUARD

    return returnValue( validation::tryAqMathDiscountFactor(
        toAQLDate( paymentDate ), toAQLDate( asOfDate ),
        etrading::toInterpolationEnum( toNarrowString( interpolation ) ),
        etrading::toStateVariableEnum( toNarrowString( stateVariable ) ),
        toDateVector( xValues, true, "XValues" ), toDoubleVector( yValues, true, "YValues" ),
        etrading::toDayCountEnum( toNarrowString( accrualDaycount ) ), toNarrowString( curveFrequencyTenor ),
        etrading::toBusinessDayAdjustmentEnum( toStrOr( fixingBusinessDayAdjustment, "NONE" ) ), toNarrowString( fixingCalendar ),
        joinDate.isMissing() || !joinDate.isNonEmpty() ? AQLDate() : toAQLDate( joinDate ),
        static_cast<size_t>( toIntOr( polynomialOrder, 0 ) ),
        toStandardStringMatrixOr( forwardAdjustmentTable ),
        etrading::toCompoundingFrequencyEnum( toStrOr( compoundFreq, "SIMPLE" ) ) ) );
}
XLO_FUNC_END( aqMathDiscountFactor )
    .help( L"Discount factor at a payment date, from a raw (dates, values) curve fit." )
    .arg( L"PaymentDate",                  L"The payment date" )
    .arg( L"AsOfDate",                     L"The curve as-of date" )
    .arg( L"Interpolation",                L"Interpolation method, e.g. LINEAR, MONOTONE_CONVEX" )
    .arg( L"StateVariable",                L"The interpolated state variable, e.g. FORWARD_RATE, DISCOUNT_FACTOR" )
    .arg( L"XValues",                      L"Column of curve pillar dates" )
    .arg( L"YValues",                      L"Column of curve pillar values, aligned with XValues" )
    .arg( L"AccrualDaycount",              L"Day count convention" )
    .arg( L"CurveFrequencyTenor",          L"The curve's own tenor, e.g. 3M" )
    .arg( L"FixingBusinessDayAdjustment",  L"Optional. Default NONE" )
    .arg( L"FixingCalendar",               L"Holiday centre(s) for the fixing" )
    .arg( L"JoinDate",                     L"Optional. Piecewise-scheme join date" )
    .arg( L"PolynomialOrder",              L"Optional. Default 0" )
    .arg( L"ForwardAdjustmentTable",       L"Optional. A forward-adjustment override table" )
    .arg( L"CompoundFreq",                 L"Optional. Default SIMPLE" );
#endif


#if AQ_XLL_ENABLED(aqMathDiscountFactors)
XLO_FUNC_START( aqMathDiscountFactors(
    const ExcelObj& paymentDates,
    const ExcelObj& asOfDate,
    const ExcelObj& interpolation,
    const ExcelObj& stateVariable,
    const ExcelObj& xValues,
    const ExcelObj& yValues,
    const ExcelObj& accrualDaycount,
    const ExcelObj& curveFrequencyTenor,
    const ExcelObj& fixingBusinessDayAdjustment,
    const ExcelObj& fixingCalendar,
    const ExcelObj& joinDate,
    const ExcelObj& polynomialOrder,
    const ExcelObj& forwardAdjustmentTable,
    const ExcelObj& compoundFreq ) )
{
    AQ_XLL_GUARD

    return returnValue( toExcelDoubleColumn( validation::tryAqMathDiscountFactors(
        toDateVector( paymentDates, true, "PaymentDates" ), toAQLDate( asOfDate ),
        etrading::toInterpolationEnum( toNarrowString( interpolation ) ),
        etrading::toStateVariableEnum( toNarrowString( stateVariable ) ),
        toDateVector( xValues, true, "XValues" ), toDoubleVector( yValues, true, "YValues" ),
        etrading::toDayCountEnum( toNarrowString( accrualDaycount ) ), toNarrowString( curveFrequencyTenor ),
        etrading::toBusinessDayAdjustmentEnum( toStrOr( fixingBusinessDayAdjustment, "NONE" ) ), toNarrowString( fixingCalendar ),
        joinDate.isMissing() || !joinDate.isNonEmpty() ? AQLDate() : toAQLDate( joinDate ),
        static_cast<size_t>( toIntOr( polynomialOrder, 0 ) ),
        toStandardStringMatrixOr( forwardAdjustmentTable ),
        etrading::toCompoundingFrequencyEnum( toStrOr( compoundFreq, "SIMPLE" ) ) ) ) );
}
XLO_FUNC_END( aqMathDiscountFactors )
    .help( L"Discount factors at a column of payment dates, from a raw (dates, values) curve fit." )
    .arg( L"PaymentDates",                 L"Column of payment dates" )
    .arg( L"AsOfDate",                     L"The curve as-of date" )
    .arg( L"Interpolation",                L"Interpolation method, e.g. LINEAR, MONOTONE_CONVEX" )
    .arg( L"StateVariable",                L"The interpolated state variable, e.g. FORWARD_RATE, DISCOUNT_FACTOR" )
    .arg( L"XValues",                      L"Column of curve pillar dates" )
    .arg( L"YValues",                      L"Column of curve pillar values, aligned with XValues" )
    .arg( L"AccrualDaycount",              L"Day count convention" )
    .arg( L"CurveFrequencyTenor",          L"The curve's own tenor, e.g. 3M" )
    .arg( L"FixingBusinessDayAdjustment",  L"Optional. Default NONE" )
    .arg( L"FixingCalendar",               L"Holiday centre(s) for the fixing" )
    .arg( L"JoinDate",                     L"Optional. Piecewise-scheme join date" )
    .arg( L"PolynomialOrder",              L"Optional. Default 0" )
    .arg( L"ForwardAdjustmentTable",       L"Optional. A forward-adjustment override table" )
    .arg( L"CompoundFreq",                 L"Optional. Default SIMPLE" );
#endif