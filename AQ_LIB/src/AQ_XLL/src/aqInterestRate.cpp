/*
 * Rate category - xlOil worksheet functions.
 *
 * aqInterestRate*             - stateless: data in, value out.
 * aqInterestRateObjectFra*    - operate on a cached FRA object (name in).
 * aqInterestRateFixingTable*  - operate on a cached named fixing table (a sub-object).
 *
 * Each function pairs with the identically named validation wrapper (plus the
 * `try` prefix). Marshalling to and from Excel is the aq_xll helpers in
 * aqXllTools.h.
 */

#include <aqMain.h>

#include <string>
#include <vector>

#include <boost/date_time/gregorian/gregorian_types.hpp>

#include <aqXllTools.h>
#include <CoreEnumerations.h>         // etrading::trim_to_upper
#include <tryAqInterestRateFixingTable.h>     // validation::tryAqInterestRateFixingTable*
#include <tryAqInterestRateFutureFra.h>       // validation::tryAqInterestRateFuturePriceToFraRate*
#include <tryAqInterestRateObjectFra.h>       // validation::tryAqInterestRateObjectFra*

// toGregorian / toGregorianVector are shared helpers in aqXllTools (promoted
// here when Curve needed the same bridge).
using namespace aq_xll;


/* -------------------------------------------------------------------------
 *  Fixing table (a cached table of historical index fixings)
 * ---------------------------------------------------------------------- */

