/*
 * AssetSwap category - xlOil worksheet functions.
 *
 * aqAssetSwap*        - stateless: data in, value out.
 * aqAssetSwapObject*  - operate on a cached swap/bond pair (names in).
 *
 * The validation wrappers overload each of Spread / SpreadFromPrice /
 * SpreadToCleanPrice for a single price and for a column of prices; only the
 * single-price form is exposed here (the column form is deferred - same
 * disposition as the Math *Prices vector overloads).
 *
 * Each function pairs with the identically named validation wrapper (plus the
 * `try` prefix). Marshalling to and from Excel is the aq_xll helpers in
 * aqXllTools.h.
 */

#include <aqMain.h>

#include <string>

#include <aqXllTools.h>
#include <tryAqAssetSwapObject.h>   // validation::tryAqAssetSwapObject*
#include <tryAqAssetSwapSpread.h>   // validation::tryAqAssetSwapSpread(LVBKeys)

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
}


// Asset-swap spread for one bond price.
XLO_FUNC_START( aqAssetSwapObjectSpread(
    const ExcelObj& swapObjectName,
    const ExcelObj& bondObjectName,
    const ExcelObj& bondPrice,
    const ExcelObj& valuationSettingsLVB,
    const ExcelObj& fixingTableNames ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqAssetSwapObjectSpread(
        getNameWithoutCounter( swapObjectName ),
        getNameWithoutCounter( bondObjectName ),
        bondPrice.get<double>(),
        toLabelValueBlock( valuationSettingsLVB ),
        toLabelValueBlockOr( fixingTableNames ) ) );
}
XLO_FUNC_END( aqAssetSwapObjectSpread )
    .help( L"Asset-swap spread for one bond price." )
    .arg( L"SwapObjectName",       L"An asset-swap handle" )
    .arg( L"BondObjectName",       L"The underlying bond handle" )
    .arg( L"BondPrice",            L"The bond price" )
    .arg( L"ValuationSettingsLVB", L"Valuation settings as a label/value block" )
    .arg( L"FixingTableNames",     L"Optional. Fixing-table name overrides as a label/value block" );


// Asset-swap spread implied by one bond price (stateless bond/price pair).
XLO_FUNC_START( aqAssetSwapObjectSpreadFromPrice(
    const ExcelObj& bondPrice,
    const ExcelObj& bondAccrualStartDate,
    const ExcelObj& isCleanPrice,
    const ExcelObj& swapObjectName,
    const ExcelObj& valuationSettingsLVB,
    const ExcelObj& fixingTableNames ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqAssetSwapObjectSpreadFromPrice(
        bondPrice.get<double>(),
        toAQLDate( bondAccrualStartDate ),
        toBool( isCleanPrice, true ),
        getNameWithoutCounter( swapObjectName ),
        toLabelValueBlock( valuationSettingsLVB ),
        toLabelValueBlockOr( fixingTableNames ) ) );
}
XLO_FUNC_END( aqAssetSwapObjectSpreadFromPrice )
    .help( L"Asset-swap spread implied by a bond price and its accrual start date." )
    .arg( L"BondPrice",             L"The bond price" )
    .arg( L"BondAccrualStartDate",  L"The bond's accrual start date" )
    .arg( L"IsCleanPrice",          L"TRUE if BondPrice is clean, FALSE if dirty" )
    .arg( L"SwapObjectName",        L"An asset-swap handle" )
    .arg( L"ValuationSettingsLVB",  L"Valuation settings as a label/value block" )
    .arg( L"FixingTableNames",      L"Optional. Fixing-table name overrides as a label/value block" );


// Bond clean price implied by an asset-swap par-par spread.
XLO_FUNC_START( aqAssetSwapObjectSpreadToCleanPrice(
    const ExcelObj& swapObjectName,
    const ExcelObj& assetSwapParSpread,
    const ExcelObj& valuationSettingsLVB,
    const ExcelObj& fixingTableNames ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqAssetSwapObjectSpreadToCleanPrice(
        getNameWithoutCounter( swapObjectName ),
        assetSwapParSpread.get<double>(),
        toLabelValueBlock( valuationSettingsLVB ),
        toLabelValueBlockOr( fixingTableNames ) ) );
}
XLO_FUNC_END( aqAssetSwapObjectSpreadToCleanPrice )
    .help( L"Bond clean price implied by an asset-swap par-par spread." )
    .arg( L"SwapObjectName",        L"An asset-swap handle" )
    .arg( L"AssetSwapParSpread",    L"The par-par asset-swap spread" )
    .arg( L"ValuationSettingsLVB",  L"Valuation settings as a label/value block" )
    .arg( L"FixingTableNames",      L"Optional. Fixing-table name overrides as a label/value block" );


