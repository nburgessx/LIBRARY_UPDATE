/*
 * Credit category - xlOil worksheet functions.
 *
 * aqCreditModel*   - operate on a cached credit model (hazard-rate curve),
 *                    calibrated from CDS or bond quotes.
 * aqCreditObject*  - CDS pricing, either driven directly by a hazard rate
 *                    (the "...FromHazardRate" forms) or by a cached credit
 *                    model; also credit index options and the fee leg/
 *                    schedule builders that share this file's golden-name
 *                    prefix despite being filed in the Swap leg/schedule
 *                    validation headers.
 *
 * All of these wrappers live in src/validation/include/tryAqSwapObjectPricing.h
 * alongside the (already-ported) CMS/TRS pricing functions - a pre-existing
 * filing quirk, not something this port changes; the golden name is what
 * decides the category, not the file it happens to live in (CLAUDE.md
 * Sec5.1a). `tryAqCreditObjectFeeLegCreate` lives in tryAqSwapObjectLeg.h and
 * `tryAqCreditObjectFeeScheduleCreate` in tryAqSwapObjectSchedule.h for the
 * same reason.
 *
 * Deferred (see rebrand/STATUS.md): one `tryAqCDSObjectHazard
 * RateFromParSpread` overload. Unlike its three siblings (PV/RiskyAnnuity/
 * ParSpread each have a hazard-rate-driven "...FromHazardRate" form and a
 * distinctly-named credit-model-driven form), this function's two overloads
 * share the exact same validation name - one takes a valuationSettingsLVB +
 * recoveryRate + includeAccruedInterest, the other a creditModelName - which
 * Excel cannot register as two worksheet functions of the same name. This
 * looks like a naming gap in the golden source relative to its siblings;
 * flagged for Nicholas rather than inventing a second name unilaterally. The
 * valuationSettingsLVB-driven overload is ported here as
 * `aqCDSObjectHazardRateFromParSpread`; the creditModelName-
 * driven overload is NOT YET exposed.
 *
 * Each function pairs with the identically named validation wrapper (plus the
 * `try` prefix). Marshalling to and from Excel is the aq_xll helpers in
 * xllSupport.h.
 */

#include <xllMain.h>

#include <string>
#include <vector>

#include <xllSupport.h>
#include <CoreEnumerations.h>          // etrading::trim_to_upper
#include <JSONInfoBlock.h>             // etrading::JSONInfoBlockTuples
#include <tryAqSwapObjectPricing.h>    // validation::tryAqCredit* / tryAqCreditModel*
#include <tryAqSwapObjectLeg.h>        // validation::tryAqCreditObjectFeeLegCreate
#include <tryAqSwapObjectSchedule.h>   // validation::tryAqCreditObjectFeeScheduleCreate

using namespace aq_xll;

namespace
{
    etrading::LabelValueBlock toLabelValueBlockOr( const xloil::ExcelObj& obj )
    {
        if ( obj.isMissing() || !obj.isNonEmpty() )
        {
            return etrading::LabelValueBlock();
        }
        return toLabelValueBlock( obj );
    }

    // An optional string worksheet argument.
    std::string toStrOr( const xloil::ExcelObj& obj, const char* defaultValue )
    {
        if ( obj.isMissing() || !obj.isNonEmpty() )
        {
            return std::string( defaultValue );
        }
        return toNarrowString( obj );
    }
}


/* -------------------------------------------------------------------------
 *  Default swap (CDS) pricing driven directly by a hazard rate
 * ---------------------------------------------------------------------- */

#if AQ_XLL_ENABLED(aqCDSObjectPVFromHazardRate)
XLO_FUNC_START( aqCDSObjectPVFromHazardRate(
    const ExcelObj& swapName,
    const ExcelObj& valuationSettingsLVB,
    const ExcelObj& hazardRate,
    const ExcelObj& recoveryRate,
    const ExcelObj& legName,
    const ExcelObj& includeAccruedInterest ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqCDSObjectPVFromHazardRate(
        getNameWithoutCounter( swapName ), toLabelValueBlock( valuationSettingsLVB ),
        hazardRate.get<double>(), recoveryRate.get<double>(),
        toAQLString( legName ), toBool( includeAccruedInterest, true ) ) );
}
XLO_FUNC_END( aqCDSObjectPVFromHazardRate )
    .help( L"PV of a cached credit default swap, given a hazard rate directly (no credit model needed)." )
    .arg( L"SwapName",              L"A cached CDS handle" )
    .arg( L"ValuationSettingsLVB",  L"ModelName, CurveCollection, ValuationDate etc, as a label/value block" )
    .arg( L"HazardRate",            L"The CDS hazard rate, used for survival probabilities" )
    .arg( L"RecoveryRate",          L"The estimated recovery amount after default" )
    .arg( L"LegName",               L"Optional. Price just this leg; default prices the whole swap" )
    .arg( L"IncludeAccruedInterest", L"Optional. Default TRUE" );
#endif


