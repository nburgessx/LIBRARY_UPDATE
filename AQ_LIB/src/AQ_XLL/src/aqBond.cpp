/*
 * Bond category - xlOil worksheet functions.
 *
 * aqBond*        - stateless: data in, value out.
 * aqBondObject*  - operate on a cached bond object (name in).
 * aqBondCurve* / aqBondGenerator* - operate on a cached named sub-object.
 *
 * Also holds the BondOption and BondFutureOption categories
 * (aqBondOptionObject* / aqBondFutureOptionObject*) - kept in this file rather
 * than their own aqBondOption.cpp / aqBondFutureOption.cpp because they share
 * the aqBond* prefix and, more to the point, operate on the exact same cached
 * object: aqBondOptionObjectCreate is the only creator for both - there is no
 * separate aqBondFutureOptionObjectCreate. tryAqBondFutureOptionObjectPV/
 * Greeks call etrading::getOption() and dynamic_pointer_cast to the same
 * etrading::BondOption the create call registered, then price it against a
 * bond-future price instead of a bond spot price. The two categories
 * themselves are unchanged (BondOption / BondFutureOption stay separate
 * entries in the locked category list) - only their code location moved.
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
#include <tryAqBondCurves.h>      // validation::tryAqBondCurve* + PriceFromBondCurve / YieldFromBondCurve
#include <tryAqBondSchedule.h>    // validation::tryAqBondSchedule / tryAqBondScheduleLVBKeys
#include <tryAqBondOptionObject.h>// validation::tryAqBondOptionObject* / tryAqBondFutureOptionObject*
#include <AQObjUtilities.h>       // etrading::getBond
#include <BondCurves.h>           // etrading::NelsonSiegelSvenssonParameters
#include <NelsonSiegelFitting.h>  // etrading::NelsonSiegelSvenssonCalibrationResults
#include <PolynomialFitting.h>    // etrading::PolynomialCalibrationResults

using namespace aq_xll;

namespace
{
    // Read a fixed-length column of doubles into a Nelson-Siegel / Svensson
    // parameter block. NS uses beta0..lambda1; Svensson also uses beta3, lambda2.
    etrading::NelsonSiegelSvenssonParameters nssParamsFromRange( const xloil::ExcelObj& range )
    {
        const std::vector<double> v = toDoubleVector( range, true, "InitialGuess" );
        etrading::NelsonSiegelSvenssonParameters p = {};
        p.beta0_   = v.size() > 0 ? v[0] : 0.0;
        p.beta1_   = v.size() > 1 ? v[1] : 0.0;
        p.beta2_   = v.size() > 2 ? v[2] : 0.0;
        p.lambda1_ = v.size() > 3 ? v[3] : 0.0;
        p.beta3_   = v.size() > 4 ? v[4] : 0.0;
        p.lambda2_ = v.size() > 5 ? v[5] : 0.0;
        return p;
    }

    // A calibrated Nelson-Siegel / Svensson result as a (label, value) block.
    xloil::ExcelObj nssResultBlock( const etrading::NelsonSiegelSvenssonCalibrationResults& r, bool isSvensson )
    {
        AQLStringMatrix m;
        auto row = []( const char* k, double val ) {
            AQLStringVector r;
            r.push_back( AQLString( k ) );
            r.push_back( AQLString( val, 10 ) );
            return r;
        };
        m.push_back( row( "beta0",   r.parameters_.beta0_ ) );
        m.push_back( row( "beta1",   r.parameters_.beta1_ ) );
        m.push_back( row( "beta2",   r.parameters_.beta2_ ) );
        m.push_back( row( "lambda1", r.parameters_.lambda1_ ) );
        if ( isSvensson )
        {
            m.push_back( row( "beta3",   r.parameters_.beta3_ ) );
            m.push_back( row( "lambda2", r.parameters_.lambda2_ ) );
        }
        m.push_back( row( "leastSquaresError", r.leastSquaresError_ ) );
        m.push_back( row( "iterations",        r.iterations_ ) );
        return toExcelMatrix( m );
    }
}


#if AQ_XLL_ENABLED(aqBondObjectCreate)
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
#endif


#if AQ_XLL_ENABLED(aqBondObjectDisplay)
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
#endif


#if AQ_XLL_ENABLED(aqBondObjectDisplaySchedule)
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
#endif


#if AQ_XLL_ENABLED(aqBondObjectDisplayCashflows)
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
#endif


#if AQ_XLL_ENABLED(aqBondObjectDirtyPrice)
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
#endif


#if AQ_XLL_ENABLED(aqBondObjectAccruedInterestDays)
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
#endif


#if AQ_XLL_ENABLED(aqBondObjectCleanPrice)
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
#endif


#if AQ_XLL_ENABLED(aqBondObjectAccruedInterest)
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
#endif


#if AQ_XLL_ENABLED(aqBondObjectYield)
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
#endif


#if AQ_XLL_ENABLED(aqBondObjectCompoundYield)
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
#endif


#if AQ_XLL_ENABLED(aqBondObjectPrice)
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
#endif


#if AQ_XLL_ENABLED(aqBondObjectDV01)
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
#endif


#if AQ_XLL_ENABLED(aqBondObjectDV01Numerical)
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
#endif


#if AQ_XLL_ENABLED(aqBondObjectModifiedDuration)
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
#endif


#if AQ_XLL_ENABLED(aqBondObjectOisSpread)
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
#endif


#if AQ_XLL_ENABLED(aqBondObjectLastCouponDate)
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
#endif


#if AQ_XLL_ENABLED(aqBondObjectPriceFromDirtyToClean)
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
#endif


#if AQ_XLL_ENABLED(aqBondObjectPriceFromCleanToDirty)
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
#endif


#if AQ_XLL_ENABLED(aqBondObjectZSpreadFromRates)
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
#endif


#if AQ_XLL_ENABLED(aqBondObjectZSpread)
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
#endif


#if AQ_XLL_ENABLED(aqBondObjectForwardPrice)
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
#endif


#if AQ_XLL_ENABLED(aqBondObjectRepoRate)
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
#endif


#if AQ_XLL_ENABLED(aqBondObjectRepoRateFromFuture)
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
#endif


#if AQ_XLL_ENABLED(aqBondObjectFuturePrice)
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
#endif


#if AQ_XLL_ENABLED(aqBondObjectConversionFactor)
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
#endif


#if AQ_XLL_ENABLED(aqBondObjectGrossBasis)
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
#endif


#if AQ_XLL_ENABLED(aqBondObjectNetBasis)
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
#endif


#if AQ_XLL_ENABLED(aqBondObjectCheapestToDeliver)
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
#endif


#if AQ_XLL_ENABLED(aqBondObjectCheapestToDeliverByNetBasis)
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
#endif


#if AQ_XLL_ENABLED(aqBondObjectFRNPriceFromDiscountMargin)
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
#endif


#if AQ_XLL_ENABLED(aqBondObjectFRNPriceFromYield)
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
#endif


#if AQ_XLL_ENABLED(aqBondObjectFRNYieldFromPrice)
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
#endif


#if AQ_XLL_ENABLED(aqBondObjectFRNDiscountMarginFromPrice)
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
#endif


#if AQ_XLL_ENABLED(aqBondObjectPriceFromCreditModel)
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
#endif


#if AQ_XLL_ENABLED(aqBondObjectBPVPerTick)
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
#endif


#if AQ_XLL_ENABLED(aqBondAverageYield)
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
#endif


#if AQ_XLL_ENABLED(aqBondYieldFromFuturePrice)
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
#endif


#if AQ_XLL_ENABLED(aqBondObjectCreateFromLVB)
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
#endif


#if AQ_XLL_ENABLED(aqBondObjectCreateFromGenerator)
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
#endif


#if AQ_XLL_ENABLED(aqBondObjectCreateAUDNotionalBond)
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
#endif


#if AQ_XLL_ENABLED(aqBondGeneratorCreate)
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
#endif


#if AQ_XLL_ENABLED(aqBondGeneratorDisplay)
XLO_FUNC_START( aqBondGeneratorDisplay(
    const ExcelObj& bondGeneratorName,
    const ExcelObj& propertyKey ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const etrading::VariantMatrix result =
        validation::tryAqBondGeneratorDisplay( getNameWithoutCounter( bondGeneratorName ),
                                                   etrading::trim_to_upper( toNarrowString( propertyKey ) ) );

    // Transposed: the underlying block is column-major (one column per
    // key/value pair); Excel users expect key/value rows instead.
    return returnValue( toExcelMatrix( etrading::toAQLStringMatrixFromVariantMatrix( result, true ) ) );
}
XLO_FUNC_END( aqBondGeneratorDisplay )
    .help( L"Return a bond generator's configuration as a key/value block. Omit PropertyKey to return every block." )
    .arg( L"BondGeneratorName", L"A bond generator handle" )
    .arg( L"PropertyKey",       L"Optional. The configuration block to return; blank returns all blocks" );
#endif


/* -------------------------------------------------------------------------
 *  Bond curve (fitted yield curve for a bond universe)
 * ---------------------------------------------------------------------- */

