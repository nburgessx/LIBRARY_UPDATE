/*
 * CapFloor category - xlOil worksheet functions.
 *
 * aqCapFloorObject* operates on a cached cap/floor (name in). Each function
 * pairs with the identically named validation wrapper (plus the `try`
 * prefix). Marshalling to and from Excel is the aq_xll helpers in
 * aqXllTools.h.
 */

#include <aqMain.h>

#include <string>

#include <aqXllTools.h>
#include <tryAqCapFloorObject.h>   // validation::tryAqCapFloorObject*

using namespace aq_xll;


// Create and store a cap/floor from a label/value block.
#if AQ_XLL_ENABLED(aqCapFloorObjectCreate)
XLO_FUNC_START( aqCapFloorObjectCreate(
    const ExcelObj& objectName,
    const ExcelObj& tradeLVB,
    const ExcelObj& validateKeys ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const std::string name = decorateWithExcelLocation( toNarrowString( objectName ) );

    const std::string storedName = validation::tryAqCapFloorObjectCreate(
        name, toLabelValueBlock( tradeLVB ), toBool( validateKeys, true ) );

    return returnValue( appendInstanceCounter( storedName ) );
}
XLO_FUNC_END( aqCapFloorObjectCreate )
    .help( L"Create and store a cap/floor from a label/value block; returns its handle." )
    .arg( L"ObjectName",   L"Name for the cap/floor object" )
    .arg( L"TradeLVB",     L"The cap/floor definition as a label/value block" )
    .arg( L"ValidateKeys", L"Optional. Default TRUE. Check the LVB keys" );
#endif


// Display a cached cap/floor as a matrix.
#if AQ_XLL_ENABLED(aqCapFloorObjectDisplay)
XLO_FUNC_START( aqCapFloorObjectDisplay(
    const ExcelObj& objectName ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelMatrix(
        validation::tryAqCapFloorObjectDisplay( getNameWithoutCounter( objectName ) ) ) );
}
XLO_FUNC_END( aqCapFloorObjectDisplay )
    .help( L"Display a cached cap/floor as a matrix." )
    .arg( L"ObjectName", L"A cap/floor handle" );
#endif


// Display the cashflows of a cached cap/floor, priced off a volatility and a curve.
#if AQ_XLL_ENABLED(aqCapFloorObjectDisplayCashflows)
XLO_FUNC_START( aqCapFloorObjectDisplayCashflows(
    const ExcelObj& objectName,
    const ExcelObj& volObjectName,
    const ExcelObj& curveCollection,
    const ExcelObj& fixingTableName,
    const ExcelObj& showColumnHeaders ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelMatrix( validation::tryAqCapFloorObjectDisplayCashflows(
        getNameWithoutCounter( objectName ),
        getNameWithoutCounter( volObjectName ),
        toNarrowString( curveCollection ),
        toNarrowString( fixingTableName ),
        toBool( showColumnHeaders, true ) ) ) );
}
XLO_FUNC_END( aqCapFloorObjectDisplayCashflows )
    .help( L"Display the cashflows of a cached cap/floor, priced off a volatility surface and a curve collection." )
    .arg( L"ObjectName",        L"A cap/floor handle" )
    .arg( L"VolObjectName",     L"A volatility-surface handle" )
    .arg( L"CurveCollection",   L"Discount / forward curve collection" )
    .arg( L"FixingTableName",   L"Optional. Fixing table for past fixings" )
    .arg( L"ShowColumnHeaders", L"Optional. Default TRUE. Include a header row" );
#endif


// Present value of a cached cap/floor, priced off a volatility and a curve.
#if AQ_XLL_ENABLED(aqCapFloorObjectPV)
XLO_FUNC_START( aqCapFloorObjectPV(
    const ExcelObj& objectName,
    const ExcelObj& volObjectName,
    const ExcelObj& curveCollection,
    const ExcelObj& fixingTableName ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqCapFloorObjectPV(
        getNameWithoutCounter( objectName ),
        getNameWithoutCounter( volObjectName ),
        toNarrowString( curveCollection ),
        toNarrowString( fixingTableName ) ) );
}
XLO_FUNC_END( aqCapFloorObjectPV )
    .help( L"Present value of a cached cap/floor, priced off a volatility surface and a curve collection." )
    .arg( L"ObjectName",      L"A cap/floor handle" )
    .arg( L"VolObjectName",   L"A volatility-surface handle" )
    .arg( L"CurveCollection", L"Discount / forward curve collection" )
    .arg( L"FixingTableName", L"Optional. Fixing table for past fixings" );
#endif


// Analytical greeks of a cached cap/floor.
#if AQ_XLL_ENABLED(aqCapFloorObjectGreeksAnalytical)
XLO_FUNC_START( aqCapFloorObjectGreeksAnalytical(
    const ExcelObj& objectName,
    const ExcelObj& volObjectName,
    const ExcelObj& curveCollection,
    const ExcelObj& fixingTableName,
    const ExcelObj& showColumnHeaders ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelMatrix( validation::tryAqCapFloorObjectGreeksAnalytical(
        getNameWithoutCounter( objectName ),
        getNameWithoutCounter( volObjectName ),
        toNarrowString( curveCollection ),
        toNarrowString( fixingTableName ),
        toBool( showColumnHeaders, true ) ) ) );
}
XLO_FUNC_END( aqCapFloorObjectGreeksAnalytical )
    .help( L"Analytical greeks of a cached cap/floor." )
    .arg( L"ObjectName",        L"A cap/floor handle" )
    .arg( L"VolObjectName",     L"A volatility-surface handle" )
    .arg( L"CurveCollection",   L"Discount / forward curve collection" )
    .arg( L"FixingTableName",   L"Optional. Fixing table for past fixings" )
    .arg( L"ShowColumnHeaders", L"Optional. Default TRUE. Include a header row" );
#endif


// Greeks of a cached cap/floor, by bump-and-revalue.
#if AQ_XLL_ENABLED(aqCapFloorObjectGreeks)
XLO_FUNC_START( aqCapFloorObjectGreeks(
    const ExcelObj& objectName,
    const ExcelObj& volObjectName,
    const ExcelObj& curveCollection,
    const ExcelObj& deltaBump,
    const ExcelObj& gammaBump,
    const ExcelObj& vegaBump,
    const ExcelObj& thetaBump,
    const ExcelObj& fixingTableName,
    const ExcelObj& showColumnHeaders ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelMatrix( validation::tryAqCapFloorObjectGreeks(
        getNameWithoutCounter( objectName ),
        getNameWithoutCounter( volObjectName ),
        toNarrowString( curveCollection ),
        deltaBump.get<double>(),
        gammaBump.get<double>(),
        vegaBump.get<double>(),
        thetaBump.get<double>(),
        toNarrowString( fixingTableName ),
        toBool( showColumnHeaders, true ) ) ) );
}
XLO_FUNC_END( aqCapFloorObjectGreeks )
    .help( L"Greeks of a cached cap/floor, by bump-and-revalue." )
    .arg( L"ObjectName",        L"A cap/floor handle" )
    .arg( L"VolObjectName",     L"A volatility-surface handle" )
    .arg( L"CurveCollection",   L"Discount / forward curve collection" )
    .arg( L"DeltaBump",         L"Bump size for delta" )
    .arg( L"GammaBump",         L"Bump size for gamma" )
    .arg( L"VegaBump",          L"Bump size for vega" )
    .arg( L"ThetaBump",         L"Bump size for theta (days)" )
    .arg( L"FixingTableName",   L"Optional. Fixing table for past fixings" )
    .arg( L"ShowColumnHeaders", L"Optional. Default TRUE. Include a header row" );
#endif


// Display the cashflows of a cached cap/floor, priced off an explicit rate matrix.
#if AQ_XLL_ENABLED(aqCapFloorObjectDisplayCashflowsFromRates)
XLO_FUNC_START( aqCapFloorObjectDisplayCashflowsFromRates(
    const ExcelObj& objectName,
    const ExcelObj& volObjectName,
    const ExcelObj& rateData,
    const ExcelObj& showColumnHeaders ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelMatrix( validation::tryAqCapFloorObjectDisplayCashflowsFromRates(
        getNameWithoutCounter( objectName ),
        getNameWithoutCounter( volObjectName ),
        toDoubleMatrix( rateData ),
        toBool( showColumnHeaders, true ) ) ) );
}
XLO_FUNC_END( aqCapFloorObjectDisplayCashflowsFromRates )
    .help( L"Display the cashflows of a cached cap/floor, priced off an explicit rate matrix." )
    .arg( L"ObjectName",        L"A cap/floor handle" )
    .arg( L"VolObjectName",     L"A volatility-surface handle" )
    .arg( L"RateData",          L"Explicit forward/discount rate matrix" )
    .arg( L"ShowColumnHeaders", L"Optional. Default TRUE. Include a header row" );
#endif


// Analytical greeks of a cached cap/floor, priced off an explicit rate matrix.
#if AQ_XLL_ENABLED(aqCapFloorObjectGreeksAnalyticalFromRates)
XLO_FUNC_START( aqCapFloorObjectGreeksAnalyticalFromRates(
    const ExcelObj& objectName,
    const ExcelObj& volObjectName,
    const ExcelObj& rateData,
    const ExcelObj& showColumnHeaders ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelMatrix( validation::tryAqCapFloorObjectGreeksAnalyticalFromRates(
        getNameWithoutCounter( objectName ),
        getNameWithoutCounter( volObjectName ),
        toDoubleMatrix( rateData ),
        toBool( showColumnHeaders, true ) ) ) );
}
XLO_FUNC_END( aqCapFloorObjectGreeksAnalyticalFromRates )
    .help( L"Analytical greeks of a cached cap/floor, priced off an explicit rate matrix." )
    .arg( L"ObjectName",        L"A cap/floor handle" )
    .arg( L"VolObjectName",     L"A volatility-surface handle" )
    .arg( L"RateData",          L"Explicit forward/discount rate matrix" )
    .arg( L"ShowColumnHeaders", L"Optional. Default TRUE. Include a header row" );
#endif


// Greeks of a cached cap/floor, by bump-and-revalue, priced off an explicit rate matrix.
#if AQ_XLL_ENABLED(aqCapFloorObjectGreeksFromRates)
XLO_FUNC_START( aqCapFloorObjectGreeksFromRates(
    const ExcelObj& objectName,
    const ExcelObj& volObjectName,
    const ExcelObj& rateData,
    const ExcelObj& deltaBump,
    const ExcelObj& gammaBump,
    const ExcelObj& vegaBump,
    const ExcelObj& thetaBump,
    const ExcelObj& showColumnHeaders ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelMatrix( validation::tryAqCapFloorObjectGreeksFromRates(
        getNameWithoutCounter( objectName ),
        getNameWithoutCounter( volObjectName ),
        toDoubleMatrix( rateData ),
        deltaBump.get<double>(),
        gammaBump.get<double>(),
        vegaBump.get<double>(),
        thetaBump.get<double>(),
        toBool( showColumnHeaders, true ) ) ) );
}
XLO_FUNC_END( aqCapFloorObjectGreeksFromRates )
    .help( L"Greeks of a cached cap/floor, by bump-and-revalue, priced off an explicit rate matrix." )
    .arg( L"ObjectName",        L"A cap/floor handle" )
    .arg( L"VolObjectName",     L"A volatility-surface handle" )
    .arg( L"RateData",          L"Explicit forward/discount rate matrix" )
    .arg( L"DeltaBump",         L"Bump size for delta" )
    .arg( L"GammaBump",         L"Bump size for gamma" )
    .arg( L"VegaBump",          L"Bump size for vega" )
    .arg( L"ThetaBump",         L"Bump size for theta (days)" )
    .arg( L"ShowColumnHeaders", L"Optional. Default TRUE. Include a header row" );
#endif


// Present value of a cached cap/floor, priced off an explicit rate matrix.
#if AQ_XLL_ENABLED(aqCapFloorObjectPVFromRates)
XLO_FUNC_START( aqCapFloorObjectPVFromRates(
    const ExcelObj& objectName,
    const ExcelObj& volObjectName,
    const ExcelObj& rateData ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqCapFloorObjectPVFromRates(
        getNameWithoutCounter( objectName ),
        getNameWithoutCounter( volObjectName ),
        toDoubleMatrix( rateData ) ) );
}
XLO_FUNC_END( aqCapFloorObjectPVFromRates )
    .help( L"Present value of a cached cap/floor, priced off an explicit rate matrix." )
    .arg( L"ObjectName",    L"A cap/floor handle" )
    .arg( L"VolObjectName", L"A volatility-surface handle" )
    .arg( L"RateData",      L"Explicit forward/discount rate matrix" );
#endif
