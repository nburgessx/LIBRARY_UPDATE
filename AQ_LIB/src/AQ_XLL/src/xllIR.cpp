/*
 * IR (InterestRate) category functions
 *
 * aqIR*             - stateless: data in, value out.
 * aqIRObjectFra*    - operate on a cached FRA object (name in).
 * aqIRFixingTable*  - operate on a cached named fixing table (a sub-object).
 *
 * Each function pairs with the identically named validation wrapper (plus the
 * `try` prefix). Marshalling to and from Excel is the aq_xll helpers in
 * xllSupport.h.
 */

#include <xllMain.h>

#include <string>
#include <vector>

#include <boost/date_time/gregorian/gregorian_types.hpp>

#include <xllSupport.h>
#include <CoreEnumerations.h>         // etrading::trim_to_upper
#include <tryAqIRFixingTable.h>     // validation::tryAqIRFixingTable*
#include <tryAqIRFutureFra.h>       // validation::tryAqIRFuturePriceToFraRate*
#include <tryAqIRObjectFra.h>       // validation::tryAqIRObjectFra*

// toGregorian / toGregorianVector are shared helpers in aqXllTools (promoted
// here when Curve needed the same bridge).
using namespace aq_xll;


/* -------------------------------------------------------------------------
 *  Fixing table (a cached table of historical index fixings)
 * ---------------------------------------------------------------------- */