// Create and store a fitted bond curve from one or two named data blocks.
#if AQ_XLL_ENABLED(aqBondCurveCreate)
XLO_FUNC_START( aqBondCurveCreate(
    const ExcelObj& bondCurveName,
    const ExcelObj& key1,
    const ExcelObj& value1,
    const ExcelObj& key2,
    const ExcelObj& value2 ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const std::string objectName =
        decorateWithExcelLocation( toNarrowString( bondCurveName ) );

    std::vector<std::string>           dataBlockNames;
    std::vector<validation::TableInfo> infoBlocks;

    dataBlockNames.push_back( etrading::trim_to_upper( toNarrowString( key1 ) ) );
    infoBlocks.push_back( toTableInfo( value1 ) );

    if ( !value2.isMissing() && value2.isNonEmpty() )
    {
        dataBlockNames.push_back( etrading::trim_to_upper( toNarrowString( key2 ) ) );
        infoBlocks.push_back( toTableInfo( value2 ) );
    }

    const std::string storedName =
        validation::tryAqBondCurveCreate( objectName, dataBlockNames, infoBlocks );

    return returnValue( appendInstanceCounter( storedName ) );
}
XLO_FUNC_END( aqBondCurveCreate )
    .help( L"Create and store a fitted bond curve from one or two named data blocks; returns its handle." )
    .arg( L"BondCurveName", L"Name for the bond curve object" )
    .arg( L"Key1",          L"Name of the first data block" )
    .arg( L"Value1",        L"First data block, as a range" )
    .arg( L"Key2",          L"Optional. Name of the second data block" )
    .arg( L"Value2",        L"Optional. Second data block, as a range" );
#endif


// Display a stored bond curve as a matrix.
#if AQ_XLL_ENABLED(aqBondCurveDisplay)
XLO_FUNC_START( aqBondCurveDisplay(
    const ExcelObj& bondCurveName ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelMatrix(
        validation::tryAqBondCurveDisplay( getNameWithoutCounter( bondCurveName ) ) ) );
}
XLO_FUNC_END( aqBondCurveDisplay )
    .help( L"Display a stored bond curve as a matrix." )
    .arg( L"BondCurveName", L"A bond curve handle" );
#endif


// Interpolated yield off a stored bond curve at a reference date.
#if AQ_XLL_ENABLED(aqBondCurveYield)
XLO_FUNC_START( aqBondCurveYield(
    const ExcelObj& bondCurveName,
    const ExcelObj& referenceDate ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqBondCurveYield(
        getNameWithoutCounter( bondCurveName ), toAQLDate( referenceDate ) ) );
}
XLO_FUNC_END( aqBondCurveYield )
    .help( L"Interpolated yield off a stored bond curve at a reference date." )
    .arg( L"BondCurveName", L"A bond curve handle" )
    .arg( L"ReferenceDate", L"The date to read the yield at" );
#endif


// Calibrate Nelson-Siegel parameters to a set of (maturity, yield) points.
#if AQ_XLL_ENABLED(aqBondCurveNelsonSiegelCalibrate)
XLO_FUNC_START( aqBondCurveNelsonSiegelCalibrate(
    const ExcelObj& bondMaturities,
    const ExcelObj& bondYields,
    const ExcelObj& initialGuess,
    const ExcelObj& maxIterations,
    const ExcelObj& maxStationaryStateIterations,
    const ExcelObj& lowerBounds,
    const ExcelObj& upperBounds ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const etrading::NelsonSiegelSvenssonCalibrationResults r =
        validation::tryAqBondCurveNelsonSiegelCalibrate(
            toDoubleVector( bondMaturities, true, "BondMaturities" ),
            toDoubleVector( bondYields, true, "BondYields" ),
            nssParamsFromRange( initialGuess ),
            static_cast<unsigned int>( maxIterations.get<double>() ),
            static_cast<unsigned int>( maxStationaryStateIterations.get<double>() ),
            toDoubleVector( lowerBounds, true, "LowerBounds" ),
            toDoubleVector( upperBounds, true, "UpperBounds" ) );

    return returnValue( nssResultBlock( r, false ) );
}
XLO_FUNC_END( aqBondCurveNelsonSiegelCalibrate )
    .help( L"Calibrate Nelson-Siegel parameters to (maturity, yield) points. Returns a key/value block." )
    .arg( L"BondMaturities",               L"Column of bond maturities in years" )
    .arg( L"BondYields",                   L"Column of bond yields, aligned with BondMaturities" )
    .arg( L"InitialGuess",                 L"4 starting values: beta0, beta1, beta2, lambda1" )
    .arg( L"MaxIterations",                L"Maximum optimiser iterations" )
    .arg( L"MaxStationaryStateIterations", L"Maximum iterations with no improvement before stopping" )
    .arg( L"LowerBounds",                  L"4 lower bounds, aligned with InitialGuess" )
    .arg( L"UpperBounds",                  L"4 upper bounds, aligned with InitialGuess" );
#endif


// Nelson-Siegel yields for a set of maturities from fitted parameters.
#if AQ_XLL_ENABLED(aqBondCurveNelsonSiegelYield)
XLO_FUNC_START( aqBondCurveNelsonSiegelYield(
    const ExcelObj& beta0,
    const ExcelObj& beta1,
    const ExcelObj& beta2,
    const ExcelObj& lambda,
    const ExcelObj& bondMaturities ) )
{
    AQ_XLL_GUARD

    return returnValue( toExcelDoubleColumn( validation::tryAqBondCurveNelsonSiegelYield(
        beta0.get<double>(), beta1.get<double>(), beta2.get<double>(), lambda.get<double>(),
        toDoubleVector( bondMaturities, true, "BondMaturities" ) ) ) );
}
XLO_FUNC_END( aqBondCurveNelsonSiegelYield )
    .help( L"Nelson-Siegel yields for a column of maturities from fitted parameters." )
    .arg( L"Beta0",          L"Long-term yield level" )
    .arg( L"Beta1",          L"Slope" )
    .arg( L"Beta2",          L"Curvature" )
    .arg( L"Lambda",         L"Time-decay" )
    .arg( L"BondMaturities", L"Column of maturities in years" );
#endif


// Calibrate polynomial coefficients to a set of (maturity, yield) points.
#if AQ_XLL_ENABLED(aqBondCurvePolynomialCalibrate)
XLO_FUNC_START( aqBondCurvePolynomialCalibrate(
    const ExcelObj& polynomialOrder,
    const ExcelObj& bondMaturities,
    const ExcelObj& bondYields,
    const ExcelObj& maxIterations,
    const ExcelObj& maxStationaryStateIterations,
    const ExcelObj& lowerBound,
    const ExcelObj& upperBound ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const etrading::PolynomialCalibrationResults r =
        validation::tryAqBondCurvePolynomialCalibrate(
            static_cast<unsigned int>( polynomialOrder.get<double>() ),
            toDoubleVector( bondMaturities, true, "BondMaturities" ),
            toDoubleVector( bondYields, true, "BondYields" ),
            static_cast<unsigned int>( maxIterations.get<double>() ),
            static_cast<unsigned int>( maxStationaryStateIterations.get<double>() ),
            lowerBound.get<double>(),
            upperBound.get<double>() );

    AQLStringMatrix m;
    for ( std::size_t i = 0; i < r.coefficients_.size(); ++i )
    {
        AQLStringVector row;
        row.push_back( AQLString( ( std::string( "c" ) + std::to_string( i ) ).c_str() ) );
        row.push_back( AQLString( r.coefficients_[ i ], 10 ) );
        m.push_back( row );
    }
    {
        AQLStringVector errRow;
        errRow.push_back( AQLString( "leastSquaresError" ) );
        errRow.push_back( AQLString( r.leastSquaresError_, 10 ) );
        m.push_back( errRow );
    }
    {
        AQLStringVector itRow;
        itRow.push_back( AQLString( "iterations" ) );
        itRow.push_back( AQLString( r.iterations_, 0 ) );
        m.push_back( itRow );
    }

    return returnValue( toExcelMatrix( m ) );
}
XLO_FUNC_END( aqBondCurvePolynomialCalibrate )
    .help( L"Calibrate polynomial coefficients to (maturity, yield) points. Returns a key/value block." )
    .arg( L"PolynomialOrder",              L"Order of the fitting polynomial" )
    .arg( L"BondMaturities",               L"Column of bond maturities in years" )
    .arg( L"BondYields",                   L"Column of bond yields, aligned with BondMaturities" )
    .arg( L"MaxIterations",                L"Maximum optimiser iterations" )
    .arg( L"MaxStationaryStateIterations", L"Maximum iterations with no improvement before stopping" )
    .arg( L"LowerBound",                   L"Lower bound applied to every coefficient" )
    .arg( L"UpperBound",                   L"Upper bound applied to every coefficient" );
#endif


// Polynomial yields for a set of maturities from fitted coefficients.
#if AQ_XLL_ENABLED(aqBondCurvePolynomialYield)
XLO_FUNC_START( aqBondCurvePolynomialYield(
    const ExcelObj& coefficients,
    const ExcelObj& bondMaturities ) )
{
    AQ_XLL_GUARD

    return returnValue( toExcelDoubleColumn( validation::tryAqBondCurvePolynomialYield(
        toDoubleVector( coefficients, true, "Coefficients" ),
        toDoubleVector( bondMaturities, true, "BondMaturities" ) ) ) );
}
XLO_FUNC_END( aqBondCurvePolynomialYield )
    .help( L"Polynomial yields for a column of maturities from fitted coefficients." )
    .arg( L"Coefficients",   L"Column of polynomial coefficients, lowest order first" )
    .arg( L"BondMaturities", L"Column of maturities in years" );
#endif


// Calibrate Svensson parameters to a set of (maturity, yield) points.
#if AQ_XLL_ENABLED(aqBondCurveSvenssonCalibrate)
XLO_FUNC_START( aqBondCurveSvenssonCalibrate(
    const ExcelObj& bondMaturities,
    const ExcelObj& bondYields,
    const ExcelObj& initialGuess,
    const ExcelObj& maxIterations,
    const ExcelObj& maxStationaryStateIterations,
    const ExcelObj& lowerBounds,
    const ExcelObj& upperBounds ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const etrading::NelsonSiegelSvenssonCalibrationResults r =
        validation::tryAqBondCurveSvenssonCalibrate(
            toDoubleVector( bondMaturities, true, "BondMaturities" ),
            toDoubleVector( bondYields, true, "BondYields" ),
            nssParamsFromRange( initialGuess ),
            static_cast<unsigned int>( maxIterations.get<double>() ),
            static_cast<unsigned int>( maxStationaryStateIterations.get<double>() ),
            toDoubleVector( lowerBounds, true, "LowerBounds" ),
            toDoubleVector( upperBounds, true, "UpperBounds" ) );

    return returnValue( nssResultBlock( r, true ) );
}
XLO_FUNC_END( aqBondCurveSvenssonCalibrate )
    .help( L"Calibrate Svensson parameters to (maturity, yield) points. Returns a key/value block." )
    .arg( L"BondMaturities",               L"Column of bond maturities in years" )
    .arg( L"BondYields",                   L"Column of bond yields, aligned with BondMaturities" )
    .arg( L"InitialGuess",                 L"6 starting values: beta0, beta1, beta2, lambda1, beta3, lambda2" )
    .arg( L"MaxIterations",                L"Maximum optimiser iterations" )
    .arg( L"MaxStationaryStateIterations", L"Maximum iterations with no improvement before stopping" )
    .arg( L"LowerBounds",                  L"6 lower bounds, aligned with InitialGuess" )
    .arg( L"UpperBounds",                  L"6 upper bounds, aligned with InitialGuess" );
#endif


// Svensson yields for a set of maturities from fitted parameters.
#if AQ_XLL_ENABLED(aqBondCurveSvenssonYield)
XLO_FUNC_START( aqBondCurveSvenssonYield(
    const ExcelObj& beta0,
    const ExcelObj& beta1,
    const ExcelObj& beta2,
    const ExcelObj& beta3,
    const ExcelObj& lambda1,
    const ExcelObj& lambda2,
    const ExcelObj& bondMaturities ) )
{
    AQ_XLL_GUARD

    return returnValue( toExcelDoubleColumn( validation::tryAqBondCurveSvenssonYield(
        beta0.get<double>(), beta1.get<double>(), beta2.get<double>(), beta3.get<double>(),
        lambda1.get<double>(), lambda2.get<double>(),
        toDoubleVector( bondMaturities, true, "BondMaturities" ) ) ) );
}
XLO_FUNC_END( aqBondCurveSvenssonYield )
    .help( L"Svensson yields for a column of maturities from fitted parameters." )
    .arg( L"Beta0",          L"Long-term yield level" )
    .arg( L"Beta1",          L"Slope" )
    .arg( L"Beta2",          L"Curvature" )
    .arg( L"Beta3",          L"Secondary curvature" )
    .arg( L"Lambda1",        L"Time-decay" )
    .arg( L"Lambda2",        L"Secondary time-decay" )
    .arg( L"BondMaturities", L"Column of maturities in years" );
#endif


/* -------------------------------------------------------------------------
 *  Bond object - additional pricing / yield functions
 * ---------------------------------------------------------------------- */

// Price a cached bond off a fitted bond curve at a settlement date.
#if AQ_XLL_ENABLED(aqBondObjectPriceFromBondCurve)
XLO_FUNC_START( aqBondObjectPriceFromBondCurve(
    const ExcelObj& bondObjectName,
    const ExcelObj& settlementDate,
    const ExcelObj& bondCurveName ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqBondObjectPriceFromBondCurve(
        getNameWithoutCounter( bondObjectName ),
        toAQLDate( settlementDate ),
        getNameWithoutCounter( bondCurveName ) ) );
}
XLO_FUNC_END( aqBondObjectPriceFromBondCurve )
    .help( L"Price a cached bond off a fitted bond curve at a settlement date." )
    .arg( L"BondObjectName", L"A bond handle" )
    .arg( L"SettlementDate", L"The settlement date" )
    .arg( L"BondCurveName",  L"A bond curve handle" );
#endif


// Yield of a cached bond implied by a fitted bond curve at a settlement date.
#if AQ_XLL_ENABLED(aqBondObjectYieldFromBondCurve)
XLO_FUNC_START( aqBondObjectYieldFromBondCurve(
    const ExcelObj& bondObjectName,
    const ExcelObj& settlementDate,
    const ExcelObj& bondCurveName ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqBondObjectYieldFromBondCurve(
        getNameWithoutCounter( bondObjectName ),
        toAQLDate( settlementDate ),
        getNameWithoutCounter( bondCurveName ) ) );
}
XLO_FUNC_END( aqBondObjectYieldFromBondCurve )
    .help( L"Yield of a cached bond implied by a fitted bond curve at a settlement date." )
    .arg( L"BondObjectName", L"A bond handle" )
    .arg( L"SettlementDate", L"The settlement date" )
    .arg( L"BondCurveName",  L"A bond curve handle" );
#endif


// Forward reinvested coupon for a cached bond over a repo period.
#if AQ_XLL_ENABLED(aqBondObjectForwardReinvestedCoupon)
XLO_FUNC_START( aqBondObjectForwardReinvestedCoupon(
    const ExcelObj& bondObjectName,
    const ExcelObj& settleDate,
    const ExcelObj& forwardSettleDate,
    const ExcelObj& price,
    const ExcelObj& repoRate,
    const ExcelObj& repoDayCount ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqBondObjectForwardReinvestedCoupon(
        getNameWithoutCounter( bondObjectName ),
        toAQLDate( settleDate ),
        toAQLDate( forwardSettleDate ),
        price.get<double>(),
        repoRate.get<double>(),
        toNarrowString( repoDayCount ) ) );
}
XLO_FUNC_END( aqBondObjectForwardReinvestedCoupon )
    .help( L"Forward reinvested coupon for a cached bond over a repo period." )
    .arg( L"BondObjectName",    L"A bond handle" )
    .arg( L"SettleDate",        L"The spot settlement date" )
    .arg( L"ForwardSettleDate", L"The forward settlement date" )
    .arg( L"Price",             L"The spot price" )
    .arg( L"RepoRate",          L"The repo rate over the period" )
    .arg( L"RepoDayCount",      L"Day count for the repo accrual, e.g. ACT/360" );
#endif


// Quote (yield -> price) for a cached bond at one or more settlement dates.
#if AQ_XLL_ENABLED(aqBondObjectQuote)
XLO_FUNC_START( aqBondObjectQuote(
    const ExcelObj& bondObjectName,
    const ExcelObj& settlementDates,
    const ExcelObj& yields,
    const ExcelObj& yieldCalculationType ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelDoubleColumn( validation::tryAqBondObjectQuote(
        getNameWithoutCounter( bondObjectName ),
        toDateVector( settlementDates, true, "SettlementDates" ),
        toDoubleVector( yields, true, "Yields" ),
        toNarrowString( yieldCalculationType ) ) ) );
}
XLO_FUNC_END( aqBondObjectQuote )
    .help( L"Quote (yield to price) for a cached bond at one or more settlement dates." )
    .arg( L"BondObjectName",       L"A bond handle" )
    .arg( L"SettlementDates",      L"Column of settlement dates" )
    .arg( L"Yields",               L"Column of yields, aligned with SettlementDates" )
    .arg( L"YieldCalculationType", L"Optional. Yield convention, e.g. STREET, TRUE" );
#endif


// Yield (price -> yield) for a cached bond at a settlement date.
#if AQ_XLL_ENABLED(aqBondObjectYieldFromObject)
XLO_FUNC_START( aqBondObjectYieldFromObject(
    const ExcelObj& bondObjectName,
    const ExcelObj& settlementDate,
    const ExcelObj& price,
    const ExcelObj& yieldCalculationType ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqBondObjectYieldFromObject(
        etrading::getBond( getNameWithoutCounter( bondObjectName ) ),
        toAQLDate( settlementDate ),
        price.get<double>(),
        toNarrowString( yieldCalculationType ) ) );
}
XLO_FUNC_END( aqBondObjectYieldFromObject )
    .help( L"Yield (price to yield) for a cached bond at a settlement date." )
    .arg( L"BondObjectName",       L"A bond handle" )
    .arg( L"SettlementDate",       L"The settlement date" )
    .arg( L"Price",                L"The price to solve the yield from" )
    .arg( L"YieldCalculationType", L"Optional. Yield convention, e.g. STREET, TRUE" );
#endif


// Optimised yield solve (price -> yield) for a cached bond at multiple dates.
#if AQ_XLL_ENABLED(aqBondObjectYieldOptimized)
XLO_FUNC_START( aqBondObjectYieldOptimized(
    const ExcelObj& bondObjectName,
    const ExcelObj& settlementDates,
    const ExcelObj& prices,
    const ExcelObj& yieldCalculationType ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelDoubleColumn( validation::tryAqBondObjectYieldOptimized(
        etrading::getBond( getNameWithoutCounter( bondObjectName ) ),
        toDateVector( settlementDates, true, "SettlementDates" ),
        toDoubleVector( prices, true, "Prices" ),
        toNarrowString( yieldCalculationType ) ) ) );
}
XLO_FUNC_END( aqBondObjectYieldOptimized )
    .help( L"Optimised yield solve (price to yield) for a cached bond at multiple settlement dates." )
    .arg( L"BondObjectName",       L"A bond handle" )
    .arg( L"SettlementDates",      L"Column of settlement dates" )
    .arg( L"Prices",               L"Column of prices, aligned with SettlementDates" )
    .arg( L"YieldCalculationType", L"Optional. Yield convention, e.g. STREET, TRUE" );
#endif


/* -------------------------------------------------------------------------
 *  Bond schedule (stateless)
 * ---------------------------------------------------------------------- */

// The expected keys for a bond schedule label/value block.
#if AQ_XLL_ENABLED(aqBondScheduleLVBKeys)
XLO_FUNC_START( aqBondScheduleLVBKeys() )
{
    AQ_XLL_GUARD

    return returnValue( toExcelColumn( validation::tryAqBondScheduleLVBKeys() ) );
}
XLO_FUNC_END( aqBondScheduleLVBKeys )
    .help( L"The expected keys for a bond schedule label/value block, as a column." );
#endif


// Build a bond schedule from a label/value block.
#if AQ_XLL_ENABLED(aqBondSchedule)
XLO_FUNC_START( aqBondSchedule(
    const ExcelObj& bondScheduleLVB,
    const ExcelObj& validateKeys,
    const ExcelObj& showColumnHeaders ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelMatrix( validation::tryAqBondSchedule(
        toLabelValueBlock( bondScheduleLVB ),
        toBool( validateKeys, true ),
        toBool( showColumnHeaders, true ) ) ) );
}
XLO_FUNC_END( aqBondSchedule )
    .help( L"Build a bond schedule from a label/value block. Returns the schedule as a matrix." )
    .arg( L"BondScheduleLVB",   L"The bond schedule label/value block" )
    .arg( L"ValidateKeys",      L"Optional. Default TRUE. Check the keys against aqBondScheduleLVBKeys" )
    .arg( L"ShowColumnHeaders", L"Optional. Default TRUE. Include a header row" );
#endif


/* -------------------------------------------------------------------------
 *  BondOption (its own category - see the file header note above)
 * ---------------------------------------------------------------------- */

// Create and store a bond option from a label/value block.
#if AQ_XLL_ENABLED(aqBondOptionObjectCreate)
XLO_FUNC_START( aqBondOptionObjectCreate(
    const ExcelObj& objectName,
    const ExcelObj& optionLVB,
    const ExcelObj& validateKeys ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const std::string name = decorateWithExcelLocation( toNarrowString( objectName ) );

    const std::string storedName = validation::tryAqBondOptionObjectCreate(
        name, toLabelValueBlock( optionLVB ), toBool( validateKeys, true ) );

    return returnValue( appendInstanceCounter( storedName ) );
}
XLO_FUNC_END( aqBondOptionObjectCreate )
    .help( L"Create and store a bond option from a label/value block; returns its handle." )
    .arg( L"ObjectName",   L"Name for the bond-option object" )
    .arg( L"OptionLVB",    L"The bond-option definition as a label/value block" )
    .arg( L"ValidateKeys", L"Optional. Default TRUE. Check the LVB keys" );
#endif


// Display a cached bond option as a matrix.
#if AQ_XLL_ENABLED(aqBondOptionObjectDisplay)
XLO_FUNC_START( aqBondOptionObjectDisplay(
    const ExcelObj& objectName ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelMatrix(
        validation::tryAqBondOptionObjectDisplay( getNameWithoutCounter( objectName ) ) ) );
}
XLO_FUNC_END( aqBondOptionObjectDisplay )
    .help( L"Display a cached bond option as a matrix." )
    .arg( L"ObjectName", L"A bond-option handle" );
#endif


// Present value of a cached bond option.
#if AQ_XLL_ENABLED(aqBondOptionObjectPV)
XLO_FUNC_START( aqBondOptionObjectPV(
    const ExcelObj& objectName,
    const ExcelObj& valuationSettings,
    const ExcelObj& bondPrice,
    const ExcelObj& repoRate,
    const ExcelObj& repoDaycount,
    const ExcelObj& discountRate,
    const ExcelObj& discountDayCount ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqBondOptionObjectPV(
        getNameWithoutCounter( objectName ),
        toAQLStringMatrix( valuationSettings ),
        bondPrice.get<double>(),
        repoRate.get<double>(),
        toNarrowString( repoDaycount ),
        discountRate.get<double>(),
        toNarrowString( discountDayCount ) ) );
}
XLO_FUNC_END( aqBondOptionObjectPV )
    .help( L"Present value of a cached bond option." )
    .arg( L"ObjectName",        L"A bond-option handle" )
    .arg( L"ValuationSettings", L"Valuation settings as a key/value matrix" )
    .arg( L"BondPrice",         L"Underlying bond price" )
    .arg( L"RepoRate",          L"Repo rate for the underlying" )
    .arg( L"RepoDaycount",      L"Day count for the repo accrual, e.g. ACT/360" )
    .arg( L"DiscountRate",      L"Discount rate for the option" )
    .arg( L"DiscountDayCount",  L"Day count for the discount accrual" );
#endif


// Greeks of a cached bond option.
#if AQ_XLL_ENABLED(aqBondOptionObjectGreeks)
XLO_FUNC_START( aqBondOptionObjectGreeks(
    const ExcelObj& greekType,
    const ExcelObj& objectName,
    const ExcelObj& valuationSettings,
    const ExcelObj& bondSpotPrice,
    const ExcelObj& repoRate,
    const ExcelObj& repoDaycount,
    const ExcelObj& discountRate,
    const ExcelObj& discountDayCount,
    const ExcelObj& deltaBump,
    const ExcelObj& gammaBump,
    const ExcelObj& vegaBump,
    const ExcelObj& thetaBump,
    const ExcelObj& rhoBump,
    const ExcelObj& showColumnHeaders ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelMatrix( validation::tryAqBondOptionObjectGreeks(
        toNarrowString( greekType ),
        getNameWithoutCounter( objectName ),
        toAQLStringMatrix( valuationSettings ),
        bondSpotPrice.get<double>(),
        repoRate.get<double>(),
        toNarrowString( repoDaycount ),
        discountRate.get<double>(),
        toNarrowString( discountDayCount ),
        deltaBump.get<double>(),
        gammaBump.get<double>(),
        vegaBump.get<double>(),
        thetaBump.get<double>(),
        rhoBump.get<double>(),
        toBool( showColumnHeaders, true ) ) ) );
}
XLO_FUNC_END( aqBondOptionObjectGreeks )
    .help( L"Greeks of a cached bond option, by bump-and-revalue." )
    .arg( L"GreekType",         L"Which greek(s) to compute, e.g. ALL, DELTA, GAMMA" )
    .arg( L"ObjectName",        L"A bond-option handle" )
    .arg( L"ValuationSettings", L"Valuation settings as a key/value matrix" )
    .arg( L"BondSpotPrice",     L"Underlying bond spot price" )
    .arg( L"RepoRate",          L"Repo rate for the underlying" )
    .arg( L"RepoDaycount",      L"Day count for the repo accrual, e.g. ACT/360" )
    .arg( L"DiscountRate",      L"Discount rate for the option" )
    .arg( L"DiscountDayCount",  L"Day count for the discount accrual" )
    .arg( L"DeltaBump",         L"Bump size for delta" )
    .arg( L"GammaBump",         L"Bump size for gamma" )
    .arg( L"VegaBump",          L"Bump size for vega" )
    .arg( L"ThetaBump",         L"Bump size for theta (days)" )
    .arg( L"RhoBump",           L"Bump size for rho" )
    .arg( L"ShowColumnHeaders", L"Optional. Default TRUE. Include a header row" );
#endif


/* -------------------------------------------------------------------------
 *  BondFutureOption (its own category - see the file header note above).
 *  No separate create function: aqBondOptionObjectCreate creates the object
 *  these price against a bond-future price instead of a bond spot price.
 * ---------------------------------------------------------------------- */

// Present value of a cached bond-future option.
#if AQ_XLL_ENABLED(aqBondFutureOptionObjectPV)
XLO_FUNC_START( aqBondFutureOptionObjectPV(
    const ExcelObj& objectName,
    const ExcelObj& valuationSettings,
    const ExcelObj& bondFuturePrice,
    const ExcelObj& discountRate,
    const ExcelObj& discountDayCount ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqBondFutureOptionObjectPV(
        getNameWithoutCounter( objectName ),
        toAQLStringMatrix( valuationSettings ),
        bondFuturePrice.get<double>(),
        discountRate.get<double>(),
        toNarrowString( discountDayCount ) ) );
}
XLO_FUNC_END( aqBondFutureOptionObjectPV )
    .help( L"Present value of a cached bond-future option (the object created by aqBondOptionObjectCreate)." )
    .arg( L"ObjectName",        L"A bond-option handle" )
    .arg( L"ValuationSettings", L"Valuation settings as a key/value matrix" )
    .arg( L"BondFuturePrice",   L"The bond-future price" )
    .arg( L"DiscountRate",      L"Discount rate for the option" )
    .arg( L"DiscountDayCount",  L"Day count for the discount accrual" );
#endif


// Greeks of a cached bond-future option.
#if AQ_XLL_ENABLED(aqBondFutureOptionObjectGreeks)
XLO_FUNC_START( aqBondFutureOptionObjectGreeks(
    const ExcelObj& greekType,
    const ExcelObj& objectName,
    const ExcelObj& valuationSettings,
    const ExcelObj& bondFuturePrice,
    const ExcelObj& discountRate,
    const ExcelObj& discountDayCount,
    const ExcelObj& deltaBump,
    const ExcelObj& gammaBump,
    const ExcelObj& vegaBump,
    const ExcelObj& thetaBump,
    const ExcelObj& rhoBump,
    const ExcelObj& showColumnHeaders ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelMatrix( validation::tryAqBondFutureOptionObjectGreeks(
        toNarrowString( greekType ),
        getNameWithoutCounter( objectName ),
        toAQLStringMatrix( valuationSettings ),
        bondFuturePrice.get<double>(),
        discountRate.get<double>(),
        toNarrowString( discountDayCount ),
        deltaBump.get<double>(),
        gammaBump.get<double>(),
        vegaBump.get<double>(),
        thetaBump.get<double>(),
        rhoBump.get<double>(),
        toBool( showColumnHeaders, true ) ) ) );
}
XLO_FUNC_END( aqBondFutureOptionObjectGreeks )
    .help( L"Greeks of a cached bond-future option, by bump-and-revalue." )
    .arg( L"GreekType",         L"Which greek(s) to compute, e.g. ALL, DELTA, GAMMA" )
    .arg( L"ObjectName",        L"A bond-option handle" )
    .arg( L"ValuationSettings", L"Valuation settings as a key/value matrix" )
    .arg( L"BondFuturePrice",   L"The bond-future price" )
    .arg( L"DiscountRate",      L"Discount rate for the option" )
    .arg( L"DiscountDayCount",  L"Day count for the discount accrual" )
    .arg( L"DeltaBump",         L"Bump size for delta" )
    .arg( L"GammaBump",         L"Bump size for gamma" )
    .arg( L"VegaBump",          L"Bump size for vega" )
    .arg( L"ThetaBump",         L"Bump size for theta (days)" )
    .arg( L"RhoBump",           L"Bump size for rho" )
    .arg( L"ShowColumnHeaders", L"Optional. Default TRUE. Include a header row" );
#endif