// Fixed-equivalent coupon of a cached asset swap.
XLO_FUNC_START( aqAssetSwapObjectFixedEqvCoupon(
    const ExcelObj& swapObjectName,
    const ExcelObj& bondObjectName,
    const ExcelObj& bondPrice,
    const ExcelObj& valuationSettingsLVB,
    const ExcelObj& fixingTableNames ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqAssetSwapObjectFixedEqvCoupon(
        getNameWithoutCounter( swapObjectName ),
        getNameWithoutCounter( bondObjectName ),
        bondPrice.get<double>(),
        toLabelValueBlock( valuationSettingsLVB ),
        toLabelValueBlockOr( fixingTableNames ) ) );
}
XLO_FUNC_END( aqAssetSwapObjectFixedEqvCoupon )
    .help( L"Fixed-equivalent coupon of a cached asset swap." )
    .arg( L"SwapObjectName",       L"An asset-swap handle" )
    .arg( L"BondObjectName",       L"The underlying bond handle" )
    .arg( L"BondPrice",            L"The bond price" )
    .arg( L"ValuationSettingsLVB", L"Valuation settings as a label/value block" )
    .arg( L"FixingTableNames",     L"Optional. Fixing-table name overrides as a label/value block" );


// Asset-swap spread implied by a fixed-equivalent coupon.
XLO_FUNC_START( aqAssetSwapObjectSpreadFromFixedEqvCoupon(
    const ExcelObj& swapObjectName,
    const ExcelObj& fixedEqvCoupon,
    const ExcelObj& valuationSettingsLVB,
    const ExcelObj& fixingTableNames ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqAssetSwapObjectSpreadFromFixedEqvCoupon(
        getNameWithoutCounter( swapObjectName ),
        fixedEqvCoupon.get<double>(),
        toLabelValueBlock( valuationSettingsLVB ),
        toLabelValueBlockOr( fixingTableNames ) ) );
}
XLO_FUNC_END( aqAssetSwapObjectSpreadFromFixedEqvCoupon )
    .help( L"Asset-swap spread implied by a fixed-equivalent coupon." )
    .arg( L"SwapObjectName",       L"An asset-swap handle" )
    .arg( L"FixedEqvCoupon",       L"The fixed-equivalent coupon" )
    .arg( L"ValuationSettingsLVB", L"Valuation settings as a label/value block" )
    .arg( L"FixingTableNames",     L"Optional. Fixing-table name overrides as a label/value block" );


/* -------------------------------------------------------------------------
 *  Asset swap spread (stateless)
 * ---------------------------------------------------------------------- */

// The expected keys for an asset-swap label/value block.
XLO_FUNC_START( aqAssetSwapSpreadLVBKeys() )
{
    AQ_XLL_GUARD

    return returnValue( toExcelColumn( validation::tryAqAssetSwapSpreadLVBKeys() ) );
}
XLO_FUNC_END( aqAssetSwapSpreadLVBKeys )
    .help( L"The expected keys for an asset-swap label/value block, as a column." );


// Asset-swap spread from a bond price and a label/value block.
XLO_FUNC_START( aqAssetSwapSpread(
    const ExcelObj& bondPrice,
    const ExcelObj& assetSwapLVB,
    const ExcelObj& validateKeys ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqAssetSwapSpread(
        bondPrice.get<double>(), toLabelValueBlock( assetSwapLVB ), toBool( validateKeys, true ) ) );
}
XLO_FUNC_END( aqAssetSwapSpread )
    .help( L"Asset-swap spread from a bond price and a label/value block." )
    .arg( L"BondPrice",    L"The bond price" )
    .arg( L"AssetSwapLVB", L"The asset-swap definition as a label/value block" )
    .arg( L"ValidateKeys", L"Optional. Default TRUE. Check the LVB keys against aqAssetSwapSpreadLVBKeys" );