// Create and store a fixing table from a currency / curve tenor and a set of
// (date, value) fixings.
#if AQ_XLL_ENABLED(aqIRFixingTableCreate)
XLO_FUNC_START( aqIRFixingTableCreate(
    const ExcelObj& tableName,
    const ExcelObj& currency,
    const ExcelObj& curveTenor,
    const ExcelObj& fixingDates,
    const ExcelObj& fixingValues ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const std::string objectName = decorateWithExcelLocation( toNarrowString( tableName ) );

    const std::string storedName = validation::tryAqIRFixingTableCreate(
        objectName,
        etrading::trim_to_upper( toNarrowString( currency ) ),
        etrading::trim_to_upper( toNarrowString( curveTenor ) ),
        toGregorianVector( fixingDates, true, "FixingDates" ),
        toDoubleVector( fixingValues, true, "FixingValues" ) );

    return returnValue( appendInstanceCounter( storedName ) );
}
XLO_FUNC_END( aqIRFixingTableCreate )
    .help( L"Create and store a fixing table from a currency / curve tenor and a set of (date, value) fixings; returns its handle." )
    .arg( L"TableName",    L"Name for the fixing table object" )
    .arg( L"Currency",     L"Currency, e.g. EUR, USD" )
    .arg( L"CurveTenor",   L"Index curve tenor, e.g. 3M, 6M" )
    .arg( L"FixingDates",  L"Column of fixing dates" )
    .arg( L"FixingValues", L"Column of fixing values, aligned with FixingDates" );
#endif


// Display a stored fixing table as a matrix.
#if AQ_XLL_ENABLED(aqIRFixingTableDisplay)
XLO_FUNC_START( aqIRFixingTableDisplay(
    const ExcelObj& tableName ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelMatrix(
        validation::tryAqIRFixingTableDisplay( getNameWithoutCounter( tableName ) ) ) );
}
XLO_FUNC_END( aqIRFixingTableDisplay )
    .help( L"Display a stored fixing table as a matrix." )
    .arg( L"TableName", L"A fixing table handle" );
#endif


// The fixing stored for one date.
#if AQ_XLL_ENABLED(aqIRFixingTableValue)
XLO_FUNC_START( aqIRFixingTableValue(
    const ExcelObj& tableName,
    const ExcelObj& fixingDate ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqIRFixingTableValue(
        getNameWithoutCounter( tableName ), toGregorian( toAQLDate( fixingDate ) ) ) );
}
XLO_FUNC_END( aqIRFixingTableValue )
    .help( L"The fixing stored in a fixing table for one date." )
    .arg( L"TableName",  L"A fixing table handle" )
    .arg( L"FixingDate", L"The fixing date to read" );
#endif


// The fixings stored for a column of dates.
#if AQ_XLL_ENABLED(aqIRFixingTableValues)
XLO_FUNC_START( aqIRFixingTableValues(
    const ExcelObj& tableName,
    const ExcelObj& fixingDates ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelDoubleColumn( validation::tryAqIRFixingTableValues(
        getNameWithoutCounter( tableName ), toGregorianVector( fixingDates, true, "FixingDates" ) ) ) );
}
XLO_FUNC_END( aqIRFixingTableValues )
    .help( L"The fixings stored in a fixing table for a column of dates." )
    .arg( L"TableName",   L"A fixing table handle" )
    .arg( L"FixingDates", L"Column of fixing dates" );
#endif


/* -------------------------------------------------------------------------
 *  Rate future <-> FRA conversion (stateless)
 * ---------------------------------------------------------------------- */

// FRA rate implied by a rate-future price, with a Hull-White convexity model.
#if AQ_XLL_ENABLED(aqIRFuturePriceToFraRate)
XLO_FUNC_START( aqIRFuturePriceToFraRate(
    const ExcelObj& futurePrice,
    const ExcelObj& curveAsOfDate,
    const ExcelObj& futuresStartDate,
    const ExcelObj& futuresEndDate,
    const ExcelObj& meanReversion,
    const ExcelObj& volatility ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqIRFuturePriceToFraRate(
        futurePrice.get<double>(),
        toAQLDate( curveAsOfDate ),
        toAQLDate( futuresStartDate ),
        toAQLDate( futuresEndDate ),
        meanReversion.get<double>(),
        volatility.get<double>() ) );
}
XLO_FUNC_END( aqIRFuturePriceToFraRate )
    .help( L"FRA rate implied by a rate-future price, using a Hull-White convexity adjustment." )
    .arg( L"FuturePrice",      L"The rate-future price" )
    .arg( L"CurveAsOfDate",    L"The curve as-of date" )
    .arg( L"FuturesStartDate", L"Start of the futures accrual period" )
    .arg( L"FuturesEndDate",   L"End of the futures accrual period" )
    .arg( L"MeanReversion",    L"Hull-White mean reversion" )
    .arg( L"Volatility",       L"Hull-White short-rate volatility" );
#endif


// FRA rate implied by a rate-future price and an explicit convexity adjustment.
#if AQ_XLL_ENABLED(aqIRFuturePriceToFraRateFromConvAdj)
XLO_FUNC_START( aqIRFuturePriceToFraRateFromConvAdj(
    const ExcelObj& futurePrice,
    const ExcelObj& convexityAdjustment ) )
{
    AQ_XLL_GUARD

    return returnValue( validation::tryAqIRFuturePriceToFraRateFromConvAdj(
        futurePrice.get<double>(), convexityAdjustment.get<double>() ) );
}
XLO_FUNC_END( aqIRFuturePriceToFraRateFromConvAdj )
    .help( L"FRA rate implied by a rate-future price and an explicit convexity adjustment." )
    .arg( L"FuturePrice",         L"The rate-future price" )
    .arg( L"ConvexityAdjustment", L"The futures / FRA convexity adjustment" );
#endif


/* -------------------------------------------------------------------------
 *  FRA object
 * ---------------------------------------------------------------------- */

// Create and store a FRA from a label/value block.
#if AQ_XLL_ENABLED(aqIRObjectFraCreate)
XLO_FUNC_START( aqIRObjectFraCreate(
    const ExcelObj& fraObjectName,
    const ExcelObj& fraLVB,
    const ExcelObj& validateKeys ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const std::string objectName = decorateWithExcelLocation( toNarrowString( fraObjectName ) );

    const std::string storedName = validation::tryAqIRObjectFraCreate(
        objectName, toLabelValueBlock( fraLVB ), toBool( validateKeys, true ) );

    return returnValue( appendInstanceCounter( storedName ) );
}
XLO_FUNC_END( aqIRObjectFraCreate )
    .help( L"Create and store a FRA from a label/value block; returns its handle." )
    .arg( L"FraObjectName", L"Name for the FRA object" )
    .arg( L"FraLVB",        L"The FRA definition as a label/value block" )
    .arg( L"ValidateKeys",  L"Optional. Default TRUE. Check the LVB keys" );
#endif


// Present value of a cached FRA.
#if AQ_XLL_ENABLED(aqIRObjectFraPV)
XLO_FUNC_START( aqIRObjectFraPV(
    const ExcelObj& fraObjectName,
    const ExcelObj& valuationSettingsLVB ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqIRObjectFraPV(
        getNameWithoutCounter( fraObjectName ), toLabelValueBlock( valuationSettingsLVB ) ) );
}
XLO_FUNC_END( aqIRObjectFraPV )
    .help( L"Present value of a cached FRA under the given valuation settings." )
    .arg( L"FraObjectName",       L"A FRA handle" )
    .arg( L"ValuationSettingsLVB", L"Valuation settings as a label/value block" );
#endif


// Display a cached FRA as a matrix.
#if AQ_XLL_ENABLED(aqIRObjectFraDisplay)
XLO_FUNC_START( aqIRObjectFraDisplay(
    const ExcelObj& fraObjectName ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelMatrix(
        validation::tryAqIRObjectFraDisplay( getNameWithoutCounter( fraObjectName ) ) ) );
}
XLO_FUNC_END( aqIRObjectFraDisplay )
    .help( L"Display a cached FRA as a matrix." )
    .arg( L"FraObjectName", L"A FRA handle" );
#endif


// Display the cashflows of a cached FRA.
#if AQ_XLL_ENABLED(aqIRObjectFraDisplayCashflows)
XLO_FUNC_START( aqIRObjectFraDisplayCashflows(
    const ExcelObj& fraObjectName,
    const ExcelObj& valuationSettingsLVB,
    const ExcelObj& showColumnHeaders ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelMatrix( validation::tryAqIRObjectFraDisplayCashflows(
        getNameWithoutCounter( fraObjectName ),
        toLabelValueBlock( valuationSettingsLVB ),
        toBool( showColumnHeaders, true ) ) ) );
}
XLO_FUNC_END( aqIRObjectFraDisplayCashflows )
    .help( L"Display the cashflows of a cached FRA as a matrix." )
    .arg( L"FraObjectName",        L"A FRA handle" )
    .arg( L"ValuationSettingsLVB", L"Valuation settings as a label/value block" )
    .arg( L"ShowColumnHeaders",    L"Optional. Default TRUE. Include a header row" );
#endif


// Fair FRA rate of a cached FRA.
#if AQ_XLL_ENABLED(aqIRObjectFraRate)
XLO_FUNC_START( aqIRObjectFraRate(
    const ExcelObj& fraObjectName,
    const ExcelObj& valuationSettingsLVB ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqIRObjectFraRate(
        getNameWithoutCounter( fraObjectName ), toLabelValueBlock( valuationSettingsLVB ) ) );
}
XLO_FUNC_END( aqIRObjectFraRate )
    .help( L"Fair forward rate of a cached FRA under the given valuation settings." )
    .arg( L"FraObjectName",        L"A FRA handle" )
    .arg( L"ValuationSettingsLVB", L"Valuation settings as a label/value block" );
#endif


// Rate-future price implied by a cached FRA, with a Hull-White convexity model.
#if AQ_XLL_ENABLED(aqIRObjectFraToFuturePrice)
XLO_FUNC_START( aqIRObjectFraToFuturePrice(
    const ExcelObj& fraObjectName,
    const ExcelObj& valuationSettingsLVB,
    const ExcelObj& meanReversion,
    const ExcelObj& volatility ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqIRObjectFraToFuturePrice(
        getNameWithoutCounter( fraObjectName ),
        toLabelValueBlock( valuationSettingsLVB ),
        meanReversion.get<double>(),
        volatility.get<double>() ) );
}
XLO_FUNC_END( aqIRObjectFraToFuturePrice )
    .help( L"Rate-future price implied by a cached FRA, using a Hull-White convexity adjustment." )
    .arg( L"FraObjectName",        L"A FRA handle" )
    .arg( L"ValuationSettingsLVB", L"Valuation settings as a label/value block" )
    .arg( L"MeanReversion",        L"Hull-White mean reversion" )
    .arg( L"Volatility",           L"Hull-White short-rate volatility" );
#endif


// Rate-future price implied by a cached FRA and an explicit convexity adjustment.
#if AQ_XLL_ENABLED(aqIRObjectFraToFuturePriceFromConvAdj)
XLO_FUNC_START( aqIRObjectFraToFuturePriceFromConvAdj(
    const ExcelObj& fraObjectName,
    const ExcelObj& valuationSettingsLVB,
    const ExcelObj& convexityAdjustment ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqIRObjectFraToFuturePriceFromConvAdj(
        getNameWithoutCounter( fraObjectName ),
        toLabelValueBlock( valuationSettingsLVB ),
        convexityAdjustment.get<double>() ) );
}
XLO_FUNC_END( aqIRObjectFraToFuturePriceFromConvAdj )
    .help( L"Rate-future price implied by a cached FRA and an explicit convexity adjustment." )
    .arg( L"FraObjectName",        L"A FRA handle" )
    .arg( L"ValuationSettingsLVB", L"Valuation settings as a label/value block" )
    .arg( L"ConvexityAdjustment",  L"The futures / FRA convexity adjustment" );
#endif
