/*
 * Bond category - xlOil worksheet functions.
 *
 * aqBond*        - stateless: data in, value out.
 * aqBondObject*  - operate on a cached bond object (name in).
 * aqBondCurve* / aqBondGenerator* - operate on a cached named sub-object.
 *
 * Handle-creating functions run: decorateWithExcelLocation -> allowAQObjUpdates
 * (create-vs-modify) -> the validation-layer create call -> appendInstanceCounter
 * (so Excel re-fires dependents on recalculation). Marshalling to and from Excel
 * is the aq_xll helpers in aqXllTools.h. Every function pairs with the identically
 * named validation wrapper (plus the `try` prefix).
 */

#include <aqMain.h>

#include <string>
#include <vector>

#include <aqXllTools.h>
#include <tryAqBondObject.h>      // validation bond wrappers

using namespace aq_xll;


XLO_FUNC_START( aqBondObjectCreate(
    const ExcelObj& bondName,
    const ExcelObj& bondLVB,
    const ExcelObj& scheduleLVB,
    const ExcelObj& validateKeys,
    const ExcelObj& allowUpdates ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const std::string objectName =
        decorateWithExcelLocation( toNarrowString( bondName ) );

    // Create-vs-modify guard. When the object already exists and AllowUpdates is
    // FALSE, hand back the existing handle unchanged and do not rebuild it.
    std::string existingHandle;
    if ( !allowAQObjUpdates( toBool( allowUpdates, true ),
                             existingHandle,
                             objectName,
                             etrading::BOND ) )
    {
        return returnValue( existingHandle );
    }

    const etrading::LabelValueBlock bondBlock     = toLabelValueBlock( bondLVB );
    const etrading::LabelValueBlock scheduleBlock = toLabelValueBlock( scheduleLVB );

    const std::string storedName =
        validation::tryAqBondObjectCreate( objectName,
                                         bondBlock,
                                         scheduleBlock,
                                         toBool( validateKeys, true ) );

    // Return the decorated handle - the counter suffix changes on every
    // recalculation so dependent cells recompute.
    return returnValue( appendInstanceCounter( storedName ) );
}
XLO_FUNC_END( aqBondObjectCreate )
    .help( L"Create a bond object and return its handle. Enter the bond and schedule as "
           L"two-column (key, value) blocks." )
    .arg( L"BondName",     L"Name for the bond object; the returned handle is this name plus a counter" )
    .arg( L"BondLVB",      L"Bond parameters as a two-column key/value range" )
    .arg( L"ScheduleLVB",  L"Schedule parameters as a two-column key/value range" )
    .arg( L"ValidateKeys", L"Optional. TRUE (default) rejects unknown keys" )
    .arg( L"AllowUpdates", L"Optional. FALSE returns the existing object unchanged if the name is in use; TRUE (default) rebuilds it" );


XLO_FUNC_START( aqBondObjectDisplay(
    const ExcelObj& bondName ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const std::string objectName = getNameWithoutCounter( bondName );

    const AnyTypeMatrix result =
        validation::tryAqBondObjectDisplay( objectName );

    return returnValue( toExcelMatrix( result ) );
}
XLO_FUNC_END( aqBondObjectDisplay )
    .help( L"Return a bond object's stored parameters as a key/value block." )
    .arg( L"BondName", L"A bond handle returned by aqBondObjectCreate" );


XLO_FUNC_START( aqBondObjectDisplaySchedule(
    const ExcelObj& bondName,
    const ExcelObj& showColumnHeaders ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const std::string objectName = getNameWithoutCounter( bondName );

    const AnyTypeMatrix result =
        validation::tryAqBondObjectDisplaySchedule( objectName,
                                                  toBool( showColumnHeaders, true ) );

    return returnValue( toExcelMatrix( result ) );
}
XLO_FUNC_END( aqBondObjectDisplaySchedule )
    .help( L"Return a bond object's cashflow schedule as a matrix." )
    .arg( L"BondName",          L"A bond handle returned by aqBondObjectCreate" )
    .arg( L"ShowColumnHeaders", L"Optional. TRUE (default) includes the header row" );


XLO_FUNC_START( aqBondObjectDisplayCashflows(
    const ExcelObj& bondName,
    const ExcelObj& settlementDate,
    const ExcelObj& yield,
    const ExcelObj& yieldCalcType,
    const ExcelObj& showColumnHeaders ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const std::string objectName = getNameWithoutCounter( bondName );

    const AnyTypeMatrix result =
        validation::tryAqBondObjectDisplayCashflows( objectName,
                                                   toAQLDate( settlementDate ),
                                                   yield.get<double>(),
                                                   toNarrowString( yieldCalcType ),
                                                   toBool( showColumnHeaders, true ) );

    return returnValue( toExcelMatrix( result ) );
}
XLO_FUNC_END( aqBondObjectDisplayCashflows )
    .help( L"Return a bond object's cashflows as a matrix." )
    .arg( L"BondName",          L"A bond handle returned by aqBondObjectCreate" )
    .arg( L"SettlementDate",    L"Bond pricing / valuation date" )
    .arg( L"Yield",             L"Bond yield" )
    .arg( L"YieldCalcType",     L"Optional. Yield calculation type, e.g. ISMA, TRUE, SIMPLE" )
    .arg( L"ShowColumnHeaders", L"Optional. TRUE (default) includes the header row" );


XLO_FUNC_START( aqBondObjectDirtyPrice(
    const ExcelObj& bondName,
    const ExcelObj& settlementDates,
    const ExcelObj& yields,
    const ExcelObj& yieldCalcType ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const std::string objectName = getNameWithoutCounter( bondName );

    const std::vector<double> results =
        validation::tryAqBondObjectDirtyPrice( objectName,
                                             toDateVector( settlementDates, true, "SettlementDates" ),
                                             toDoubleVector( yields, true, "Yields" ),
                                             toNarrowString( yieldCalcType ) );

    return returnValue( toExcelDoubleColumn( results ) );
}
XLO_FUNC_END( aqBondObjectDirtyPrice )
    .help( L"Return a bond's dirty price(s) for the given settlement date(s) and yield(s)." )
    .arg( L"BondName",        L"A bond handle returned by aqBondObjectCreate" )
    .arg( L"SettlementDates", L"Bond settlement date(s)" )
    .arg( L"Yields",          L"Bond yield(s)" )
    .arg( L"YieldCalcType",   L"Optional. Yield calculation type, e.g. ISMA, TRUE, SIMPLE" );


XLO_FUNC_START( aqBondObjectAccruedInterestDays(
    const ExcelObj& bondName,
    const ExcelObj& settlementDates ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const std::string objectName = getNameWithoutCounter( bondName );

    const std::vector<int> results =
        validation::tryAqBondObjectAccruedInterestDays( objectName,
                                                      toDateVector( settlementDates, true, "SettlementDates" ) );

    return returnValue( toExcelIntColumn( results ) );
}
XLO_FUNC_END( aqBondObjectAccruedInterestDays )
    .help( L"Return a bond's accrued interest day count(s) for the given settlement date(s)." )
    .arg( L"BondName",        L"A bond handle returned by aqBondObjectCreate" )
    .arg( L"SettlementDates", L"Bond settlement date(s)" );


XLO_FUNC_START( aqBondObjectCleanPrice(
    const ExcelObj& bondName,
    const ExcelObj& settlementDates,
    const ExcelObj& yields,
    const ExcelObj& yieldCalcType ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const std::string objectName = getNameWithoutCounter( bondName );

    const std::vector<double> results =
        validation::tryAqBondObjectCleanPrice( objectName,
                                             toDateVector( settlementDates, true, "SettlementDates" ),
                                             toDoubleVector( yields, true, "Yields" ),
                                             toNarrowString( yieldCalcType ) );

    return returnValue( toExcelDoubleColumn( results ) );
}
XLO_FUNC_END( aqBondObjectCleanPrice )
    .help( L"Return a bond's clean price(s) for the given settlement date(s) and yield(s)." )
    .arg( L"BondName",        L"A bond handle returned by aqBondObjectCreate" )
    .arg( L"SettlementDates", L"Bond settlement date(s)" )
    .arg( L"Yields",          L"Bond yield(s)" )
    .arg( L"YieldCalcType",   L"Optional. Yield calculation type, e.g. ISMA, TRUE, SIMPLE" );


XLO_FUNC_START( aqBondObjectAccruedInterest(
    const ExcelObj& bondName,
    const ExcelObj& valuationSettings ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const std::string objectName = getNameWithoutCounter( bondName );

    const double result =
        validation::tryAqBondObjectAccruedInterest( objectName,
                                                  toLabelValueBlock( valuationSettings ) );

    return returnValue( result );
}
XLO_FUNC_END( aqBondObjectAccruedInterest )
    .help( L"Return a bond's accrued interest. Set SettlementDate in the block; for an FRN also set FloatBondCoupon." )
    .arg( L"BondName",          L"A bond handle returned by aqBondObjectCreate" )
    .arg( L"ValuationSettings", L"A two-column (key, value) block; set SettlementDate (and FloatBondCoupon for an FRN)" );


XLO_FUNC_START( aqBondObjectYield(
    const ExcelObj& bondName,
    const ExcelObj& valuationSettings,
    const ExcelObj& price,
    const ExcelObj& yieldCalcType ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const std::string objectName = getNameWithoutCounter( bondName );

    const double result =
        validation::tryAqBondObjectYield( objectName,
                                        toLabelValueBlock( valuationSettings ),
                                        price.get<double>(),
                                        toNarrowString( yieldCalcType ) );

    return returnValue( result );
}
XLO_FUNC_END( aqBondObjectYield )
    .help( L"Return a bond's yield given its price. Set SettlementDate in the block (plus the FRN fields for a floater)." )
    .arg( L"BondName",          L"A bond handle returned by aqBondObjectCreate" )
    .arg( L"ValuationSettings", L"A two-column (key, value) block; set SettlementDate and, for an FRN, FloatBondCurrentCoupon / FloatBondAssumedRate / FloatBondIndexToNextCoupon / FloatBondQuotedMargin" )
    .arg( L"Price",             L"Bond price" )
    .arg( L"YieldCalcType",     L"Optional. Yield calculation type, e.g. ISMA, TRUE, SIMPLE" );


XLO_FUNC_START( aqBondObjectCompoundYield(
    const ExcelObj& bondName,
    const ExcelObj& settlementDates,
    const ExcelObj& prices,
    const ExcelObj& yieldCalcType ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const std::string objectName = getNameWithoutCounter( bondName );

    const std::vector<double> results =
        validation::tryAqBondObjectCompoundYield( objectName,
                                                 toDateVector( settlementDates, true, "SettlementDates" ),
                                                 toDoubleVector( prices, true, "Prices" ),
                                                 toNarrowString( yieldCalcType ) );

    return returnValue( toExcelDoubleColumn( results ) );
}
XLO_FUNC_END( aqBondObjectCompoundYield )
    .help( L"Return a bond's compound yield(s) for the given settlement date(s) and price(s)." )
    .arg( L"BondName",        L"A bond handle returned by aqBondObjectCreate" )
    .arg( L"SettlementDates", L"Bond settlement date(s)" )
    .arg( L"Prices",          L"Bond price(s)" )
    .arg( L"YieldCalcType",   L"Optional. Yield calculation type, e.g. ISMA, TRUE, SIMPLE" );


XLO_FUNC_START( aqBondObjectPrice(
    const ExcelObj& bondName,
    const ExcelObj& settlementDates,
    const ExcelObj& yields,
    const ExcelObj& yieldCalcType ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const std::string objectName = getNameWithoutCounter( bondName );

    const std::vector<double> results =
        validation::tryAqBondObjectPrice( objectName,
                                        toDateVector( settlementDates, true, "SettlementDates" ),
                                        toDoubleVector( yields, true, "Yields" ),
                                        toNarrowString( yieldCalcType ) );

    return returnValue( toExcelDoubleColumn( results ) );
}
XLO_FUNC_END( aqBondObjectPrice )
    .help( L"Return a bond's price(s) for the given settlement date(s) and yield(s)." )
    .arg( L"BondName",        L"A bond handle returned by aqBondObjectCreate" )
    .arg( L"SettlementDates", L"Bond settlement date(s)" )
    .arg( L"Yields",          L"Bond yield(s)" )
    .arg( L"YieldCalcType",   L"Optional. Yield calculation type, e.g. ISMA, TRUE, SIMPLE" );


XLO_FUNC_START( aqBondObjectDV01(
    const ExcelObj& bondName,
    const ExcelObj& settlementDates,
    const ExcelObj& yields,
    const ExcelObj& yieldCalcType ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const std::string objectName = getNameWithoutCounter( bondName );

    const std::vector<double> results =
        validation::tryAqBondObjectDV01( objectName,
                                       toDateVector( settlementDates, true, "SettlementDates" ),
                                       toDoubleVector( yields, true, "Yields" ),
                                       toNarrowString( yieldCalcType ) );

    return returnValue( toExcelDoubleColumn( results ) );
}
XLO_FUNC_END( aqBondObjectDV01 )
    .help( L"Return a bond's DV01(s) computed analytically for the given settlement date(s) and yield(s)." )
    .arg( L"BondName",        L"A bond handle returned by aqBondObjectCreate" )
    .arg( L"SettlementDates", L"Bond settlement date(s)" )
    .arg( L"Yields",          L"Bond yield(s)" )
    .arg( L"YieldCalcType",   L"Optional. Yield calculation type, e.g. ISMA, TRUE, SIMPLE" );


XLO_FUNC_START( aqBondObjectDV01Numerical(
    const ExcelObj& bondName,
    const ExcelObj& settlementDates,
    const ExcelObj& yields,
    const ExcelObj& bumpSize,
    const ExcelObj& bumpMode,
    const ExcelObj& yieldCalcType ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const std::string objectName = getNameWithoutCounter( bondName );

    // Defaults when omitted: bump 0.01 bp, mode "Up".
    const double    bumpSizeValue = bumpSize.isMissing() ? 0.01 : bumpSize.get<double>();
    const AQLString bumpModeValue = bumpMode.isMissing() ? AQLString( "Up" ) : toAQLString( bumpMode );

    const std::vector<double> results =
        validation::tryAqBondObjectDV01Numerical( objectName,
                                                toDateVector( settlementDates, true, "SettlementDates" ),
                                                toDoubleVector( yields, true, "Yields" ),
                                                bumpSizeValue,
                                                bumpModeValue,
                                                toNarrowString( yieldCalcType ) );

    return returnValue( toExcelDoubleColumn( results ) );
}
XLO_FUNC_END( aqBondObjectDV01Numerical )
    .help( L"Return a bond's DV01(s) by numerically bumping the yield." )
    .arg( L"BondName",        L"A bond handle returned by aqBondObjectCreate" )
    .arg( L"SettlementDates", L"Bond settlement date(s)" )
    .arg( L"Yields",          L"Bond yield(s)" )
    .arg( L"BumpSize",        L"Optional. Bump size in bp; default 0.01" )
    .arg( L"BumpMode",        L"Optional. 'Up', 'Down' or 'Central'; default 'Up'" )
    .arg( L"YieldCalcType",   L"Optional. Yield calculation type, e.g. ISMA, TRUE, SIMPLE" );


XLO_FUNC_START( aqBondObjectModifiedDuration(
    const ExcelObj& bondName,
    const ExcelObj& settlementDates,
    const ExcelObj& yields,
    const ExcelObj& yieldCalcType ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const std::string objectName = getNameWithoutCounter( bondName );

    const std::vector<double> results =
        validation::tryAqBondObjectModifiedDuration( objectName,
                                                   toDateVector( settlementDates, true, "SettlementDates" ),
                                                   toDoubleVector( yields, true, "Yields" ),
                                                   toNarrowString( yieldCalcType ) );

    return returnValue( toExcelDoubleColumn( results ) );
}
XLO_FUNC_END( aqBondObjectModifiedDuration )
    .help( L"Return a bond's modified duration(s), computed analytically, for the given settlement date(s) and yield(s)." )
    .arg( L"BondName",        L"A bond handle returned by aqBondObjectCreate" )
    .arg( L"SettlementDates", L"Bond settlement date(s)" )
    .arg( L"Yields",          L"Bond yield(s)" )
    .arg( L"YieldCalcType",   L"Optional. Yield calculation type, e.g. ISMA, TRUE, SIMPLE" );


XLO_FUNC_START( aqBondObjectOisSpread(
    const ExcelObj& bondName,
    const ExcelObj& marketDataName,
    const ExcelObj& settlementDates,
    const ExcelObj& bondYields ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const std::vector<double> results =
        validation::tryAqBondObjectOisSpread( getNameWithoutCounter( bondName ),
                                            getNameWithoutCounter( marketDataName ),
                                            toDateVector( settlementDates, true, "SettlementDates" ),
                                            toDoubleVector( bondYields, true, "BondYields" ) );

    return returnValue( toExcelDoubleColumn( results ) );
}
XLO_FUNC_END( aqBondObjectOisSpread )
    .help( L"Return the spread(s) of a bond's yield over the OIS par rate." )
    .arg( L"BondName",        L"A bond handle returned by aqBondObjectCreate" )
    .arg( L"MarketDataName",  L"An OIS market-data object handle" )
    .arg( L"SettlementDates", L"Bond settlement date(s)" )
    .arg( L"BondYields",      L"Bond yield(s)" );


XLO_FUNC_START( aqBondObjectLastCouponDate(
    const ExcelObj& bondName,
    const ExcelObj& settlementDates ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const std::vector<double> results =
        validation::tryAqBondObjectLastCouponDate( getNameWithoutCounter( bondName ),
                                                 toDateVector( settlementDates, true, "SettlementDates" ) );

    return returnValue( toExcelDoubleColumn( results ) );
}
XLO_FUNC_END( aqBondObjectLastCouponDate )
    .help( L"Return the bond's last coupon date(s) for the given settlement date(s). Format the cells as dates." )
    .arg( L"BondName",        L"A bond handle returned by aqBondObjectCreate" )
    .arg( L"SettlementDates", L"Bond settlement date(s) / valuation date(s)" );


XLO_FUNC_START( aqBondObjectPriceFromDirtyToClean(
    const ExcelObj& bondName,
    const ExcelObj& valuationSettings,
    const ExcelObj& dirtyPrice ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const double result =
        validation::tryAqBondObjectPriceFromDirtyToClean( getNameWithoutCounter( bondName ),
                                                        toLabelValueBlock( valuationSettings ),
                                                        dirtyPrice.get<double>() );

    return returnValue( result );
}
XLO_FUNC_END( aqBondObjectPriceFromDirtyToClean )
    .help( L"Return a bond's clean price given its dirty price. Set SettlementDate (and FloatBondCoupon for an FRN) in the block." )
    .arg( L"BondName",          L"A bond handle returned by aqBondObjectCreate" )
    .arg( L"ValuationSettings", L"A two-column (key, value) block" )
    .arg( L"DirtyPrice",        L"Bond dirty price" );


XLO_FUNC_START( aqBondObjectPriceFromCleanToDirty(
    const ExcelObj& bondName,
    const ExcelObj& valuationSettings,
    const ExcelObj& cleanPrice ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const double result =
        validation::tryAqBondObjectPriceFromCleanToDirty( getNameWithoutCounter( bondName ),
                                                        toLabelValueBlock( valuationSettings ),
                                                        cleanPrice.get<double>() );

    return returnValue( result );
}
XLO_FUNC_END( aqBondObjectPriceFromCleanToDirty )
    .help( L"Return a bond's dirty price given its clean price. Set SettlementDate (and FloatBondCoupon for an FRN) in the block." )
    .arg( L"BondName",          L"A bond handle returned by aqBondObjectCreate" )
    .arg( L"ValuationSettings", L"A two-column (key, value) block" )
    .arg( L"CleanPrice",        L"Bond clean price" );


XLO_FUNC_START( aqBondObjectZSpreadFromRates(
    const ExcelObj& bondName,
    const ExcelObj& settlementDate,
    const ExcelObj& bondPrice,
    const ExcelObj& zeroRates,
    const ExcelObj& continuouslyCompounding ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const double result =
        validation::tryAqBondObjectZSpreadFromRates( getNameWithoutCounter( bondName ),
                                                   toAQLDate( settlementDate ),
                                                   bondPrice.get<double>(),
                                                   toDoubleVector( zeroRates, true, "ZeroRates" ),
                                                   toBool( continuouslyCompounding, true ) );

    return returnValue( result );
}
XLO_FUNC_END( aqBondObjectZSpreadFromRates )
    .help( L"Return a bond's Z-spread against an external set of zero rates." )
    .arg( L"BondName",                L"A bond handle returned by aqBondObjectCreate" )
    .arg( L"SettlementDate",          L"Bond settlement / valuation date" )
    .arg( L"BondPrice",               L"Bond price (clean or dirty per the bond's convention)" )
    .arg( L"ZeroRates",               L"The external curve's zero rates" )
    .arg( L"ContinuouslyCompounding", L"Optional. TRUE (default) uses continuous compounding; FALSE uses discrete" );


XLO_FUNC_START( aqBondObjectZSpread(
    const ExcelObj& bondName,
    const ExcelObj& settlementDates,
    const ExcelObj& bondPrices,
    const ExcelObj& curveCollections,
    const ExcelObj& forecastCurves,
    const ExcelObj& continuouslyCompounding ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const std::vector<double> results =
        validation::tryAqBondObjectZSpreads( getNameWithoutCounter( bondName ),
                                           toDateVector( settlementDates, true, "SettlementDates" ),
                                           toDoubleVector( bondPrices, true, "BondPrices" ),
                                           toStringVector( curveCollections ),
                                           toStringVector( forecastCurves ),
                                           toBool( continuouslyCompounding, true ) );

    return returnValue( toExcelDoubleColumn( results ) );
}
XLO_FUNC_END( aqBondObjectZSpread )
    .help( L"Return a bond's Z-spread(s) against the given curve collection(s) and forecast curve(s)." )
    .arg( L"BondName",                L"A bond handle returned by aqBondObjectCreate" )
    .arg( L"SettlementDates",         L"Bond settlement / valuation date(s)" )
    .arg( L"BondPrices",              L"Bond price(s)" )
    .arg( L"CurveCollections",        L"Curve collection name(s)" )
    .arg( L"ForecastCurves",          L"Forecast curve name(s)" )
    .arg( L"ContinuouslyCompounding", L"Optional. TRUE (default) uses continuous compounding; FALSE uses discrete" );


XLO_FUNC_START( aqBondObjectForwardPrice(
    const ExcelObj& bondName,
    const ExcelObj& settleDate,
    const ExcelObj& forwardSettleDate,
    const ExcelObj& price,
    const ExcelObj& repoRate,
    const ExcelObj& repoDayCount ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const double result =
        validation::tryAqBondObjectForwardPrice( getNameWithoutCounter( bondName ),
                                               toAQLDate( settleDate ),
                                               toAQLDate( forwardSettleDate ),
                                               price.get<double>(),
                                               repoRate.get<double>(),
                                               toNarrowString( repoDayCount ) );

    return returnValue( result );
}
XLO_FUNC_END( aqBondObjectForwardPrice )
    .help( L"Return a bond's forward price implied by a repo rate. Actual repo rate -> fair forward price; implied repo rate -> actual forward price." )
    .arg( L"BondName",          L"A bond handle returned by aqBondObjectCreate" )
    .arg( L"SettleDate",        L"Bond settlement date" )
    .arg( L"ForwardSettleDate", L"Bond forward settlement date" )
    .arg( L"Price",             L"Bond price at the settlement date" )
    .arg( L"RepoRate",          L"Bond repo rate" )
    .arg( L"RepoDayCount",      L"Repo rate day count" );


XLO_FUNC_START( aqBondObjectRepoRate(
    const ExcelObj& bondName,
    const ExcelObj& settlementDate,
    const ExcelObj& forwardSettleDate,
    const ExcelObj& price,
    const ExcelObj& forwardPrice,
    const ExcelObj& repoDayCount ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const double result =
        validation::tryAqBondObjectRepoRate( getNameWithoutCounter( bondName ),
                                           toAQLDate( settlementDate ),
                                           toAQLDate( forwardSettleDate ),
                                           price.get<double>(),
                                           forwardPrice.get<double>(),
                                           toNarrowString( repoDayCount ) );

    return returnValue( result );
}
XLO_FUNC_END( aqBondObjectRepoRate )
    .help( L"Return a bond's repo rate implied from its forward price." )
    .arg( L"BondName",          L"A bond handle returned by aqBondObjectCreate" )
    .arg( L"SettlementDate",    L"Bond settlement date" )
    .arg( L"ForwardSettleDate", L"Bond forward settlement date" )
    .arg( L"Price",             L"Bond current price (clean or dirty)" )
    .arg( L"ForwardPrice",      L"Bond forward price (clean or dirty)" )
    .arg( L"RepoDayCount",      L"Repo day count" );


XLO_FUNC_START( aqBondObjectRepoRateFromFuture(
    const ExcelObj& bondName,
    const ExcelObj& settlementDate,
    const ExcelObj& deliveryDate,
    const ExcelObj& bondPrice,
    const ExcelObj& futurePrice,
    const ExcelObj& conversionFactor,
    const ExcelObj& repoDayCount ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const double result =
        validation::tryAqBondObjectRepoRateFromFuture( getNameWithoutCounter( bondName ),
                                                     toAQLDate( settlementDate ),
                                                     toAQLDate( deliveryDate ),
                                                     bondPrice.get<double>(),
                                                     futurePrice.get<double>(),
                                                     conversionFactor.get<double>(),
                                                     toNarrowString( repoDayCount ) );

    return returnValue( result );
}
XLO_FUNC_END( aqBondObjectRepoRateFromFuture )
    .help( L"Return a bond's repo rate implied from a future price. Actual/quoted future price -> implied repo rate; fair future price -> actual repo rate." )
    .arg( L"BondName",         L"A bond handle returned by aqBondObjectCreate" )
    .arg( L"SettlementDate",   L"Bond settlement date" )
    .arg( L"DeliveryDate",     L"Future delivery / final settlement date" )
    .arg( L"BondPrice",        L"Bond current price (clean or dirty)" )
    .arg( L"FuturePrice",      L"Actual / quoted future price" )
    .arg( L"ConversionFactor", L"Conversion factor" )
    .arg( L"RepoDayCount",     L"Repo day count" );


XLO_FUNC_START( aqBondObjectFuturePrice(
    const ExcelObj& bondName,
    const ExcelObj& settleDate,
    const ExcelObj& deliveryDate,
    const ExcelObj& bondPrice,
    const ExcelObj& repoRate,
    const ExcelObj& repoDayCount,
    const ExcelObj& conversionFactor ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const double result =
        validation::tryAqBondObjectFuturePrice( getNameWithoutCounter( bondName ),
                                              toAQLDate( settleDate ),
                                              toAQLDate( deliveryDate ),
                                              bondPrice.get<double>(),
                                              repoRate.get<double>(),
                                              toNarrowString( repoDayCount ),
                                              conversionFactor.get<double>() );

    return returnValue( result );
}
XLO_FUNC_END( aqBondObjectFuturePrice )
    .help( L"Return a bond's future price implied by a repo rate." )
    .arg( L"BondName",         L"A bond handle returned by aqBondObjectCreate" )
    .arg( L"SettleDate",       L"Bond settlement date" )
    .arg( L"DeliveryDate",     L"Bond delivery / final future settlement date" )
    .arg( L"BondPrice",        L"Bond price at the settlement date" )
    .arg( L"RepoRate",         L"Bond repo rate" )
    .arg( L"RepoDayCount",     L"Repo rate day count" )
    .arg( L"ConversionFactor", L"Bond's conversion factor against the future" );


XLO_FUNC_START( aqBondObjectConversionFactor(
    const ExcelObj& bondName,
    const ExcelObj& firstFutureSettleDate,
    const ExcelObj& notionalBondCouponRate ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const double result =
        validation::tryAqBondObjectConversionFactor( getNameWithoutCounter( bondName ),
                                                   toAQLDate( firstFutureSettleDate ),
                                                   notionalBondCouponRate.get<double>() );

    return returnValue( result );
}
XLO_FUNC_END( aqBondObjectConversionFactor )
    .help( L"Return a bond's conversion factor against a futures contract." )
    .arg( L"BondName",               L"A bond handle returned by aqBondObjectCreate" )
    .arg( L"FirstFutureSettleDate",  L"The future's first settlement date" )
    .arg( L"NotionalBondCouponRate", L"The future's notional coupon rate" );


XLO_FUNC_START( aqBondObjectGrossBasis(
    const ExcelObj& bondName,
    const ExcelObj& settleDate,
    const ExcelObj& bondPrice,
    const ExcelObj& futurePrice,
    const ExcelObj& conversionFactor ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const double result =
        validation::tryAqBondObjectGrossBasis( getNameWithoutCounter( bondName ),
                                             toAQLDate( settleDate ),
                                             bondPrice.get<double>(),
                                             futurePrice.get<double>(),
                                             conversionFactor.get<double>() );

    return returnValue( result );
}
XLO_FUNC_END( aqBondObjectGrossBasis )
    .help( L"Return a bond's gross basis against a futures contract." )
    .arg( L"BondName",         L"The future's underlying bond handle" )
    .arg( L"SettleDate",       L"Settlement date" )
    .arg( L"BondPrice",        L"Bond price at the settlement date" )
    .arg( L"FuturePrice",      L"Future price" )
    .arg( L"ConversionFactor", L"Conversion factor of the underlying bond against the future" );


XLO_FUNC_START( aqBondObjectNetBasis(
    const ExcelObj& bondName,
    const ExcelObj& settleDate,
    const ExcelObj& deliveryDate,
    const ExcelObj& bondPrice,
    const ExcelObj& actualRepoRate,
    const ExcelObj& repoDayCount,
    const ExcelObj& futurePrice,
    const ExcelObj& conversionFactor ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const double result =
        validation::tryAqBondObjectNetBasis( getNameWithoutCounter( bondName ),
                                           toAQLDate( settleDate ),
                                           toAQLDate( deliveryDate ),
                                           bondPrice.get<double>(),
                                           actualRepoRate.get<double>(),
                                           toNarrowString( repoDayCount ),
                                           futurePrice.get<double>(),
                                           conversionFactor.get<double>() );

    return returnValue( result );
}
XLO_FUNC_END( aqBondObjectNetBasis )
    .help( L"Return a bond's net basis against a futures contract." )
    .arg( L"BondName",         L"A bond handle returned by aqBondObjectCreate" )
    .arg( L"SettleDate",       L"Bond settlement date" )
    .arg( L"DeliveryDate",     L"Bond future's delivery date" )
    .arg( L"BondPrice",        L"Bond price at the settlement date" )
    .arg( L"ActualRepoRate",   L"Bond's actual repo rate" )
    .arg( L"RepoDayCount",     L"Repo rate day count" )
    .arg( L"FuturePrice",      L"Future price" )
    .arg( L"ConversionFactor", L"Bond's conversion factor against the future" );


XLO_FUNC_START( aqBondObjectCheapestToDeliver(
    const ExcelObj& futurePrice,
    const ExcelObj& settleDate,
    const ExcelObj& deliveryDate,
    const ExcelObj& repoDayCount,
    const ExcelObj& bondNames,
    const ExcelObj& bondPrices,
    const ExcelObj& conversionFactors ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const std::string result =
        validation::tryAqBondObjectCheapestToDeliver( futurePrice.get<double>(),
                                                    toAQLDate( settleDate ),
                                                    toAQLDate( deliveryDate ),
                                                    toNarrowString( repoDayCount ),
                                                    getNamesWithoutCounter( bondNames ),
                                                    toDoubleVector( bondPrices, true, "BondPrices" ),
                                                    toDoubleVector( conversionFactors, true, "ConversionFactors" ) );

    return returnValue( result );
}
XLO_FUNC_END( aqBondObjectCheapestToDeliver )
    .help( L"Return the cheapest-to-deliver bond name, identified by the highest implied repo rate." )
    .arg( L"FuturePrice",       L"Future price" )
    .arg( L"SettleDate",        L"Settlement date" )
    .arg( L"DeliveryDate",      L"Delivery / future final settlement date" )
    .arg( L"RepoDayCount",      L"Repo year-fraction day count" )
    .arg( L"BondNames",         L"A list of bond object handles" )
    .arg( L"BondPrices",        L"A list of bond prices at the settlement date" )
    .arg( L"ConversionFactors", L"A list of bond conversion factors" );


XLO_FUNC_START( aqBondObjectCheapestToDeliverByNetBasis(
    const ExcelObj& futurePrice,
    const ExcelObj& settleDate,
    const ExcelObj& deliveryDate,
    const ExcelObj& repoDayCount,
    const ExcelObj& bondNames,
    const ExcelObj& bondPrices,
    const ExcelObj& conversionFactors,
    const ExcelObj& actualRepoRates ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const std::string result =
        validation::tryAqBondObjectCheapestToDeliverByNetBasis( futurePrice.get<double>(),
                                                              toAQLDate( settleDate ),
                                                              toAQLDate( deliveryDate ),
                                                              toNarrowString( repoDayCount ),
                                                              getNamesWithoutCounter( bondNames ),
                                                              toDoubleVector( bondPrices, true, "BondPrices" ),
                                                              toDoubleVector( conversionFactors, true, "ConversionFactors" ),
                                                              toDoubleVector( actualRepoRates, true, "ActualRepoRates" ) );

    return returnValue( result );
}
XLO_FUNC_END( aqBondObjectCheapestToDeliverByNetBasis )
    .help( L"Return the cheapest-to-deliver bond name, identified by the lowest net basis." )
    .arg( L"FuturePrice",       L"Future price" )
    .arg( L"SettleDate",        L"Settlement date" )
    .arg( L"DeliveryDate",      L"Delivery / future final settlement date" )
    .arg( L"RepoDayCount",      L"Repo year-fraction day count" )
    .arg( L"BondNames",         L"A list of bond object handles" )
    .arg( L"BondPrices",        L"A list of bond prices at the settlement date" )
    .arg( L"ConversionFactors", L"A list of bond conversion factors" )
    .arg( L"ActualRepoRates",   L"A list of the bonds' actual repo rates" );


XLO_FUNC_START( aqBondObjectFRNPriceFromDiscountMargin(
    const ExcelObj& bondName,
    const ExcelObj& settlementDate,
    const ExcelObj& discountMargin,
    const ExcelObj& assumedRate,
    const ExcelObj& indexToNextCoupon,
    const ExcelObj& annualizedNextCouponRate ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const double result =
        validation::tryAqBondObjectFRNPriceFromDiscountMargin( getNameWithoutCounter( bondName ),
                                                             toAQLDate( settlementDate ),
                                                             discountMargin.get<double>(),
                                                             assumedRate.get<double>(),
                                                             indexToNextCoupon.get<double>(),
                                                             annualizedNextCouponRate.get<double>() );

    return returnValue( result );
}
XLO_FUNC_END( aqBondObjectFRNPriceFromDiscountMargin )
    .help( L"Return a floating-rate note's price from a discount margin, without discount or forecast curves (BBG CalcType 21)." )
    .arg( L"BondName",                 L"An FRN handle returned by aqBondObjectCreate" )
    .arg( L"SettlementDate",           L"Settlement date" )
    .arg( L"DiscountMargin",           L"Spread over the reference index rate for discounting" )
    .arg( L"AssumedRate",              L"Assumed index level for projected coupons" )
    .arg( L"IndexToNextCoupon",        L"Index discount rate for the next coupon period" )
    .arg( L"AnnualizedNextCouponRate", L"Annualized rate of the next (already fixed) coupon" );


XLO_FUNC_START( aqBondObjectFRNPriceFromYield(
    const ExcelObj& bondName,
    const ExcelObj& settlementDate,
    const ExcelObj& yield,
    const ExcelObj& assumedRate,
    const ExcelObj& indexToNextCoupon,
    const ExcelObj& annualizedNextCouponRate ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const double result =
        validation::tryAqBondObjectFRNPriceFromYield( getNameWithoutCounter( bondName ),
                                                    toAQLDate( settlementDate ),
                                                    yield.get<double>(),
                                                    assumedRate.get<double>(),
                                                    indexToNextCoupon.get<double>(),
                                                    annualizedNextCouponRate.get<double>() );

    return returnValue( result );
}
XLO_FUNC_END( aqBondObjectFRNPriceFromYield )
    .help( L"Return a floating-rate note's price from its yield to maturity, without discount or forecast curves (BBG CalcType 21)." )
    .arg( L"BondName",                 L"An FRN handle returned by aqBondObjectCreate" )
    .arg( L"SettlementDate",           L"Settlement date" )
    .arg( L"Yield",                    L"Bond yield to maturity" )
    .arg( L"AssumedRate",              L"Assumed index level for projected coupons" )
    .arg( L"IndexToNextCoupon",        L"Index discount rate for the next coupon period" )
    .arg( L"AnnualizedNextCouponRate", L"Annualized rate of the next (already fixed) coupon" );


XLO_FUNC_START( aqBondObjectFRNYieldFromPrice(
    const ExcelObj& bondName,
    const ExcelObj& settlementDate,
    const ExcelObj& price,
    const ExcelObj& assumedRate,
    const ExcelObj& indexToNextCoupon,
    const ExcelObj& annualizedNextCouponRate ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const double result =
        validation::tryAqBondObjectFRNYieldFromPrice( getNameWithoutCounter( bondName ),
                                                    toAQLDate( settlementDate ),
                                                    price.get<double>(),
                                                    assumedRate.get<double>(),
                                                    indexToNextCoupon.get<double>(),
                                                    annualizedNextCouponRate.get<double>() );

    return returnValue( result );
}
XLO_FUNC_END( aqBondObjectFRNYieldFromPrice )
    .help( L"Return a floating-rate note's yield from its quoted price, without discount or forecast curves (BBG CalcType 21)." )
    .arg( L"BondName",                 L"An FRN handle returned by aqBondObjectCreate" )
    .arg( L"SettlementDate",           L"Settlement date" )
    .arg( L"Price",                    L"Bond price in the quoted convention (clean or dirty)" )
    .arg( L"AssumedRate",              L"Assumed index level for projected coupons" )
    .arg( L"IndexToNextCoupon",        L"Index discount rate for the next coupon period" )
    .arg( L"AnnualizedNextCouponRate", L"Annualized rate of the next (already fixed) coupon" );


XLO_FUNC_START( aqBondObjectFRNDiscountMarginFromPrice(
    const ExcelObj& bondName,
    const ExcelObj& settlementDate,
    const ExcelObj& price,
    const ExcelObj& assumedRate,
    const ExcelObj& indexToNextCoupon,
    const ExcelObj& annualizedNextCouponRate ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const double result =
        validation::tryAqBondObjectFRNDiscountMarginFromPrice( getNameWithoutCounter( bondName ),
                                                             toAQLDate( settlementDate ),
                                                             price.get<double>(),
                                                             assumedRate.get<double>(),
                                                             indexToNextCoupon.get<double>(),
                                                             annualizedNextCouponRate.get<double>() );

    return returnValue( result );
}
XLO_FUNC_END( aqBondObjectFRNDiscountMarginFromPrice )
    .help( L"Return a floating-rate note's discount margin from its quoted price, without discount or forecast curves (BBG CalcType 21)." )
    .arg( L"BondName",                 L"An FRN handle returned by aqBondObjectCreate" )
    .arg( L"SettlementDate",           L"Settlement date" )
    .arg( L"Price",                    L"Bond price in the quoted convention (clean or dirty)" )
    .arg( L"AssumedRate",              L"Assumed index level for projected coupons" )
    .arg( L"IndexToNextCoupon",        L"Index discount rate for the next coupon period" )
    .arg( L"AnnualizedNextCouponRate", L"Annualized rate of the next (already fixed) coupon" );


XLO_FUNC_START( aqBondObjectPriceFromCreditModel(
    const ExcelObj& bondName,
    const ExcelObj& settlementDate,
    const ExcelObj& creditModelName ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const double result =
        validation::tryAqBondObjectPriceFromCreditModel( getNameWithoutCounter( bondName ),
                                                       toAQLDate( settlementDate ),
                                                       getNameWithoutCounter( creditModelName ) );

    return returnValue( result );
}
XLO_FUNC_END( aqBondObjectPriceFromCreditModel )
    .help( L"Return a bond's price implied by a credit model." )
    .arg( L"BondName",        L"A bond handle returned by aqBondObjectCreate" )
    .arg( L"SettlementDate",  L"Bond settlement / valuation date" )
    .arg( L"CreditModelName", L"A credit model object handle" );


XLO_FUNC_START( aqBondObjectBPVPerTick(
    const ExcelObj& bondName,
    const ExcelObj& valuationSettings,
    const ExcelObj& price,
    const ExcelObj& yieldCalcType,
    const ExcelObj& tickSize ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const double result =
        validation::tryAqBondObjectBPVPerTick( getNameWithoutCounter( bondName ),
                                             toLabelValueBlock( valuationSettings ),
                                             price.get<double>(),
                                             toNarrowString( yieldCalcType ),
                                             tickSize.isMissing() ? 0.01325 : tickSize.get<double>() );

    return returnValue( result );
}
XLO_FUNC_END( aqBondObjectBPVPerTick )
    .help( L"Return a bond's basis-point value per price tick. Set SettlementDate in the block." )
    .arg( L"BondName",          L"A bond handle returned by aqBondObjectCreate" )
    .arg( L"ValuationSettings", L"A two-column (key, value) block" )
    .arg( L"Price",             L"Bond price" )
    .arg( L"YieldCalcType",     L"Optional. Yield calculation type, e.g. ISMA, TRUE, SIMPLE" )
    .arg( L"TickSize",          L"Optional. Price tick size; default 0.01325" );


XLO_FUNC_START( aqBondAverageYield(
    const ExcelObj& underlyingBondYields ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const double result =
        validation::tryAqToolBondAverageYield( toDoubleVector( underlyingBondYields, true, "UnderlyingBondYields" ) );

    return returnValue( result );
}
XLO_FUNC_END( aqBondAverageYield )
    .help( L"Return the notional bond's yield as the average of the underlying bonds' yields." )
    .arg( L"UnderlyingBondYields", L"The bond future's underlying bonds' yields" );


XLO_FUNC_START( aqBondYieldFromFuturePrice(
    const ExcelObj& futurePrice ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const double result =
        validation::tryAqToolBondYieldFromFuturePrice( futurePrice.get<double>() );

    return returnValue( result );
}
XLO_FUNC_END( aqBondYieldFromFuturePrice )
    .help( L"Return the notional bond's yield implied by a bond future price." )
    .arg( L"FuturePrice", L"Bond future price" );


XLO_FUNC_START( aqBondObjectCreateFromLVB(
    const ExcelObj& bondName,
    const ExcelObj& bondLVB,
    const ExcelObj& validateKeys,
    const ExcelObj& verticalLVBKeys,
    const ExcelObj& allowUpdates ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const std::string objectName =
        decorateWithExcelLocation( toNarrowString( bondName ) );

    std::string existingHandle;
    if ( !allowAQObjUpdates( toBool( allowUpdates, true ),
                             existingHandle,
                             objectName,
                             etrading::BOND ) )
    {
        return returnValue( existingHandle );
    }

    const std::string storedName =
        validation::tryAqBondObjectCreateFromLVB( objectName,
                                                toLabelValueBlock( bondLVB, toBool( verticalLVBKeys, true ) ),
                                                toBool( validateKeys, true ) );

    return returnValue( appendInstanceCounter( storedName ) );
}
XLO_FUNC_END( aqBondObjectCreateFromLVB )
    .help( L"Create a bond object from a single (key, value) block and return its handle." )
    .arg( L"BondName",        L"Name for the bond object; the returned handle is this name plus a counter" )
    .arg( L"BondLVB",         L"Bond parameters as a key/value block" )
    .arg( L"ValidateKeys",    L"Optional. TRUE (default) rejects unknown keys" )
    .arg( L"VerticalLVBKeys", L"Optional. TRUE (default) reads the block as two columns (key, value); FALSE reads two rows (keys, values)" )
    .arg( L"AllowUpdates",    L"Optional. FALSE returns the existing object unchanged if the name is in use; TRUE (default) rebuilds it" );


XLO_FUNC_START( aqBondObjectCreateFromGenerator(
    const ExcelObj& bondName,
    const ExcelObj& bondGeneratorName,
    const ExcelObj& bondExpressionLVB,
    const ExcelObj& validateKeys,
    const ExcelObj& allowUpdates ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const std::string objectName =
        decorateWithExcelLocation( toNarrowString( bondName ) );

    std::string existingHandle;
    if ( !allowAQObjUpdates( toBool( allowUpdates, true ),
                             existingHandle,
                             objectName,
                             etrading::BOND ) )
    {
        return returnValue( existingHandle );
    }

    const std::string storedName =
        validation::tryAqBondObjectCreateFromGenerator( objectName,
                                                      getNameWithoutCounter( bondGeneratorName ),
                                                      toLabelValueBlock( bondExpressionLVB ),
                                                      toBool( validateKeys, true ) );

    return returnValue( appendInstanceCounter( storedName ) );
}
XLO_FUNC_END( aqBondObjectCreateFromGenerator )
    .help( L"Create a bond object from a bond generator and a (key, value) override block; return its handle." )
    .arg( L"BondName",           L"Name for the bond object; the returned handle is this name plus a counter" )
    .arg( L"BondGeneratorName",  L"A bond generator handle (see aqBondGeneratorCreate)" )
    .arg( L"BondExpressionLVB",  L"Override parameters as a two-column key/value block" )
    .arg( L"ValidateKeys",       L"Optional. TRUE (default) rejects unknown keys" )
    .arg( L"AllowUpdates",       L"Optional. FALSE returns the existing object unchanged if the name is in use; TRUE (default) rebuilds it" );


XLO_FUNC_START( aqBondObjectCreateAUDNotionalBond(
    const ExcelObj& bondName,
    const ExcelObj& settleDate,
    const ExcelObj& maturityDate,
    const ExcelObj& couponRate,
    const ExcelObj& payReceive,
    const ExcelObj& calendar,
    const ExcelObj& frequency,
    const ExcelObj& dayCount,
    const ExcelObj& bondQuoteConvention,
    const ExcelObj& allowUpdates ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const std::string objectName =
        decorateWithExcelLocation( toNarrowString( bondName ) );

    std::string existingHandle;
    if ( !allowAQObjUpdates( toBool( allowUpdates, true ),
                             existingHandle,
                             objectName,
                             etrading::BOND ) )
    {
        return returnValue( existingHandle );
    }

    const std::string storedName =
        validation::tryAqBondObjectCreateAUDNotionalBond( objectName,
                                                        toAQLDate( settleDate ),
                                                        toNarrowString( maturityDate ),
                                                        toNarrowString( couponRate ),
                                                        toNarrowString( payReceive ),
                                                        toNarrowString( calendar ),
                                                        toNarrowString( frequency ),
                                                        toNarrowString( dayCount ),
                                                        toNarrowString( bondQuoteConvention ) );

    return returnValue( appendInstanceCounter( storedName ) );
}
XLO_FUNC_END( aqBondObjectCreateAUDNotionalBond )
    .help( L"Create the notional bond of an AUD bond future and return its handle." )
    .arg( L"BondName",            L"Notional bond object name; the returned handle is this name plus a counter" )
    .arg( L"SettleDate",          L"Settle date" )
    .arg( L"MaturityDate",        L"Maturity date or tenor" )
    .arg( L"CouponRate",          L"Optional. Notional bond coupon rate; default 6%" )
    .arg( L"PayReceive",          L"Optional. Pay/receive flag; default RECEIVE" )
    .arg( L"Calendar",            L"Optional. Calendar; default SYB" )
    .arg( L"Frequency",           L"Optional. Coupon frequency; default SEMI-ANNUAL" )
    .arg( L"DayCount",            L"Optional. Day count; default ACT/ACT" )
    .arg( L"BondQuoteConvention", L"Optional. Bond quote convention; default blank (AUD bonds are QUOTE_TO_3_DECIMAL_PLACES)" )
    .arg( L"AllowUpdates",        L"Optional. FALSE returns the existing object unchanged if the name is in use; TRUE (default) rebuilds it" );


XLO_FUNC_START( aqBondGeneratorCreate(
    const ExcelObj& bondGeneratorName,
    const ExcelObj& key1,
    const ExcelObj& value1,
    const ExcelObj& key2,
    const ExcelObj& value2 ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const std::string objectName =
        decorateWithExcelLocation( toNarrowString( bondGeneratorName ) );

    std::vector<std::string>          propertyNames;
    std::vector<validation::TableInfo> infoBlocks;

    propertyNames.push_back( etrading::trim_to_upper( toNarrowString( key1 ) ) );
    infoBlocks.push_back( toTableInfo( value1 ) );

    if ( !value2.isMissing() && value2.isNonEmpty() )
    {
        propertyNames.push_back( etrading::trim_to_upper( toNarrowString( key2 ) ) );
        infoBlocks.push_back( toTableInfo( value2 ) );
    }

    const std::string storedName =
        validation::tryAqBondGeneratorCreate( objectName, propertyNames, infoBlocks );

    return returnValue( appendInstanceCounter( storedName ) );
}
XLO_FUNC_END( aqBondGeneratorCreate )
    .help( L"Create and store a bond generator from one or two named configuration blocks; returns its handle." )
    .arg( L"BondGeneratorName", L"Name for the bond generator object" )
    .arg( L"Key1",              L"Name of the first configuration block" )
    .arg( L"Value1",            L"First configuration block, as a range of property values" )
    .arg( L"Key2",              L"Optional. Name of the second configuration block" )
    .arg( L"Value2",            L"Optional. Second configuration block, as a range of property values" );


XLO_FUNC_START( aqBondGeneratorDisplay(
    const ExcelObj& bondGeneratorName,
    const ExcelObj& propertyKey ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const etrading::VariantMatrix result =
        validation::tryAqBondGeneratorDisplay( getNameWithoutCounter( bondGeneratorName ),
                                                   etrading::trim_to_upper( toNarrowString( propertyKey ) ) );

    return returnValue( toExcelMatrix( etrading::toAQLStringMatrixFromVariantMatrix( result, false ) ) );
}
XLO_FUNC_END( aqBondGeneratorDisplay )
    .help( L"Return a bond generator's configuration as a key/value block. Omit PropertyKey to return every block." )
    .arg( L"BondGeneratorName", L"A bond generator handle" )
    .arg( L"PropertyKey",       L"Optional. The configuration block to return; blank returns all blocks" );
