/*
 * CMS (Constant Maturity Swap) category - xlOil worksheet functions.
 *
 * aqCMSObject* operates on a cached CMS swap (created via the
 * vanilla aqSwapObjectCreate machinery; these are its convexity-adjusted
 * pricing functions). Each function pairs with the identically named
 * validation wrapper (plus the `try` prefix). Marshalling to and from Excel is
 * the aq_xll helpers in aqXllTools.h.
 */

#include <aqMain.h>

#include <string>

#include <aqXllTools.h>
#include <tryAqSwapObjectPricing.h>   // validation::tryAqCMSObject*

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


// Present value of a cached CMS swap, using an explicit convexity adjustment.
#if AQ_XLL_ENABLED(aqCMSObjectPVUsingConvexityAdjustment)
XLO_FUNC_START( aqCMSObjectPVUsingConvexityAdjustment(
    const ExcelObj& swapName,
    const ExcelObj& valuationSettingsLVB,
    const ExcelObj& convexityAdjustment,
    const ExcelObj& legName,
    const ExcelObj& fixingTableNames ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqCMSObjectPVUsingConvexityAdjustment(
        getNameWithoutCounter( swapName ),
        toLabelValueBlock( valuationSettingsLVB ),
        convexityAdjustment.get<double>(),
        toAQLString( legName ),
        toLabelValueBlockOr( fixingTableNames ) ) );
}
XLO_FUNC_END( aqCMSObjectPVUsingConvexityAdjustment )
    .help( L"Present value of a cached CMS swap, applying an explicit convexity adjustment." )
    .arg( L"SwapName",             L"A swap handle" )
    .arg( L"ValuationSettingsLVB", L"Valuation settings as a label/value block" )
    .arg( L"ConvexityAdjustment",  L"The CMS convexity adjustment to apply" )
    .arg( L"LegName",              L"The CMS leg to value" )
    .arg( L"FixingTableNames",     L"Optional. Fixing-table name overrides as a label/value block" );
#endif


// Par rate of a cached CMS swap, using an explicit convexity adjustment.
#if AQ_XLL_ENABLED(aqCMSObjectParRateUsingConvexityAdjustment)
XLO_FUNC_START( aqCMSObjectParRateUsingConvexityAdjustment(
    const ExcelObj& swapName,
    const ExcelObj& valuationSettingsLVB,
    const ExcelObj& convexityAdjustment,
    const ExcelObj& fixingTableNames ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqCMSObjectParRateUsingConvexityAdjustment(
        getNameWithoutCounter( swapName ),
        toLabelValueBlock( valuationSettingsLVB ),
        convexityAdjustment.get<double>(),
        toLabelValueBlockOr( fixingTableNames ) ) );
}
XLO_FUNC_END( aqCMSObjectParRateUsingConvexityAdjustment )
    .help( L"Par rate of a cached CMS swap, applying an explicit convexity adjustment." )
    .arg( L"SwapName",             L"A swap handle" )
    .arg( L"ValuationSettingsLVB", L"Valuation settings as a label/value block" )
    .arg( L"ConvexityAdjustment",  L"The CMS convexity adjustment to apply" )
    .arg( L"FixingTableNames",     L"Optional. Fixing-table name overrides as a label/value block" );
#endif
