/*
 * Volatility category - xlOil worksheet functions.
 *
 * aqVolatilityObject* operates on a cached vol surface / SABR market data /
 * SABR model (names in). Each function pairs with the identically named
 * validation wrapper (plus the `try` prefix). Marshalling to and from Excel is
 * the aq_xll helpers in aqXllTools.h.
 *
 * NOT ported here: the legacy tryAqVolatilitySABR{Calibrate,GetPrem,GetVol,
 * OutputParameter,SetupConvention,SetupParameter,SetupSwaptionVol} family
 * (tryAqVolatilitySABRCalibrate.h and siblings) - a procedural, ID-string-
 * driven calibration workflow that predates the object-based Sabr* API below
 * and appears superseded by it. Needs Nicholas's call: port as-is, or drop as
 * legacy (rebrand\STATUS.md).
 */

#include <aqMain.h>

#include <string>
#include <vector>

#include <aqXllTools.h>
#include <CoreEnumerations.h>        // etrading::trim_to_upper
#include <tryAqVolatilityObject.h>   // validation::tryAqVolatilityObject*

using namespace aq_xll;


// Create and store a volatility surface from a label/value block.
XLO_FUNC_START( aqVolatilityObjectCreate(
    const ExcelObj& objectName,
    const ExcelObj& volLVB,
    const ExcelObj& validateKeys ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const std::string name = decorateWithExcelLocation( toNarrowString( objectName ) );

    const std::string storedName = validation::tryAqVolatilityObjectCreate(
        name, toLabelValueBlock( volLVB ), toBool( validateKeys, true ) );

    return returnValue( appendInstanceCounter( storedName ) );
}
XLO_FUNC_END( aqVolatilityObjectCreate )
    .help( L"Create and store a volatility surface from a label/value block; returns its handle." )
    .arg( L"ObjectName",   L"Name for the volatility-surface object" )
    .arg( L"VolLVB",       L"The volatility-surface definition as a label/value block" )
    .arg( L"ValidateKeys", L"Optional. Default TRUE. Check the LVB keys" );


// Create and store SABR market data from one or two named data blocks.
XLO_FUNC_START( aqVolatilityObjectSabrMarketDataCreate(
    const ExcelObj& objectName,
    const ExcelObj& key1,
    const ExcelObj& value1,
    const ExcelObj& key2,
    const ExcelObj& value2 ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const std::string name = decorateWithExcelLocation( toNarrowString( objectName ) );

    std::vector<std::string>           dataBlockNames;
    etrading::JSONInfoBlockTuples infoBlocks;

    dataBlockNames.push_back( etrading::trim_to_upper( toNarrowString( key1 ) ) );
    infoBlocks.push_back( toTableInfo( value1 ) );

    if ( !value2.isMissing() && value2.isNonEmpty() )
    {
        dataBlockNames.push_back( etrading::trim_to_upper( toNarrowString( key2 ) ) );
        infoBlocks.push_back( toTableInfo( value2 ) );
    }

    const std::string storedName =
        validation::tryAqVolatilityObjectSabrMarketDataCreate( name, dataBlockNames, infoBlocks );

    return returnValue( appendInstanceCounter( storedName ) );
}
XLO_FUNC_END( aqVolatilityObjectSabrMarketDataCreate )
    .help( L"Create and store SABR market data from one or two named data blocks; returns its handle." )
    .arg( L"ObjectName", L"Name for the SABR market-data object" )
    .arg( L"Key1",       L"Name of the first data block" )
    .arg( L"Value1",     L"First data block, as a range" )
    .arg( L"Key2",       L"Optional. Name of the second data block" )
    .arg( L"Value2",     L"Optional. Second data block, as a range" );


// Calibrate and store a SABR model from one or two named data blocks.
XLO_FUNC_START( aqVolatilityObjectSabrModelCalibrate(
    const ExcelObj& objectName,
    const ExcelObj& key1,
    const ExcelObj& value1,
    const ExcelObj& key2,
    const ExcelObj& value2 ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const std::string name = decorateWithExcelLocation( toNarrowString( objectName ) );

    std::vector<std::string>           dataBlockNames;
    etrading::JSONInfoBlockTuples infoBlocks;

    dataBlockNames.push_back( etrading::trim_to_upper( toNarrowString( key1 ) ) );
    infoBlocks.push_back( toTableInfo( value1 ) );

    if ( !value2.isMissing() && value2.isNonEmpty() )
    {
        dataBlockNames.push_back( etrading::trim_to_upper( toNarrowString( key2 ) ) );
        infoBlocks.push_back( toTableInfo( value2 ) );
    }

    const std::string storedName =
        validation::tryAqVolatilityObjectSabrModelCalibrate( name, dataBlockNames, infoBlocks );

    return returnValue( appendInstanceCounter( storedName ) );
}
XLO_FUNC_END( aqVolatilityObjectSabrModelCalibrate )
    .help( L"Calibrate and store a SABR model from one or two named data blocks; returns its handle." )
    .arg( L"ObjectName", L"Name for the SABR model object" )
    .arg( L"Key1",       L"Name of the first data block" )
    .arg( L"Value1",     L"First data block, as a range" )
    .arg( L"Key2",       L"Optional. Name of the second data block" )
    .arg( L"Value2",     L"Optional. Second data block, as a range" );


// SABR-implied volatility at an expiry/tenor/strike/forward point.
XLO_FUNC_START( aqVolatilityObjectSabrVolatility(
    const ExcelObj& volSurfaceName,
    const ExcelObj& expiry,
    const ExcelObj& tenor,
    const ExcelObj& strike,
    const ExcelObj& forward ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqVolatilityObjectSabrVolatility(
        getNameWithoutCounter( volSurfaceName ),
        toNarrowString( expiry ),
        toNarrowString( tenor ),
        strike.get<double>(),
        forward.get<double>() ) );
}
XLO_FUNC_END( aqVolatilityObjectSabrVolatility )
    .help( L"SABR-implied volatility at an expiry/tenor/strike/forward point." )
    .arg( L"VolSurfaceName", L"A volatility-surface handle" )
    .arg( L"Expiry",         L"Option expiry, e.g. 3M, 1Y" )
    .arg( L"Tenor",          L"Underlying tenor, e.g. 5Y, 10Y" )
    .arg( L"Strike",         L"Strike" )
    .arg( L"Forward",        L"Forward rate" );


// One SABR parameter at an expiry/tenor point.
XLO_FUNC_START( aqVolatilityObjectSabrParameter(
    const ExcelObj& volSurfaceName,
    const ExcelObj& expiry,
    const ExcelObj& tenor,
    const ExcelObj& paramName ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqVolatilityObjectSabrParameter(
        getNameWithoutCounter( volSurfaceName ),
        toNarrowString( expiry ),
        toNarrowString( tenor ),
        toNarrowString( paramName ) ) );
}
XLO_FUNC_END( aqVolatilityObjectSabrParameter )
    .help( L"One SABR parameter (alpha, beta, rho, nu, ...) at an expiry/tenor point." )
    .arg( L"VolSurfaceName", L"A volatility-surface handle" )
    .arg( L"Expiry",         L"Option expiry, e.g. 3M, 1Y" )
    .arg( L"Tenor",          L"Underlying tenor, e.g. 5Y, 10Y" )
    .arg( L"ParamName",      L"Which parameter, e.g. ALPHA, BETA, RHO, NU" );


// Display stored SABR market data.
XLO_FUNC_START( aqVolatilityObjectSabrMarketDataDisplay(
    const ExcelObj& marketDataObjectName,
    const ExcelObj& marketDataKey ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelMatrix( validation::tryAqVolatilityObjectSabrMarketDataDisplay(
        getNameWithoutCounter( marketDataObjectName ), toNarrowString( marketDataKey ) ) ) );
}
XLO_FUNC_END( aqVolatilityObjectSabrMarketDataDisplay )
    .help( L"Display stored SABR market data as a matrix." )
    .arg( L"MarketDataObjectName", L"A SABR market-data handle" )
    .arg( L"MarketDataKey",        L"The data block to display" );


// Display a stored SABR model.
XLO_FUNC_START( aqVolatilityObjectSabrModelDisplay(
    const ExcelObj& modelObjectName,
    const ExcelObj& modelKey ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelMatrix( validation::tryAqVolatilityObjectSabrModelDisplay(
        getNameWithoutCounter( modelObjectName ), toNarrowString( modelKey ) ) ) );
}
XLO_FUNC_END( aqVolatilityObjectSabrModelDisplay )
    .help( L"Display a stored SABR model as a matrix." )
    .arg( L"ModelObjectName", L"A SABR model handle" )
    .arg( L"ModelKey",        L"The model block to display" );
