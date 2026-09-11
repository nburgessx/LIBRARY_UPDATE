/*
 * TRS (Total Return Swap) category - xlOil worksheet functions.
 *
 * aqTRSObject* operates on a cached TRS (created via the vanilla
 * aqSwapObjectCreate machinery; these are its credit-model-aware pricing
 * functions). Each function pairs with the identically named validation
 * wrapper (plus the `try` prefix). Marshalling to and from Excel is the
 * aq_xll helpers in aqXllTools.h.
 */

#include <aqMain.h>

#include <string>

#include <aqXllTools.h>
#include <tryAqSwapObjectPricing.h>   // validation::tryAqTRSObject*

using namespace aq_xll;

namespace
{
    // An optional label/value block: a missing/blank range is the default
    // (empty) LabelValueBlock rather than a 1x1 block of an empty string.
    etrading::LabelValueBlock toLabelValueBlockOr( const xloil::ExcelObj& obj )
    {
        if ( obj.isMissing() || !obj.isNonEmpty() )
        {
            return etrading::LabelValueBlock();
        }
        return toLabelValueBlock( obj );
    }
}


// Present value of a cached total return swap.
XLO_FUNC_START( aqTRSObjectPV(
    const ExcelObj& swapName,
    const ExcelObj& creditModelName,
    const ExcelObj& legName,
    const ExcelObj& fixingTableNames ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqTRSObjectPV(
        getNameWithoutCounter( swapName ),
        getNameWithoutCounter( creditModelName ),
        toNarrowString( legName ),
        toLabelValueBlockOr( fixingTableNames ) ) );
}
XLO_FUNC_END( aqTRSObjectPV )
    .help( L"Present value of a cached total return swap." )
    .arg( L"SwapName",         L"A swap handle" )
    .arg( L"CreditModelName",  L"A credit-model handle for the reference asset" )
    .arg( L"LegName",          L"Optional. The leg to value; blank values the whole swap" )
    .arg( L"FixingTableNames", L"Optional. Fixing-table name overrides as a label/value block" );


// Par rate of a cached total return swap.
XLO_FUNC_START( aqTRSObjectParRate(
    const ExcelObj& swapName,
    const ExcelObj& creditModelName,
    const ExcelObj& fixingTableNames ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqTRSObjectParRate(
        getNameWithoutCounter( swapName ),
        getNameWithoutCounter( creditModelName ),
        toLabelValueBlockOr( fixingTableNames ) ) );
}
XLO_FUNC_END( aqTRSObjectParRate )
    .help( L"Par rate of a cached total return swap." )
    .arg( L"SwapName",         L"A swap handle" )
    .arg( L"CreditModelName",  L"A credit-model handle for the reference asset" )
    .arg( L"FixingTableNames", L"Optional. Fixing-table name overrides as a label/value block" );


// Par spread of a cached total return swap.
XLO_FUNC_START( aqTRSObjectParSpread(
    const ExcelObj& swapName,
    const ExcelObj& creditModelName,
    const ExcelObj& fixingTableNames ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqTRSObjectParSpread(
        getNameWithoutCounter( swapName ),
        getNameWithoutCounter( creditModelName ),
        toLabelValueBlockOr( fixingTableNames ) ) );
}
XLO_FUNC_END( aqTRSObjectParSpread )
    .help( L"Par spread of a cached total return swap." )
    .arg( L"SwapName",         L"A swap handle" )
    .arg( L"CreditModelName",  L"A credit-model handle for the reference asset" )
    .arg( L"FixingTableNames", L"Optional. Fixing-table name overrides as a label/value block" );


// Annuity of a cached total return swap leg.
XLO_FUNC_START( aqTRSObjectAnnuity(
    const ExcelObj& swapName,
    const ExcelObj& creditModelName,
    const ExcelObj& legName ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqTRSObjectAnnuity(
        getNameWithoutCounter( swapName ),
        getNameWithoutCounter( creditModelName ),
        toNarrowString( legName ) ) );
}
XLO_FUNC_END( aqTRSObjectAnnuity )
    .help( L"Annuity of a cached total return swap leg." )
    .arg( L"SwapName",        L"A swap handle" )
    .arg( L"CreditModelName", L"A credit-model handle for the reference asset" )
    .arg( L"LegName",         L"The leg to value" );
