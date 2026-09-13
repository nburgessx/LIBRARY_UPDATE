/*
 * FX category - xlOil worksheet functions.
 *
 * aqFXCurveCreate operates on a named sub-object (no "Object" word); the rest
 * take curve/currency identifiers directly (stateless) or a cached xccy-curve
 * handle. Each function pairs with the identically named validation wrapper
 * (plus the `try` prefix). Marshalling to and from Excel is the aq_xll
 * helpers in aqXllTools.h.
 */

#include <aqMain.h>

#include <string>
#include <vector>

#include <aqXllTools.h>
#include <tryAqFXObject.h>   // validation::tryAqFXCurveCreate / tryAqFXObject*

using namespace aq_xll;

namespace
{
    // An optional column of strings: a missing/blank range is an empty vector.
    std::vector<std::string> toStringVectorOr( const xloil::ExcelObj& obj )
    {
        if ( obj.isMissing() || !obj.isNonEmpty() )
        {
            return std::vector<std::string>();
        }
        return toStringVector( obj, true );
    }
}


// Create and store an FX curve from a curve generator and market data.
#if AQ_XLL_ENABLED(aqFXCurveCreate)
XLO_FUNC_START( aqFXCurveCreate(
    const ExcelObj& objectName,
    const ExcelObj& curveGeneratorName,
    const ExcelObj& curveMarketDataName ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const std::string name = decorateWithExcelLocation( toNarrowString( objectName ) );

    const std::string storedName = validation::tryAqFXCurveCreate(
        name, getNameWithoutCounter( curveGeneratorName ), getNameWithoutCounter( curveMarketDataName ) );

    return returnValue( appendInstanceCounter( storedName ) );
}
XLO_FUNC_END( aqFXCurveCreate )
    .help( L"Create and store an FX curve from a curve generator and market data; returns its handle." )
    .arg( L"ObjectName",          L"Name for the FX curve object" )
    .arg( L"CurveGeneratorName",  L"A curve-generator handle" )
    .arg( L"CurveMarketDataName", L"A curve-market-data handle" );
#endif


// FX forwards from discount curves.
#if AQ_XLL_ENABLED(aqFXObjectForwardsFromDiscountCurves)
XLO_FUNC_START( aqFXObjectForwardsFromDiscountCurves(
    const ExcelObj& objectName,
    const ExcelObj& settleDatesOrTenors,
    const ExcelObj& baseCurveCollection,
    const ExcelObj& termCurveCollection,
    const ExcelObj& showColumnHeaders,
    const ExcelObj& columnList ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelMatrix( validation::tryAqFXObjectForwardsFromDiscountCurves(
        getNameWithoutCounter( objectName ),
        toStringVector( settleDatesOrTenors, true ),
        toNarrowString( baseCurveCollection ),
        toNarrowString( termCurveCollection ),
        toBool( showColumnHeaders, false ),
        toStringVectorOr( columnList ) ) ) );
}
XLO_FUNC_END( aqFXObjectForwardsFromDiscountCurves )
    .help( L"FX forwards derived from base/term discount curves." )
    .arg( L"ObjectName",           L"An FX curve handle" )
    .arg( L"SettleDatesOrTenors",  L"Column of settlement dates or tenors" )
    .arg( L"BaseCurveCollection",  L"Base-currency discount curve collection" )
    .arg( L"TermCurveCollection",  L"Term-currency discount curve collection" )
    .arg( L"ShowColumnHeaders",    L"Optional. Default FALSE. Include a header row" )
    .arg( L"ColumnList",           L"Optional. Columns to include" );
#endif


// FX forwards from a cached FX curve.
#if AQ_XLL_ENABLED(aqFXObjectForwards)
XLO_FUNC_START( aqFXObjectForwards(
    const ExcelObj& objectName,
    const ExcelObj& settleDatesOrTenors,
    const ExcelObj& showColumnHeaders,
    const ExcelObj& columnList ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelMatrix( validation::tryAqFXObjectForwards(
        getNameWithoutCounter( objectName ),
        toStringVector( settleDatesOrTenors, true ),
        toBool( showColumnHeaders, false ),
        toStringVectorOr( columnList ) ) ) );
}
XLO_FUNC_END( aqFXObjectForwards )
    .help( L"FX forwards from a cached FX curve." )
    .arg( L"ObjectName",          L"An FX curve handle" )
    .arg( L"SettleDatesOrTenors", L"Column of settlement dates or tenors" )
    .arg( L"ShowColumnHeaders",   L"Optional. Default FALSE. Include a header row" )
    .arg( L"ColumnList",          L"Optional. Columns to include" );
#endif


// FX swap points from discount curves.
#if AQ_XLL_ENABLED(aqFXObjectSwapFromDiscountCurves)
XLO_FUNC_START( aqFXObjectSwapFromDiscountCurves(
    const ExcelObj& objectName,
    const ExcelObj& nearLegSettlementDatesOrTenors,
    const ExcelObj& farLegSettlementDatesOrTenors,
    const ExcelObj& baseCurveCollection,
    const ExcelObj& termCurveCollection,
    const ExcelObj& outputFarLeg,
    const ExcelObj& showColumnHeaders,
    const ExcelObj& columnList ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelMatrix( validation::tryAqFXObjectSwapFromDiscountCurves(
        getNameWithoutCounter( objectName ),
        toStringVector( nearLegSettlementDatesOrTenors, true ),
        toStringVector( farLegSettlementDatesOrTenors, true ),
        toNarrowString( baseCurveCollection ),
        toNarrowString( termCurveCollection ),
        toBool( outputFarLeg, true ),
        toBool( showColumnHeaders, false ),
        toStringVectorOr( columnList ) ) ) );
}
XLO_FUNC_END( aqFXObjectSwapFromDiscountCurves )
    .help( L"FX swap points derived from base/term discount curves." )
    .arg( L"ObjectName",                     L"An FX curve handle" )
    .arg( L"NearLegSettlementDatesOrTenors", L"Column of near-leg settlement dates or tenors" )
    .arg( L"FarLegSettlementDatesOrTenors",  L"Column of far-leg settlement dates or tenors" )
    .arg( L"BaseCurveCollection",            L"Base-currency discount curve collection" )
    .arg( L"TermCurveCollection",            L"Term-currency discount curve collection" )
    .arg( L"OutputFarLeg",                   L"TRUE to output the far leg, FALSE the near leg" )
    .arg( L"ShowColumnHeaders",              L"Optional. Default FALSE. Include a header row" )
    .arg( L"ColumnList",                     L"Optional. Columns to include" );
#endif


// FX swap points from a cached FX curve.
#if AQ_XLL_ENABLED(aqFXObjectSwap)
XLO_FUNC_START( aqFXObjectSwap(
    const ExcelObj& objectName,
    const ExcelObj& nearLegSettlementDatesOrTenors,
    const ExcelObj& farLegSettlementDatesOrTenors,
    const ExcelObj& outputFarLeg,
    const ExcelObj& showColumnHeaders,
    const ExcelObj& columnList ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelMatrix( validation::tryAqFXObjectSwap(
        getNameWithoutCounter( objectName ),
        toStringVector( nearLegSettlementDatesOrTenors, true ),
        toStringVector( farLegSettlementDatesOrTenors, true ),
        toBool( outputFarLeg, true ),
        toBool( showColumnHeaders, false ),
        toStringVectorOr( columnList ) ) ) );
}
XLO_FUNC_END( aqFXObjectSwap )
    .help( L"FX swap points from a cached FX curve." )
    .arg( L"ObjectName",                     L"An FX curve handle" )
    .arg( L"NearLegSettlementDatesOrTenors", L"Column of near-leg settlement dates or tenors" )
    .arg( L"FarLegSettlementDatesOrTenors",  L"Column of far-leg settlement dates or tenors" )
    .arg( L"OutputFarLeg",                   L"TRUE to output the far leg, FALSE the near leg" )
    .arg( L"ShowColumnHeaders",              L"Optional. Default FALSE. Include a header row" )
    .arg( L"ColumnList",                     L"Optional. Columns to include" );
#endif


// FX forwards from a cached cross-currency curve object.
#if AQ_XLL_ENABLED(aqFXObjectForwardsFromXccyCurveObject)
XLO_FUNC_START( aqFXObjectForwardsFromXccyCurveObject(
    const ExcelObj& xccyCurveObjectName,
    const ExcelObj& settleDatesOrTenors,
    const ExcelObj& xccyCurveSwapRateBumpSize,
    const ExcelObj& showColumnHeaders,
    const ExcelObj& columnList ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelMatrix( validation::tryAqFXObjectForwardsFromXccyCurveObject(
        getNameWithoutCounter( xccyCurveObjectName ),
        toStringVector( settleDatesOrTenors, true ),
        xccyCurveSwapRateBumpSize.get<double>(),
        toBool( showColumnHeaders, false ),
        toStringVectorOr( columnList ) ) ) );
}
XLO_FUNC_END( aqFXObjectForwardsFromXccyCurveObject )
    .help( L"FX forwards from a cached cross-currency curve object." )
    .arg( L"XccyCurveObjectName",       L"A cross-currency curve handle" )
    .arg( L"SettleDatesOrTenors",       L"Column of settlement dates or tenors" )
    .arg( L"XccyCurveSwapRateBumpSize", L"Bump size applied to the xccy swap rate" )
    .arg( L"ShowColumnHeaders",         L"Optional. Default FALSE. Include a header row" )
    .arg( L"ColumnList",                L"Optional. Columns to include" );
#endif


// FX forwards from base/term curve collections via a cross-currency basis.
#if AQ_XLL_ENABLED(aqFXObjectForwardsFromXccyCurve)
XLO_FUNC_START( aqFXObjectForwardsFromXccyCurve(
    const ExcelObj& baseCurveCollection,
    const ExcelObj& baseCurveIndex,
    const ExcelObj& termCurveCollection,
    const ExcelObj& termCurveIndex,
    const ExcelObj& fxSpotRate,
    const ExcelObj& pipSize,
    const ExcelObj& xccyCurveSwapRateBumpSize,
    const ExcelObj& settleDatesOrTenors,
    const ExcelObj& showColumnHeaders,
    const ExcelObj& columnList ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelMatrix( validation::tryAqFXObjectForwardsFromXccyCurve(
        toNarrowString( baseCurveCollection ),
        toNarrowString( baseCurveIndex ),
        toNarrowString( termCurveCollection ),
        toNarrowString( termCurveIndex ),
        fxSpotRate.get<double>(),
        pipSize.get<double>(),
        xccyCurveSwapRateBumpSize.get<double>(),
        toStringVector( settleDatesOrTenors, true ),
        toBool( showColumnHeaders, false ),
        toStringVectorOr( columnList ) ) ) );
}
XLO_FUNC_END( aqFXObjectForwardsFromXccyCurve )
    .help( L"FX forwards from base/term curve collections via a cross-currency basis." )
    .arg( L"BaseCurveCollection",       L"Base-currency curve collection" )
    .arg( L"BaseCurveIndex",            L"Base-currency curve index" )
    .arg( L"TermCurveCollection",       L"Term-currency curve collection" )
    .arg( L"TermCurveIndex",            L"Term-currency curve index" )
    .arg( L"FxSpotRate",                L"FX spot rate" )
    .arg( L"PipSize",                   L"Pip size for quoting" )
    .arg( L"XccyCurveSwapRateBumpSize", L"Bump size applied to the xccy swap rate" )
    .arg( L"SettleDatesOrTenors",       L"Column of settlement dates or tenors" )
    .arg( L"ShowColumnHeaders",         L"Optional. Default FALSE. Include a header row" )
    .arg( L"ColumnList",                L"Optional. Columns to include" );
#endif


// FX swap points from a cached cross-currency curve object.
#if AQ_XLL_ENABLED(aqFXObjectSwapFromXccyCurveObject)
XLO_FUNC_START( aqFXObjectSwapFromXccyCurveObject(
    const ExcelObj& xccyCurveObjectName,
    const ExcelObj& nearLegSettlementDatesOrTenors,
    const ExcelObj& farLegSettlementDatesOrTenors,
    const ExcelObj& outputFarLeg,
    const ExcelObj& showColumnHeaders,
    const ExcelObj& columnList ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelMatrix( validation::tryAqFXObjectSwapFromXccyCurveObject(
        getNameWithoutCounter( xccyCurveObjectName ),
        toStringVector( nearLegSettlementDatesOrTenors, true ),
        toStringVector( farLegSettlementDatesOrTenors, true ),
        toBool( outputFarLeg, true ),
        toBool( showColumnHeaders, false ),
        toStringVectorOr( columnList ) ) ) );
}
XLO_FUNC_END( aqFXObjectSwapFromXccyCurveObject )
    .help( L"FX swap points from a cached cross-currency curve object." )
    .arg( L"XccyCurveObjectName",            L"A cross-currency curve handle" )
    .arg( L"NearLegSettlementDatesOrTenors", L"Column of near-leg settlement dates or tenors" )
    .arg( L"FarLegSettlementDatesOrTenors",  L"Column of far-leg settlement dates or tenors" )
    .arg( L"OutputFarLeg",                   L"TRUE to output the far leg, FALSE the near leg" )
    .arg( L"ShowColumnHeaders",              L"Optional. Default FALSE. Include a header row" )
    .arg( L"ColumnList",                     L"Optional. Columns to include" );
#endif


// FX swap points from base/term curve collections via a cross-currency basis.
#if AQ_XLL_ENABLED(aqFXObjectSwapFromXccyCurve)
XLO_FUNC_START( aqFXObjectSwapFromXccyCurve(
    const ExcelObj& baseCurveCollection,
    const ExcelObj& baseCurveIndex,
    const ExcelObj& termCurveCollection,
    const ExcelObj& termCurveIndex,
    const ExcelObj& fxSpotRate,
    const ExcelObj& pipSize,
    const ExcelObj& nearLegSettlementDatesOrTenors,
    const ExcelObj& farLegSettlementDatesOrTenors,
    const ExcelObj& outputFarLeg,
    const ExcelObj& showColumnHeaders,
    const ExcelObj& columnList ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelMatrix( validation::tryAqFXObjectSwapFromXccyCurve(
        toNarrowString( baseCurveCollection ),
        toNarrowString( baseCurveIndex ),
        toNarrowString( termCurveCollection ),
        toNarrowString( termCurveIndex ),
        fxSpotRate.get<double>(),
        pipSize.get<double>(),
        toStringVector( nearLegSettlementDatesOrTenors, true ),
        toStringVector( farLegSettlementDatesOrTenors, true ),
        toBool( outputFarLeg, true ),
        toBool( showColumnHeaders, false ),
        toStringVectorOr( columnList ) ) ) );
}
XLO_FUNC_END( aqFXObjectSwapFromXccyCurve )
    .help( L"FX swap points from base/term curve collections via a cross-currency basis." )
    .arg( L"BaseCurveCollection",            L"Base-currency curve collection" )
    .arg( L"BaseCurveIndex",                 L"Base-currency curve index" )
    .arg( L"TermCurveCollection",            L"Term-currency curve collection" )
    .arg( L"TermCurveIndex",                 L"Term-currency curve index" )
    .arg( L"FxSpotRate",                     L"FX spot rate" )
    .arg( L"PipSize",                        L"Pip size for quoting" )
    .arg( L"NearLegSettlementDatesOrTenors", L"Column of near-leg settlement dates or tenors" )
    .arg( L"FarLegSettlementDatesOrTenors",  L"Column of far-leg settlement dates or tenors" )
    .arg( L"OutputFarLeg",                   L"TRUE to output the far leg, FALSE the near leg" )
    .arg( L"ShowColumnHeaders",              L"Optional. Default FALSE. Include a header row" )
    .arg( L"ColumnList",                     L"Optional. Columns to include" );
#endif


// FX spot implied by an as-of-date rate.
#if AQ_XLL_ENABLED(aqFXObjectSpotToAsOfDate)
XLO_FUNC_START( aqFXObjectSpotToAsOfDate(
    const ExcelObj& fxSpot,
    const ExcelObj& fxSpotDate,
    const ExcelObj& baseCurveCollection,
    const ExcelObj& baseDiscountCurveIndex,
    const ExcelObj& termCurveCollection,
    const ExcelObj& termDiscountCurveIndex ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqFXObjectSpotToAsOfDate(
        fxSpot.get<double>(),
        toAQLDate( fxSpotDate ),
        toNarrowString( baseCurveCollection ),
        toNarrowString( baseDiscountCurveIndex ),
        toNarrowString( termCurveCollection ),
        toNarrowString( termDiscountCurveIndex ) ) );
}
XLO_FUNC_END( aqFXObjectSpotToAsOfDate )
    .help( L"The as-of-date FX rate implied by a spot rate." )
    .arg( L"FxSpot",                  L"FX spot rate" )
    .arg( L"FxSpotDate",              L"The spot date" )
    .arg( L"BaseCurveCollection",     L"Base-currency discount curve collection" )
    .arg( L"BaseDiscountCurveIndex",  L"Base-currency discount curve index" )
    .arg( L"TermCurveCollection",     L"Term-currency discount curve collection" )
    .arg( L"TermDiscountCurveIndex",  L"Term-currency discount curve index" );
#endif


// FX as-of-date rate implied by a spot rate.
#if AQ_XLL_ENABLED(aqFXObjectAsOfDateToSpot)
XLO_FUNC_START( aqFXObjectAsOfDateToSpot(
    const ExcelObj& fxAsOfDateRate,
    const ExcelObj& fxSpotDate,
    const ExcelObj& baseCurveCollection,
    const ExcelObj& baseDiscountCurveIndex,
    const ExcelObj& termCurveCollection,
    const ExcelObj& termDiscountCurveIndex ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqFXObjectAsOfDateToSpot(
        fxAsOfDateRate.get<double>(),
        toAQLDate( fxSpotDate ),
        toNarrowString( baseCurveCollection ),
        toNarrowString( baseDiscountCurveIndex ),
        toNarrowString( termCurveCollection ),
        toNarrowString( termDiscountCurveIndex ) ) );
}
XLO_FUNC_END( aqFXObjectAsOfDateToSpot )
    .help( L"FX spot rate implied by an as-of-date rate." )
    .arg( L"FxAsOfDateRate",          L"The as-of-date FX rate" )
    .arg( L"FxSpotDate",              L"The spot date" )
    .arg( L"BaseCurveCollection",     L"Base-currency discount curve collection" )
    .arg( L"BaseDiscountCurveIndex",  L"Base-currency discount curve index" )
    .arg( L"TermCurveCollection",     L"Term-currency discount curve collection" )
    .arg( L"TermDiscountCurveIndex",  L"Term-currency discount curve index" );
#endif
