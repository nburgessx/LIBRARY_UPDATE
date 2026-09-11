/*
 * Curve category - xlOil worksheet functions.
 *
 * This is the rates yield-curve framework only (bond-curve fitting is under
 * Bond, credit/hazard curves under Credit - see CLAUDE.md Sec5.1).
 *
 * aqCurveObject*        - operate on a cached curve object (name in).
 * aqCurveGenerator*     - operate on a cached named sub-object (conventions).
 * aqCurveMarketData*    - operate on a cached named sub-object (rates).
 * aqCurveZeroRates*     - stateless: curve-collection/index in, values out.
 *
 * Batch 1: object lifecycle, calibrate (+hedge), discount factor / forward
 * rate / zero rate families, generator, market data. Deferred to a later
 * batch (see rebrand/STATUS.md): the curve Results/Jacobian risk family, the
 * four legacy one-shot Create{Basis,FXForwards,OIS,Swap} functions (10-17 raw
 * convention/rate AQLStringMatrix params each - need Nicholas's call on
 * whether they are still live or superseded by the Generator+MarketData+
 * Calibrate path below), DualBootstrap, HullWhite/Vasicek, CTD calibration,
 * and the misc utilities (CompoundRate, EuroDollarConvexityAdjustment,
 * Frequency, InterpolationJoinDate, USDSpotDate, curve groups). The legacy
 * stateless tryAqCurvesDiscountFactor/tryAqCurvesForwardRate (and the
 * matching deferred Math ForwardRate/DiscountFactor functions) need a
 * string->enum marshalling helper not yet written.
 *
 * Each function pairs with the identically named validation wrapper (plus the
 * `try` prefix). Marshalling to and from Excel is the aq_xll helpers in
 * aqXllTools.h.
 */

#include <aqMain.h>

#include <string>
#include <vector>

#include <aqXllTools.h>
#include <CoreEnumerations.h>                // etrading::trim_to_upper
#include <Variant.h>                        // etrading::Variant, VariantMatrix
#include <tryAqCurveObjectUtilities.h>       // validation::tryAqCurveObject{List,Delete,DeleteAll,Save,Load}
#include <tryAqCurveObjectDisplay.h>         // validation::tryAqCurveObjectDisplay
#include <tryAqCurveObjectCalibrate.h>       // validation::tryAqCurveObjectCalibrate
#include <tryAqCurveObjectCalibrateHedge.h>  // validation::tryAqCurveObjectCalibrateHedge
#include <tryAqCurveObjectDiscountFactor.h>  // validation::tryAqCurveObjectDiscountFactors*
#include <tryAqCurveObjectForwardRate.h>     // validation::tryAqCurveObjectForwardRates*
#include <tryAqCurveZeroRate.h>              // validation::tryAqCurveZeroRatesFrom{YearFractions,Tenors}
#include <tryAqCurveGenerator.h>             // validation::tryAqCurveGenerator* / tryAqCurveObjectDisplayConventions
#include <tryAqCurveMarketData.h>            // validation::tryAqCurveMarketData* / tryAqCurveObjectBump*

using namespace aq_xll;

namespace
{
    // An optional string worksheet argument.
    std::string toStrOr( const xloil::ExcelObj& obj, const char* defaultValue )
    {
        if ( obj.isMissing() || !obj.isNonEmpty() )
        {
            return std::string( defaultValue );
        }
        return toNarrowString( obj );
    }

    // Optional integer worksheet argument: missing / empty / blank yields defaultValue.
    int toIntOr( const xloil::ExcelObj& obj, int defaultValue )
    {
        if ( obj.isMissing() || !obj.isNonEmpty() )
        {
            return defaultValue;
        }
        return static_cast<int>( obj.get<double>() );
    }
}


/* -------------------------------------------------------------------------
 *  Object lifecycle
 * ---------------------------------------------------------------------- */

// The names of every cached curve.
XLO_FUNC_START( aqCurveObjectList() )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelColumn( validation::tryAqCurveObjectList() ) );
}
XLO_FUNC_END( aqCurveObjectList )
    .help( L"The names of every cached curve, as a column." );


// Remove one curve from the cache.
XLO_FUNC_START( aqCurveObjectDelete(
    const ExcelObj& curveName ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqCurveObjectDelete( getNameWithoutCounter( curveName ) ) );
}
XLO_FUNC_END( aqCurveObjectDelete )
    .help( L"Remove one cached curve. Returns TRUE on success." )
    .arg( L"CurveName", L"A curve handle" );


// Remove every cached curve.
XLO_FUNC_START( aqCurveObjectDeleteAll() )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( static_cast<double>( validation::tryAqCurveObjectDeleteAll() ) );
}
XLO_FUNC_END( aqCurveObjectDeleteAll )
    .help( L"Remove every cached curve. Returns the number removed." );