#if AQ_XLL_ENABLED(aqCDSObjectPV)
XLO_FUNC_START( aqCDSObjectPV(
    const ExcelObj& swapName,
    const ExcelObj& creditModelName,
    const ExcelObj& legName ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqCDSObjectPV(
        getNameWithoutCounter( swapName ), getNameWithoutCounter( creditModelName ), toAQLString( legName ) ) );
}
XLO_FUNC_END( aqCDSObjectPV )
    .help( L"PV of a cached credit default swap, priced off a cached credit model." )
    .arg( L"SwapName",         L"A cached CDS handle" )
    .arg( L"CreditModelName",  L"A cached credit-model handle" )
    .arg( L"LegName",          L"Optional. Price just this leg; default prices the whole swap" );
#endif


#if AQ_XLL_ENABLED(aqCDSObjectPVByIntegration)
XLO_FUNC_START( aqCDSObjectPVByIntegration(
    const ExcelObj& swapName,
    const ExcelObj& creditModelName,
    const ExcelObj& legName,
    const ExcelObj& numberOfIntegrationPoints,
    const ExcelObj& evaluateInParallel,
    const ExcelObj& payDefaultCashflowsOnNextCouponDate ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqCDSObjectPVByIntegration(
        getNameWithoutCounter( swapName ), getNameWithoutCounter( creditModelName ), toAQLString( legName ),
        static_cast<size_t>( numberOfIntegrationPoints.get<double>() ),
        toBool( evaluateInParallel, true ), toBool( payDefaultCashflowsOnNextCouponDate, true ) ) );
}
XLO_FUNC_END( aqCDSObjectPVByIntegration )
    .help( L"PV of a cached credit default swap, by integrating the payoff over survival probability." )
    .arg( L"SwapName",                          L"A cached CDS handle" )
    .arg( L"CreditModelName",                   L"A cached credit-model handle" )
    .arg( L"LegName",                           L"Optional. Price just this leg; default prices the whole swap" )
    .arg( L"NumberOfIntegrationPoints",         L"Number of (x,y) points to use in the numerical integration" )
    .arg( L"EvaluateInParallel",                L"Optional. Default TRUE. Evaluate loops in parallel where possible" )
    .arg( L"PayDefaultCashflowsOnNextCouponDate", L"Optional. Default TRUE. Wait to the next coupon date to pay protection/accrued on default" );
#endif