// Create and store a fixing table from a currency / curve tenor and a set of
// (date, value) fixings.
XLO_FUNC_START( aqInterestRateFixingTableCreate(
    const ExcelObj& tableName,
    const ExcelObj& currency,
    const ExcelObj& curveTenor,
    const ExcelObj& fixingDates,
    const ExcelObj& fixingValues ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const std::string objectName = decorateWithExcelLocation( toNarrowString( tableName ) );

    const std::string storedName = validation::tryAqInterestRateFixingTableCreate(
        objectName,
        etrading::trim_to_upper( toNarrowString( currency ) ),
        etrading::trim_to_upper( toNarrowString( curveTenor ) ),
        toGregorianVector( fixingDates, true, "FixingDates" ),
        toDoubleVector( fixingValues, true, "FixingValues" ) );

    return returnValue( appendInstanceCounter( storedName ) );
}
XLO_FUNC_END( aqInterestRateFixingTableCreate )
    .help( L"Create and store a fixing table from a currency / curve tenor and a set of (date, value) fixings; returns its handle." )
    .arg( L"TableName",    L"Name for the fixing table object" )
    .arg( L"Currency",     L"Currency, e.g. EUR, USD" )
    .arg( L"CurveTenor",   L"Index curve tenor, e.g. 3M, 6M" )
    .arg( L"FixingDates",  L"Column of fixing dates" )
    .arg( L"FixingValues", L"Column of fixing values, aligned with FixingDates" );


// Display a stored fixing table as a matrix.
XLO_FUNC_START( aqInterestRateFixingTableDisplay(
    const ExcelObj& tableName ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelMatrix(
        validation::tryAqInterestRateFixingTableDisplay( getNameWithoutCounter( tableName ) ) ) );
}
XLO_FUNC_END( aqInterestRateFixingTableDisplay )
    .help( L"Display a stored fixing table as a matrix." )
    .arg( L"TableName", L"A fixing table handle" );


// The fixing stored for one date.
XLO_FUNC_START( aqInterestRateFixingTableValue(
    const ExcelObj& tableName,
    const ExcelObj& fixingDate ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqInterestRateFixingTableValue(
        getNameWithoutCounter( tableName ), toGregorian( toAQLDate( fixingDate ) ) ) );
}
XLO_FUNC_END( aqInterestRateFixingTableValue )
    .help( L"The fixing stored in a fixing table for one date." )
    .arg( L"TableName",  L"A fixing table handle" )
    .arg( L"FixingDate", L"The fixing date to read" );


// The fixings stored for a column of dates.
XLO_FUNC_START( aqInterestRateFixingTableValues(
    const ExcelObj& tableName,
    const ExcelObj& fixingDates ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelDoubleColumn( validation::tryAqInterestRateFixingTableValues(
        getNameWithoutCounter( tableName ), toGregorianVector( fixingDates, true, "FixingDates" ) ) ) );
}
XLO_FUNC_END( aqInterestRateFixingTableValues )
    .help( L"The fixings stored in a fixing table for a column of dates." )
    .arg( L"TableName",   L"A fixing table handle" )
    .arg( L"FixingDates", L"Column of fixing dates" );


/* -------------------------------------------------------------------------
 *  Rate future <-> FRA conversion (stateless)
 * ---------------------------------------------------------------------- */

// FRA rate implied by a rate-future price, with a Hull-White convexity model.
XLO_FUNC_START( aqInterestRateFuturePriceToFraRate(
    const ExcelObj& futurePrice,
    const ExcelObj& curveAsOfDate,
    const ExcelObj& futuresStartDate,
    const ExcelObj& futuresEndDate,
    const ExcelObj& meanReversion,
    const ExcelObj& volatility ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqInterestRateFuturePriceToFraRate(
        futurePrice.get<double>(),
        toAQLDate( curveAsOfDate ),
        toAQLDate( futuresStartDate ),
        toAQLDate( futuresEndDate ),
        meanReversion.get<double>(),
        volatility.get<double>() ) );
}
XLO_FUNC_END( aqInterestRateFuturePriceToFraRate )
    .help( L"FRA rate implied by a rate-future price, using a Hull-White convexity adjustment." )
    .arg( L"FuturePrice",      L"The rate-future price" )
    .arg( L"CurveAsOfDate",    L"The curve as-of date" )
    .arg( L"FuturesStartDate", L"Start of the futures accrual period" )
    .arg( L"FuturesEndDate",   L"End of the futures accrual period" )
    .arg( L"MeanReversion",    L"Hull-White mean reversion" )
    .arg( L"Volatility",       L"Hull-White short-rate volatility" );


// FRA rate implied by a rate-future price and an explicit convexity adjustment.
XLO_FUNC_START( aqInterestRateFuturePriceToFraRateFromConvAdj(
    const ExcelObj& futurePrice,
    const ExcelObj& convexityAdjustment ) )
{
    AQ_XLL_GUARD

    return returnValue( validation::tryAqInterestRateFuturePriceToFraRateFromConvAdj(
        futurePrice.get<double>(), convexityAdjustment.get<double>() ) );
}
XLO_FUNC_END( aqInterestRateFuturePriceToFraRateFromConvAdj )
    .help( L"FRA rate implied by a rate-future price and an explicit convexity adjustment." )
    .arg( L"FuturePrice",         L"The rate-future price" )
    .arg( L"ConvexityAdjustment", L"The futures / FRA convexity adjustment" );


/* -------------------------------------------------------------------------
 *  FRA object
 * ---------------------------------------------------------------------- */

// Create and store a FRA from a label/value block.
XLO_FUNC_START( aqInterestRateObjectFraCreate(
    const ExcelObj& fraObjectName,
    const ExcelObj& fraLVB,
    const ExcelObj& validateKeys ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const std::string objectName = decorateWithExcelLocation( toNarrowString( fraObjectName ) );

    const std::string storedName = validation::tryAqInterestRateObjectFraCreate(
        objectName, toLabelValueBlock( fraLVB ), toBool( validateKeys, true ) );

    return returnValue( appendInstanceCounter( storedName ) );
}
XLO_FUNC_END( aqInterestRateObjectFraCreate )
    .help( L"Create and store a FRA from a label/value block; returns its handle." )
    .arg( L"FraObjectName", L"Name for the FRA object" )
    .arg( L"FraLVB",        L"The FRA definition as a label/value block" )
    .arg( L"ValidateKeys",  L"Optional. Default TRUE. Check the LVB keys" );


// Present value of a cached FRA.
XLO_FUNC_START( aqInterestRateObjectFraPV(
    const ExcelObj& fraObjectName,
    const ExcelObj& valuationSettingsLVB ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqInterestRateObjectFraPV(
        getNameWithoutCounter( fraObjectName ), toLabelValueBlock( valuationSettingsLVB ) ) );
}
XLO_FUNC_END( aqInterestRateObjectFraPV )
    .help( L"Present value of a cached FRA under the given valuation settings." )
    .arg( L"FraObjectName",       L"A FRA handle" )
    .arg( L"ValuationSettingsLVB", L"Valuation settings as a label/value block" );


// Display a cached FRA as a matrix.
XLO_FUNC_START( aqInterestRateObjectFraDisplay(
    const ExcelObj& fraObjectName ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelMatrix(
        validation::tryAqInterestRateObjectFraDisplay( getNameWithoutCounter( fraObjectName ) ) ) );
}
XLO_FUNC_END( aqInterestRateObjectFraDisplay )
    .help( L"Display a cached FRA as a matrix." )
    .arg( L"FraObjectName", L"A FRA handle" );


// Display the cashflows of a cached FRA.
XLO_FUNC_START( aqInterestRateObjectFraDisplayCashflows(
    const ExcelObj& fraObjectName,
    const ExcelObj& valuationSettingsLVB,
    const ExcelObj& showColumnHeaders ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelMatrix( validation::tryAqInterestRateObjectFraDisplayCashflows(
        getNameWithoutCounter( fraObjectName ),
        toLabelValueBlock( valuationSettingsLVB ),
        toBool( showColumnHeaders, true ) ) ) );
}
XLO_FUNC_END( aqInterestRateObjectFraDisplayCashflows )
    .help( L"Display the cashflows of a cached FRA as a matrix." )
    .arg( L"FraObjectName",        L"A FRA handle" )
    .arg( L"ValuationSettingsLVB", L"Valuation settings as a label/value block" )
    .arg( L"ShowColumnHeaders",    L"Optional. Default TRUE. Include a header row" );


// Fair FRA rate of a cached FRA.
XLO_FUNC_START( aqInterestRateObjectFraRate(
    const ExcelObj& fraObjectName,
    const ExcelObj& valuationSettingsLVB ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqInterestRateObjectFraRate(
        getNameWithoutCounter( fraObjectName ), toLabelValueBlock( valuationSettingsLVB ) ) );
}
XLO_FUNC_END( aqInterestRateObjectFraRate )
    .help( L"Fair forward rate of a cached FRA under the given valuation settings." )
    .arg( L"FraObjectName",        L"A FRA handle" )
    .arg( L"ValuationSettingsLVB", L"Valuation settings as a label/value block" );


// Rate-future price implied by a cached FRA, with a Hull-White convexity model.
XLO_FUNC_START( aqInterestRateObjectFraToFuturePrice(
    const ExcelObj& fraObjectName,
    const ExcelObj& valuationSettingsLVB,
    const ExcelObj& meanReversion,
    const ExcelObj& volatility ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqInterestRateObjectFraToFuturePrice(
        getNameWithoutCounter( fraObjectName ),
        toLabelValueBlock( valuationSettingsLVB ),
        meanReversion.get<double>(),
        volatility.get<double>() ) );
}
XLO_FUNC_END( aqInterestRateObjectFraToFuturePrice )
    .help( L"Rate-future price implied by a cached FRA, using a Hull-White convexity adjustment." )
    .arg( L"FraObjectName",        L"A FRA handle" )
    .arg( L"ValuationSettingsLVB", L"Valuation settings as a label/value block" )
    .arg( L"MeanReversion",        L"Hull-White mean reversion" )
    .arg( L"Volatility",           L"Hull-White short-rate volatility" );


// Rate-future price implied by a cached FRA and an explicit convexity adjustment.
XLO_FUNC_START( aqInterestRateObjectFraToFuturePriceFromConvAdj(
    const ExcelObj& fraObjectName,
    const ExcelObj& valuationSettingsLVB,
    const ExcelObj& convexityAdjustment ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqInterestRateObjectFraToFuturePriceFromConvAdj(
        getNameWithoutCounter( fraObjectName ),
        toLabelValueBlock( valuationSettingsLVB ),
        convexityAdjustment.get<double>() ) );
}
XLO_FUNC_END( aqInterestRateObjectFraToFuturePriceFromConvAdj )
    .help( L"Rate-future price implied by a cached FRA and an explicit convexity adjustment." )
    .arg( L"FraObjectName",        L"A FRA handle" )
    .arg( L"ValuationSettingsLVB", L"Valuation settings as a label/value block" )
    .arg( L"ConvexityAdjustment",  L"The futures / FRA convexity adjustment" );