// Save a cached curve to a file.
XLO_FUNC_START( aqCurveObjectSave(
    const ExcelObj& aqObjCurveName,
    const ExcelObj& fileNameToWriteTo ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqCurveObjectSave(
        getNameWithoutCounter( aqObjCurveName ), toNarrowString( fileNameToWriteTo ) ) );
}
XLO_FUNC_END( aqCurveObjectSave )
    .help( L"Save a cached curve to a file. Returns a status string." )
    .arg( L"AqObjCurveName",   L"A curve handle" )
    .arg( L"FileNameToWriteTo", L"Full path to write the curve to" );


// Load a curve from a file.
XLO_FUNC_START( aqCurveObjectLoad(
    const ExcelObj& fileName ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqCurveObjectLoad( toNarrowString( fileName ) ).second );
}
XLO_FUNC_END( aqCurveObjectLoad )
    .help( L"Load a curve from a file. Returns a status string." )
    .arg( L"FileName", L"Full path to the curve file" );


// Display a cached curve as a matrix.
XLO_FUNC_START( aqCurveObjectDisplay(
    const ExcelObj& curveObjectName ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelMatrix(
        validation::tryAqCurveObjectDisplay( getNameWithoutCounter( curveObjectName ) ) ) );
}
XLO_FUNC_END( aqCurveObjectDisplay )
    .help( L"Display a cached curve as a matrix." )
    .arg( L"CurveObjectName", L"A curve handle" );


/* -------------------------------------------------------------------------
 *  Calibration
 * ---------------------------------------------------------------------- */

