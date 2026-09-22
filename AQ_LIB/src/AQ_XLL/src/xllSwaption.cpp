/*
 * Swaption category - xlOil worksheet functions.
 *
 * aqSwaptionObject* operates on a cached swaption (name in). Each function
 * pairs with the identically named validation wrapper (plus the `try`
 * prefix). Marshalling to and from Excel is the aq_xll helpers in
 * xllSupport.h.
 */

#include <xllMain.h>

#include <string>

#include <xllSupport.h>
#include <tryAqSwaptionObject.h>   // validation::tryAqSwaptionObject*

using namespace aq_xll;


// Create and store a swaption from a label/value block.
#if AQ_XLL_ENABLED(aqSwaptionObjectCreate)
XLO_FUNC_START( aqSwaptionObjectCreate(
    const ExcelObj& objectName,
    const ExcelObj& tradeLVB,
    const ExcelObj& validateKeys ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const std::string name = decorateWithExcelLocation( toNarrowString( objectName ) );

    const std::string storedName = validation::tryAqSwaptionObjectCreate(
        name, toLabelValueBlock( tradeLVB ), toBool( validateKeys, true ) );

    return returnValue( appendInstanceCounter( storedName ) );
}
XLO_FUNC_END( aqSwaptionObjectCreate )
    .help( L"Create and store a swaption from a label/value block; returns its handle." )
    .arg( L"ObjectName",   L"Name for the swaption object" )
    .arg( L"TradeLVB",     L"The swaption definition as a label/value block" )
    .arg( L"ValidateKeys", L"Optional. Default TRUE. Check the LVB keys" );
#endif


// Display a cached swaption as a matrix.
#if AQ_XLL_ENABLED(aqSwaptionObjectDisplay)
XLO_FUNC_START( aqSwaptionObjectDisplay(
    const ExcelObj& objectName ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelMatrix(
        validation::tryAqSwaptionObjectDisplay( getNameWithoutCounter( objectName ) ) ) );
}
XLO_FUNC_END( aqSwaptionObjectDisplay )
    .help( L"Display a cached swaption as a matrix." )
    .arg( L"ObjectName", L"A swaption handle" );
#endif


// Present value of a cached swaption.
#if AQ_XLL_ENABLED(aqSwaptionObjectPV)
XLO_FUNC_START( aqSwaptionObjectPV(
    const ExcelObj& objectName,
    const ExcelObj& valuationSettingsLVB ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqSwaptionObjectPV(
        getNameWithoutCounter( objectName ), toAQLStringMatrix( valuationSettingsLVB ) ) );
}
XLO_FUNC_END( aqSwaptionObjectPV )
    .help( L"Present value of a cached swaption." )
    .arg( L"ObjectName",           L"A swaption handle" )
    .arg( L"ValuationSettingsLVB", L"Valuation settings as a key/value matrix" );
#endif


// Implied volatility from a cached swaption's price.
#if AQ_XLL_ENABLED(aqSwaptionObjectImpliedVol)
XLO_FUNC_START( aqSwaptionObjectImpliedVol(
    const ExcelObj& objectName,
    const ExcelObj& price,
    const ExcelObj& valuationSettingsLVB ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqSwaptionObjectImpliedVol(
        getNameWithoutCounter( objectName ), price.get<double>(), toAQLStringMatrix( valuationSettingsLVB ) ) );
}
XLO_FUNC_END( aqSwaptionObjectImpliedVol )
    .help( L"Implied volatility from a cached swaption's price." )
    .arg( L"ObjectName",           L"A swaption handle" )
    .arg( L"Price",                L"The swaption price" )
    .arg( L"ValuationSettingsLVB", L"Valuation settings as a key/value matrix" );
#endif


// Delta of a cached swaption.
#if AQ_XLL_ENABLED(aqSwaptionObjectDelta)
XLO_FUNC_START( aqSwaptionObjectDelta(
    const ExcelObj& objectName,
    const ExcelObj& valuationSettingsLVB ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqSwaptionObjectDelta(
        getNameWithoutCounter( objectName ), toAQLStringMatrix( valuationSettingsLVB ) ) );
}
XLO_FUNC_END( aqSwaptionObjectDelta )
    .help( L"Delta of a cached swaption." )
    .arg( L"ObjectName",           L"A swaption handle" )
    .arg( L"ValuationSettingsLVB", L"Valuation settings as a key/value matrix" );
#endif


// Gamma of a cached swaption.
#if AQ_XLL_ENABLED(aqSwaptionObjectGamma)
XLO_FUNC_START( aqSwaptionObjectGamma(
    const ExcelObj& objectName,
    const ExcelObj& valuationSettingsLVB ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqSwaptionObjectGamma(
        getNameWithoutCounter( objectName ), toAQLStringMatrix( valuationSettingsLVB ) ) );
}
XLO_FUNC_END( aqSwaptionObjectGamma )
    .help( L"Gamma of a cached swaption." )
    .arg( L"ObjectName",           L"A swaption handle" )
    .arg( L"ValuationSettingsLVB", L"Valuation settings as a key/value matrix" );
#endif


// Vega of a cached swaption.
#if AQ_XLL_ENABLED(aqSwaptionObjectVega)
XLO_FUNC_START( aqSwaptionObjectVega(
    const ExcelObj& objectName,
    const ExcelObj& valuationSettingsLVB ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqSwaptionObjectVega(
        getNameWithoutCounter( objectName ), toAQLStringMatrix( valuationSettingsLVB ) ) );
}
XLO_FUNC_END( aqSwaptionObjectVega )
    .help( L"Vega of a cached swaption." )
    .arg( L"ObjectName",           L"A swaption handle" )
    .arg( L"ValuationSettingsLVB", L"Valuation settings as a key/value matrix" );
#endif


// Theta of a cached swaption.
#if AQ_XLL_ENABLED(aqSwaptionObjectTheta)
XLO_FUNC_START( aqSwaptionObjectTheta(
    const ExcelObj& objectName,
    const ExcelObj& valuationSettingsLVB ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqSwaptionObjectTheta(
        getNameWithoutCounter( objectName ), toAQLStringMatrix( valuationSettingsLVB ) ) );
}
XLO_FUNC_END( aqSwaptionObjectTheta )
    .help( L"Theta of a cached swaption." )
    .arg( L"ObjectName",           L"A swaption handle" )
    .arg( L"ValuationSettingsLVB", L"Valuation settings as a key/value matrix" );
#endif
