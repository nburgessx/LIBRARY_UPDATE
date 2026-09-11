/*
 * Rate category - xlOil worksheet functions.
 *
 * aqRate*             - stateless: data in, value out.
 * aqRateObjectFra*    - operate on a cached FRA object (name in).
 * aqRateFixingTable*  - operate on a cached named fixing table (a sub-object).
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
#include <DateUtilities.h>            // etrading::toGregorianDateFromAQLDate
#include <tryAqRateFixingTable.h>     // validation::tryAqRateFixingTable*
#include <tryAqRateFutureFra.h>       // validation::tryAqRateFuturePriceToFraRate*
#include <tryAqRateObjectFra.h>       // validation::tryAqRateObjectFra*

using namespace aq_xll;

namespace
{
    // The fixing-table wrappers take boost::gregorian::date, not AQLDate.
    boost::gregorian::date toGregorian( const AQLDate& date )
    {
        return etrading::toGregorianDateFromAQLDate( date );
    }

    std::vector<boost::gregorian::date> toGregorianVector( const xloil::ExcelObj& obj, const char* nameOfVariable )
    {
        const DateVector aqlDates = toDateVector( obj, true, nameOfVariable );

        std::vector<boost::gregorian::date> dates;
        dates.reserve( aqlDates.size() );
        for ( const AQLDate& date : aqlDates )
        {
            dates.push_back( toGregorian( date ) );
        }
        return dates;
    }
}


/* -------------------------------------------------------------------------
 *  Fixing table (a cached table of historical index fixings)
 * ---------------------------------------------------------------------- */