// Calibrate and store a curve from a curve generator and market data.
XLO_FUNC_START( aqCurveObjectCalibrate(
    const ExcelObj& objectName,
    const ExcelObj& curveGeneratorName,
    const ExcelObj& curveMarketDataName,
    const ExcelObj& domesticCurveCollection,
    const ExcelObj& foreignCurveCollection ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const std::string name = decorateWithExcelLocation( toNarrowString( objectName ) );

    const std::string storedName = validation::tryAqCurveObjectCalibrate(
        name,
        getNameWithoutCounter( curveGeneratorName ),
        getNameWithoutCounter( curveMarketDataName ),
        toNarrowString( domesticCurveCollection ),
        toNarrowString( foreignCurveCollection ) );

    return returnValue( appendInstanceCounter( storedName ) );
}
XLO_FUNC_END( aqCurveObjectCalibrate )
    .help( L"Calibrate and store a curve from a curve generator and market data; returns its handle." )
    .arg( L"ObjectName",              L"Name for the curve object" )
    .arg( L"CurveGeneratorName",      L"A curve-generator handle (conventions)" )
    .arg( L"CurveMarketDataName",     L"A curve-market-data handle (rates)" )
    .arg( L"DomesticCurveCollection", L"Optional. Domestic discount curve collection, for cross-currency curves" )
    .arg( L"ForeignCurveCollection",  L"Optional. Foreign discount curve collection, for cross-currency curves" );


// Calibrate a hedge-curve pairing (OIS discounting + Libor forwarding).
XLO_FUNC_START( aqCurveObjectCalibrateHedge(
    const ExcelObj& oisCurveObjectName,
    const ExcelObj& swapCurveObjectName,
    const ExcelObj& pricingCurveCollection,
    const ExcelObj& hedgeCurveCollection,
    const ExcelObj& oisCurveGeneratorName,
    const ExcelObj& oisCurveMarketDataName,
    const ExcelObj& swapCurveGeneratorName,
    const ExcelObj& swapCurveMarketDataName,
    const ExcelObj& swapGeneratorName ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const etrading::HedgeCurveInfo info = validation::tryAqCurveObjectCalibrateHedge(
        getNameWithoutCounter( oisCurveObjectName ),
        getNameWithoutCounter( swapCurveObjectName ),
        toNarrowString( pricingCurveCollection ),
        toNarrowString( hedgeCurveCollection ),
        getNameWithoutCounter( oisCurveGeneratorName ),
        getNameWithoutCounter( oisCurveMarketDataName ),
        getNameWithoutCounter( swapCurveGeneratorName ),
        getNameWithoutCounter( swapCurveMarketDataName ),
        getNameWithoutCounter( swapGeneratorName ) );

    AQLStringMatrix result;
    auto row = [&result]( const char* key, const std::string& value )
    {
        AQLStringVector r;
        r.push_back( AQLString( key ) );
        r.push_back( AQLString( value.c_str() ) );
        result.push_back( r );
    };
    row( "OisCurveName",  info.oisCurveName );
    row( "OisCurveIndex", info.oisCurveIndex );
    row( "SwapCurveName", info.swapCurveName );
    row( "SwapCurveIndex", info.swapCurveIndex );

    return returnValue( toExcelMatrix( result ) );
}
XLO_FUNC_END( aqCurveObjectCalibrateHedge )
    .help( L"Calibrate a hedge-curve pairing (OIS discounting + Libor forwarding). Returns a key/value block." )
    .arg( L"OisCurveObjectName",     L"Name for the OIS discount curve" )
    .arg( L"SwapCurveObjectName",    L"Name for the swap forward curve" )
    .arg( L"PricingCurveCollection", L"Curve collection to price the hedge instruments off" )
    .arg( L"HedgeCurveCollection",   L"Curve collection the hedge curves are stored under" )
    .arg( L"OisCurveGeneratorName",  L"A curve-generator handle for the OIS curve" )
    .arg( L"OisCurveMarketDataName", L"A curve-market-data handle for the OIS curve" )
    .arg( L"SwapCurveGeneratorName", L"A curve-generator handle for the swap curve" )
    .arg( L"SwapCurveMarketDataName", L"A curve-market-data handle for the swap curve" )
    .arg( L"SwapGeneratorName",      L"A swap-generator handle for the hedge instruments" );


/* -------------------------------------------------------------------------
 *  Discount factors
 * ---------------------------------------------------------------------- */

XLO_FUNC_START( aqCurveObjectDiscountFactorsFromYearFractions(
    const ExcelObj& aqObjCurveName,
    const ExcelObj& yearFractions,
    const ExcelObj& dayCount ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelDoubleColumn( validation::tryAqCurveObjectDiscountFactorsFromYearFractions(
        getNameWithoutCounter( aqObjCurveName ),
        toDoubleVector( yearFractions, true, "YearFractions" ),
        toAQLString( dayCount ) ) ) );
}
XLO_FUNC_END( aqCurveObjectDiscountFactorsFromYearFractions )
    .help( L"Discount factors from a cached curve at a column of year fractions." )
    .arg( L"AqObjCurveName", L"A curve handle" )
    .arg( L"YearFractions",  L"Column of year fractions" )
    .arg( L"DayCount",       L"Day count convention, e.g. ACT/360, ACT/365" );


XLO_FUNC_START( aqCurveObjectDiscountFactorsFromTenors(
    const ExcelObj& aqObjCurveName,
    const ExcelObj& tenors,
    const ExcelObj& businessDayAdj,
    const ExcelObj& calendar ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelDoubleColumn( validation::tryAqCurveObjectDiscountFactorsFromTenors(
        getNameWithoutCounter( aqObjCurveName ),
        toStringVector( tenors, true ),
        toStrOr( businessDayAdj, "NO_CHANGE" ),
        toStrOr( calendar, "" ) ) ) );
}
XLO_FUNC_END( aqCurveObjectDiscountFactorsFromTenors )
    .help( L"Discount factors from a cached curve at a column of tenors." )
    .arg( L"AqObjCurveName",   L"A curve handle" )
    .arg( L"Tenors",           L"Column of tenors, e.g. 3M, 5Y" )
    .arg( L"BusinessDayAdj",   L"Optional. Default NO_CHANGE" )
    .arg( L"Calendar",         L"Optional. Holiday centre(s) for the tenor roll" );


XLO_FUNC_START( aqCurveObjectDiscountFactors(
    const ExcelObj& aqObjCurveName,
    const ExcelObj& paymentDates ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelDoubleColumn( validation::tryAqCurveObjectDiscountFactors(
        getNameWithoutCounter( aqObjCurveName ), toGregorianVector( paymentDates, true, "PaymentDates" ) ) ) );
}
XLO_FUNC_END( aqCurveObjectDiscountFactors )
    .help( L"Discount factors from a cached curve at a column of payment dates." )
    .arg( L"AqObjCurveName", L"A curve handle" )
    .arg( L"PaymentDates",   L"Column of payment dates" );


XLO_FUNC_START( aqCurveObjectDiscountFactorsForwardStarting(
    const ExcelObj& aqObjCurveName,
    const ExcelObj& fromDates,
    const ExcelObj& toDates ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelDoubleColumn( validation::tryAqCurveObjectDiscountFactorsForwardStarting(
        getNameWithoutCounter( aqObjCurveName ),
        toGregorianVector( fromDates, true, "FromDates" ),
        toGregorianVector( toDates, true, "ToDates" ) ) ) );
}
XLO_FUNC_END( aqCurveObjectDiscountFactorsForwardStarting )
    .help( L"Forward-starting discount factors from a cached curve between paired (from, to) dates." )
    .arg( L"AqObjCurveName", L"A curve handle" )
    .arg( L"FromDates",      L"Column of forward-start dates" )
    .arg( L"ToDates",        L"Column of end dates, aligned with FromDates" );


XLO_FUNC_START( aqCurveObjectDiscountFactorsForwardStartingFromYearFractions(
    const ExcelObj& aqObjCurveName,
    const ExcelObj& fromDates,
    const ExcelObj& yearFractions,
    const ExcelObj& dayCount ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelDoubleColumn( validation::tryAqCurveObjectDiscountFactorsForwardStartingFromYearFractions(
        getNameWithoutCounter( aqObjCurveName ),
        toGregorianVector( fromDates, true, "FromDates" ),
        toDoubleVector( yearFractions, true, "YearFractions" ),
        toAQLString( dayCount ) ) ) );
}
XLO_FUNC_END( aqCurveObjectDiscountFactorsForwardStartingFromYearFractions )
    .help( L"Forward-starting discount factors from a cached curve, from-dates plus year fractions." )
    .arg( L"AqObjCurveName", L"A curve handle" )
    .arg( L"FromDates",      L"Column of forward-start dates" )
    .arg( L"YearFractions",  L"Column of year fractions forward from FromDates, aligned" )
    .arg( L"DayCount",       L"Day count convention, e.g. ACT/360, ACT/365" );


XLO_FUNC_START( aqCurveObjectDiscountFactorsForwardStartingFromTenors(
    const ExcelObj& aqObjCurveName,
    const ExcelObj& fromDates,
    const ExcelObj& tenors,
    const ExcelObj& businessDayAdj,
    const ExcelObj& calendar ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelDoubleColumn( validation::tryAqCurveObjectDiscountFactorsForwardStartingFromTenors(
        getNameWithoutCounter( aqObjCurveName ),
        toGregorianVector( fromDates, true, "FromDates" ),
        toStringVector( tenors, true ),
        toStrOr( businessDayAdj, "NO_CHANGE" ),
        toStrOr( calendar, "" ) ) ) );
}
XLO_FUNC_END( aqCurveObjectDiscountFactorsForwardStartingFromTenors )
    .help( L"Forward-starting discount factors from a cached curve, from-dates plus tenors." )
    .arg( L"AqObjCurveName", L"A curve handle" )
    .arg( L"FromDates",      L"Column of forward-start dates" )
    .arg( L"Tenors",         L"Column of tenors forward from FromDates, aligned, e.g. 3M, 5Y" )
    .arg( L"BusinessDayAdj", L"Optional. Default NO_CHANGE" )
    .arg( L"Calendar",       L"Optional. Holiday centre(s) for the tenor roll" );


// Discount factor table across multiple curve indices in one curve collection.
XLO_FUNC_START( aqCurveObjectDiscountFactorsTable(
    const ExcelObj& curveCollection,
    const ExcelObj& curveIndices,
    const ExcelObj& startDate,
    const ExcelObj& maturity,
    const ExcelObj& businessDayAdjust,
    const ExcelObj& calendar,
    const ExcelObj& rollConvention,
    const ExcelObj& frequency ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const AQLStringVector indices = toAQLStringVector( curveIndices );

    DateVector paymentDates;
    DoubleMatrix discountFactors;

    validation::tryAqCurveObjectDiscountFactorsTable(
        paymentDates, discountFactors,
        toAQLString( curveCollection ),
        indices,
        toAQLString( startDate ),
        toAQLString( maturity ),
        toAQLString( businessDayAdjust ),
        toAQLString( calendar ),
        toAQLString( rollConvention ),
        toAQLString( frequency ) );

    etrading::VariantMatrix result;
    etrading::VariantVector header;
    header.push_back( etrading::Variant( "Date" ) );
    for ( const AQLString& index : indices )
    {
        header.push_back( etrading::Variant( index.getCString() ) );
    }
    result.push_back( header );

    for ( std::size_t i = 0; i < paymentDates.size(); ++i )
    {
        etrading::VariantVector row;
        row.push_back( etrading::Variant( toExcelDate( paymentDates[i] ) ) );
        if ( i < discountFactors.size() )
        {
            for ( double df : discountFactors[i] )
            {
                row.push_back( etrading::Variant( df ) );
            }
        }
        result.push_back( row );
    }

    return returnValue( toExcelMatrix( result ) );
}
XLO_FUNC_END( aqCurveObjectDiscountFactorsTable )
    .help( L"Discount factor table across one or more curve indices in a curve collection. "
           L"First column is the payment date, one further column per curve index." )
    .arg( L"CurveCollection",    L"The curve collection" )
    .arg( L"CurveIndices",       L"Column of curve indices to include" )
    .arg( L"StartDate",          L"Table start date" )
    .arg( L"Maturity",           L"Table end tenor/date" )
    .arg( L"BusinessDayAdjust",  L"Business day adjustment" )
    .arg( L"Calendar",           L"Holiday centre(s)" )
    .arg( L"RollConvention",     L"Roll convention, e.g. Normal, IMM, EOM" )
    .arg( L"Frequency",          L"Table row frequency, e.g. 3M, 6M" );


XLO_FUNC_START( aqCurveObjectDiscountFactorsWithSpread(
    const ExcelObj& paymentDates,
    const ExcelObj& curveCollection,
    const ExcelObj& curveIndex,
    const ExcelObj& spread,
    const ExcelObj& fixingTableName ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelDoubleColumn( validation::tryAqCurveObjectDiscountFactorsWithSpread(
        toDateVector( paymentDates, true, "PaymentDates" ),
        toNarrowString( curveCollection ),
        toNarrowString( curveIndex ),
        spread.get<double>(),
        toNarrowString( fixingTableName ) ) ) );
}
XLO_FUNC_END( aqCurveObjectDiscountFactorsWithSpread )
    .help( L"Discount factors from a curve collection/index, with a spread applied over a fixing table." )
    .arg( L"PaymentDates",    L"Column of payment dates" )
    .arg( L"CurveCollection", L"The curve collection" )
    .arg( L"CurveIndex",      L"The curve index" )
    .arg( L"Spread",          L"Spread to apply" )
    .arg( L"FixingTableName", L"A fixing table handle for the spread's reset basis" );


/* -------------------------------------------------------------------------
 *  Forward rates
 * ---------------------------------------------------------------------- */

XLO_FUNC_START( aqCurveObjectForwardRatesFromYearFraction(
    const ExcelObj& aqObjCurveName,
    const ExcelObj& fromDates,
    const ExcelObj& yearFraction,
    const ExcelObj& dayCount ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelDoubleColumn( validation::tryAqCurveObjectForwardRatesFromYearFraction(
        getNameWithoutCounter( aqObjCurveName ),
        toGregorianVector( fromDates, true, "FromDates" ),
        yearFraction.get<double>(),
        toNarrowString( dayCount ) ) ) );
}
XLO_FUNC_END( aqCurveObjectForwardRatesFromYearFraction )
    .help( L"Forward rates from a cached curve, from-dates plus a common forward year fraction." )
    .arg( L"AqObjCurveName", L"A curve handle" )
    .arg( L"FromDates",      L"Column of forward-start dates" )
    .arg( L"YearFraction",   L"Forward period length, in years, common to every date" )
    .arg( L"DayCount",       L"Day count convention, e.g. ACT/360, ACT/365" );


XLO_FUNC_START( aqCurveObjectForwardRatesFromForwardDates(
    const ExcelObj& aqObjCurveName,
    const ExcelObj& fromDates,
    const ExcelObj& toDates ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelDoubleColumn( validation::tryAqCurveObjectForwardRatesFromForwardDates(
        getNameWithoutCounter( aqObjCurveName ),
        toGregorianVector( fromDates, true, "FromDates" ),
        toGregorianVector( toDates, true, "ToDates" ) ) ) );
}
XLO_FUNC_END( aqCurveObjectForwardRatesFromForwardDates )
    .help( L"Forward rates from a cached curve between paired (from, to) dates." )
    .arg( L"AqObjCurveName", L"A curve handle" )
    .arg( L"FromDates",      L"Column of forward-start dates" )
    .arg( L"ToDates",        L"Column of forward-end dates, aligned with FromDates" );


XLO_FUNC_START( aqCurveObjectForwardRates(
    const ExcelObj& aqObjCurveName,
    const ExcelObj& fixingDates ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelDoubleColumn( validation::tryAqCurveObjectForwardRates(
        getNameWithoutCounter( aqObjCurveName ), toGregorianVector( fixingDates, true, "FixingDates" ) ) ) );
}
XLO_FUNC_END( aqCurveObjectForwardRates )
    .help( L"Forward rates from a cached curve at a column of fixing dates (the curve's own tenor convention)." )
    .arg( L"AqObjCurveName", L"A curve handle" )
    .arg( L"FixingDates",    L"Column of fixing dates" );


/* -------------------------------------------------------------------------
 *  Zero rates (stateless - curve collection/index in, values out)
 * ---------------------------------------------------------------------- */

XLO_FUNC_START( aqCurveZeroRatesFromYearFractions(
    const ExcelObj& yearFractions,
    const ExcelObj& curveCollection,
    const ExcelObj& curveIndex,
    const ExcelObj& frequency,
    const ExcelObj& dayCount,
    const ExcelObj& fwdInter ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelDoubleColumn( validation::tryAqCurveZeroRatesFromYearFractions(
        toDoubleVector( yearFractions, true, "YearFractions" ),
        toAQLString( curveCollection ),
        toAQLString( curveIndex ),
        toAQLString( frequency ),
        toAQLString( dayCount ),
        toStrOr( fwdInter, "" ) ) ) );
}
XLO_FUNC_END( aqCurveZeroRatesFromYearFractions )
    .help( L"Zero rates from a curve collection/index at a column of year fractions." )
    .arg( L"YearFractions",  L"Column of year fractions" )
    .arg( L"CurveCollection", L"The curve collection" )
    .arg( L"CurveIndex",     L"The curve index" )
    .arg( L"Frequency",      L"Compounding frequency, e.g. Annual, SemiAnnual" )
    .arg( L"DayCount",       L"Day count convention, e.g. ACT/360, ACT/365" )
    .arg( L"FwdInter",       L"Optional. Forward-interpolation override" );


XLO_FUNC_START( aqCurveZeroRatesFromTenors(
    const ExcelObj& tenors,
    const ExcelObj& curveCollection,
    const ExcelObj& curveIndex,
    const ExcelObj& frequency,
    const ExcelObj& dayCount,
    const ExcelObj& calendar,
    const ExcelObj& businessDayAdj,
    const ExcelObj& fwdInter ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelDoubleColumn( validation::tryAqCurveZeroRatesFromTenors(
        toAQLStringVector( tenors ),
        toAQLString( curveCollection ),
        toAQLString( curveIndex ),
        toAQLString( frequency ),
        toAQLString( dayCount ),
        toAQLString( calendar ),
        toAQLString( businessDayAdj ),
        toStrOr( fwdInter, "" ) ) ) );
}
XLO_FUNC_END( aqCurveZeroRatesFromTenors )
    .help( L"Zero rates from a curve collection/index at a column of tenors." )
    .arg( L"Tenors",          L"Column of tenors, e.g. 3M, 5Y" )
    .arg( L"CurveCollection", L"The curve collection" )
    .arg( L"CurveIndex",      L"The curve index" )
    .arg( L"Frequency",       L"Compounding frequency, e.g. Annual, SemiAnnual" )
    .arg( L"DayCount",        L"Day count convention, e.g. ACT/360, ACT/365" )
    .arg( L"Calendar",        L"Holiday centre(s) for the tenor roll" )
    .arg( L"BusinessDayAdj",  L"Business day adjustment for the tenor roll" )
    .arg( L"FwdInter",        L"Optional. Forward-interpolation override" );


/* -------------------------------------------------------------------------
 *  Curve generator (a named sub-object - conventions)
 * ---------------------------------------------------------------------- */

// Create and store a curve generator from one or two named data blocks.
XLO_FUNC_START( aqCurveGeneratorCreate(
    const ExcelObj& objectName,
    const ExcelObj& key1,
    const ExcelObj& value1,
    const ExcelObj& key2,
    const ExcelObj& value2 ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const std::string name = decorateWithExcelLocation( toNarrowString( objectName ) );

    std::vector<std::string>           propertyNames;
    std::vector<validation::TableInfo> infoBlocks;

    propertyNames.push_back( etrading::trim_to_upper( toNarrowString( key1 ) ) );
    infoBlocks.push_back( toTableInfo( value1 ) );

    if ( !value2.isMissing() && value2.isNonEmpty() )
    {
        propertyNames.push_back( etrading::trim_to_upper( toNarrowString( key2 ) ) );
        infoBlocks.push_back( toTableInfo( value2 ) );
    }

    const std::string storedName = validation::tryAqCurveGeneratorCreate( name, propertyNames, infoBlocks );

    return returnValue( appendInstanceCounter( storedName ) );
}
XLO_FUNC_END( aqCurveGeneratorCreate )
    .help( L"Create and store a curve generator from one or two named data blocks; returns its handle." )
    .arg( L"ObjectName", L"Name for the curve-generator object" )
    .arg( L"Key1",       L"Name of the first data block" )
    .arg( L"Value1",     L"First data block, as a range" )
    .arg( L"Key2",       L"Optional. Name of the second data block" )
    .arg( L"Value2",     L"Optional. Second data block, as a range" );


// Display a stored curve generator's configuration.
XLO_FUNC_START( aqCurveGeneratorDisplay(
    const ExcelObj& objectName,
    const ExcelObj& propertyName ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelMatrix( validation::tryAqCurveGeneratorDisplay(
        getNameWithoutCounter( objectName ), toNarrowString( propertyName ) ) ) );
}
XLO_FUNC_END( aqCurveGeneratorDisplay )
    .help( L"Display a stored curve generator's configuration block as a matrix." )
    .arg( L"ObjectName",   L"A curve-generator handle" )
    .arg( L"PropertyName", L"The configuration block to display" );


// Create a new curve generator by overriding values on an existing one.
XLO_FUNC_START( aqCurveGeneratorModify(
    const ExcelObj& newObjectName,
    const ExcelObj& baseObjectName,
    const ExcelObj& modifiedValues ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const std::string newName = decorateWithExcelLocation( toNarrowString( newObjectName ) );

    const std::string storedName = validation::tryAqCurveGeneratorModify(
        newName, getNameWithoutCounter( baseObjectName ), toLabelValueBlock( modifiedValues ) );

    return returnValue( appendInstanceCounter( storedName ) );
}
XLO_FUNC_END( aqCurveGeneratorModify )
    .help( L"Create a new curve generator by overriding values on an existing one; returns its handle." )
    .arg( L"NewObjectName",  L"Name for the new curve-generator object" )
    .arg( L"BaseObjectName", L"The existing curve-generator handle to copy from" )
    .arg( L"ModifiedValues", L"The overrides as a label/value block" );


// Display a curve object's resolved conventions.
XLO_FUNC_START( aqCurveObjectDisplayConventions(
    const ExcelObj& objectName,
    const ExcelObj& propertyKey ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelMatrix( validation::tryAqCurveObjectDisplayConventions(
        getNameWithoutCounter( objectName ), toNarrowString( propertyKey ) ) ) );
}
XLO_FUNC_END( aqCurveObjectDisplayConventions )
    .help( L"Display a curve object's resolved conventions as a matrix." )
    .arg( L"ObjectName",   L"A curve handle" )
    .arg( L"PropertyKey",  L"The convention block to display" );


/* -------------------------------------------------------------------------
 *  Curve market data (a named sub-object - rates)
 * ---------------------------------------------------------------------- */

// Create and store curve market data from one or two named data blocks.
XLO_FUNC_START( aqCurveMarketDataCreate(
    const ExcelObj& objectName,
    const ExcelObj& key1,
    const ExcelObj& value1,
    const ExcelObj& key2,
    const ExcelObj& value2 ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const std::string name = decorateWithExcelLocation( toNarrowString( objectName ) );

    std::vector<std::string>           marketDataKeys;
    std::vector<validation::TableInfo> infoBlocks;

    marketDataKeys.push_back( etrading::trim_to_upper( toNarrowString( key1 ) ) );
    infoBlocks.push_back( toTableInfo( value1 ) );

    if ( !value2.isMissing() && value2.isNonEmpty() )
    {
        marketDataKeys.push_back( etrading::trim_to_upper( toNarrowString( key2 ) ) );
        infoBlocks.push_back( toTableInfo( value2 ) );
    }

    const std::string storedName = validation::tryAqCurveMarketDataCreate( name, marketDataKeys, infoBlocks );

    return returnValue( appendInstanceCounter( storedName ) );
}
XLO_FUNC_END( aqCurveMarketDataCreate )
    .help( L"Create and store curve market data from one or two named data blocks; returns its handle." )
    .arg( L"ObjectName", L"Name for the curve-market-data object" )
    .arg( L"Key1",       L"Name of the first data block" )
    .arg( L"Value1",     L"First data block, as a range" )
    .arg( L"Key2",       L"Optional. Name of the second data block" )
    .arg( L"Value2",     L"Optional. Second data block, as a range" );


// Clear every bump applied to stored curve market data.
XLO_FUNC_START( aqCurveMarketDataBumpClear(
    const ExcelObj& objectName ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqCurveMarketDataBumpClear( getNameWithoutCounter( objectName ) ) );
}
XLO_FUNC_END( aqCurveMarketDataBumpClear )
    .help( L"Clear every bump applied to stored curve market data. Returns a status string." )
    .arg( L"ObjectName", L"A curve-market-data handle" );


// Bump one instrument type in stored curve market data.
XLO_FUNC_START( aqCurveMarketDataBumpInstrument(
    const ExcelObj& objectName,
    const ExcelObj& marketDataType,
    const ExcelObj& bumpSize,
    const ExcelObj& clearExistingBumps ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqCurveMarketDataBumpInstrument(
        getNameWithoutCounter( objectName ), toNarrowString( marketDataType ),
        bumpSize.get<double>(), toBool( clearExistingBumps, false ) ) );
}
XLO_FUNC_END( aqCurveMarketDataBumpInstrument )
    .help( L"Bump one instrument type in stored curve market data. Returns a status string." )
    .arg( L"ObjectName",          L"A curve-market-data handle" )
    .arg( L"MarketDataType",      L"Which instrument type to bump" )
    .arg( L"BumpSize",            L"Bump size to apply" )
    .arg( L"ClearExistingBumps",  L"Optional. Default FALSE. Clear prior bumps first" );


// Bump one instrument type on a cached curve object.
XLO_FUNC_START( aqCurveObjectBumpInstrument(
    const ExcelObj& objectName,
    const ExcelObj& marketDataType,
    const ExcelObj& bumpSize,
    const ExcelObj& clearExistingBumps ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqCurveObjectBumpInstrument(
        getNameWithoutCounter( objectName ), toNarrowString( marketDataType ),
        bumpSize.get<double>(), toBool( clearExistingBumps, false ) ) );
}
XLO_FUNC_END( aqCurveObjectBumpInstrument )
    .help( L"Bump one instrument type on a cached curve object. Returns a status string." )
    .arg( L"ObjectName",          L"A curve handle" )
    .arg( L"MarketDataType",      L"Which instrument type to bump" )
    .arg( L"BumpSize",            L"Bump size to apply" )
    .arg( L"ClearExistingBumps",  L"Optional. Default FALSE. Clear prior bumps first" );


// Bump every instrument in stored curve market data.
XLO_FUNC_START( aqCurveMarketDataBumpAll(
    const ExcelObj& objectName,
    const ExcelObj& bumpSize,
    const ExcelObj& onlyBumpOutrightInstruments ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqCurveMarketDataBumpAll(
        getNameWithoutCounter( objectName ), bumpSize.get<double>(),
        toBool( onlyBumpOutrightInstruments, true ) ) );
}
XLO_FUNC_END( aqCurveMarketDataBumpAll )
    .help( L"Bump every instrument in stored curve market data. Returns a status string." )
    .arg( L"ObjectName",                   L"A curve-market-data handle" )
    .arg( L"BumpSize",                     L"Bump size to apply" )
    .arg( L"OnlyBumpOutrightInstruments",  L"Optional. Default TRUE. Skip basis/spread instruments" );


// Bump every instrument on a cached curve object.
XLO_FUNC_START( aqCurveObjectBumpAll(
    const ExcelObj& objectName,
    const ExcelObj& bumpSize,
    const ExcelObj& onlyBumpOutrightInstruments ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqCurveObjectBumpAll(
        getNameWithoutCounter( objectName ), bumpSize.get<double>(),
        toBool( onlyBumpOutrightInstruments, true ) ) );
}
XLO_FUNC_END( aqCurveObjectBumpAll )
    .help( L"Bump every instrument on a cached curve object. Returns a status string." )
    .arg( L"ObjectName",                   L"A curve handle" )
    .arg( L"BumpSize",                     L"Bump size to apply" )
    .arg( L"OnlyBumpOutrightInstruments",  L"Optional. Default TRUE. Skip basis/spread instruments" );


// Display stored curve market data.
XLO_FUNC_START( aqCurveMarketDataDisplay(
    const ExcelObj& marketDataObjectName,
    const ExcelObj& marketDataKey,
    const ExcelObj& columnIndexToDisplay ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelMatrix( validation::tryAqCurveMarketDataDisplay(
        getNameWithoutCounter( marketDataObjectName ),
        toNarrowString( marketDataKey ),
        toIntOr( columnIndexToDisplay, -1 ) ) ) );
}
XLO_FUNC_END( aqCurveMarketDataDisplay )
    .help( L"Display stored curve market data as a matrix." )
    .arg( L"MarketDataObjectName", L"A curve-market-data handle" )
    .arg( L"MarketDataKey",        L"The data block to display" )
    .arg( L"ColumnIndexToDisplay", L"Optional. Default -1 (all columns). A single column index to display" );


// Display the curve market data referenced by a cached curve object.
XLO_FUNC_START( aqCurveMarketDataDisplayFromCurve(
    const ExcelObj& curveObjectName,
    const ExcelObj& marketDataKey,
    const ExcelObj& columnIndexToDisplay ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelMatrix( validation::tryAqCurveMarketDataDisplayFromCurve(
        getNameWithoutCounter( curveObjectName ),
        toNarrowString( marketDataKey ),
        toIntOr( columnIndexToDisplay, -1 ) ) ) );
}
XLO_FUNC_END( aqCurveMarketDataDisplayFromCurve )
    .help( L"Display the curve market data referenced by a cached curve object, as a matrix." )
    .arg( L"CurveObjectName",      L"A curve handle" )
    .arg( L"MarketDataKey",        L"The data block to display" )
    .arg( L"ColumnIndexToDisplay", L"Optional. Default -1 (all columns). A single column index to display" );


// One column of curve market data.
XLO_FUNC_START( aqCurveMarketDataColumn(
    const ExcelObj& curveObjectName,
    const ExcelObj& marketDataKey,
    const ExcelObj& columnNumber ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const etrading::VariantVector column = validation::tryAqCurveMarketDataColumn(
        getNameWithoutCounter( curveObjectName ), toNarrowString( marketDataKey ), toIntOr( columnNumber, 0 ) );

    etrading::VariantMatrix asMatrix;
    for ( const etrading::Variant& cell : column )
    {
        etrading::VariantVector row;
        row.push_back( cell );
        asMatrix.push_back( row );
    }

    return returnValue( toExcelMatrix( asMatrix ) );
}
XLO_FUNC_END( aqCurveMarketDataColumn )
    .help( L"One column of curve market data, as a column." )
    .arg( L"CurveObjectName", L"A curve handle" )
    .arg( L"MarketDataKey",   L"The data block to read from" )
    .arg( L"ColumnNumber",    L"Which column to return" );
