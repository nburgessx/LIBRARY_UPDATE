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
 * rate / zero rate families, generator, market data.
 *
 * Batch 2: the legacy stateless curveCollection+curveIndex family that
 * predates the AQObj curve handle (CompoundRate, Delete, DiscountFactor,
 * ForwardRate, Display, Frequency, Interpolation join-date, HullWhite/
 * Vasicek forward-rate checks, EuroDollarConvexityAdjustment, the four heavy
 * one-shot Calibrate{Basis,CTD,FXForwards,OIS,Swap} functions and their
 * ObjectCreate{Basis,FXForwards,OIS,Swap} handle-returning twins), plus the
 * Results/Jacobian risk family, MarketData "ObjectData" LVB family, dual
 * bootstrap and engine calibrate, and curve groups. Decided with Nicholas
 * 2026-09-11: **both families are live and both get ported** - stateless
 * functions keep the `aqCurve<Function>` form (no `Object`), object/handle
 * functions keep `aqCurveObject<Function>`, exactly per the golden `tryAq*`
 * name (no renaming - these are pre-existing validation wrappers, not new
 * names being coined). This closes out the Curve category (93 functions) bar
 * three internal-only helpers deliberately NOT exposed: getDayCount,
 * isBasisFlag and populateDiscountFactorConventions (in
 * tryAqCurveDiscountFactor.h) are legacy default-population plumbing with no
 * `aqCurve` naming and nothing meaningful for a user to call directly.
 *
 * Each function pairs with the identically named validation wrapper (plus the
 * `try` prefix). Marshalling to and from Excel is the aq_xll helpers in
 * aqXllTools.h.
 */

#include <aqMain.h>

#include <string>
#include <vector>
#include <map>
#include <limits>