#if AQ_XLL_ENABLED(aqCDSObjectPVByMonteCarlo)
XLO_FUNC_START( aqCDSObjectPVByMonteCarlo(
    const ExcelObj& swapName,
    const ExcelObj& creditModelName,
    const ExcelObj& legName,
    const ExcelObj& mcParametersLVB,
    const ExcelObj& payDefaultCashflowsOnNextCouponDate ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    double standardError = 0.0;
    const double pv = validation::tryAqCDSObjectPVByMonteCarlo(
        getNameWithoutCounter( swapName ), getNameWithoutCounter( creditModelName ), toAQLString( legName ),
        toLabelValueBlockOr( mcParametersLVB ), toBool( payDefaultCashflowsOnNextCouponDate, true ), standardError );

    std::vector<double> result;
    result.push_back( pv );
    result.push_back( standardError );
    return returnValue( toExcelDoubleColumn( result ) );
}
XLO_FUNC_END( aqCDSObjectPVByMonteCarlo )
    .help( L"PV of a cached credit default swap, by Monte-Carlo simulation over survival probability. "
           L"Returns a 2-row column: [PV, Monte-Carlo standard error]." )
    .arg( L"SwapName",                          L"A cached CDS handle" )
    .arg( L"CreditModelName",                   L"A cached credit-model handle" )
    .arg( L"LegName",                           L"Optional. Price just this leg; default prices the whole swap" )
    .arg( L"McParametersLVB",                   L"Optional. Monte-Carlo / random-number-generator parameters" )
    .arg( L"PayDefaultCashflowsOnNextCouponDate", L"Optional. Default TRUE. Wait to the next coupon date to pay protection/accrued on default" );
#endif


#if AQ_XLL_ENABLED(aqCDSObjectRiskyAnnuityFromHazardRate)
XLO_FUNC_START( aqCDSObjectRiskyAnnuityFromHazardRate(
    const ExcelObj& swapName,
    const ExcelObj& valuationSettingsLVB,
    const ExcelObj& hazardRate,
    const ExcelObj& recoveryRate,
    const ExcelObj& legName,
    const ExcelObj& includeAccruedInterest ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqCDSObjectRiskyAnnuityFromHazardRate(
        getNameWithoutCounter( swapName ), toLabelValueBlock( valuationSettingsLVB ),
        hazardRate.get<double>(), recoveryRate.get<double>(),
        toAQLString( legName ), toBool( includeAccruedInterest, true ) ) );
}
XLO_FUNC_END( aqCDSObjectRiskyAnnuityFromHazardRate )
    .help( L"Risky annuity of a cached CDS premium leg, given a hazard rate directly." )
    .arg( L"SwapName",              L"A cached CDS handle" )
    .arg( L"ValuationSettingsLVB",  L"ModelName, CurveCollection, ValuationDate etc, as a label/value block" )
    .arg( L"HazardRate",            L"The CDS hazard rate, used for survival probabilities" )
    .arg( L"RecoveryRate",          L"The estimated recovery amount after default" )
    .arg( L"LegName",               L"The premium leg to use (mandatory)" )
    .arg( L"IncludeAccruedInterest", L"Optional. Default TRUE" );
#endif


#if AQ_XLL_ENABLED(aqCDSObjectRiskyAnnuity)
XLO_FUNC_START( aqCDSObjectRiskyAnnuity(
    const ExcelObj& swapName,
    const ExcelObj& creditModelName,
    const ExcelObj& legName ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqCDSObjectRiskyAnnuity(
        getNameWithoutCounter( swapName ), getNameWithoutCounter( creditModelName ), toAQLString( legName ) ) );
}
XLO_FUNC_END( aqCDSObjectRiskyAnnuity )
    .help( L"Risky annuity of a cached CDS premium leg, priced off a cached credit model." )
    .arg( L"SwapName",         L"A cached CDS handle" )
    .arg( L"CreditModelName",  L"A cached credit-model handle" )
    .arg( L"LegName",          L"The premium leg to use (mandatory)" );
#endif


#if AQ_XLL_ENABLED(aqCDSObjectAccruedYearFraction)
XLO_FUNC_START( aqCDSObjectAccruedYearFraction(
    const ExcelObj& swapName,
    const ExcelObj& creditModelName,
    const ExcelObj& toDate,
    const ExcelObj& legName ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqCDSObjectAccruedYearFraction(
        getNameWithoutCounter( swapName ), getNameWithoutCounter( creditModelName ), toAQLDate( toDate ), toAQLString( legName ) ) );
}
XLO_FUNC_END( aqCDSObjectAccruedYearFraction )
    .help( L"The accrued year fraction from the previous coupon date to a given date, for a cached CDS leg." )
    .arg( L"SwapName",         L"A cached CDS handle" )
    .arg( L"CreditModelName",  L"A cached credit-model handle" )
    .arg( L"ToDate",           L"The date to calculate the year fraction to" )
    .arg( L"LegName",          L"The premium leg name" );
#endif


#if AQ_XLL_ENABLED(aqCDSObjectCS01)
XLO_FUNC_START( aqCDSObjectCS01(
    const ExcelObj& swapName,
    const ExcelObj& creditModelName,
    const ExcelObj& legName ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqCDSObjectCS01(
        getNameWithoutCounter( swapName ), getNameWithoutCounter( creditModelName ), toAQLString( legName ) ) );
}
XLO_FUNC_END( aqCDSObjectCS01 )
    .help( L"CS01 (credit spread sensitivity) of a cached CDS premium leg." )
    .arg( L"SwapName",         L"A cached CDS handle" )
    .arg( L"CreditModelName",  L"A cached credit-model handle" )
    .arg( L"LegName",          L"The premium leg to use (mandatory)" );
#endif


#if AQ_XLL_ENABLED(aqCDSObjectParSpreadFromHazardRate)
XLO_FUNC_START( aqCDSObjectParSpreadFromHazardRate(
    const ExcelObj& swapName,
    const ExcelObj& valuationSettingsLVB,
    const ExcelObj& hazardRate,
    const ExcelObj& recoveryRate,
    const ExcelObj& premiumLegName,
    const ExcelObj& protectionLegName,
    const ExcelObj& includeAccruedInterest ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqCDSObjectParSpreadFromHazardRate(
        getNameWithoutCounter( swapName ), toLabelValueBlock( valuationSettingsLVB ),
        hazardRate.get<double>(), recoveryRate.get<double>(),
        toAQLString( premiumLegName ), toAQLString( protectionLegName ), toBool( includeAccruedInterest, true ) ) );
}
XLO_FUNC_END( aqCDSObjectParSpreadFromHazardRate )
    .help( L"Par spread of a cached CDS, given a hazard rate directly." )
    .arg( L"SwapName",               L"A cached CDS handle" )
    .arg( L"ValuationSettingsLVB",   L"A single collection name, or a curveCollection per leg, as a label/value block" )
    .arg( L"HazardRate",             L"The CDS hazard rate, used for survival probabilities" )
    .arg( L"RecoveryRate",           L"The estimated recovery amount after default" )
    .arg( L"PremiumLegName",         L"The premium leg name" )
    .arg( L"ProtectionLegName",      L"The protection leg name" )
    .arg( L"IncludeAccruedInterest", L"Optional. Default TRUE" );
#endif


#if AQ_XLL_ENABLED(aqCDSObjectParSpread)
XLO_FUNC_START( aqCDSObjectParSpread(
    const ExcelObj& swapName,
    const ExcelObj& creditModelName,
    const ExcelObj& premiumLegName,
    const ExcelObj& protectionLegName ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqCDSObjectParSpread(
        getNameWithoutCounter( swapName ), getNameWithoutCounter( creditModelName ),
        toAQLString( premiumLegName ), toAQLString( protectionLegName ) ) );
}
XLO_FUNC_END( aqCDSObjectParSpread )
    .help( L"Par spread of a cached CDS, priced off a cached credit model." )
    .arg( L"SwapName",          L"A cached CDS handle" )
    .arg( L"CreditModelName",   L"A cached credit-model handle" )
    .arg( L"PremiumLegName",    L"The premium leg name" )
    .arg( L"ProtectionLegName", L"The protection leg name" );
#endif


#if AQ_XLL_ENABLED(aqCDSObjectHazardRateFromParSpread)
XLO_FUNC_START( aqCDSObjectHazardRateFromParSpread(
    const ExcelObj& swapName,
    const ExcelObj& valuationSettingsLVB,
    const ExcelObj& parSpread,
    const ExcelObj& recoveryRate,
    const ExcelObj& premiumLegName,
    const ExcelObj& protectionLegName,
    const ExcelObj& includeAccruedInterest ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqCDSObjectHazardRateFromParSpread(
        getNameWithoutCounter( swapName ), toLabelValueBlock( valuationSettingsLVB ),
        parSpread.get<double>(), recoveryRate.get<double>(),
        toAQLString( premiumLegName ), toAQLString( protectionLegName ), toBool( includeAccruedInterest, true ) ) );
}
XLO_FUNC_END( aqCDSObjectHazardRateFromParSpread )
    .help( L"Hazard rate implied by a target CDS par spread. NOTE: the validation layer also has a "
           L"credit-model-driven overload of this exact name, not yet exposed here - see the file header." )
    .arg( L"SwapName",               L"A cached CDS handle" )
    .arg( L"ValuationSettingsLVB",   L"A single collection name, or a curveCollection per leg, as a label/value block" )
    .arg( L"ParSpread",              L"The target CDS par spread, as a decimal" )
    .arg( L"RecoveryRate",           L"The estimated recovery amount after default" )
    .arg( L"PremiumLegName",         L"The premium leg name" )
    .arg( L"ProtectionLegName",      L"The protection leg name" )
    .arg( L"IncludeAccruedInterest", L"Optional. Default TRUE" );
#endif


/* -------------------------------------------------------------------------
 *  Credit model lifecycle
 * ---------------------------------------------------------------------- */

// Create and store a credit model, calibrated from CDS or bond quotes.
#if AQ_XLL_ENABLED(aqCreditModelCreate)
XLO_FUNC_START( aqCreditModelCreate(
    const ExcelObj& objectName,
    const ExcelObj& key1,
    const ExcelObj& value1,
    const ExcelObj& key2,
    const ExcelObj& value2 ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const std::string name = decorateWithExcelLocation( toNarrowString( objectName ) );

    std::vector<std::string>      dataBlockNames;
    etrading::JSONInfoBlockTuples infoBlocks;

    dataBlockNames.push_back( etrading::trim_to_upper( toNarrowString( key1 ) ) );
    infoBlocks.push_back( toTableInfo( value1 ) );

    if ( !value2.isMissing() && value2.isNonEmpty() )
    {
        dataBlockNames.push_back( etrading::trim_to_upper( toNarrowString( key2 ) ) );
        infoBlocks.push_back( toTableInfo( value2 ) );
    }

    const std::string storedName = validation::tryAqCreditModelCreate( name, dataBlockNames, infoBlocks );

    return returnValue( appendInstanceCounter( storedName ) );
}
XLO_FUNC_END( aqCreditModelCreate )
    .help( L"Create and store a credit model (hazard-rate curve), calibrated from CDS or bond quotes; returns its handle." )
    .arg( L"ObjectName", L"Name for the credit-model object" )
    .arg( L"Key1",       L"Name of the first data block, e.g. MODEL_PROPERTIES" )
    .arg( L"Value1",     L"First data block, as a range" )
    .arg( L"Key2",       L"Optional. Name of the second data block, e.g. CDS_MARKETDATA / BOND_MARKETDATA" )
    .arg( L"Value2",     L"Optional. Second data block, as a range" );
#endif


#if AQ_XLL_ENABLED(aqCreditModelAsOfDate)
XLO_FUNC_START( aqCreditModelAsOfDate(
    const ExcelObj& creditModelName ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelDate( validation::tryAqCreditModelAsOfDate(
        getNameWithoutCounter( creditModelName ) ) ) );
}
XLO_FUNC_END( aqCreditModelAsOfDate )
    .help( L"The as-of / valuation date of a cached credit model." )
    .arg( L"CreditModelName", L"A cached credit-model handle" );
#endif


#if AQ_XLL_ENABLED(aqCreditModelCalibrationParameters)
XLO_FUNC_START( aqCreditModelCalibrationParameters(
    const ExcelObj& creditModelName ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelMatrix( validation::tryAqCreditModelCalibrationParameters(
        getNameWithoutCounter( creditModelName ) ) ) );
}
XLO_FUNC_END( aqCreditModelCalibrationParameters )
    .help( L"The calibrated payment dates and hazard rates of a cached credit model." )
    .arg( L"CreditModelName", L"A cached credit-model handle" );
#endif


#if AQ_XLL_ENABLED(aqCreditModelHazardRate)
XLO_FUNC_START( aqCreditModelHazardRate(
    const ExcelObj& creditModelName,
    const ExcelObj& paymentDate ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqCreditModelHazardRate(
        getNameWithoutCounter( creditModelName ), toAQLDate( paymentDate ) ) );
}
XLO_FUNC_END( aqCreditModelHazardRate )
    .help( L"The hazard rate at a given date from a cached credit model." )
    .arg( L"CreditModelName", L"A cached credit-model handle" )
    .arg( L"PaymentDate",     L"The date to extract the hazard rate at" );
#endif


#if AQ_XLL_ENABLED(aqCreditModelSurvivalProbability)
XLO_FUNC_START( aqCreditModelSurvivalProbability(
    const ExcelObj& creditModelName,
    const ExcelObj& toDate,
    const ExcelObj& fromDate ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqCreditModelSurvivalProbability(
        getNameWithoutCounter( creditModelName ), toAQLDate( toDate ), toAQLDate( fromDate ) ) );
}
XLO_FUNC_END( aqCreditModelSurvivalProbability )
    .help( L"Survival probability between two dates, from a cached credit model." )
    .arg( L"CreditModelName", L"A cached credit-model handle" )
    .arg( L"ToDate",          L"The date survival is measured to" )
    .arg( L"FromDate",        L"The date survival is measured from" );
#endif


#if AQ_XLL_ENABLED(aqCreditModelDefaultProbability)
XLO_FUNC_START( aqCreditModelDefaultProbability(
    const ExcelObj& creditModelName,
    const ExcelObj& toDate,
    const ExcelObj& fromDate ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqCreditModelDefaultProbability(
        getNameWithoutCounter( creditModelName ), toAQLDate( toDate ), toAQLDate( fromDate ) ) );
}
XLO_FUNC_END( aqCreditModelDefaultProbability )
    .help( L"Default probability between two dates, from a cached credit model." )
    .arg( L"CreditModelName", L"A cached credit-model handle" )
    .arg( L"ToDate",          L"The date default is measured to" )
    .arg( L"FromDate",        L"The date default is measured from" );
#endif


#if AQ_XLL_ENABLED(aqCreditModelImpliedSurvivalDate)
XLO_FUNC_START( aqCreditModelImpliedSurvivalDate(
    const ExcelObj& creditModelName,
    const ExcelObj& survivalProbability ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelDate( validation::tryAqCreditModelImpliedSurvivalDate(
        getNameWithoutCounter( creditModelName ), survivalProbability.get<double>() ) ) );
}
XLO_FUNC_END( aqCreditModelImpliedSurvivalDate )
    .help( L"The date implied by a target survival probability, from a cached credit model (inverse of aqCreditModelSurvivalProbability)." )
    .arg( L"CreditModelName",      L"A cached credit-model handle" )
    .arg( L"SurvivalProbability",  L"The target survival probability" );
#endif


#if AQ_XLL_ENABLED(aqCreditModelRiskyDiscountFactors)
XLO_FUNC_START( aqCreditModelRiskyDiscountFactors(
    const ExcelObj& creditModelName,
    const ExcelObj& paymentDates ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelDoubleColumn( validation::tryAqCreditModelRiskyDiscountFactors(
        getNameWithoutCounter( creditModelName ), toDateVector( paymentDates, true, "PaymentDates" ) ) ) );
}
XLO_FUNC_END( aqCreditModelRiskyDiscountFactors )
    .help( L"Risky (survival-weighted) discount factors from a cached credit model at a column of payment dates." )
    .arg( L"CreditModelName", L"A cached credit-model handle" )
    .arg( L"PaymentDates",    L"Column of payment dates" );
#endif


/* -------------------------------------------------------------------------
 *  Credit spread and credit index options
 * ---------------------------------------------------------------------- */

#if AQ_XLL_ENABLED(aqCreditObjectSpread)
XLO_FUNC_START( aqCreditObjectSpread(
    const ExcelObj& creditModelName,
    const ExcelObj& startDate,
    const ExcelObj& endDate ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqCreditObjectSpread(
        getNameWithoutCounter( creditModelName ), toAQLDate( startDate ), toAQLDate( endDate ) ) );
}
XLO_FUNC_END( aqCreditObjectSpread )
    .help( L"Forward credit spread between two dates, implied by a cached credit model (spot spread if StartDate is the model's as-of date)." )
    .arg( L"CreditModelName", L"A cached credit-model handle" )
    .arg( L"StartDate",       L"The date protection begins" )
    .arg( L"EndDate",         L"The date protection ends" );
#endif


#if AQ_XLL_ENABLED(aqCreditObjectIndexSpread)
XLO_FUNC_START( aqCreditObjectIndexSpread(
    const ExcelObj& creditModelName,
    const ExcelObj& startDate,
    const ExcelObj& endDate ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqCreditObjectIndexSpread(
        getNameWithoutCounter( creditModelName ), toAQLDate( startDate ), toAQLDate( endDate ) ) );
}
XLO_FUNC_END( aqCreditObjectIndexSpread )
    .help( L"Forward credit-index spread between two dates (Bloomberg credit-index convention: protection starts "
           L"immediately, the annuity leg starts on the forward start date)." )
    .arg( L"CreditModelName", L"A cached credit-model handle" )
    .arg( L"StartDate",       L"The date protection begins" )
    .arg( L"EndDate",         L"The date protection ends" );
#endif


#if AQ_XLL_ENABLED(aqCreditObjectOptionPV)
XLO_FUNC_START( aqCreditObjectOptionPV(
    const ExcelObj& creditModelName,
    const ExcelObj& payerReceiver,
    const ExcelObj& strike,
    const ExcelObj& optionExpiryDate,
    const ExcelObj& cdsMaturityDate,
    const ExcelObj& volatility ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqCreditObjectOptionPV(
        getNameWithoutCounter( creditModelName ), toNarrowString( payerReceiver ), strike.get<double>(),
        toAQLDate( optionExpiryDate ), toAQLDate( cdsMaturityDate ), volatility.get<double>() ) );
}
XLO_FUNC_END( aqCreditObjectOptionPV )
    .help( L"PV of an option on a credit default swap (Hull-White CDS option model)." )
    .arg( L"CreditModelName",  L"A cached credit-model handle" )
    .arg( L"PayerReceiver",    L"PAYER (pay CDS spread for protection) or RECEIVER (receive CDS spread)" )
    .arg( L"Strike",           L"The strike spread" )
    .arg( L"OptionExpiryDate", L"The option expiry / underlying CDS effective date" )
    .arg( L"CdsMaturityDate",  L"The underlying CDS maturity date" )
    .arg( L"Volatility",       L"The underlying CDS spread volatility" );
#endif


#if AQ_XLL_ENABLED(aqCreditObjectOptionPVFromForward)
XLO_FUNC_START( aqCreditObjectOptionPVFromForward(
    const ExcelObj& creditModelName,
    const ExcelObj& payerReceiver,
    const ExcelObj& strike,
    const ExcelObj& optionExpiryDate,
    const ExcelObj& cdsStartDate,
    const ExcelObj& cdsMaturityDate,
    const ExcelObj& volatility,
    const ExcelObj& forwardSpread ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqCreditObjectOptionPVFromForward(
        getNameWithoutCounter( creditModelName ), toNarrowString( payerReceiver ), strike.get<double>(),
        toAQLDate( optionExpiryDate ), toAQLDate( cdsStartDate ), toAQLDate( cdsMaturityDate ),
        volatility.get<double>(), forwardSpread.get<double>() ) );
}
XLO_FUNC_END( aqCreditObjectOptionPVFromForward )
    .help( L"PV of an option on a CDS, given its forward spread at expiry directly (Hull-White CDS option model)." )
    .arg( L"CreditModelName",  L"A cached credit-model handle" )
    .arg( L"PayerReceiver",    L"PAYER (pay CDS spread for protection) or RECEIVER (receive CDS spread)" )
    .arg( L"Strike",           L"The strike spread" )
    .arg( L"OptionExpiryDate", L"The option expiry date" )
    .arg( L"CdsStartDate",     L"The underlying CDS's delivery/start date if exercised" )
    .arg( L"CdsMaturityDate",  L"The underlying CDS maturity date" )
    .arg( L"Volatility",       L"The underlying CDS spread volatility" )
    .arg( L"ForwardSpread",    L"The CDS forward spread at the option expiry date" );
#endif


#if AQ_XLL_ENABLED(aqCreditObjectOptionImpliedVol)
XLO_FUNC_START( aqCreditObjectOptionImpliedVol(
    const ExcelObj& creditModelName,
    const ExcelObj& payerReceiver,
    const ExcelObj& strike,
    const ExcelObj& optionExpiryDate,
    const ExcelObj& cdsMaturityDate,
    const ExcelObj& targetOptionValue ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqCreditObjectOptionImpliedVol(
        getNameWithoutCounter( creditModelName ), toNarrowString( payerReceiver ), strike.get<double>(),
        toAQLDate( optionExpiryDate ), toAQLDate( cdsMaturityDate ), targetOptionValue.get<double>() ) );
}
XLO_FUNC_END( aqCreditObjectOptionImpliedVol )
    .help( L"Implied volatility of a CDS option, given a target quoted option value." )
    .arg( L"CreditModelName",    L"A cached credit-model handle" )
    .arg( L"PayerReceiver",      L"PAYER (pay CDS spread for protection) or RECEIVER (receive CDS spread)" )
    .arg( L"Strike",             L"The strike spread" )
    .arg( L"OptionExpiryDate",   L"The option expiry / underlying CDS effective date" )
    .arg( L"CdsMaturityDate",    L"The underlying CDS maturity date" )
    .arg( L"TargetOptionValue",  L"The target option value to solve the volatility for" );
#endif


#if AQ_XLL_ENABLED(aqCreditObjectOptionImpliedVolFromForward)
XLO_FUNC_START( aqCreditObjectOptionImpliedVolFromForward(
    const ExcelObj& creditModelName,
    const ExcelObj& payerReceiver,
    const ExcelObj& strike,
    const ExcelObj& optionExpiryDate,
    const ExcelObj& cdsStartDate,
    const ExcelObj& cdsMaturityDate,
    const ExcelObj& targetOptionValue,
    const ExcelObj& forwardSpread ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqCreditObjectOptionImpliedVolFromForward(
        getNameWithoutCounter( creditModelName ), toNarrowString( payerReceiver ), strike.get<double>(),
        toAQLDate( optionExpiryDate ), toAQLDate( cdsStartDate ), toAQLDate( cdsMaturityDate ),
        targetOptionValue.get<double>(), forwardSpread.get<double>() ) );
}
XLO_FUNC_END( aqCreditObjectOptionImpliedVolFromForward )
    .help( L"Implied volatility of a CDS option, given a target quoted option value and the CDS forward spread." )
    .arg( L"CreditModelName",    L"A cached credit-model handle" )
    .arg( L"PayerReceiver",      L"PAYER (pay CDS spread for protection) or RECEIVER (receive CDS spread)" )
    .arg( L"Strike",             L"The strike spread" )
    .arg( L"OptionExpiryDate",   L"The option expiry date" )
    .arg( L"CdsStartDate",       L"The underlying CDS's delivery/start date if exercised" )
    .arg( L"CdsMaturityDate",    L"The underlying CDS maturity date" )
    .arg( L"TargetOptionValue",  L"The target option value to solve the volatility for" )
    .arg( L"ForwardSpread",      L"The CDS forward spread at the option expiry date" );
#endif


#if AQ_XLL_ENABLED(aqCreditObjectIndexOptionPV)
XLO_FUNC_START( aqCreditObjectIndexOptionPV(
    const ExcelObj& creditModelName,
    const ExcelObj& optionLVB ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqCreditObjectIndexOptionPV(
        getNameWithoutCounter( creditModelName ), toLabelValueBlock( optionLVB ) ) );
}
XLO_FUNC_END( aqCreditObjectIndexOptionPV )
    .help( L"PV of a credit index option (Bloomberg credit-index-option model)." )
    .arg( L"CreditModelName", L"A cached credit-model handle" )
    .arg( L"OptionLVB",       L"Strike, expiry date, CDS coupon, volatility etc, as a label/value block" );
#endif


#if AQ_XLL_ENABLED(aqCreditObjectIndexOptionImpliedVol)
XLO_FUNC_START( aqCreditObjectIndexOptionImpliedVol(
    const ExcelObj& creditModelName,
    const ExcelObj& optionLVB ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqCreditObjectIndexOptionImpliedVol(
        getNameWithoutCounter( creditModelName ), toLabelValueBlock( optionLVB ) ) );
}
XLO_FUNC_END( aqCreditObjectIndexOptionImpliedVol )
    .help( L"Implied volatility of a credit index option, given a target quote and forward." )
    .arg( L"CreditModelName", L"A cached credit-model handle" )
    .arg( L"OptionLVB",       L"Strike, expiry date, CDS coupon, target quote etc, as a label/value block" );
#endif


#if AQ_XLL_ENABLED(aqCreditObjectIndexOptionVega)
XLO_FUNC_START( aqCreditObjectIndexOptionVega(
    const ExcelObj& creditModelName,
    const ExcelObj& optionLVB,
    const ExcelObj& volatilityBump ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqCreditObjectIndexOptionVega(
        getNameWithoutCounter( creditModelName ), toLabelValueBlock( optionLVB ), volatilityBump.get<double>() ) );
}
XLO_FUNC_END( aqCreditObjectIndexOptionVega )
    .help( L"Vega of a credit index option." )
    .arg( L"CreditModelName", L"A cached credit-model handle" )
    .arg( L"OptionLVB",       L"Strike, expiry date, CDS coupon, volatility etc, as a label/value block" )
    .arg( L"VolatilityBump",  L"The volatility bump size" );
#endif


#if AQ_XLL_ENABLED(aqCreditObjectIndexOptionCS01)
XLO_FUNC_START( aqCreditObjectIndexOptionCS01(
    const ExcelObj& creditModelName,
    const ExcelObj& optionLVB ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqCreditObjectIndexOptionCS01(
        getNameWithoutCounter( creditModelName ), toLabelValueBlock( optionLVB ) ) );
}
XLO_FUNC_END( aqCreditObjectIndexOptionCS01 )
    .help( L"CS01 of a credit index option." )
    .arg( L"CreditModelName", L"A cached credit-model handle" )
    .arg( L"OptionLVB",       L"Strike, expiry date, CDS coupon, volatility etc, as a label/value block" );
#endif


#if AQ_XLL_ENABLED(aqCreditObjectIndexOptionTheta)
XLO_FUNC_START( aqCreditObjectIndexOptionTheta(
    const ExcelObj& creditModelName,
    const ExcelObj& optionLVB ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqCreditObjectIndexOptionTheta(
        getNameWithoutCounter( creditModelName ), toLabelValueBlock( optionLVB ) ) );
}
XLO_FUNC_END( aqCreditObjectIndexOptionTheta )
    .help( L"Theta of a credit index option." )
    .arg( L"CreditModelName", L"A cached credit-model handle" )
    .arg( L"OptionLVB",       L"Strike, expiry date, CDS coupon, volatility etc, as a label/value block" );
#endif


/* -------------------------------------------------------------------------
 *  Credit basket model
 * ---------------------------------------------------------------------- */

#if AQ_XLL_ENABLED(aqCreditBasketModelCreate)
XLO_FUNC_START( aqCreditBasketModelCreate(
    const ExcelObj& objectName,
    const ExcelObj& key1,
    const ExcelObj& value1,
    const ExcelObj& key2,
    const ExcelObj& value2 ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const std::string name = decorateWithExcelLocation( toNarrowString( objectName ) );

    std::vector<std::string>      dataBlockNames;
    etrading::JSONInfoBlockTuples infoBlocks;

    dataBlockNames.push_back( etrading::trim_to_upper( toNarrowString( key1 ) ) );
    infoBlocks.push_back( toTableInfo( value1 ) );

    if ( !value2.isMissing() && value2.isNonEmpty() )
    {
        dataBlockNames.push_back( etrading::trim_to_upper( toNarrowString( key2 ) ) );
        infoBlocks.push_back( toTableInfo( value2 ) );
    }

    const std::string storedName = validation::tryAqCreditBasketModelCreate( name, dataBlockNames, infoBlocks );

    return returnValue( appendInstanceCounter( storedName ) );
}
XLO_FUNC_END( aqCreditBasketModelCreate )
    .help( L"Create and store a credit basket model from underlying credit models; returns its handle." )
    .arg( L"ObjectName", L"Name for the credit-basket-model object" )
    .arg( L"Key1",       L"Name of the first data block, e.g. MODEL_PROPERTIES" )
    .arg( L"Value1",     L"First data block, as a range" )
    .arg( L"Key2",       L"Optional. Name of the second data block, e.g. CREDIT_MODELS (CreditModelName + correlationBeta per row)" )
    .arg( L"Value2",     L"Optional. Second data block, as a range" );
#endif


#if AQ_XLL_ENABLED(aqCreditBasketModelSurvivalProbability)
XLO_FUNC_START( aqCreditBasketModelSurvivalProbability(
    const ExcelObj& creditBasketModelName,
    const ExcelObj& toDate,
    const ExcelObj& fromDate ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqCreditBasketModelSurvivalProbability(
        getNameWithoutCounter( creditBasketModelName ), toAQLDate( toDate ), toAQLDate( fromDate ) ) );
}
XLO_FUNC_END( aqCreditBasketModelSurvivalProbability )
    .help( L"Survival probability between two dates, computed by a cached credit basket model." )
    .arg( L"CreditBasketModelName", L"A cached credit-basket-model handle" )
    .arg( L"ToDate",                L"The date survival is measured to" )
    .arg( L"FromDate",              L"The date survival is measured from" );
#endif


/* -------------------------------------------------------------------------
 *  Fee leg / schedule builders (golden-named Credit, filed in the Swap leg
 *  and schedule validation headers - see the file header note)
 * ---------------------------------------------------------------------- */

#if AQ_XLL_ENABLED(aqCreditObjectFeeLegCreate)
XLO_FUNC_START( aqCreditObjectFeeLegCreate(
    const ExcelObj& legObjectName,
    const ExcelObj& feeProperties,
    const ExcelObj& feeScheduleLVB,
    const ExcelObj& validateKeys ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const std::string name = decorateWithExcelLocation( toNarrowString( legObjectName ) );

    const std::string storedName = validation::tryAqCreditObjectFeeLegCreate(
        name, toLabelValueBlock( feeProperties ), toAQLStringMatrix( feeScheduleLVB ), toBool( validateKeys, true ) );

    return returnValue( appendInstanceCounter( storedName ) );
}
XLO_FUNC_END( aqCreditObjectFeeLegCreate )
    .help( L"Create and store a CDS fee (premium) leg; returns its handle." )
    .arg( L"LegObjectName",  L"Name for the fee-leg object" )
    .arg( L"FeeProperties",  L"Fee leg properties as a label/value block" )
    .arg( L"FeeScheduleLVB", L"The fee schedule cashflows" )
    .arg( L"ValidateKeys",   L"Optional. Default TRUE. Check FeeProperties' keys" );
#endif


#if AQ_XLL_ENABLED(aqCreditObjectFeeScheduleCreate)
XLO_FUNC_START( aqCreditObjectFeeScheduleCreate(
    const ExcelObj& scheduleName,
    const ExcelObj& feeScheduleLVB,
    const ExcelObj& validateKeys ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const std::string name = decorateWithExcelLocation( toNarrowString( scheduleName ) );

    const std::string storedName = validation::tryAqCreditObjectFeeScheduleCreate(
        name, toAQLStringMatrix( feeScheduleLVB ), toBool( validateKeys, true ) );

    return returnValue( appendInstanceCounter( storedName ) );
}
XLO_FUNC_END( aqCreditObjectFeeScheduleCreate )
    .help( L"Create and store a CDS fee schedule from a cashflow matrix; returns its handle." )
    .arg( L"ScheduleName",   L"Name for the fee-schedule object" )
    .arg( L"FeeScheduleLVB", L"The fee schedule cashflows" )
    .arg( L"ValidateKeys",   L"Optional. Default TRUE. Check the schedule keys" );
#endif