// Create and store a fixing table from a currency / curve tenor and a set of
// (date, value) fixings.
XLO_FUNC_START( aqRateFixingTableCreate(
    const ExcelObj& tableName,
    const ExcelObj& currency,
    const ExcelObj& curveTenor,
    const ExcelObj& fixingDates,
    const ExcelObj& fixingValues ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const std::string objectName = decorateWithExcelLocation( toNarrowString( tableName ) );

    const std::string storedName = validation::tryAqRateFixingTableCreate(
        objectName,
        etrading::trim_to_upper( toNarrowString( currency ) ),
        etrading::trim_to_upper( toNarrowString( curveTenor ) ),
        toGregorianVector( fixingDates, "FixingDates" ),
        toDoubleVector( fixingValues, true, "FixingValues" ) );

    return returnValue( appendInstanceCounter( storedName ) );
}
XLO_FUNC_END( aqRateFixingTableCreate )
    .help( L"Create and store a fixing table from a currency / curve tenor and a set of (date, value) fixings; returns its handle." )
    .arg( L"TableName",    L"Name for the fixing table object" )
    .arg( L"Currency",     L"Currency, e.g. EUR, USD" )
    .arg( L"CurveTenor",   L"Index curve tenor, e.g. 3M, 6M" )
    .arg( L"FixingDates",  L"Column of fixing dates" )
    .arg( L"FixingValues", L"Column of fixing values, aligned with FixingDates" );


// Display a stored fixing table as a matrix.
XLO_FUNC_START( aqRateFixingTableDisplay(
    const ExcelObj& tableName ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelMatrix(
        validation::tryAqRateFixingTableDisplay( getNameWithoutCounter( tableName ) ) ) );
}
XLO_FUNC_END( aqRateFixingTableDisplay )
    .help( L"Display a stored fixing table as a matrix." )
    .arg( L"TableName", L"A fixing table handle" );


// The fixing stored for one date.
XLO_FUNC_START( aqRateFixingTableValue(
    const ExcelObj& tableName,
    const ExcelObj& fixingDate ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqRateFixingTableValue(
        getNameWithoutCounter( tableName ), toGregorian( toAQLDate( fixingDate ) ) ) );
}
XLO_FUNC_END( aqRateFixingTableValue )
    .help( L"The fixing stored in a fixing table for one date." )
    .arg( L"TableName",  L"A fixing table handle" )
    .arg( L"FixingDate", L"The fixing date to read" );


// The fixings stored for a column of dates.
XLO_FUNC_START( aqRateFixingTableValues(
    const ExcelObj& tableName,
    const ExcelObj& fixingDates ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelDoubleColumn( validation::tryAqRateFixingTableValues(
        getNameWithoutCounter( tableName ), toGregorianVector( fixingDates, "FixingDates" ) ) ) );
}
XLO_FUNC_END( aqRateFixingTableValues )
    .help( L"The fixings stored in a fixing table for a column of dates." )
    .arg( L"TableName",   L"A fixing table handle" )
    .arg( L"FixingDates", L"Column of fixing dates" );


/* -------------------------------------------------------------------------
 *  Rate future <-> FRA conversion (stateless)
 * ---------------------------------------------------------------------- */

// FRA rate implied by a rate-future price, with a Hull-White convexity model.
XLO_FUNC_START( aqRateFuturePriceToFraRate(
    const ExcelObj& futurePrice,
    const ExcelObj& curveAsOfDate,
    const ExcelObj& futuresStartDate,
    const ExcelObj& futuresEndDate,
    const ExcelObj& meanReversion,
    const ExcelObj& volatility ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqRateFuturePriceToFraRate(
        futurePrice.get<double>(),
        toAQLDate( curveAsOfDate ),
        toAQLDate( futuresStartDate ),
        toAQLDate( futuresEndDate ),
        meanReversion.get<double>(),
        volatility.get<double>() ) );
}
XLO_FUNC_END( aqRateFuturePriceToFraRate )
    .help( L"FRA rate implied by a rate-future price, using a Hull-White convexity adjustment." )
    .arg( L"FuturePrice",      L"The rate-future price" )
    .arg( L"CurveAsOfDate",    L"The curve as-of date" )
    .arg( L"FuturesStartDate", L"Start of the futures accrual period" )
    .arg( L"FuturesEndDate",   L"End of the futures accrual period" )
    .arg( L"MeanReversion",    L"Hull-White mean reversion" )
    .arg( L"Volatility",       L"Hull-White short-rate volatility" );


// FRA rate implied by a rate-future price and an explicit convexity adjustment.
XLO_FUNC_START( aqRateFuturePriceToFraRateFromConvAdj(
    const ExcelObj& futurePrice,
    const ExcelObj& convexityAdjustment ) )
{
    AQ_XLL_GUARD

    return returnValue( validation::tryAqRateFuturePriceToFraRateFromConvAdj(
        futurePrice.get<double>(), convexityAdjustment.get<double>() ) );
}
XLO_FUNC_END( aqRateFuturePriceToFraRateFromConvAdj )
    .help( L"FRA rate implied by a rate-future price and an explicit convexity adjustment." )
    .arg( L"FuturePrice",         L"The rate-future price" )
    .arg( L"ConvexityAdjustment", L"The futures / FRA convexity adjustment" );


/* -------------------------------------------------------------------------
 *  FRA object
 * ---------------------------------------------------------------------- */

// Create and store a FRA from a label/value block.
XLO_FUNC_START( aqRateObjectFraCreate(
    const ExcelObj& fraObjectName,
    const ExcelObj& fraLVB,
    const ExcelObj& validateKeys ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const std::string objectName = decorateWithExcelLocation( toNarrowString( fraObjectName ) );

    const std::string storedName = validation::tryAqRateObjectFraCreate(
        objectName, toLabelValueBlock( fraLVB ), toBool( validateKeys, true ) );

    return returnValue( appendInstanceCounter( storedName ) );
}
XLO_FUNC_END( aqRateObjectFraCreate )
    .help( L"Create and store a FRA from a label/value block; returns its handle." )
    .arg( L"FraObjectName", L"Name for the FRA object" )
    .arg( L"FraLVB",        L"The FRA definition as a label/value block" )
    .arg( L"ValidateKeys",  L"Optional. Default TRUE. Check the LVB keys" );


// Present value of a cached FRA.
XLO_FUNC_START( aqRateObjectFraPV(
    const ExcelObj& fraObjectName,
    const ExcelObj& valuationSettingsLVB ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqRateObjectFraPV(
        getNameWithoutCounter( fraObjectName ), toLabelValueBlock( valuationSettingsLVB ) ) );
}
XLO_FUNC_END( aqRateObjectFraPV )
    .help( L"Present value of a cached FRA under the given valuation settings." )
    .arg( L"FraObjectName",       L"A FRA handle" )
    .arg( L"ValuationSettingsLVB", L"Valuation settings as a label/value block" );


// Display a cached FRA as a matrix.
XLO_FUNC_START( aqRateObjectFraDisplay(
    const ExcelObj& fraObjectName ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelMatrix(
        validation::tryAqRateObjectFraDisplay( getNameWithoutCounter( fraObjectName ) ) ) );
}
XLO_FUNC_END( aqRateObjectFraDisplay )
    .help( L"Display a cached FRA as a matrix." )
    .arg( L"FraObjectName", L"A FRA handle" );


// Display the cashflows of a cached FRA.
XLO_FUNC_START( aqRateObjectFraDisplayCashflows(
    const ExcelObj& fraObjectName,
    const ExcelObj& valuationSettingsLVB,
    const ExcelObj& showColumnHeaders ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelMatrix( validation::tryAqRateObjectFraDisplayCashflows(
        getNameWithoutCounter( fraObjectName ),
        toLabelValueBlock( valuationSettingsLVB ),
        toBool( showColumnHeaders, true ) ) ) );
}
XLO_FUNC_END( aqRateObjectFraDisplayCashflows )
    .help( L"Display the cashflows of a cached FRA as a matrix." )
    .arg( L"FraObjectName",        L"A FRA handle" )
    .arg( L"ValuationSettingsLVB", L"Valuation settings as a label/value block" )
    .arg( L"ShowColumnHeaders",    L"Optional. Default TRUE. Include a header row" );


// Fair FRA rate of a cached FRA.
XLO_FUNC_START( aqRateObjectFraRate(
    const ExcelObj& fraObjectName,
    const ExcelObj& valuationSettingsLVB ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqRateObjectFraRate(
        getNameWithoutCounter( fraObjectName ), toLabelValueBlock( valuationSettingsLVB ) ) );
}
XLO_FUNC_END( aqRateObjectFraRate )
    .help( L"Fair forward rate of a cached FRA under the given valuation settings." )
    .arg( L"FraObjectName",        L"A FRA handle" )
    .arg( L"ValuationSettingsLVB", L"Valuation settings as a label/value block" );


// Rate-future price implied by a cached FRA, with a Hull-White convexity model.
XLO_FUNC_START( aqRateObjectFraToFuturePrice(
    const ExcelObj& fraObjectName,
    const ExcelObj& valuationSettingsLVB,
    const ExcelObj& meanReversion,
    const ExcelObj& volatility ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqRateObjectFraToFuturePrice(
        getNameWithoutCounter( fraObjectName ),
        toLabelValueBlock( valuationSettingsLVB ),
        meanReversion.get<double>(),
        volatility.get<double>() ) );
}
XLO_FUNC_END( aqRateObjectFraToFuturePrice )
    .help( L"Rate-future price implied by a cached FRA, using a Hull-White convexity adjustment." )
    .arg( L"FraObjectName",        L"A FRA handle" )
    .arg( L"ValuationSettingsLVB", L"Valuation settings as a label/value block" )
    .arg( L"MeanReversion",        L"Hull-White mean reversion" )
    .arg( L"Volatility",           L"Hull-White short-rate volatility" );


// Rate-future price implied by a cached FRA and an explicit convexity adjustment.
XLO_FUNC_START( aqRateObjectFraToFuturePriceFromConvAdj(
    const ExcelObj& fraObjectName,
    const ExcelObj& valuationSettingsLVB,
    const ExcelObj& convexityAdjustment ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqRateObjectFraToFuturePriceFromConvAdj(
        getNameWithoutCounter( fraObjectName ),
        toLabelValueBlock( valuationSettingsLVB ),
        convexityAdjustment.get<double>() ) );
}
XLO_FUNC_END( aqRateObjectFraToFuturePriceFromConvAdj )
    .help( L"Rate-future price implied by a cached FRA and an explicit convexity adjustment." )
    .arg( L"FraObjectName",        L"A FRA handle" )
    .arg( L"ValuationSettingsLVB", L"Valuation settings as a label/value block" )
    .arg( L"ConvexityAdjustment",  L"The futures / FRA convexity adjustment" );