#include <aqXllTools.h>
#include <CoreEnumerations.h>                // etrading::trim_to_upper / toBusinessDayAdjustmentEnum / toRiskTypeEnum
#include <Variant.h>                        // etrading::Variant, VariantMatrix
#include <AQLCurveCalibrationHelpers.h>      // etrading::DiscountFactorTable
#include <tryAqCurveObjectUtilities.h>       // validation::tryAqCurveObject{List,Delete,DeleteAll,Save,Load}
#include <tryAqCurveObjectDisplay.h>         // validation::tryAqCurveObjectDisplay
#include <tryAqCurveObjectCalibrate.h>       // validation::tryAqCurveObjectCalibrate
#include <tryAqCurveObjectCalibrateHedge.h>  // validation::tryAqCurveObjectCalibrateHedge
#include <tryAqCurveObjectDiscountFactor.h>  // validation::tryAqCurveObjectDiscountFactors*
#include <tryAqCurveObjectForwardRate.h>     // validation::tryAqCurveObjectForwardRates*
#include <tryAqCurveZeroRate.h>              // validation::tryAqCurveZeroRatesFrom{YearFractions,Tenors}
#include <tryAqCurveGenerator.h>             // validation::tryAqCurveGenerator* / tryAqCurveObjectDisplayConventions
#include <tryAqCurveMarketData.h>            // validation::tryAqCurveMarketData* / tryAqCurveObjectBump*
#include <tryAqCurveCompoundRate.h>          // validation::tryAqCurveCompoundRate*
#include <tryAqCurveDelete.h>                // validation::tryAqCurveDelete
#include <tryAqCurveDiscountFactor.h>        // validation::tryAqCurveDiscountFactors* / tryAqCurve{Terms,Dates}To*
#include <tryAqCurveDisplay.h>               // validation::tryAqCurveDisplay
#include <tryAqCurveDualBootstrap.h>         // validation::tryAqCurveDualBootstrap
#include <tryAqCurveEuroDollarConvexityAdjustment.h>  // validation::tryAqCurveEuroDollarConvexityAdjustment
#include <tryAqCurveForwardRate.h>           // validation::tryAqCurveForwardRates* / tryAqCurveObjectForwardRatesTable
#include <tryAqCurveFrequency.h>             // validation::tryAqCurveFrequency
#include <tryAqCurveGetInterpolationJoinDate.h>  // validation::tryAqCurveGetInterpolationJoinDate
#include <tryAqCurveHullWhite.h>             // validation::tryAqCurveHullWhiteForwardRates
#include <tryAqCurveInterpolation.h>         // validation::tryAqCurveInterpolationJoinDate
#include <tryAqCurveObjectData.h>            // validation::tryAqCurveObjectDataCreate/Display
#include <tryAqCurveObjectDualBootstrap.h>   // validation::tryAqCurveObjectDualBootstrap
#include <tryAqCurveObjectEngineCalibrate.h> // validation::tryAqCurveObjectEngineCalibrate
#include <tryAqCurveObjectJacobianDisplay.h> // validation::tryAqCurveObjectEngineJacobianDisplay / tryAqCurveObjectJacobianDisplay
#include <tryAqCurveResults.h>               // validation::tryAqCurveResults* / tryAqCurveGroup*
#include <tryAqCurveVasicek.h>               // validation::tryAqCurveVasicek*
#include <tryAqCurveCalibrateBasis.h>        // validation::tryAqCurveCalibrateBasis
#include <tryAqCurveCalibrateCTD.h>          // validation::tryAqCurveCalibrateCTD
#include <tryAqCurveCalibrateFXForwards.h>   // validation::tryAqCurveCalibrateFXForwards
#include <tryAqCurveCalibrateOIS.h>          // validation::tryAqCurveCalibrateOIS
#include <tryAqCurveCalibrateSwap.h>         // validation::tryAqCurveCalibrateSwap
#include <tryAqCurveObjectCreateBasis.h>     // validation::tryAqCurveObjectCreateBasis
#include <tryAqCurveObjectCreateFXForwards.h> // validation::tryAqCurveObjectCreateFXForwards
#include <tryAqCurveObjectCreateOIS.h>       // validation::tryAqCurveObjectCreateOIS
#include <tryAqCurveObjectCreateSwap.h>      // validation::tryAqCurveObjectCreateSwap
#include <tryAqDate.h>                       // validation::tryAqCurveUSDSpotDate (filed under Date, golden-named Curve)

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

    // Optional floating-point worksheet argument.
    double toDoubleOr( const xloil::ExcelObj& obj, double defaultValue )
    {
        if ( obj.isMissing() || !obj.isNonEmpty() )
        {
            return defaultValue;
        }
        return obj.get<double>();
    }

    // Optional date worksheet argument: missing / empty / blank yields a
    // default-constructed AQLDate() (the "not provided" sentinel several
    // Curve model functions use for valuationDate, falling back internally
    // to the curve's own as-of date).
    AQLDate toAQLDateOr( const xloil::ExcelObj& obj )
    {
        if ( obj.isMissing() || !obj.isNonEmpty() )
        {
            return AQLDate();
        }
        return toAQLDate( obj );
    }

    // Optional AQLStringMatrix worksheet argument: missing / empty / blank
    // yields a genuinely empty matrix (not a 1x1 block holding an empty
    // string, which toAQLStringMatrix would otherwise produce for a missing
    // cell) - several calibration/data-block functions test .empty() on
    // these to decide whether an optional data block was supplied at all.
    AQLStringMatrix toAQLStringMatrixOr( const xloil::ExcelObj& obj )
    {
        if ( obj.isMissing() || !obj.isNonEmpty() )
        {
            return AQLStringMatrix();
        }
        return toAQLStringMatrix( obj );
    }

    // A column of TRUE/FALSE (or 0/1) cells -> a vector<bool>, one flag per
    // curve calibration instrument. Backs tryAqCurveResultsJacobianUpdate's
    // outrightInstruments flag column.
    std::vector<bool> toBoolVector( const xloil::ExcelObj& obj )
    {
        const etrading::VariantVector flags = toVariantVector( obj );
        std::vector<bool> result;
        result.reserve( flags.size() );
        for ( const etrading::Variant& flag : flags )
        {
            result.push_back( flag.getValue<bool>() );
        }
        return result;
    }

    // A plain DoubleMatrix -> an Excel array. aqXllTools has no direct
    // DoubleMatrix output overload (only VariantMatrix / AQLStringMatrix /
    // AnyTypeMatrix), so wrap it as a VariantMatrix here. Backs the Jacobian
    // display functions.
    xloil::ExcelObj toExcelDoubleMatrix( const DoubleMatrix& matrix )
    {
        etrading::VariantMatrix result;
        result.reserve( matrix.size() );
        for ( const std::vector<double>& row : matrix )
        {
            etrading::VariantVector variantRow;
            variantRow.reserve( row.size() );
            for ( double value : row )
            {
                variantRow.push_back( etrading::Variant( value ) );
            }
            result.push_back( variantRow );
        }
        return toExcelMatrix( result );
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


/* -------------------------------------------------------------------------
 *  Batch 2 - legacy stateless curveCollection+curveIndex family, plus
 *  Results/Jacobian risk, dual bootstrap, engine calibrate and curve groups.
 * ---------------------------------------------------------------------- */

// Compounded interest rate over a schedule of accrual periods.
XLO_FUNC_START( aqCurveCompoundRate(
    const ExcelObj& startDates,
    const ExcelObj& endDates,
    const ExcelObj& curveCollection,
    const ExcelObj& forecastCurveIndex,
    const ExcelObj& frequency,
    const ExcelObj& spread,
    const ExcelObj& stubType,
    const ExcelObj& rollDay,
    const ExcelObj& calendar,
    const ExcelObj& businessDayAdj,
    const ExcelObj& dayCount,
    const ExcelObj& interpolation,
    const ExcelObj& compoundType,
    const ExcelObj& firstStubDate,
    const ExcelObj& lastStubDate ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelDoubleColumn( validation::tryAqCurveCompoundRate(
        toDateVector( startDates, true, "StartDates" ),
        toDateVector( endDates, true, "EndDates" ),
        toAQLString( curveCollection ),
        toAQLString( forecastCurveIndex ),
        toAQLString( frequency ),
        spread.get<double>(),
        toAQLString( stubType ),
        toAQLString( rollDay ),
        toAQLString( calendar ),
        toAQLString( businessDayAdj ),
        toAQLString( dayCount ),
        toAQLString( interpolation ),
        toAQLString( compoundType ),
        toAQLString( firstStubDate ),
        toAQLString( lastStubDate ) ) ) );
}
XLO_FUNC_END( aqCurveCompoundRate )
    .help( L"Compounded interest rate over a schedule of accrual periods, priced off a curve collection/index." )
    .arg( L"StartDates",         L"Column of accrual start dates" )
    .arg( L"EndDates",           L"Column of accrual end dates, aligned with StartDates" )
    .arg( L"CurveCollection",    L"The curve collection" )
    .arg( L"ForecastCurveIndex", L"The forecasting curve index" )
    .arg( L"Frequency",          L"Compounding frequency" )
    .arg( L"Spread",             L"Spread in basis points" )
    .arg( L"StubType",           L"None, ShortStart (SS), LongStart (LS), ShortEnd (SE) or LongEnd (LE)" )
    .arg( L"RollDay",            L"Roll day convention, e.g. ENDDATE" )
    .arg( L"Calendar",           L"Holiday centre(s)" )
    .arg( L"BusinessDayAdj",     L"Business day adjustment, e.g. MODFOLLOWING" )
    .arg( L"DayCount",           L"Day count convention" )
    .arg( L"Interpolation",      L"Interpolation method" )
    .arg( L"CompoundType",       L"NORMAL, FLAT, SIMPLE or AVERAGE" )
    .arg( L"FirstStubDate",      L"End date of the front stub period" )
    .arg( L"LastStubDate",       L"Start date of the end stub period" );


// As aqCurveCompoundRate, applying a spread over a cached fixing table.
XLO_FUNC_START( aqCurveCompoundRateWithFixingTable(
    const ExcelObj& startDates,
    const ExcelObj& endDates,
    const ExcelObj& curveCollection,
    const ExcelObj& forecastCurveIndex,
    const ExcelObj& frequency,
    const ExcelObj& spread,
    const ExcelObj& stubType,
    const ExcelObj& rollDay,
    const ExcelObj& calendar,
    const ExcelObj& businessDayAdj,
    const ExcelObj& dayCount,
    const ExcelObj& interpolation,
    const ExcelObj& compoundType,
    const ExcelObj& firstStubDate,
    const ExcelObj& lastStubDate,
    const ExcelObj& fixingTableName,
    const ExcelObj& annualized ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelDoubleColumn( validation::tryAqCurveCompoundRateWithFixingTable(
        toDateVector( startDates, true, "StartDates" ),
        toDateVector( endDates, true, "EndDates" ),
        toAQLString( curveCollection ),
        toAQLString( forecastCurveIndex ),
        toAQLString( frequency ),
        spread.get<double>(),
        toAQLString( stubType ),
        toAQLString( rollDay ),
        toAQLString( calendar ),
        toAQLString( businessDayAdj ),
        toAQLString( dayCount ),
        toAQLString( interpolation ),
        toAQLString( compoundType ),
        toAQLString( firstStubDate ),
        toAQLString( lastStubDate ),
        toNarrowString( fixingTableName ),
        toBool( annualized, true ) ) ) );
}
XLO_FUNC_END( aqCurveCompoundRateWithFixingTable )
    .help( L"As aqCurveCompoundRate, applying a spread over a cached fixing table's resets." )
    .arg( L"StartDates",         L"Column of accrual start dates" )
    .arg( L"EndDates",           L"Column of accrual end dates, aligned with StartDates" )
    .arg( L"CurveCollection",    L"The curve collection" )
    .arg( L"ForecastCurveIndex", L"The forecasting curve index" )
    .arg( L"Frequency",          L"Compounding frequency" )
    .arg( L"Spread",             L"Spread in basis points" )
    .arg( L"StubType",           L"None, ShortStart (SS), LongStart (LS), ShortEnd (SE) or LongEnd (LE)" )
    .arg( L"RollDay",            L"Roll day convention, e.g. ENDDATE" )
    .arg( L"Calendar",           L"Holiday centre(s)" )
    .arg( L"BusinessDayAdj",     L"Business day adjustment, e.g. MODFOLLOWING" )
    .arg( L"DayCount",           L"Day count convention" )
    .arg( L"Interpolation",      L"Interpolation method" )
    .arg( L"CompoundType",       L"NORMAL, FLAT, SIMPLE or AVERAGE" )
    .arg( L"FirstStubDate",      L"End date of the front stub period" )
    .arg( L"LastStubDate",       L"Start date of the end stub period" )
    .arg( L"FixingTableName",    L"A fixing table handle for the spread's reset basis" )
    .arg( L"Annualized",         L"Optional. Default TRUE. Return an annualized rate" );


// Remove one curve index from a curve collection (legacy stateless twin of aqCurveObjectDelete).
XLO_FUNC_START( aqCurveDelete(
    const ExcelObj& curveCollection,
    const ExcelObj& curveIndex ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( std::string( validation::tryAqCurveDelete( toAQLString( curveCollection ), toAQLString( curveIndex ) ).getCString() ) );
}
XLO_FUNC_END( aqCurveDelete )
    .help( L"Remove one curve index from a curve collection. Returns a status string." )
    .arg( L"CurveCollection", L"The curve collection" )
    .arg( L"CurveIndex",      L"The curve index to remove" );


// Terms and discount factors from a curve collection/index, as a 2-column array.
XLO_FUNC_START( aqCurveDisplay(
    const ExcelObj& curveCollection,
    const ExcelObj& curveIndex ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelDoubleColumn(
        validation::tryAqCurveDisplay( toAQLString( curveCollection ), toAQLString( curveIndex ) ) ) );
}
XLO_FUNC_END( aqCurveDisplay )
    .help( L"Terms and discount factors from a curve collection/index." )
    .arg( L"CurveCollection", L"The curve collection" )
    .arg( L"CurveIndex",      L"The curve index" );


// The floating index frequency of a curve collection/index.
XLO_FUNC_START( aqCurveFrequency(
    const ExcelObj& curveCollection,
    const ExcelObj& curveIndex ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( std::string( validation::tryAqCurveFrequency( toAQLString( curveCollection ), toAQLString( curveIndex ) ).getCString() ) );
}
XLO_FUNC_END( aqCurveFrequency )
    .help( L"The floating index frequency of a curve collection/index, e.g. 3M." )
    .arg( L"CurveCollection", L"The curve collection" )
    .arg( L"CurveIndex",      L"The curve index" );


// The interpolation join date of a curve collection/index.
XLO_FUNC_START( aqCurveGetInterpolationJoinDate(
    const ExcelObj& curveCollection,
    const ExcelObj& curveIndex,
    const ExcelObj& interpolation ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelDate( validation::tryAqCurveGetInterpolationJoinDate(
        toAQLString( curveCollection ), toAQLString( curveIndex ), toStrOr( interpolation, "" ) ) ) );
}
XLO_FUNC_END( aqCurveGetInterpolationJoinDate )
    .help( L"The interpolation join date of a curve collection/index." )
    .arg( L"CurveCollection", L"The curve collection" )
    .arg( L"CurveIndex",      L"The curve index" )
    .arg( L"Interpolation",   L"Optional. Interpolation type override" );


// As aqCurveGetInterpolationJoinDate (a separate validation wrapper, kept
// distinct per the golden source rather than treated as a duplicate).
XLO_FUNC_START( aqCurveInterpolationJoinDate(
    const ExcelObj& curveCollection,
    const ExcelObj& curveIndex,
    const ExcelObj& interpolation ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelDate( validation::tryAqCurveInterpolationJoinDate(
        toAQLString( curveCollection ), toAQLString( curveIndex ), toStrOr( interpolation, "" ) ) ) );
}
XLO_FUNC_END( aqCurveInterpolationJoinDate )
    .help( L"The interpolation join date of a curve collection/index." )
    .arg( L"CurveCollection", L"The curve collection" )
    .arg( L"CurveIndex",      L"The curve index" )
    .arg( L"Interpolation",   L"Optional. Interpolation type override" );


// The EuroDollar futures convexity adjustment under a Hull-White 1F model.
XLO_FUNC_START( aqCurveEuroDollarConvexityAdjustment(
    const ExcelObj& curveAsOfDate,
    const ExcelObj& futuresStartDate,
    const ExcelObj& futuresEndDate,
    const ExcelObj& meanReversion,
    const ExcelObj& volatility ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqCurveEuroDollarConvexityAdjustment(
        toAQLDate( curveAsOfDate ), toAQLDate( futuresStartDate ), toAQLDate( futuresEndDate ),
        meanReversion.get<double>(), volatility.get<double>() ) );
}
XLO_FUNC_END( aqCurveEuroDollarConvexityAdjustment )
    .help( L"The EuroDollar futures convexity adjustment under a Hull-White 1F model." )
    .arg( L"CurveAsOfDate",     L"The yield curve as-of / valuation date" )
    .arg( L"FuturesStartDate",  L"The futures start date" )
    .arg( L"FuturesEndDate",    L"The futures end date" )
    .arg( L"MeanReversion",     L"The Hull-White 1F mean reversion parameter" )
    .arg( L"Volatility",        L"The Hull-White 1F volatility parameter" );


/* -------------------------------------------------------------------------
 *  Legacy stateless discount factors (curveCollection+curveIndex)
 * ---------------------------------------------------------------------- */

XLO_FUNC_START( aqCurveDiscountFactorsFromYearFractions(
    const ExcelObj& yearFractions,
    const ExcelObj& dayCount,
    const ExcelObj& curveCollection,
    const ExcelObj& curveIndex ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelDoubleColumn( validation::tryAqCurveDiscountFactorsFromYearFractions(
        toDoubleVector( yearFractions, true, "YearFractions" ),
        toAQLString( dayCount ), toAQLString( curveCollection ), toAQLString( curveIndex ) ) ) );
}
XLO_FUNC_END( aqCurveDiscountFactorsFromYearFractions )
    .help( L"Discount factors from a curve collection/index at a column of year fractions." )
    .arg( L"YearFractions",  L"Column of year fractions" )
    .arg( L"DayCount",       L"Day count convention used to generate the year fraction" )
    .arg( L"CurveCollection", L"The curve collection" )
    .arg( L"CurveIndex",     L"The curve index" );


XLO_FUNC_START( aqCurveDiscountFactorsFromTenors(
    const ExcelObj& tenors,
    const ExcelObj& businessDayAdj,
    const ExcelObj& calendar,
    const ExcelObj& curveCollection,
    const ExcelObj& curveIndex ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelDoubleColumn( validation::tryAqCurveDiscountFactorsFromTenors(
        toAQLStringVector( tenors ), toAQLString( businessDayAdj ), toAQLString( calendar ),
        toAQLString( curveCollection ), toAQLString( curveIndex ) ) ) );
}
XLO_FUNC_END( aqCurveDiscountFactorsFromTenors )
    .help( L"Discount factors from a curve collection/index at a column of tenors." )
    .arg( L"Tenors",          L"Column of tenors, e.g. 3M, 5Y" )
    .arg( L"BusinessDayAdj",  L"Business day adjustment for the tenor roll, e.g. NO_CHANGE" )
    .arg( L"Calendar",        L"Holiday centre(s)" )
    .arg( L"CurveCollection", L"The curve collection" )
    .arg( L"CurveIndex",      L"The curve index" );


XLO_FUNC_START( aqCurveDiscountFactors(
    const ExcelObj& toDates,
    const ExcelObj& curveCollectionOrHandle,
    const ExcelObj& curveIndex ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelDoubleColumn( validation::tryAqCurveDiscountFactors(
        toDateVector( toDates, true, "ToDates" ), toAQLString( curveCollectionOrHandle ), toAQLString( curveIndex ) ) ) );
}
XLO_FUNC_END( aqCurveDiscountFactors )
    .help( L"Discount factors from a curve collection/index at a column of payment dates." )
    .arg( L"ToDates",               L"Column of payment dates" )
    .arg( L"CurveCollectionOrHandle", L"The curve collection (or a curve handle)" )
    .arg( L"CurveIndex",            L"The curve index" );


XLO_FUNC_START( aqCurveDiscountFactorsForwardStarting(
    const ExcelObj& fromDates,
    const ExcelObj& toDates,
    const ExcelObj& curveCollection,
    const ExcelObj& curveIndex ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelDoubleColumn( validation::tryAqCurveDiscountFactorsForwardStarting(
        toDateVector( fromDates, true, "FromDates" ), toDateVector( toDates, true, "ToDates" ),
        toAQLString( curveCollection ), toAQLString( curveIndex ) ) ) );
}
XLO_FUNC_END( aqCurveDiscountFactorsForwardStarting )
    .help( L"Forward-starting discount factors from a curve collection/index between paired (from, to) dates." )
    .arg( L"FromDates",       L"Column of forward-start dates" )
    .arg( L"ToDates",         L"Column of end dates, aligned with FromDates" )
    .arg( L"CurveCollection", L"The curve collection" )
    .arg( L"CurveIndex",      L"The curve index" );


XLO_FUNC_START( aqCurveDiscountFactorsForwardStartingFromYearFractions(
    const ExcelObj& fromDates,
    const ExcelObj& yearFractions,
    const ExcelObj& dayCount,
    const ExcelObj& curveCollection,
    const ExcelObj& curveIndex ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelDoubleColumn( validation::tryAqCurveDiscountFactorsForwardStartingFromYearFractions(
        toDateVector( fromDates, true, "FromDates" ), toDoubleVector( yearFractions, true, "YearFractions" ),
        toAQLString( dayCount ), toAQLString( curveCollection ), toAQLString( curveIndex ) ) ) );
}
XLO_FUNC_END( aqCurveDiscountFactorsForwardStartingFromYearFractions )
    .help( L"Forward-starting discount factors from a curve collection/index, from-dates plus year fractions." )
    .arg( L"FromDates",       L"Column of forward-start dates" )
    .arg( L"YearFractions",   L"Column of year fractions forward from FromDates, aligned" )
    .arg( L"DayCount",        L"Day count convention" )
    .arg( L"CurveCollection", L"The curve collection" )
    .arg( L"CurveIndex",      L"The curve index" );


XLO_FUNC_START( aqCurveDiscountFactorsForwardStartingFromTenor(
    const ExcelObj& fromDates,
    const ExcelObj& tenor,
    const ExcelObj& curveCollection,
    const ExcelObj& curveIndex,
    const ExcelObj& businessDayAdj,
    const ExcelObj& calendar ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelDoubleColumn( validation::tryAqCurveDiscountFactorsForwardStartingFromTenor(
        toDateVector( fromDates, true, "FromDates" ), toAQLString( tenor ),
        toAQLString( curveCollection ), toAQLString( curveIndex ),
        toAQLString( businessDayAdj ), toAQLString( calendar ) ) ) );
}
XLO_FUNC_END( aqCurveDiscountFactorsForwardStartingFromTenor )
    .help( L"Forward-starting discount factors from a curve collection/index, from-dates plus a single tenor." )
    .arg( L"FromDates",       L"Column of forward-start dates" )
    .arg( L"Tenor",           L"A single tenor forward from every FromDates entry, e.g. 3M" )
    .arg( L"CurveCollection", L"The curve collection" )
    .arg( L"CurveIndex",      L"The curve index" )
    .arg( L"BusinessDayAdj",  L"Business day adjustment for the tenor roll, e.g. NO_CHANGE" )
    .arg( L"Calendar",        L"Holiday centre(s)" );


// Override a curve's forward rates by setting equivalent discount factors.
XLO_FUNC_START( aqCurveForwardRatesOverride(
    const ExcelObj& curveCollection,
    const ExcelObj& curveIndex,
    const ExcelObj& fixingDates,
    const ExcelObj& forwardRates,
    const ExcelObj& setCorrespondingDiscountFactors ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( std::string( validation::tryAqCurveForwardRatesOverride(
        toAQLString( curveCollection ), toAQLString( curveIndex ),
        toDateVector( fixingDates, true, "FixingDates" ),
        toDoubleVector( forwardRates, true, "ForwardRates" ),
        toBool( setCorrespondingDiscountFactors, true ) ).getCString() ) );
}
XLO_FUNC_END( aqCurveForwardRatesOverride )
    .help( L"Override a curve's forward rates by setting equivalent discount factors. Returns a status string." )
    .arg( L"CurveCollection",                  L"The curve collection" )
    .arg( L"CurveIndex",                       L"The curve index" )
    .arg( L"FixingDates",                      L"Column of fixing dates" )
    .arg( L"ForwardRates",                     L"Column of new forward rates, aligned with FixingDates" )
    .arg( L"SetCorrespondingDiscountFactors",  L"Optional. Default TRUE. Also set STD-curve discount factors to 1.0 if FALSE" );


// Override a curve's discount factors directly.
XLO_FUNC_START( aqCurveDiscountFactorsOverride(
    const ExcelObj& curveCollection,
    const ExcelObj& curveIndex,
    const ExcelObj& paymentDates,
    const ExcelObj& discountFactors,
    const ExcelObj& setCorrespondingForwards ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( std::string( validation::tryAqCurveDiscountFactorsOverride(
        toAQLString( curveCollection ), toAQLString( curveIndex ),
        toDateVector( paymentDates, true, "PaymentDates" ),
        toDoubleVector( discountFactors, true, "DiscountFactors" ),
        toBool( setCorrespondingForwards, true ) ).getCString() ) );
}
XLO_FUNC_END( aqCurveDiscountFactorsOverride )
    .help( L"Override a curve's discount factors directly. Returns a status string. "
           L"Note: solving the corresponding forwards can fail to converge for extreme data." )
    .arg( L"CurveCollection",             L"The curve collection" )
    .arg( L"CurveIndex",                  L"The curve index" )
    .arg( L"PaymentDates",                L"Column of payment dates" )
    .arg( L"DiscountFactors",             L"Column of new discount factors, aligned with PaymentDates" )
    .arg( L"SetCorrespondingForwards",    L"Optional. Default TRUE. Sets STD-curve forwards to zero if FALSE" );


// Set every discount factor on a curve to one.
XLO_FUNC_START( aqCurveDiscountFactorsSetToOne(
    const ExcelObj& curveCollection,
    const ExcelObj& curveIndex ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( std::string( validation::tryAqCurveDiscountFactorsSetToOne(
        toAQLString( curveCollection ), toAQLString( curveIndex ) ).getCString() ) );
}
XLO_FUNC_END( aqCurveDiscountFactorsSetToOne )
    .help( L"Set every discount factor on a curve collection/index to one. Returns a status string." )
    .arg( L"CurveCollection", L"The curve collection" )
    .arg( L"CurveIndex",      L"The curve index" );


// Every discount factor currently stored on a curve, as a terms/dates/DF table.
XLO_FUNC_START( aqCurveDiscountFactorsDisplay(
    const ExcelObj& curveCollection,
    const ExcelObj& curveIndex ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const ::DiscountFactorTable table = validation::tryAqCurveDiscountFactorsDisplay(
        toAQLString( curveCollection ), toAQLString( curveIndex ) );

    etrading::VariantMatrix result;
    etrading::VariantVector header;
    header.push_back( etrading::Variant( "Term" ) );
    header.push_back( etrading::Variant( "PaymentDate" ) );
    header.push_back( etrading::Variant( "DiscountFactor" ) );
    result.push_back( header );

    for ( std::size_t i = 0; i < table.paymentDates_.size(); ++i )
    {
        etrading::VariantVector row;
        row.push_back( etrading::Variant( i < table.terms_.size() ? table.terms_[i] : 0.0 ) );
        row.push_back( etrading::Variant( toExcelDate( table.paymentDates_[i] ) ) );
        row.push_back( etrading::Variant( i < table.discountFactors_.size() ? table.discountFactors_[i] : 0.0 ) );
        result.push_back( row );
    }

    return returnValue( toExcelMatrix( result ) );
}
XLO_FUNC_END( aqCurveDiscountFactorsDisplay )
    .help( L"Every discount factor stored on a curve collection/index, as a Term/PaymentDate/DiscountFactor table." )
    .arg( L"CurveCollection", L"The curve collection" )
    .arg( L"CurveIndex",      L"The curve index" );


XLO_FUNC_START( aqCurveTermsToDates(
    const ExcelObj& curveCollection,
    const ExcelObj& terms ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelDateColumn( validation::tryAqCurveTermsToDates(
        toAQLString( curveCollection ), toDoubleVector( terms, true, "Terms" ) ) ) );
}
XLO_FUNC_END( aqCurveTermsToDates )
    .help( L"Payment dates for a column of term year fractions, from a curve collection's as-of date." )
    .arg( L"CurveCollection", L"The curve collection" )
    .arg( L"Terms",           L"Column of term year fractions" );


XLO_FUNC_START( aqCurveDatesToTerms(
    const ExcelObj& curveCollection,
    const ExcelObj& paymentDates ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelDoubleColumn( validation::tryAqCurveDatesToTerms(
        toAQLString( curveCollection ), toDateVector( paymentDates, true, "PaymentDates" ) ) ) );
}
XLO_FUNC_END( aqCurveDatesToTerms )
    .help( L"Term year fractions for a column of payment dates, from a curve collection's as-of date." )
    .arg( L"CurveCollection", L"The curve collection" )
    .arg( L"PaymentDates",    L"Column of payment dates" );


/* -------------------------------------------------------------------------
 *  Legacy stateless forward rates (curveCollection+curveIndex)
 * ---------------------------------------------------------------------- */

XLO_FUNC_START( aqCurveForwardRatesFromYearFraction(
    const ExcelObj& fromDates,
    const ExcelObj& yearFraction,
    const ExcelObj& dayCount,
    const ExcelObj& curveCollection,
    const ExcelObj& curveIndex,
    const ExcelObj& fwdInter,
    const ExcelObj& businessDayAdjust ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelDoubleColumn( validation::tryAqCurveForwardRatesFromYearFraction(
        toDateVector( fromDates, true, "FromDates" ), yearFraction.get<double>(),
        toAQLString( dayCount ), toAQLString( curveCollection ), toAQLString( curveIndex ),
        toStrOr( fwdInter, "" ), toStrOr( businessDayAdjust, "" ) ) ) );
}
XLO_FUNC_END( aqCurveForwardRatesFromYearFraction )
    .help( L"Forward rates from a curve collection/index, from-dates plus a common forward year fraction." )
    .arg( L"FromDates",         L"Column of forward-start dates" )
    .arg( L"YearFraction",      L"Forward period length, in years, common to every date" )
    .arg( L"DayCount",          L"Day count convention" )
    .arg( L"CurveCollection",   L"The curve collection" )
    .arg( L"CurveIndex",        L"The curve index" )
    .arg( L"FwdInter",          L"Optional. Forward-interpolation override; default uses the curve's own setting" )
    .arg( L"BusinessDayAdjust", L"Optional. Default MODFOLLOWING" );


XLO_FUNC_START( aqCurveForwardRatesFromForwardDates(
    const ExcelObj& fromDates,
    const ExcelObj& toDates,
    const ExcelObj& curveCollection,
    const ExcelObj& curveIndex,
    const ExcelObj& fwdInter,
    const ExcelObj& businessDayAdjust ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelDoubleColumn( validation::tryAqCurveForwardRatesFromForwardDates(
        toDateVector( fromDates, true, "FromDates" ), toDateVector( toDates, true, "ToDates" ),
        toAQLString( curveCollection ), toAQLString( curveIndex ),
        toStrOr( fwdInter, "" ), toStrOr( businessDayAdjust, "" ) ) ) );
}
XLO_FUNC_END( aqCurveForwardRatesFromForwardDates )
    .help( L"Forward rates from a curve collection/index between paired (from, to) dates." )
    .arg( L"FromDates",         L"Column of forward-start dates" )
    .arg( L"ToDates",           L"Column of forward-end dates, aligned with FromDates" )
    .arg( L"CurveCollection",   L"The curve collection" )
    .arg( L"CurveIndex",        L"The curve index" )
    .arg( L"FwdInter",          L"Optional. Forward-interpolation override; default uses the curve's own setting" )
    .arg( L"BusinessDayAdjust", L"Optional. Default MODFOLLOWING" );


// As aqCurveForwardRatesFromForwardDates; a separate validation wrapper
// distinguished by taking a curve collection OR a curve handle.
XLO_FUNC_START( aqCurveForwardRatesFromForwardDatesFromObject(
    const ExcelObj& fromDates,
    const ExcelObj& toDates,
    const ExcelObj& curveCollectionOrHandle,
    const ExcelObj& curveIndex,
    const ExcelObj& fwdInter,
    const ExcelObj& businessDayAdjust ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelDoubleColumn( validation::tryAqCurveForwardRatesFromForwardDatesFromObject(
        toDateVector( fromDates, true, "FromDates" ), toDateVector( toDates, true, "ToDates" ),
        toAQLString( curveCollectionOrHandle ), toAQLString( curveIndex ),
        toStrOr( fwdInter, "" ), toStrOr( businessDayAdjust, "" ) ) ) );
}
XLO_FUNC_END( aqCurveForwardRatesFromForwardDatesFromObject )
    .help( L"Forward rates between paired (from, to) dates, from either a curve collection or a curve handle." )
    .arg( L"FromDates",               L"Column of forward-start dates" )
    .arg( L"ToDates",                 L"Column of forward-end dates, aligned with FromDates" )
    .arg( L"CurveCollectionOrHandle", L"The curve collection (or a curve handle)" )
    .arg( L"CurveIndex",              L"The curve index" )
    .arg( L"FwdInter",                L"Optional. Forward-interpolation override; default uses the curve's own setting" )
    .arg( L"BusinessDayAdjust",       L"Optional. Default MODFOLLOWING" );


XLO_FUNC_START( aqCurveForwardRates(
    const ExcelObj& fixingDates,
    const ExcelObj& curveCollectionOrHandle,
    const ExcelObj& curveIndex,
    const ExcelObj& fwdInter,
    const ExcelObj& businessDayAdjust ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelDoubleColumn( validation::tryAqCurveForwardRates(
        toDateVector( fixingDates, true, "FixingDates" ),
        toAQLString( curveCollectionOrHandle ), toAQLString( curveIndex ),
        toStrOr( fwdInter, "" ), toStrOr( businessDayAdjust, "" ) ) ) );
}
XLO_FUNC_END( aqCurveForwardRates )
    .help( L"Forward rates at a column of fixing dates (the curve's own tenor convention), from a curve collection or handle." )
    .arg( L"FixingDates",             L"Column of fixing dates" )
    .arg( L"CurveCollectionOrHandle", L"The curve collection (or a curve handle)" )
    .arg( L"CurveIndex",              L"The curve index" )
    .arg( L"FwdInter",                L"Optional. Forward-interpolation override; default uses the curve's own setting" )
    .arg( L"BusinessDayAdjust",       L"Optional. Default MODFOLLOWING" );


// Forward rate table across multiple curve indices in one curve collection.
XLO_FUNC_START( aqCurveObjectForwardRatesTable(
    const ExcelObj& curveCollection,
    const ExcelObj& curveIndices,
    const ExcelObj& startDate,
    const ExcelObj& maturity,
    const ExcelObj& businessDayAdjust,
    const ExcelObj& calendar,
    const ExcelObj& rollConvention,
    const ExcelObj& frequency,
    const ExcelObj& fwdInterps ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const AQLStringVector indices = toAQLStringVector( curveIndices );

    DateVector fixingDates;
    DoubleMatrix forwardRates;

    validation::tryAqCurveObjectForwardRatesTable(
        fixingDates, forwardRates,
        toAQLString( curveCollection ), indices,
        toAQLString( startDate ), toAQLString( maturity ),
        toAQLString( businessDayAdjust ), toAQLString( calendar ),
        toAQLString( rollConvention ), toAQLString( frequency ),
        fwdInterps.isMissing() || !fwdInterps.isNonEmpty() ? AQLStringVector() : toAQLStringVector( fwdInterps ) );

    etrading::VariantMatrix result;
    etrading::VariantVector header;
    header.push_back( etrading::Variant( "Date" ) );
    for ( const AQLString& index : indices )
    {
        header.push_back( etrading::Variant( index.getCString() ) );
    }
    result.push_back( header );

    for ( std::size_t i = 0; i < fixingDates.size(); ++i )
    {
        etrading::VariantVector row;
        row.push_back( etrading::Variant( toExcelDate( fixingDates[i] ) ) );
        if ( i < forwardRates.size() )
        {
            for ( double fwd : forwardRates[i] )
            {
                row.push_back( etrading::Variant( fwd ) );
            }
        }
        result.push_back( row );
    }

    return returnValue( toExcelMatrix( result ) );
}
XLO_FUNC_END( aqCurveObjectForwardRatesTable )
    .help( L"Forward rate table across one or more curve indices in a curve collection. "
           L"First column is the fixing date, one further column per curve index." )
    .arg( L"CurveCollection",   L"The curve collection" )
    .arg( L"CurveIndices",      L"Column of curve indices to include" )
    .arg( L"StartDate",         L"Table start date" )
    .arg( L"Maturity",          L"Table end tenor/date" )
    .arg( L"BusinessDayAdjust", L"Business day adjustment" )
    .arg( L"Calendar",          L"Holiday centre(s)" )
    .arg( L"RollConvention",    L"Roll convention, e.g. Normal, IMM, EOM" )
    .arg( L"Frequency",         L"Table row frequency, e.g. 3M, 6M" )
    .arg( L"FwdInterps",        L"Optional. Column of per-index forward-interpolation overrides" );


/* -------------------------------------------------------------------------
 *  Short-rate model checks (Hull-White, Vasicek)
 * ---------------------------------------------------------------------- */

XLO_FUNC_START( aqCurveHullWhiteForwardRates(
    const ExcelObj& fixingDates,
    const ExcelObj& curveCollection,
    const ExcelObj& curveIndex,
    const ExcelObj& sigma,
    const ExcelObj& alpha,
    const ExcelObj& rt,
    const ExcelObj& valuationDate ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelDoubleColumn( validation::tryAqCurveHullWhiteForwardRates(
        toDateVector( fixingDates, true, "FixingDates" ), toNarrowString( curveCollection ), toNarrowString( curveIndex ),
        sigma.get<double>(), toDoubleOr( alpha, 0.03 ),
        toDoubleOr( rt, std::numeric_limits<double>::quiet_NaN() ), toAQLDateOr( valuationDate ) ) ) );
}
XLO_FUNC_END( aqCurveHullWhiteForwardRates )
    .help( L"Forward rates implied by a Hull-White 1F short-rate model, for comparison against the curve's own forwards." )
    .arg( L"FixingDates",    L"Column of fixing dates" )
    .arg( L"CurveCollection", L"The curve collection" )
    .arg( L"CurveIndex",     L"The curve index" )
    .arg( L"Sigma",          L"The Hull-White 1F volatility parameter" )
    .arg( L"Alpha",          L"Optional. Default 0.03. The Hull-White 1F mean reversion speed parameter" )
    .arg( L"Rt",             L"Optional. Short rate at time t; default derives it from the curve" )
    .arg( L"ValuationDate",  L"Optional. Default the curve's as-of date" );


XLO_FUNC_START( aqCurveVasicekChecking(
    const ExcelObj& fixingDates,
    const ExcelObj& targetForwardRates,
    const ExcelObj& curveCollection,
    const ExcelObj& curveIndex,
    const ExcelObj& initialTheta,
    const ExcelObj& initialSigma,
    const ExcelObj& alpha,
    const ExcelObj& rt,
    const ExcelObj& valuationDate,
    const ExcelObj& showColumnHeaders ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const AnyTypeMatrix result = validation::tryAqCurveVasicekChecking(
        toDateVector( fixingDates, true, "FixingDates" ), toDoubleVector( targetForwardRates, true, "TargetForwardRates" ),
        toNarrowString( curveCollection ), toNarrowString( curveIndex ),
        initialTheta.get<double>(), initialSigma.get<double>(), toDoubleOr( alpha, 0.03 ),
        toDoubleOr( rt, std::numeric_limits<double>::quiet_NaN() ), toAQLDateOr( valuationDate ),
        toBool( showColumnHeaders, true ) );

    return returnValue( toExcelMatrix( result ) );
}
XLO_FUNC_END( aqCurveVasicekChecking )
    .help( L"Fits a Vasicek 1F model's initial volatility to target forward rates; a near-zero result confirms the fit." )
    .arg( L"FixingDates",         L"Column of fixing dates" )
    .arg( L"TargetForwardRates",  L"Column of target forward rates to fit to, aligned with FixingDates" )
    .arg( L"CurveCollection",     L"The curve collection" )
    .arg( L"CurveIndex",          L"The curve index" )
    .arg( L"InitialTheta",        L"The Vasicek 1F theta parameter" )
    .arg( L"InitialSigma",        L"The Vasicek 1F volatility parameter" )
    .arg( L"Alpha",               L"Optional. Default 0.03. The Vasicek 1F mean reversion speed parameter" )
    .arg( L"Rt",                  L"Optional. Short rate at time t; default derives it from the curve" )
    .arg( L"ValuationDate",       L"Optional. Default the curve's as-of date" )
    .arg( L"ShowColumnHeaders",   L"Optional. Default TRUE" );


XLO_FUNC_START( aqCurveVasicekForwardRates(
    const ExcelObj& fixingDates,
    const ExcelObj& curveCollection,
    const ExcelObj& curveIndex,
    const ExcelObj& theta,
    const ExcelObj& sigma,
    const ExcelObj& alpha,
    const ExcelObj& rt,
    const ExcelObj& valuationDate ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelDoubleColumn( validation::tryAqCurveVasicekForwardRates(
        toDateVector( fixingDates, true, "FixingDates" ), toNarrowString( curveCollection ), toNarrowString( curveIndex ),
        theta.get<double>(), sigma.get<double>(), toDoubleOr( alpha, 0.03 ),
        toDoubleOr( rt, std::numeric_limits<double>::quiet_NaN() ), toAQLDateOr( valuationDate ) ) ) );
}
XLO_FUNC_END( aqCurveVasicekForwardRates )
    .help( L"Forward rates implied by a Vasicek 1F short-rate model, for comparison against the curve's own forwards." )
    .arg( L"FixingDates",    L"Column of fixing dates" )
    .arg( L"CurveCollection", L"The curve collection" )
    .arg( L"CurveIndex",     L"The curve index" )
    .arg( L"Theta",          L"The Vasicek 1F theta parameter" )
    .arg( L"Sigma",          L"The Vasicek 1F volatility parameter" )
    .arg( L"Alpha",          L"Optional. Default 0.03. The Vasicek 1F mean reversion speed parameter" )
    .arg( L"Rt",             L"Optional. Short rate at time t; default derives it from the curve" )
    .arg( L"ValuationDate",  L"Optional. Default the curve's as-of date" );


/* -------------------------------------------------------------------------
 *  Market-data LVB helper ("ObjectData") - a convenience builder that turns
 *  raw swap/FRA/futures/central-bank/FX static data straight into the
 *  named data blocks the CurveMarketData family above consumes.
 * ---------------------------------------------------------------------- */

XLO_FUNC_START( aqCurveObjectDataCreate(
    const ExcelObj& mdcName,
    const ExcelObj& currency,
    const ExcelObj& tenorString,
    const ExcelObj& swapType,
    const ExcelObj& swapStringBlock,
    const ExcelObj& toTenorString,
    const ExcelObj& fraStringBlock,
    const ExcelObj& irFuturesStringBlock,
    const ExcelObj& centralBankTypeString,
    const ExcelObj& centralBankStringBlock,
    const ExcelObj& fxStringBlock,
    const ExcelObj& unitCurrency,
    const ExcelObj& isInvertedFX ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const std::string name = decorateWithExcelLocation( toNarrowString( mdcName ) );

    const std::string storedName = validation::tryAqCurveObjectDataCreate(
        name, toNarrowString( currency ), toNarrowString( tenorString ),
        toStrOr( swapType, "" ), toAQLStringMatrixOr( swapStringBlock ),
        toStrOr( toTenorString, "" ), toAQLStringMatrixOr( fraStringBlock ),
        toAQLStringMatrixOr( irFuturesStringBlock ),
        toStrOr( centralBankTypeString, "" ), toAQLStringMatrixOr( centralBankStringBlock ),
        toAQLStringMatrixOr( fxStringBlock ), toStrOr( unitCurrency, "" ),
        toBool( isInvertedFX, false ) );

    return returnValue( appendInstanceCounter( storedName ) );
}
XLO_FUNC_END( aqCurveObjectDataCreate )
    .help( L"Build curve market data from raw swap/FRA/futures/central-bank/FX static data blocks; returns its handle." )
    .arg( L"MdcName",                L"Name for the market-data object" )
    .arg( L"Currency",               L"Currency" )
    .arg( L"TenorString",            L"Swap tenors to include" )
    .arg( L"SwapType",               L"Optional. Swap instrument type" )
    .arg( L"SwapStringBlock",        L"Optional. Swap rates block" )
    .arg( L"ToTenorString",          L"Optional. FRA/futures end tenors" )
    .arg( L"FraStringBlock",         L"Optional. FRA rates block" )
    .arg( L"IrFuturesStringBlock",   L"Optional. IR futures rates block" )
    .arg( L"CentralBankTypeString",  L"Optional. Central bank meeting type" )
    .arg( L"CentralBankStringBlock", L"Optional. Central bank meeting dates block" )
    .arg( L"FxStringBlock",          L"Optional. FX rates block" )
    .arg( L"UnitCurrency",           L"Optional. FX unit currency" )
    .arg( L"IsInvertedFX",           L"Optional. Default FALSE. FX quote is inverted" );


XLO_FUNC_START( aqCurveObjectDataDisplay(
    const ExcelObj& mdcName ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelMatrix( validation::tryAqCurveObjectDataDisplay( getNameWithoutCounter( mdcName ) ) ) );
}
XLO_FUNC_END( aqCurveObjectDataDisplay )
    .help( L"Display a curve market-data object built via aqCurveObjectDataCreate." )
    .arg( L"MdcName", L"A market-data object handle" );


/* -------------------------------------------------------------------------
 *  Dual bootstrap / engine calibration
 * ---------------------------------------------------------------------- */

XLO_FUNC_START( aqCurveObjectDualBootstrap(
    const ExcelObj& objectName,
    const ExcelObj& curveCollection,
    const ExcelObj& swapCurveGeneratorName,
    const ExcelObj& oisCurveGeneratorName,
    const ExcelObj& aqObjSwapMarketObj,
    const ExcelObj& aqObjOISMarketObj,
    const ExcelObj& commonParams ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const std::string name = decorateWithExcelLocation( toNarrowString( objectName ) );

    const std::map<std::string, std::string> curveIndexes = validation::tryAqCurveObjectDualBootstrap(
        name, toNarrowString( curveCollection ),
        getNameWithoutCounter( swapCurveGeneratorName ), getNameWithoutCounter( oisCurveGeneratorName ),
        getNameWithoutCounter( aqObjSwapMarketObj ), getNameWithoutCounter( aqObjOISMarketObj ),
        toAQLStringMatrixOr( commonParams ) );

    AQLStringMatrix result;
    for ( const auto& entry : curveIndexes )
    {
        AQLStringVector row;
        row.push_back( AQLString( entry.first.c_str() ) );
        row.push_back( AQLString( entry.second.c_str() ) );
        result.push_back( row );
    }

    return returnValue( toExcelMatrix( result ) );
}
XLO_FUNC_END( aqCurveObjectDualBootstrap )
    .help( L"Dual-bootstrap OIS and swap curves together; returns the resulting curve index names as a key/value block." )
    .arg( L"ObjectName",              L"Name for the dual-bootstrapped object" )
    .arg( L"CurveCollection",         L"Curve collection the calibrated curves belong to" )
    .arg( L"SwapCurveGeneratorName",  L"A curve-generator handle defining the swap curve's conventions" )
    .arg( L"OisCurveGeneratorName",   L"A curve-generator handle defining the OIS curve's conventions" )
    .arg( L"AqObjSwapMarketObj",      L"A curve-market-data handle for the swap curve" )
    .arg( L"AqObjOISMarketObj",       L"A curve-market-data handle for the OIS curve" )
    .arg( L"CommonParams",            L"Optional. Parameters common across both curves" );


XLO_FUNC_START( aqCurveObjectEngineCalibrate(
    const ExcelObj& engineObjectName,
    const ExcelObj& curveCollection,
    const ExcelObj& engineSettings,
    const ExcelObj& curveGeneratorNames,
    const ExcelObj& marketDataObjects ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const std::string name = decorateWithExcelLocation( toNarrowString( engineObjectName ) );

    const AQLStringVector curveIndexNames = validation::tryAqCurveObjectEngineCalibrate(
        name, toNarrowString( curveCollection ), toAQLStringMatrix( engineSettings ),
        getNamesWithoutCounter( curveGeneratorNames ), getNamesWithoutCounter( marketDataObjects ) );

    std::vector<std::string> result;
    result.reserve( curveIndexNames.size() );
    for ( const AQLString& indexName : curveIndexNames )
    {
        result.push_back( indexName.getCString() );
    }

    return returnValue( toExcelColumn( result ) );
}
XLO_FUNC_END( aqCurveObjectEngineCalibrate )
    .help( L"Calibrate a global yield curve engine, producing several synchronous curves at once; returns their index names." )
    .arg( L"EngineObjectName",     L"Name for the curve engine object" )
    .arg( L"CurveCollection",      L"Curve collection the calibrated curves belong to" )
    .arg( L"EngineSettings",       L"Engine-level parameters, as a label/value block" )
    .arg( L"CurveGeneratorNames",  L"Column of curve-generator handles, one per curve" )
    .arg( L"MarketDataObjects",    L"Column of curve-market-data handles, aligned with CurveGeneratorNames" );


// Display the yield curve engine's full Jacobian matrix (multi-curve).
XLO_FUNC_START( aqCurveObjectEngineJacobianDisplay(
    const ExcelObj& curveEngineObject,
    const ExcelObj& curveCollection,
    const ExcelObj& displayLabels,
    const ExcelObj& displayInverseMatrix ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    DoubleMatrix    matrix;
    AQLStringMatrix labelMatrix;

    validation::tryAqCurveObjectEngineJacobianDisplay(
        matrix, labelMatrix, getNameWithoutCounter( curveEngineObject ), toNarrowString( curveCollection ),
        toBool( displayLabels, true ), toBool( displayInverseMatrix, false ) );

    if ( toBool( displayLabels, true ) )
    {
        return returnValue( toExcelMatrix( labelMatrix ) );
    }
    return returnValue( toExcelDoubleMatrix( matrix ) );
}
XLO_FUNC_END( aqCurveObjectEngineJacobianDisplay )
    .help( L"Display the multi-curve yield curve engine's Jacobian matrix (or its row/column labels)." )
    .arg( L"CurveEngineObject",      L"A curve-engine handle (from aqCurveObjectEngineCalibrate)" )
    .arg( L"CurveCollection",        L"The curve collection" )
    .arg( L"DisplayLabels",          L"Optional. Default TRUE. Display the label matrix instead of the values" )
    .arg( L"DisplayInverseMatrix",   L"Optional. Default FALSE. Display the inverse Jacobian" );


// Display one yield curve's Jacobian matrix.
XLO_FUNC_START( aqCurveObjectJacobianDisplay(
    const ExcelObj& curveCollection,
    const ExcelObj& curveName,
    const ExcelObj& displayInverseMatrix ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    DoubleMatrix matrix;
    validation::tryAqCurveObjectJacobianDisplay(
        matrix, toNarrowString( curveCollection ), toNarrowString( curveName ), toBool( displayInverseMatrix, false ) );

    return returnValue( toExcelDoubleMatrix( matrix ) );
}
XLO_FUNC_END( aqCurveObjectJacobianDisplay )
    .help( L"Display one yield curve's Jacobian matrix." )
    .arg( L"CurveCollection",      L"The curve collection" )
    .arg( L"CurveName",            L"The curve index/name" )
    .arg( L"DisplayInverseMatrix", L"Optional. Default FALSE. Display the inverse Jacobian" );


/* -------------------------------------------------------------------------
 *  Curve results / Jacobian risk store
 * ---------------------------------------------------------------------- */

XLO_FUNC_START( aqCurveResultsEnable(
    const ExcelObj& enable ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqCurveResultsEnable( toBool( enable, true ) ) );
}
XLO_FUNC_END( aqCurveResultsEnable )
    .help( L"Enable or disable the curve results (Jacobian risk) store. Returns a status string." )
    .arg( L"Enable", L"TRUE to enable, FALSE to disable" );


XLO_FUNC_START( aqCurveResultsIsEnabled() )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqCurveResultsIsEnabled() );
}
XLO_FUNC_END( aqCurveResultsIsEnabled )
    .help( L"Whether the curve results (Jacobian risk) store is currently enabled." );


XLO_FUNC_START( aqCurveResultsDiscountFactorsUpdate(
    const ExcelObj& curveLVB,
    const ExcelObj& parameterLVB,
    const ExcelObj& discountFactorLVB,
    const ExcelObj& forwardAdjustments ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqCurveResultsDiscountFactorsUpdate(
        toAQLStringMatrix( curveLVB ), toAQLStringMatrix( parameterLVB ), toAQLStringMatrix( discountFactorLVB ),
        forwardAdjustments.isMissing() || !forwardAdjustments.isNonEmpty() ? StandardStringMatrix() : toStandardStringMatrix( forwardAdjustments ) ) );
}
XLO_FUNC_END( aqCurveResultsDiscountFactorsUpdate )
    .help( L"Update stored discount factors for a curve results object. Returns a status string." )
    .arg( L"CurveLVB",             L"Curve identity, as a label/value block" )
    .arg( L"ParameterLVB",         L"Calibration parameters, as a label/value block" )
    .arg( L"DiscountFactorLVB",    L"Discount factors to store, as a label/value block" )
    .arg( L"ForwardAdjustments",   L"Optional. Forward-rate adjustments" );


XLO_FUNC_START( aqCurveResultsDiscountFactorsDisplayAll(
    const ExcelObj& curveCollection,
    const ExcelObj& curveIndex ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelMatrix( validation::tryAqCurveResultsDiscountFactorsDisplayAll(
        toNarrowString( curveCollection ), toNarrowString( curveIndex ) ) ) );
}
XLO_FUNC_END( aqCurveResultsDiscountFactorsDisplayAll )
    .help( L"Display every discount factor stored in a curve results object." )
    .arg( L"CurveCollection", L"The curve collection" )
    .arg( L"CurveIndex",      L"The curve index" );


XLO_FUNC_START( aqCurveResultsDiscountFactorsDisplay(
    const ExcelObj& curveCollection,
    const ExcelObj& curveIndex,
    const ExcelObj& paymentDates ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelDoubleColumn( validation::tryAqCurveResultsDiscountFactorsDisplay(
        toNarrowString( curveCollection ), toNarrowString( curveIndex ), toDateVector( paymentDates, true, "PaymentDates" ) ) ) );
}
XLO_FUNC_END( aqCurveResultsDiscountFactorsDisplay )
    .help( L"Discount factors from a curve results object at a column of payment dates." )
    .arg( L"CurveCollection", L"The curve collection" )
    .arg( L"CurveIndex",      L"The curve index" )
    .arg( L"PaymentDates",    L"Column of payment dates" );


XLO_FUNC_START( aqCurveResultsDelete(
    const ExcelObj& curveCollection,
    const ExcelObj& curveIndex ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqCurveResultsDelete( toNarrowString( curveCollection ), toNarrowString( curveIndex ) ) );
}
XLO_FUNC_END( aqCurveResultsDelete )
    .help( L"Delete one curve results object. Returns a status string." )
    .arg( L"CurveCollection", L"The curve collection" )
    .arg( L"CurveIndex",      L"The curve index" );


XLO_FUNC_START( aqCurveResultsDeleteAll() )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqCurveResultsDeleteAll() );
}
XLO_FUNC_END( aqCurveResultsDeleteAll )
    .help( L"Delete every curve results object. Returns a status string." );


XLO_FUNC_START( aqCurveResultsForwardRatesDisplay(
    const ExcelObj& curveCollection,
    const ExcelObj& curveIndex,
    const ExcelObj& fixingDates,
    const ExcelObj& isFwdInter,
    const ExcelObj& fixingBusinessDayAdj,
    const ExcelObj& fixingCalendar ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelDoubleColumn( validation::tryAqCurveResultsForwardRatesDisplay(
        toNarrowString( curveCollection ), toNarrowString( curveIndex ), toDateVector( fixingDates, true, "FixingDates" ),
        toBool( isFwdInter, false ),
        etrading::toBusinessDayAdjustmentEnum( toStrOr( fixingBusinessDayAdj, "NONE" ) ),
        toStrOr( fixingCalendar, "" ) ) ) );
}
XLO_FUNC_END( aqCurveResultsForwardRatesDisplay )
    .help( L"Forward rates from a curve results object at a column of fixing dates." )
    .arg( L"CurveCollection",        L"The curve collection" )
    .arg( L"CurveIndex",             L"The curve index" )
    .arg( L"FixingDates",            L"Column of fixing dates" )
    .arg( L"IsFwdInter",             L"Optional. Default FALSE" )
    .arg( L"FixingBusinessDayAdj",   L"Optional. Default NONE. Business day adjustment for the fixing dates" )
    .arg( L"FixingCalendar",         L"Optional. Holiday centre(s) for the fixing dates" );


XLO_FUNC_START( aqCurveResultsJacobianUpdate(
    const ExcelObj& curveLVB,
    const ExcelObj& discountFactorParameterLVB,
    const ExcelObj& discountFactors,
    const ExcelObj& jacobianParameterLVB,
    const ExcelObj& outrightInstruments,
    const ExcelObj& marketDataShiftSizeInPercent,
    const ExcelObj& jacobianMatrix ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqCurveResultsJacobianUpdate(
        toAQLStringMatrix( curveLVB ), toAQLStringMatrix( discountFactorParameterLVB ), toAQLStringMatrix( discountFactors ),
        toAQLStringMatrix( jacobianParameterLVB ), toBoolVector( outrightInstruments ),
        toDoubleVector( marketDataShiftSizeInPercent, true, "MarketDataShiftSizeInPercent" ),
        toAQLStringMatrix( jacobianMatrix ) ) );
}
XLO_FUNC_END( aqCurveResultsJacobianUpdate )
    .help( L"Update the stored Jacobian for a curve results object. Returns a status string." )
    .arg( L"CurveLVB",                       L"Curve identity, as a label/value block" )
    .arg( L"DiscountFactorParameterLVB",     L"Discount factor parameters, as a label/value block" )
    .arg( L"DiscountFactors",                L"Discount factors, as a label/value block" )
    .arg( L"JacobianParameterLVB",           L"Jacobian parameters, as a label/value block" )
    .arg( L"OutrightInstruments",            L"Column of TRUE/FALSE flags, one per calibration instrument" )
    .arg( L"MarketDataShiftSizeInPercent",   L"Column of market-data shift sizes used to build the Jacobian" )
    .arg( L"JacobianMatrix",                 L"The Jacobian matrix values" );


XLO_FUNC_START( aqCurveResultsJacobianDisplay(
    const ExcelObj& curveCollection,
    const ExcelObj& curveIndex,
    const ExcelObj& riskType ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelMatrix( validation::tryAqCurveResultsJacobianDisplay(
        toNarrowString( curveCollection ), toNarrowString( curveIndex ),
        etrading::toRiskTypeEnum( toNarrowString( riskType ) ) ) ) );
}
XLO_FUNC_END( aqCurveResultsJacobianDisplay )
    .help( L"Display the Jacobian matrix stored for a curve results object." )
    .arg( L"CurveCollection", L"The curve collection" )
    .arg( L"CurveIndex",      L"The curve index" )
    .arg( L"RiskType",        L"The risk type to display" );


XLO_FUNC_START( aqCurveResultsJacobianDiscountFactorDelta(
    const ExcelObj& curveCollection,
    const ExcelObj& curveIndex ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelMatrix( validation::tryAqCurveResultsJacobianDiscountFactorDelta(
        toNarrowString( curveCollection ), toNarrowString( curveIndex ) ) ) );
}
XLO_FUNC_END( aqCurveResultsJacobianDiscountFactorDelta )
    .help( L"Display the Jacobian discount-factor delta for a curve results object." )
    .arg( L"CurveCollection", L"The curve collection" )
    .arg( L"CurveIndex",      L"The curve index" );


XLO_FUNC_START( aqCurveResultsJacobianRiskTotals(
    const ExcelObj& curveCollection,
    const ExcelObj& curveIndex,
    const ExcelObj& riskType,
    const ExcelObj& useOutrightInstrumentsOnly ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelMatrix( validation::tryAqCurveResultsJacobianRiskTotals(
        toNarrowString( curveCollection ), toNarrowString( curveIndex ),
        etrading::toRiskTypeEnum( toNarrowString( riskType ) ), toBool( useOutrightInstrumentsOnly, true ) ) ) );
}
XLO_FUNC_END( aqCurveResultsJacobianRiskTotals )
    .help( L"Display Jacobian risk totals for a curve results object." )
    .arg( L"CurveCollection",             L"The curve collection" )
    .arg( L"CurveIndex",                  L"The curve index" )
    .arg( L"RiskType",                    L"The risk type to total" )
    .arg( L"UseOutrightInstrumentsOnly",  L"Optional. Default TRUE" );


XLO_FUNC_START( aqCurveResultsJacobianImplyNewDiscountFactors(
    const ExcelObj& curveCollection,
    const ExcelObj& curveIndex ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelMatrix( validation::tryAqCurveResultsJacobianImplyNewDiscountFactors(
        toNarrowString( curveCollection ), toNarrowString( curveIndex ) ) ) );
}
XLO_FUNC_END( aqCurveResultsJacobianImplyNewDiscountFactors )
    .help( L"Discount factors implied by the Jacobian after a market-data shift, for a curve results object." )
    .arg( L"CurveCollection", L"The curve collection" )
    .arg( L"CurveIndex",      L"The curve index" );


/* -------------------------------------------------------------------------
 *  Curve groups
 * ---------------------------------------------------------------------- */

XLO_FUNC_START( aqCurveGroupCreate(
    const ExcelObj& groupName,
    const ExcelObj& curveHandles ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqCurveGroupCreate( toNarrowString( groupName ), toStringVector( curveHandles, true ) ) );
}
XLO_FUNC_END( aqCurveGroupCreate )
    .help( L"Create a named group of curve handles. Returns a status string." )
    .arg( L"GroupName",    L"Name for the curve group" )
    .arg( L"CurveHandles", L"Column of curve handles to include" );


XLO_FUNC_START( aqCurveGroupCollectionName(
    const ExcelObj& groupName ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqCurveGroupCollectionName( toNarrowString( groupName ) ) );
}
XLO_FUNC_END( aqCurveGroupCollectionName )
    .help( L"The curve collection name for a given curve group." )
    .arg( L"GroupName", L"A curve group name" );


/* -------------------------------------------------------------------------
 *  Stateless dual bootstrap (one-shot: builds OIS and swap curves together
 *  from raw conventions/rates, no separate CurveGenerator/CurveMarketData
 *  step - see the file header for the "both families get ported" decision).
 * ---------------------------------------------------------------------- */

XLO_FUNC_START( aqCurveDualBootstrap(
    const ExcelObj& curveCollection,
    const ExcelObj& curveNameDb,
    const ExcelObj& curveNameOIS,
    const ExcelObj& curveNameSwap,
    const ExcelObj& curveIndexOIS,
    const ExcelObj& curveIndexSwap,
    const ExcelObj& commonParams,
    const ExcelObj& generatePropOIS,
    const ExcelObj& oisRatesOIS,
    const ExcelObj& oisConvOIS,
    const ExcelObj& histRatesOIS,
    const ExcelObj& lobasisRatesOIS,
    const ExcelObj& lobasisConvOIS,
    const ExcelObj& swapConvOIS,
    const ExcelObj& generatePropSwap,
    const ExcelObj& moneyConvSwap,
    const ExcelObj& liborRatesSwap,
    const ExcelObj& liborConvSwap,
    const ExcelObj& swapRatesSwap,
    const ExcelObj& swapConvSwap,
    const ExcelObj& fra3mRatesSwap,
    const ExcelObj& fra6mRatesSwap,
    const ExcelObj& fraConvSwap,
    const ExcelObj& futureRatesSwap,
    const ExcelObj& futureConvSwap,
    const ExcelObj& tenorBasisConvSwap,
    const ExcelObj& tenorBasisRatesSwap ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( std::string( validation::tryAqCurveDualBootstrap(
        toAQLString( curveCollection ), toAQLString( curveNameDb ), toAQLString( curveNameOIS ), toAQLString( curveNameSwap ),
        toAQLString( curveIndexOIS ), toAQLString( curveIndexSwap ),
        toAQLStringMatrix( commonParams ), toAQLStringMatrix( generatePropOIS ),
        toAQLStringMatrix( oisRatesOIS ), toAQLStringMatrix( oisConvOIS ), toAQLStringMatrix( histRatesOIS ),
        toAQLStringMatrix( lobasisRatesOIS ), toAQLStringMatrix( lobasisConvOIS ), toAQLStringMatrix( swapConvOIS ),
        toAQLStringMatrix( generatePropSwap ), toAQLStringMatrix( moneyConvSwap ),
        toAQLStringMatrix( liborRatesSwap ), toAQLStringMatrix( liborConvSwap ),
        toAQLStringMatrix( swapRatesSwap ), toAQLStringMatrix( swapConvSwap ),
        toAQLStringMatrix( fra3mRatesSwap ), toAQLStringMatrix( fra6mRatesSwap ), toAQLStringMatrix( fraConvSwap ),
        toAQLStringMatrix( futureRatesSwap ), toAQLStringMatrix( futureConvSwap ),
        toAQLStringMatrix( tenorBasisConvSwap ), toAQLStringMatrix( tenorBasisRatesSwap ) ).getCString() ) );
}
XLO_FUNC_END( aqCurveDualBootstrap )
    .help( L"One-shot dual bootstrap of an OIS curve and a swap curve from raw conventions and rates. Returns a status string." )
    .arg( L"CurveCollection",     L"The curve collection ID" )
    .arg( L"CurveNameDb",         L"Name of the dual-bootstrapping curve object" )
    .arg( L"CurveNameOIS",        L"Name of the OIS curve" )
    .arg( L"CurveNameSwap",       L"Name of the swap curve" )
    .arg( L"CurveIndexOIS",       L"Curve index of the OIS curve" )
    .arg( L"CurveIndexSwap",      L"Curve index of the swap curve" )
    .arg( L"CommonParams",        L"Parameters common across all curves" )
    .arg( L"GeneratePropOIS",     L"Curve build properties of the OIS curve" )
    .arg( L"OisRatesOIS",         L"OIS rates" )
    .arg( L"OisConvOIS",          L"OIS conventions" )
    .arg( L"HistRatesOIS",        L"Historical OIS rates" )
    .arg( L"LobasisRatesOIS",     L"Libor-OIS basis rates" )
    .arg( L"LobasisConvOIS",      L"Libor-OIS basis conventions" )
    .arg( L"SwapConvOIS",         L"Swap conventions used on the OIS side" )
    .arg( L"GeneratePropSwap",    L"Curve build properties of the swap curve" )
    .arg( L"MoneyConvSwap",       L"Money market conventions" )
    .arg( L"LiborRatesSwap",      L"Libor rates" )
    .arg( L"LiborConvSwap",       L"Libor conventions" )
    .arg( L"SwapRatesSwap",       L"Swap rates in the swap curve" )
    .arg( L"SwapConvSwap",        L"Swap conventions in the swap curve" )
    .arg( L"Fra3mRatesSwap",      L"3M FRA rates" )
    .arg( L"Fra6mRatesSwap",      L"6M FRA rates" )
    .arg( L"FraConvSwap",         L"FRA conventions" )
    .arg( L"FutureRatesSwap",     L"Futures rates" )
    .arg( L"FutureConvSwap",      L"Futures conventions" )
    .arg( L"TenorBasisConvSwap",  L"Tenor basis conventions" )
    .arg( L"TenorBasisRatesSwap", L"Tenor basis market data" );


/* -------------------------------------------------------------------------
 *  Heavy one-shot curve calibration (stateless) and its handle-returning
 *  ObjectCreate twin - decided with Nicholas 2026-09-11: both are live,
 *  both get ported, verbatim golden names (no renaming).
 * ---------------------------------------------------------------------- */

XLO_FUNC_START( aqCurveCalibrateBasis(
    const ExcelObj& curveCollection,
    const ExcelObj& staticDataTable,
    const ExcelObj& curveIndex,
    const ExcelObj& curveConv,
    const ExcelObj& basisConv,
    const ExcelObj& basisRates,
    const ExcelObj& fxFwdConv,
    const ExcelObj& fxFwdRates,
    const ExcelObj& spotFxRates,
    const ExcelObj& fraConv,
    const ExcelObj& fraRates,
    const ExcelObj& liborConv,
    const ExcelObj& liborRates ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( std::string( validation::tryAqCurveCalibrateBasis(
        toAQLString( curveCollection ), toAQLString( staticDataTable ), toAQLString( curveIndex ),
        toAQLStringMatrix( curveConv ), toAQLStringMatrix( basisConv ), toAQLStringMatrix( basisRates ),
        toAQLStringMatrix( fxFwdConv ), toAQLStringMatrix( fxFwdRates ), toAQLStringMatrix( spotFxRates ),
        toAQLStringMatrixOr( fraConv ), toAQLStringMatrixOr( fraRates ),
        toAQLStringMatrixOr( liborConv ), toAQLStringMatrixOr( liborRates ) ).getCString() ) );
}
XLO_FUNC_END( aqCurveCalibrateBasis )
    .help( L"One-shot calibration of a cross-currency basis curve from raw conventions and rates. Returns a status string." )
    .arg( L"CurveCollection",  L"The curve collection ID" )
    .arg( L"StaticDataTable",  L"Name of the curve constructed by this method" )
    .arg( L"CurveIndex",       L"Equivalent names of the curve being built" )
    .arg( L"CurveConv",        L"General curve properties: as-of date, ccy, interpolation, etc" )
    .arg( L"BasisConv",        L"Basis swap conventions" )
    .arg( L"BasisRates",       L"Basis swap market rates" )
    .arg( L"FxFwdConv",        L"FX forward market conventions" )
    .arg( L"FxFwdRates",       L"Forward FX rates" )
    .arg( L"SpotFxRates",      L"Spot FX rates" )
    .arg( L"FraConv",          L"Optional. FRA conventions" )
    .arg( L"FraRates",         L"Optional. FRA market data" )
    .arg( L"LiborConv",        L"Optional. Libor instrument conventions" )
    .arg( L"LiborRates",       L"Optional. Libor market data" );


XLO_FUNC_START( aqCurveCalibrateCTD(
    const ExcelObj& curveCollection,
    const ExcelObj& curveName,
    const ExcelObj& curveIndex,
    const ExcelObj& curveConv,
    const ExcelObj& collateralCurves ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( std::string( validation::tryAqCurveCalibrateCTD(
        toAQLString( curveCollection ), toAQLString( curveName ), toAQLString( curveIndex ),
        toAQLStringMatrix( curveConv ), toAQLStringVector( collateralCurves ) ).getCString() ) );
}
XLO_FUNC_END( aqCurveCalibrateCTD )
    .help( L"One-shot calibration of a cheapest-to-deliver collateral curve. Returns a status string." )
    .arg( L"CurveCollection",   L"The curve collection ID" )
    .arg( L"CurveName",         L"Name of the curve constructed by this method" )
    .arg( L"CurveIndex",        L"Equivalent names of the curve being built" )
    .arg( L"CurveConv",         L"General curve properties: as-of date, ccy, interpolation, etc" )
    .arg( L"CollateralCurves",  L"The group of collateral curves the CTD curve is constructed from" );


XLO_FUNC_START( aqCurveCalibrateFXForwards(
    const ExcelObj& curveCollection,
    const ExcelObj& staticDataTable,
    const ExcelObj& curveIndex,
    const ExcelObj& curveConv,
    const ExcelObj& fxFwdConv ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( std::string( validation::tryAqCurveCalibrateFXForwards(
        toAQLString( curveCollection ), toAQLString( staticDataTable ), toAQLString( curveIndex ),
        toAQLStringMatrix( curveConv ), toAQLStringMatrix( fxFwdConv ) ).getCString() ) );
}
XLO_FUNC_END( aqCurveCalibrateFXForwards )
    .help( L"One-shot calibration of an FX forwards curve. Returns a status string." )
    .arg( L"CurveCollection",  L"The curve collection ID" )
    .arg( L"StaticDataTable",  L"Name of the curve constructed by this method" )
    .arg( L"CurveIndex",       L"Equivalent names of the curve being built" )
    .arg( L"CurveConv",        L"General curve properties: as-of date, ccy, interpolation, etc" )
    .arg( L"FxFwdConv",        L"FX forward conventions" );


XLO_FUNC_START( aqCurveCalibrateOIS(
    const ExcelObj& curveCollection,
    const ExcelObj& staticDataTable,
    const ExcelObj& curveIndex,
    const ExcelObj& curveConv,
    const ExcelObj& oisConv,
    const ExcelObj& oisRates,
    const ExcelObj& oisHistoricalRates,
    const ExcelObj& liborOisBasisConv,
    const ExcelObj& liborOisBasisRates,
    const ExcelObj& swapConv,
    const ExcelObj& swapRates ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( std::string( validation::tryAqCurveCalibrateOIS(
        toAQLString( curveCollection ), toAQLString( staticDataTable ), toAQLString( curveIndex ),
        toAQLStringMatrix( curveConv ), toAQLStringMatrix( oisConv ), toAQLStringMatrix( oisRates ),
        toAQLStringMatrix( oisHistoricalRates ), toAQLStringMatrix( liborOisBasisConv ), toAQLStringMatrix( liborOisBasisRates ),
        toAQLStringMatrix( swapConv ), toAQLStringMatrix( swapRates ) ).getCString() ) );
}
XLO_FUNC_END( aqCurveCalibrateOIS )
    .help( L"One-shot calibration of an OIS curve from raw conventions and rates. Returns a status string." )
    .arg( L"CurveCollection",       L"The curve collection ID" )
    .arg( L"StaticDataTable",       L"Name of the curve constructed by this method" )
    .arg( L"CurveIndex",            L"Equivalent names of the curve being built" )
    .arg( L"CurveConv",             L"General curve properties: as-of date, ccy, interpolation, etc" )
    .arg( L"OisConv",               L"The OIS curve configuration" )
    .arg( L"OisRates",              L"Constituent OIS instrument rates" )
    .arg( L"OisHistoricalRates",    L"Historical OIS fixings" )
    .arg( L"LiborOisBasisConv",     L"Libor-OIS swap conventions" )
    .arg( L"LiborOisBasisRates",    L"Libor-OIS basis spreads" )
    .arg( L"SwapConv",              L"Libor swap conventions" )
    .arg( L"SwapRates",             L"Libor swap market rates" );


XLO_FUNC_START( aqCurveCalibrateSwap(
    const ExcelObj& curveCollection,
    const ExcelObj& staticDataTable,
    const ExcelObj& curveIndex,
    const ExcelObj& curveConv,
    const ExcelObj& moneyMarketConv,
    const ExcelObj& liborConv,
    const ExcelObj& liborRates,
    const ExcelObj& swapConv,
    const ExcelObj& swapRates,
    const ExcelObj& fraConv,
    const ExcelObj& fra3mRates,
    const ExcelObj& fra6mRates,
    const ExcelObj& futureConv,
    const ExcelObj& futureRates,
    const ExcelObj& tenorBasisConv,
    const ExcelObj& tenorBasisRates ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( std::string( validation::tryAqCurveCalibrateSwap(
        toAQLString( curveCollection ), toAQLString( staticDataTable ), toAQLString( curveIndex ),
        toAQLStringMatrix( curveConv ), toAQLStringMatrix( moneyMarketConv ),
        toAQLStringMatrix( liborConv ), toAQLStringMatrix( liborRates ),
        toAQLStringMatrix( swapConv ), toAQLStringMatrix( swapRates ),
        toAQLStringMatrix( fraConv ), toAQLStringMatrix( fra3mRates ), toAQLStringMatrix( fra6mRates ),
        toAQLStringMatrix( futureConv ), toAQLStringMatrix( futureRates ),
        toAQLStringMatrix( tenorBasisConv ), toAQLStringMatrix( tenorBasisRates ) ).getCString() ) );
}
XLO_FUNC_END( aqCurveCalibrateSwap )
    .help( L"One-shot calibration of a Libor swap curve from raw conventions and rates. Returns a status string." )
    .arg( L"CurveCollection",   L"The curve collection ID" )
    .arg( L"StaticDataTable",   L"Name of the curve constructed by this method" )
    .arg( L"CurveIndex",        L"Equivalent names of the curve being built" )
    .arg( L"CurveConv",         L"General yield curve data" )
    .arg( L"MoneyMarketConv",   L"Money market conventions" )
    .arg( L"LiborConv",         L"Libor market conventions" )
    .arg( L"LiborRates",        L"Libor market data" )
    .arg( L"SwapConv",          L"Swap market conventions" )
    .arg( L"SwapRates",         L"Swap market data" )
    .arg( L"FraConv",           L"FRA market conventions" )
    .arg( L"Fra3mRates",        L"3M FRA market data" )
    .arg( L"Fra6mRates",        L"6M FRA market data" )
    .arg( L"FutureConv",        L"Futures market conventions" )
    .arg( L"FutureRates",       L"Futures market data" )
    .arg( L"TenorBasisConv",    L"Tenor basis market conventions" )
    .arg( L"TenorBasisRates",   L"Tenor basis market data" );


XLO_FUNC_START( aqCurveObjectCreateBasis(
    const ExcelObj& aqObjCurveName,
    const ExcelObj& curveCollection,
    const ExcelObj& staticDataTable,
    const ExcelObj& curveIndex,
    const ExcelObj& curveConv,
    const ExcelObj& basisConv,
    const ExcelObj& basisRates,
    const ExcelObj& fxFwdConv,
    const ExcelObj& fxFwdRates,
    const ExcelObj& spotFxRates ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const std::string name = decorateWithExcelLocation( toNarrowString( aqObjCurveName ) );

    const std::string storedName = validation::tryAqCurveObjectCreateBasis(
        name, toAQLString( curveCollection ), toAQLString( staticDataTable ), toAQLString( curveIndex ),
        toAQLStringMatrix( curveConv ), toAQLStringMatrix( basisConv ), toAQLStringMatrix( basisRates ),
        toAQLStringMatrix( fxFwdConv ), toAQLStringMatrix( fxFwdRates ), toAQLStringMatrix( spotFxRates ) ).getCString();

    return returnValue( appendInstanceCounter( storedName ) );
}
XLO_FUNC_END( aqCurveObjectCreateBasis )
    .help( L"One-shot creation of a cached cross-currency basis curve from raw conventions and rates; returns its handle." )
    .arg( L"AqObjCurveName",  L"Name for the curve object" )
    .arg( L"CurveCollection", L"The curve collection ID" )
    .arg( L"StaticDataTable", L"Name of the curve constructed by this method" )
    .arg( L"CurveIndex",      L"Equivalent names of the curve being built" )
    .arg( L"CurveConv",       L"General curve properties: as-of date, ccy, interpolation, etc" )
    .arg( L"BasisConv",       L"Basis swap conventions" )
    .arg( L"BasisRates",      L"Basis swap market rates" )
    .arg( L"FxFwdConv",       L"FX forward market conventions" )
    .arg( L"FxFwdRates",      L"Forward FX rates" )
    .arg( L"SpotFxRates",     L"Spot FX rates" );


XLO_FUNC_START( aqCurveObjectCreateFXForwards(
    const ExcelObj& aqObjCurveName,
    const ExcelObj& curveCollection,
    const ExcelObj& staticDataTable,
    const ExcelObj& curveIndex,
    const ExcelObj& curveConv,
    const ExcelObj& fxFwdConv ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const std::string name = decorateWithExcelLocation( toNarrowString( aqObjCurveName ) );

    const std::string storedName = validation::tryAqCurveObjectCreateFXForwards(
        name, toAQLString( curveCollection ), toAQLString( staticDataTable ), toAQLString( curveIndex ),
        toAQLStringMatrix( curveConv ), toAQLStringMatrix( fxFwdConv ) ).getCString();

    return returnValue( appendInstanceCounter( storedName ) );
}
XLO_FUNC_END( aqCurveObjectCreateFXForwards )
    .help( L"One-shot creation of a cached FX forwards curve; returns its handle." )
    .arg( L"AqObjCurveName",  L"Name for the curve object" )
    .arg( L"CurveCollection", L"The curve collection ID" )
    .arg( L"StaticDataTable", L"Name of the curve constructed by this method" )
    .arg( L"CurveIndex",      L"Equivalent names of the curve being built" )
    .arg( L"CurveConv",       L"General curve properties: as-of date, ccy, interpolation, etc" )
    .arg( L"FxFwdConv",       L"FX forward conventions" );


XLO_FUNC_START( aqCurveObjectCreateOIS(
    const ExcelObj& aqObjCurveName,
    const ExcelObj& curveCollection,
    const ExcelObj& staticDataTable,
    const ExcelObj& curveIndex,
    const ExcelObj& curveConv,
    const ExcelObj& oisConv,
    const ExcelObj& oisRates,
    const ExcelObj& oisHistoricalRates,
    const ExcelObj& liborOisBasisConv,
    const ExcelObj& liborOisBasisRates,
    const ExcelObj& swapConv,
    const ExcelObj& swapRates ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const std::string name = decorateWithExcelLocation( toNarrowString( aqObjCurveName ) );

    const std::string storedName = validation::tryAqCurveObjectCreateOIS(
        name, toAQLString( curveCollection ), toAQLString( staticDataTable ), toAQLString( curveIndex ),
        toAQLStringMatrix( curveConv ), toAQLStringMatrix( oisConv ), toAQLStringMatrix( oisRates ),
        toAQLStringMatrix( oisHistoricalRates ), toAQLStringMatrix( liborOisBasisConv ), toAQLStringMatrix( liborOisBasisRates ),
        toAQLStringMatrix( swapConv ), toAQLStringMatrix( swapRates ) ).getCString();

    return returnValue( appendInstanceCounter( storedName ) );
}
XLO_FUNC_END( aqCurveObjectCreateOIS )
    .help( L"One-shot creation of a cached OIS curve from raw conventions and rates; returns its handle." )
    .arg( L"AqObjCurveName",     L"Name for the curve object" )
    .arg( L"CurveCollection",    L"The curve collection ID" )
    .arg( L"StaticDataTable",    L"Name of the curve constructed by this method" )
    .arg( L"CurveIndex",         L"Equivalent names of the curve being built" )
    .arg( L"CurveConv",          L"General curve properties: as-of date, ccy, interpolation, etc" )
    .arg( L"OisConv",            L"The OIS curve configuration" )
    .arg( L"OisRates",           L"Constituent OIS instrument rates" )
    .arg( L"OisHistoricalRates", L"Historical OIS fixings" )
    .arg( L"LiborOisBasisConv",  L"Libor-OIS swap conventions" )
    .arg( L"LiborOisBasisRates", L"Libor-OIS basis spreads" )
    .arg( L"SwapConv",           L"Libor swap conventions" )
    .arg( L"SwapRates",          L"Libor swap market rates" );


XLO_FUNC_START( aqCurveObjectCreateSwap(
    const ExcelObj& aqObjCurveName,
    const ExcelObj& curveCollection,
    const ExcelObj& staticDataTable,
    const ExcelObj& curveIndex,
    const ExcelObj& curveConv,
    const ExcelObj& moneyMarketConv,
    const ExcelObj& liborConv,
    const ExcelObj& liborRates,
    const ExcelObj& swapConv,
    const ExcelObj& swapRates,
    const ExcelObj& fraConv,
    const ExcelObj& fra3mRates,
    const ExcelObj& fra6mRates,
    const ExcelObj& futureConv,
    const ExcelObj& futureRates,
    const ExcelObj& convexityAdjConv,
    const ExcelObj& convexityAdjRates ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const std::string name = decorateWithExcelLocation( toNarrowString( aqObjCurveName ) );

    const std::string storedName = validation::tryAqCurveObjectCreateSwap(
        name, toAQLString( curveCollection ), toAQLString( staticDataTable ), toAQLString( curveIndex ),
        toAQLStringMatrix( curveConv ), toAQLStringMatrix( moneyMarketConv ),
        toAQLStringMatrix( liborConv ), toAQLStringMatrix( liborRates ),
        toAQLStringMatrix( swapConv ), toAQLStringMatrix( swapRates ),
        toAQLStringMatrix( fraConv ), toAQLStringMatrix( fra3mRates ), toAQLStringMatrix( fra6mRates ),
        toAQLStringMatrix( futureConv ), toAQLStringMatrix( futureRates ),
        toAQLStringMatrix( convexityAdjConv ), toAQLStringMatrix( convexityAdjRates ) ).getCString();

    return returnValue( appendInstanceCounter( storedName ) );
}
XLO_FUNC_END( aqCurveObjectCreateSwap )
    .help( L"One-shot creation of a cached Libor swap curve from raw conventions and rates; returns its handle." )
    .arg( L"AqObjCurveName",   L"Name for the curve object" )
    .arg( L"CurveCollection",  L"The curve collection ID" )
    .arg( L"StaticDataTable",  L"Name of the curve constructed by this method" )
    .arg( L"CurveIndex",       L"Equivalent names of the curve being built" )
    .arg( L"CurveConv",        L"General yield curve data" )
    .arg( L"MoneyMarketConv",  L"Money market conventions" )
    .arg( L"LiborConv",        L"Libor market conventions" )
    .arg( L"LiborRates",       L"Libor market data" )
    .arg( L"SwapConv",         L"Swap market conventions" )
    .arg( L"SwapRates",        L"Swap market data" )
    .arg( L"FraConv",          L"FRA market conventions" )
    .arg( L"Fra3mRates",       L"3M FRA market data" )
    .arg( L"Fra6mRates",       L"6M FRA market data" )
    .arg( L"FutureConv",       L"Futures market conventions" )
    .arg( L"FutureRates",      L"Futures market data" )
    .arg( L"ConvexityAdjConv", L"Convexity adjustment market conventions" )
    .arg( L"ConvexityAdjRates", L"Convexity adjustment market data" );


// The USD money-market spot date from a curve as-of date (filed under Date's
// validation header, golden-named Curve - see the #include comment above).
XLO_FUNC_START( aqCurveUSDSpotDate(
    const ExcelObj& curveAsOfDate,
    const ExcelObj& spotLag,
    const ExcelObj& fixingCalendar,
    const ExcelObj& paymentCalendar,
    const ExcelObj& businessDayAdj,
    const ExcelObj& rollConvention ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelDate( validation::tryAqCurveUSDSpotDate(
        toAQLDate( curveAsOfDate ), toAQLString( spotLag ), toAQLString( fixingCalendar ),
        toAQLString( paymentCalendar ), toAQLString( businessDayAdj ), toAQLString( rollConvention ) ) ) );
}
XLO_FUNC_END( aqCurveUSDSpotDate )
    .help( L"The USD money-market spot date from a curve as-of date." )
    .arg( L"CurveAsOfDate",   L"The curve as-of date" )
    .arg( L"SpotLag",         L"Tenor added to the as-of date, e.g. 2D" )
    .arg( L"FixingCalendar",  L"Holiday centre(s) for the fixing" )
    .arg( L"PaymentCalendar", L"Holiday centre(s) for the payment" )
    .arg( L"BusinessDayAdj",  L"Business day adjustment, e.g. MODFOLLOWING" )
    .arg( L"RollConvention",  L"Roll convention, e.g. Normal, IMM, EOM" );
