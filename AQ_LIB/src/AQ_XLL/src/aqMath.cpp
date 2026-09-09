#include <aqMain.h>
#include <aqXllTools.h>

// include
#include <CoreEnumerations.h>
#include <tryAqMathNormalDistribution.h>
#include <tryAqMathBlackScholes.h>
#include <tryAqMathInterpolation.h>   // validation::tryAqMathInterpolation
#include <tryAqMathPCA.h>             // validation::tryAqMathPCA

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
}


// Black-Scholes Price Method
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


/*
 * Interpolate a Y value at XPoint from the (XValues, YValues) data.
 */
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


/*
 * Principal-component analysis of a data matrix. Returns the factor loadings as
 * a labelled matrix.
 */
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