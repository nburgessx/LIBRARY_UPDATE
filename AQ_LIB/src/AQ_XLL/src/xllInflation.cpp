/*
 * Inflation category - xlOil worksheet functions.
 *
 * aqInflationCurve*   - operate on a cached inflation curve (a named
 *                       sub-object; no "Object" word).
 * aqInflationObject*  - operate on a cached zero-coupon inflation swap.
 *
 * Each function pairs with the identically named validation wrapper (plus the
 * `try` prefix). Marshalling to and from Excel is the aq_xll helpers in
 * xllSupport.h.
 */

#include <xllMain.h>

#include <string>
#include <vector>

#include <xllSupport.h>
#include <CoreEnumerations.h>              // etrading::trim_to_upper
#include <JSONInfoBlock.h>                 // etrading::JSONInfoBlockTuples
#include <tryAqInflationObjectPricing.h>   // validation::tryAqInflationCurve* / tryAqInflationObject*

using namespace aq_xll;


// Create and store an inflation curve from one or two named data blocks.
#if AQ_XLL_ENABLED(aqInflationCurveCreate)
XLO_FUNC_START( aqInflationCurveCreate(
    const ExcelObj& inflationCurveName,
    const ExcelObj& key1,
    const ExcelObj& value1,
    const ExcelObj& key2,
    const ExcelObj& value2 ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const std::string objectName = decorateWithExcelLocation( toNarrowString( inflationCurveName ) );

    std::vector<std::string>    dataBlockNames;
    etrading::JSONInfoBlockTuples infoBlocks;

    dataBlockNames.push_back( etrading::trim_to_upper( toNarrowString( key1 ) ) );
    infoBlocks.push_back( toTableInfo( value1 ) );

    if ( !value2.isMissing() && value2.isNonEmpty() )
    {
        dataBlockNames.push_back( etrading::trim_to_upper( toNarrowString( key2 ) ) );
        infoBlocks.push_back( toTableInfo( value2 ) );
    }

    const std::string storedName =
        validation::tryAqInflationCurveCreate( objectName, dataBlockNames, infoBlocks );

    return returnValue( appendInstanceCounter( storedName ) );
}
XLO_FUNC_END( aqInflationCurveCreate )
    .help( L"Create and store an inflation curve from one or two named data blocks; returns its handle." )
    .arg( L"InflationCurveName", L"Name for the inflation curve object" )
    .arg( L"Key1",               L"Name of the first data block" )
    .arg( L"Value1",             L"First data block, as a range" )
    .arg( L"Key2",               L"Optional. Name of the second data block" )
    .arg( L"Value2",             L"Optional. Second data block, as a range" );
#endif


// The calibration parameters of a stored inflation curve.
#if AQ_XLL_ENABLED(aqInflationCurveCalibrationParameters)
XLO_FUNC_START( aqInflationCurveCalibrationParameters(
    const ExcelObj& inflationCurveName ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelMatrix(
        validation::tryAqInflationCurveCalibrationParameters( getNameWithoutCounter( inflationCurveName ) ) ) );
}
XLO_FUNC_END( aqInflationCurveCalibrationParameters )
    .help( L"The calibration parameters of a stored inflation curve, as a matrix." )
    .arg( L"InflationCurveName", L"An inflation curve handle" );
#endif


// CPI index level from a stored inflation curve.
#if AQ_XLL_ENABLED(aqInflationObjectCPI)
XLO_FUNC_START( aqInflationObjectCPI(
    const ExcelObj& inflationCurveName,
    const ExcelObj& date,
    const ExcelObj& inflationResetType,
    const ExcelObj& lag ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqInflationObjectCPI(
        getNameWithoutCounter( inflationCurveName ),
        toAQLDate( date ),
        toNarrowString( inflationResetType ),
        toNarrowString( lag ) ) );
}
XLO_FUNC_END( aqInflationObjectCPI )
    .help( L"CPI index level from a stored inflation curve at a date." )
    .arg( L"InflationCurveName", L"An inflation curve handle" )
    .arg( L"Date",               L"The date to read the index at" )
    .arg( L"InflationResetType", L"Reset convention, e.g. DAILY, MONTHLY" )
    .arg( L"Lag",                L"Publication lag, e.g. 3M" );
#endif


// Present value of a cached zero-coupon inflation swap, off a stored curve.
#if AQ_XLL_ENABLED(aqInflationObjectZCSwapPV)
XLO_FUNC_START( aqInflationObjectZCSwapPV(
    const ExcelObj& swapName,
    const ExcelObj& inflationCurveName,
    const ExcelObj& valuationSettingsLVB,
    const ExcelObj& legName ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqInflationObjectZCSwapPV(
        getNameWithoutCounter( swapName ),
        getNameWithoutCounter( inflationCurveName ),
        toLabelValueBlock( valuationSettingsLVB ),
        toNarrowString( legName ) ) );
}
XLO_FUNC_END( aqInflationObjectZCSwapPV )
    .help( L"Present value of a cached zero-coupon inflation swap, off a stored inflation curve." )
    .arg( L"SwapName",             L"A swap handle" )
    .arg( L"InflationCurveName",   L"An inflation curve handle" )
    .arg( L"ValuationSettingsLVB", L"Valuation settings as a label/value block" )
    .arg( L"LegName",              L"The leg to value" );
#endif


// Present value of a cached zero-coupon inflation swap, off explicit index levels.
#if AQ_XLL_ENABLED(aqInflationObjectZCSwapPVFromIndex)
XLO_FUNC_START( aqInflationObjectZCSwapPVFromIndex(
    const ExcelObj& swapName,
    const ExcelObj& valuationSettingsLVB,
    const ExcelObj& baseIndex,
    const ExcelObj& resetIndex,
    const ExcelObj& legName ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqInflationObjectZCSwapPVFromIndex(
        getNameWithoutCounter( swapName ),
        toLabelValueBlock( valuationSettingsLVB ),
        baseIndex.get<double>(),
        resetIndex.get<double>(),
        toNarrowString( legName ) ) );
}
XLO_FUNC_END( aqInflationObjectZCSwapPVFromIndex )
    .help( L"Present value of a cached zero-coupon inflation swap, off explicit base/reset index levels." )
    .arg( L"SwapName",             L"A swap handle" )
    .arg( L"ValuationSettingsLVB", L"Valuation settings as a label/value block" )
    .arg( L"BaseIndex",            L"The base CPI index level" )
    .arg( L"ResetIndex",           L"The reset CPI index level" )
    .arg( L"LegName",              L"The leg to value" );
#endif


// Par rate of a cached zero-coupon inflation swap, off a stored curve.
#if AQ_XLL_ENABLED(aqInflationObjectZCSwapParRate)
XLO_FUNC_START( aqInflationObjectZCSwapParRate(
    const ExcelObj& swapName,
    const ExcelObj& inflationCurveName,
    const ExcelObj& valuationSettingsLVB ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqInflationObjectZCSwapParRate(
        getNameWithoutCounter( swapName ),
        getNameWithoutCounter( inflationCurveName ),
        toLabelValueBlock( valuationSettingsLVB ) ) );
}
XLO_FUNC_END( aqInflationObjectZCSwapParRate )
    .help( L"Par rate of a cached zero-coupon inflation swap, off a stored inflation curve." )
    .arg( L"SwapName",             L"A swap handle" )
    .arg( L"InflationCurveName",   L"An inflation curve handle" )
    .arg( L"ValuationSettingsLVB", L"Valuation settings as a label/value block" );
#endif


// Par rate of a cached zero-coupon inflation swap, off explicit index levels.
#if AQ_XLL_ENABLED(aqInflationObjectZCSwapParRateFromIndex)
XLO_FUNC_START( aqInflationObjectZCSwapParRateFromIndex(
    const ExcelObj& swapName,
    const ExcelObj& valuationSettingsLVB,
    const ExcelObj& baseIndex,
    const ExcelObj& resetIndex ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqInflationObjectZCSwapParRateFromIndex(
        getNameWithoutCounter( swapName ),
        toLabelValueBlock( valuationSettingsLVB ),
        baseIndex.get<double>(),
        resetIndex.get<double>() ) );
}
XLO_FUNC_END( aqInflationObjectZCSwapParRateFromIndex )
    .help( L"Par rate of a cached zero-coupon inflation swap, off explicit base/reset index levels." )
    .arg( L"SwapName",             L"A swap handle" )
    .arg( L"ValuationSettingsLVB", L"Valuation settings as a label/value block" )
    .arg( L"BaseIndex",            L"The base CPI index level" )
    .arg( L"ResetIndex",           L"The reset CPI index level" );
#endif
