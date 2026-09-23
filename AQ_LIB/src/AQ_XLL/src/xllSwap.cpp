/*
 * Swap category - xlOil worksheet functions.
 *
 * aqSwap<Function>               - stateless: an LVB defining the whole swap
 *                                  in, value out (PV, PV01, ParRate, DV01,
 *                                  StubRate, schedule display).
 * aqSwapOis<Function>            - as above, restricted to the OIS variant.
 * aqSwapLeg<Function>            - stateless: a single-leg LVB in, value out.
 * aqSwapObject<Function>         - operate on a cached swap (name in).
 * aqSwapObjectLeg<Function>      - operate on a cached leg (name in).
 * aqSwapObjectSchedule<Function> - operate on a cached schedule (name in).
 * aqSwapGenerator<Function>      - operate on a cached swap generator.
 * aqSwapResults<Function>        - the swap discount/forward risk (Jacobian)
 *                                  store.
 *
 * Each function pairs with the identically named validation wrapper (plus the
 * `try` prefix). Marshalling to and from Excel is the aq_xll helpers in
 * xllSupport.h.
 */

#include <xllMain.h>

#include <string>
#include <vector>

#include <xllSupport.h>
#include <Variant.h>                   // etrading::Variant, VariantMatrix
#include <CoreEnumerations.h>          // etrading::toRiskTypeEnum
#include <tryAqSwapOisPV.h>            // validation::tryAqSwapOisPV(LVBKeys)
#include <tryAqSwapOisParRate.h>       // validation::tryAqSwapOisParRate(LVBKeys)
#include <tryAqSwapPV.h>               // validation::tryAqSwapPV(LVBKeys)
#include <tryAqSwapPV01.h>             // validation::tryAqSwapPV01(LVBKeys)
#include <tryAqSwapParRate.h>          // validation::tryAqSwapParRate(LVBKeys)
#include <tryAqSwapDV01.h>             // validation::tryAqSwapDV01(LVBKeys)
#include <tryAqSwapStubRate.h>         // validation::tryAqSwapStubRate / tryAqSwapStubFixingDate
#include <tryAqSwapLeg.h>              // validation::tryAqSwapLeg*
#include <tryAqSwapSchedule.h>         // validation::tryAqSwapSchedule*
#include <tryAqSwapObjectCreation.h>   // validation::tryAqSwapObject{Create*,AddLeg,AddFee,Display} / tryAqSwapGenerator*
#include <tryAqSwapObjectLeg.h>        // validation::tryAqSwapObjectLeg*
#include <tryAqSwapObjectSchedule.h>   // validation::tryAqSwapObjectSchedule*
#include <tryAqSwapObjectPricing.h>    // validation::tryAqSwapObject{PV,PV01,ParRate,Annuity,Spread,ParSpread,AccruedInterest,DisplayCashflows}
#include <tryAqSwapObjectDelta.h>      // validation::tryAqSwapObjectDelta / DeltaLadder(Horizontally)
#include <tryAqSwapDelta.h>            // validation::tryAqSwapDelta (stateless multi-trade)
#include <tryAqSwapResults.h>          // validation::tryAqSwapResults*
#include <ParameterValidation.h>       // etrading::getDataInstance

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

    std::vector<std::string> toStringVectorOr( const xloil::ExcelObj& obj )
    {
        if ( obj.isMissing() || !obj.isNonEmpty() )
        {
            return std::vector<std::string>();
        }
        return toStringVector( obj, true );
    }

    AQLStringVector toAQLStringVectorOr( const xloil::ExcelObj& obj )
    {
        if ( obj.isMissing() || !obj.isNonEmpty() )
        {
            return AQLStringVector();
        }
        return toAQLStringVector( obj );
    }

    std::vector<double> toDoubleVectorOr( const xloil::ExcelObj& obj, const char* nameOfVariable )
    {
        if ( obj.isMissing() || !obj.isNonEmpty() )
        {
            return std::vector<double>();
        }
        return toDoubleVector( obj, true, nameOfVariable );
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

    // A table of trade definitions -> one LabelValueBlock per trade. Layout
    // convention: row 0 is the shared key headers, each subsequent row is
    // one trade's values against those same keys (a normal Excel table with
    // a header row) - there is no prior marshalling pattern for
    // vector<LabelValueBlock> in this codebase, this is the new one, used by
    // tryAqSwapDelta and tryAqSwapObjectPVs. Missing/blank yields no trades.
    std::vector<etrading::LabelValueBlock> toLabelValueBlockVector( const xloil::ExcelObj& obj )
    {
        std::vector<etrading::LabelValueBlock> result;

        if ( obj.isMissing() || !obj.isNonEmpty() )
        {
            return result;
        }

        const AQLStringMatrix table = toAQLStringMatrix( obj );
        if ( table.empty() )
        {
            return result;
        }

        const AQLStringVector& keys = table[0];
        for ( std::size_t r = 1; r < table.size(); ++r )
        {
            AQLStringMatrix keyValuePairs;
            for ( std::size_t c = 0; c < keys.size() && c < table[r].size(); ++c )
            {
                AQLStringVector pair;
                pair.push_back( keys[c] );
                pair.push_back( table[r][c] );
                keyValuePairs.push_back( pair );
            }
            result.push_back( etrading::LabelValueBlock( keyValuePairs ) );
        }
        return result;
    }

    // A DoubleMatrix + row/column labels -> a single Excel block: a header
    // row ("" then each column header) followed by one row per pillar
    // (pillar name then its values). Backs the delta-ladder displays.
    xloil::ExcelObj toExcelLabeledMatrix( const AQLStringVector& columnHeaders,
                                          const AQLStringVector& rowLabels,
                                          const DoubleMatrix& values )
    {
        etrading::VariantMatrix result;

        etrading::VariantVector header;
        header.push_back( etrading::Variant( "" ) );
        for ( const AQLString& columnHeader : columnHeaders )
        {
            header.push_back( etrading::Variant( columnHeader.getCString() ) );
        }
        result.push_back( header );

        for ( std::size_t r = 0; r < rowLabels.size(); ++r )
        {
            etrading::VariantVector row;
            row.push_back( etrading::Variant( rowLabels[r].getCString() ) );
            if ( r < values.size() )
            {
                for ( double v : values[r] )
                {
                    row.push_back( etrading::Variant( v ) );
                }
            }
            result.push_back( row );
        }

        return toExcelMatrix( result );
    }

    // A column of AQObj handles, instance-counter stripped, as an
    // AQLStringVector - tryAqSwapObjectDelta takes its swapNames this way
    // rather than as a plain std::vector<std::string>.
    AQLStringVector toAQLStringVectorWithoutCounter( const xloil::ExcelObj& obj )
    {
        const std::vector<std::string> names = getNamesWithoutCounter( obj );
        AQLStringVector result;
        result.reserve( names.size() );
        for ( const std::string& name : names )
        {
            result.push_back( AQLString( name.c_str() ) );
        }
        return result;
    }

    // Every leg's display matrix, stacked vertically with a "Leg N" label row
    // ahead of each block. Backs aqSwapObjectDisplay / DisplayCashflows,
    // which return one AnyTypeMatrix per leg.
    xloil::ExcelObj toExcelStackedLegMatrices( const std::vector<AnyTypeMatrix>& legs )
    {
        AnyTypeMatrix combined;
        for ( std::size_t i = 0; i < legs.size(); ++i )
        {
            if ( i > 0 )
            {
                combined.push_back( std::vector<AnyType>() );   // blank separator row
            }
            std::vector<AnyType> labelRow;
            labelRow.push_back( std::string( "Leg " ) + std::to_string( i + 1 ) );
            combined.push_back( labelRow );

            for ( const std::vector<AnyType>& row : legs[i] )
            {
                combined.push_back( row );
            }
        }
        return toExcelMatrix( combined );
    }
}


/* -------------------------------------------------------------------------
 *  Stateless swap pricing (a whole-swap LVB in, value out)
 * ---------------------------------------------------------------------- */

#if AQ_XLL_ENABLED(aqSwapPVLVBKeys)
XLO_FUNC_START( aqSwapPVLVBKeys() )
{
    AQ_XLL_GUARD

    return returnValue( toExcelColumn( validation::tryAqSwapPVLVBKeys() ) );
}
XLO_FUNC_END( aqSwapPVLVBKeys )
    .help( L"The expected keys for a swap PV label/value block, as a column." );
#endif


#if AQ_XLL_ENABLED(aqSwapPV)
XLO_FUNC_START( aqSwapPV(
    const ExcelObj& swapLVB,
    const ExcelObj& validateKeys ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqSwapPV( toLabelValueBlock( swapLVB ), toBool( validateKeys, true ) ) );
}
XLO_FUNC_END( aqSwapPV )
    .help( L"Swap PV from a label/value block." )
    .arg( L"SwapLVB",      L"The swap definition as a label/value block" )
    .arg( L"ValidateKeys", L"Optional. Default TRUE. Check the LVB keys against aqSwapPVLVBKeys" );
#endif


#if AQ_XLL_ENABLED(aqSwapPV01LVBKeys)
XLO_FUNC_START( aqSwapPV01LVBKeys() )
{
    AQ_XLL_GUARD

    return returnValue( toExcelColumn( validation::tryAqSwapPV01LVBKeys() ) );
}
XLO_FUNC_END( aqSwapPV01LVBKeys )
    .help( L"The expected keys for a swap PV01 label/value block, as a column." );
#endif


#if AQ_XLL_ENABLED(aqSwapPV01)
XLO_FUNC_START( aqSwapPV01(
    const ExcelObj& swapLVB,
    const ExcelObj& validateKeys ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqSwapPV01( toLabelValueBlock( swapLVB ), toBool( validateKeys, true ) ) );
}
XLO_FUNC_END( aqSwapPV01 )
    .help( L"Swap PV01 from a label/value block." )
    .arg( L"SwapLVB",      L"The swap definition as a label/value block" )
    .arg( L"ValidateKeys", L"Optional. Default TRUE. Check the LVB keys against aqSwapPV01LVBKeys" );
#endif


#if AQ_XLL_ENABLED(aqSwapParRateLVBKeys)
XLO_FUNC_START( aqSwapParRateLVBKeys() )
{
    AQ_XLL_GUARD

    return returnValue( toExcelColumn( validation::tryAqSwapParRateLVBKeys() ) );
}
XLO_FUNC_END( aqSwapParRateLVBKeys )
    .help( L"The expected keys for a par-swap label/value block, as a column." );
#endif


#if AQ_XLL_ENABLED(aqSwapParRate)
XLO_FUNC_START( aqSwapParRate(
    const ExcelObj& parSwapLVB,
    const ExcelObj& validateKeys ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqSwapParRate( toLabelValueBlock( parSwapLVB ), toBool( validateKeys, true ) ) );
}
XLO_FUNC_END( aqSwapParRate )
    .help( L"Par swap rate from a label/value block." )
    .arg( L"ParSwapLVB",   L"The par swap definition as a label/value block" )
    .arg( L"ValidateKeys", L"Optional. Default TRUE. Check the LVB keys against aqSwapParRateLVBKeys" );
#endif


#if AQ_XLL_ENABLED(aqSwapDV01LVBKeys)
XLO_FUNC_START( aqSwapDV01LVBKeys() )
{
    AQ_XLL_GUARD

    return returnValue( toExcelColumn( validation::tryAqSwapDV01LVBKeys() ) );
}
XLO_FUNC_END( aqSwapDV01LVBKeys )
    .help( L"The expected keys for a swap DV01 label/value block, as a column." );
#endif


#if AQ_XLL_ENABLED(aqSwapDV01)
XLO_FUNC_START( aqSwapDV01(
    const ExcelObj& swapLVB,
    const ExcelObj& validateKeys ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqSwapDV01( toLabelValueBlock( swapLVB ), toBool( validateKeys, true ) ) );
}
XLO_FUNC_END( aqSwapDV01 )
    .help( L"Swap DV01 from a label/value block." )
    .arg( L"SwapLVB",      L"The swap definition as a label/value block" )
    .arg( L"ValidateKeys", L"Optional. Default TRUE. Check the LVB keys against aqSwapDV01LVBKeys" );
#endif


#if AQ_XLL_ENABLED(aqSwapStubRateLVBKeys)
XLO_FUNC_START( aqSwapStubRateLVBKeys() )
{
    AQ_XLL_GUARD

    return returnValue( toExcelColumn( validation::tryAqSwapStubRateLVBKeys() ) );
}
XLO_FUNC_END( aqSwapStubRateLVBKeys )
    .help( L"The expected keys for a swap stub-rate label/value block, as a column." );
#endif


#if AQ_XLL_ENABLED(aqSwapStubRate)
XLO_FUNC_START( aqSwapStubRate(
    const ExcelObj& swapLVB,
    const ExcelObj& curveIndices,
    const ExcelObj& curveTenors,
    const ExcelObj& tenorCurveFixings,
    const ExcelObj& validateKeys ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqSwapStubRate(
        toLabelValueBlock( swapLVB ),
        toAQLStringVectorOr( curveIndices ), toAQLStringVectorOr( curveTenors ),
        toDoubleVectorOr( tenorCurveFixings, "TenorCurveFixings" ),
        toBool( validateKeys, true ) ) );
}
XLO_FUNC_END( aqSwapStubRate )
    .help( L"The interpolated stub rate for a swap's initial short/long stub period." )
    .arg( L"SwapLVB",           L"The swap definition as a label/value block" )
    .arg( L"CurveIndices",      L"Optional. Column of curve indices to interpolate from" )
    .arg( L"CurveTenors",       L"Optional. Column of tenors corresponding to CurveIndices" )
    .arg( L"TenorCurveFixings", L"Optional. Column of fixings corresponding to CurveIndices" )
    .arg( L"ValidateKeys",      L"Optional. Default TRUE. Check the LVB keys against aqSwapStubRateLVBKeys" );
#endif


#if AQ_XLL_ENABLED(aqSwapStubFixingDate)
XLO_FUNC_START( aqSwapStubFixingDate(
    const ExcelObj& swapLVB,
    const ExcelObj& curveIndices,
    const ExcelObj& curveTenors,
    const ExcelObj& tenorCurveFixings,
    const ExcelObj& validateKeys ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelDate( validation::tryAqSwapStubFixingDate(
        toLabelValueBlock( swapLVB ),
        toAQLStringVectorOr( curveIndices ), toAQLStringVectorOr( curveTenors ),
        toDoubleVectorOr( tenorCurveFixings, "TenorCurveFixings" ),
        toBool( validateKeys, true ) ) ) );
}
XLO_FUNC_END( aqSwapStubFixingDate )
    .help( L"The fixing date used for a swap's initial short/long stub period." )
    .arg( L"SwapLVB",           L"The swap definition as a label/value block" )
    .arg( L"CurveIndices",      L"Optional. Column of curve indices to interpolate from" )
    .arg( L"CurveTenors",       L"Optional. Column of tenors corresponding to CurveIndices" )
    .arg( L"TenorCurveFixings", L"Optional. Column of fixings corresponding to CurveIndices" )
    .arg( L"ValidateKeys",      L"Optional. Default TRUE. Check the LVB keys against aqSwapStubRateLVBKeys" );
#endif


/* -------------------------------------------------------------------------
 *  Stateless OIS pricing (Swap's overnight-index product variant)
 * ---------------------------------------------------------------------- */

#if AQ_XLL_ENABLED(aqSwapOisPVLVBKeys)
XLO_FUNC_START( aqSwapOisPVLVBKeys() )
{
    AQ_XLL_GUARD

    return returnValue( toExcelColumn( validation::tryAqSwapOisPVLVBKeys() ) );
}
XLO_FUNC_END( aqSwapOisPVLVBKeys )
    .help( L"The expected keys for an OIS PV label/value block, as a column." );
#endif


#if AQ_XLL_ENABLED(aqSwapOisPV)
XLO_FUNC_START( aqSwapOisPV(
    const ExcelObj& oisLVB,
    const ExcelObj& validateKeys ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqSwapOisPV( toLabelValueBlock( oisLVB ), toBool( validateKeys, true ) ) );
}
XLO_FUNC_END( aqSwapOisPV )
    .help( L"OIS PV from a label/value block." )
    .arg( L"OisLVB",       L"The OIS definition as a label/value block" )
    .arg( L"ValidateKeys", L"Optional. Default TRUE. Check the LVB keys against aqSwapOisPVLVBKeys" );
#endif


#if AQ_XLL_ENABLED(aqSwapOisParRateLVBKeys)
XLO_FUNC_START( aqSwapOisParRateLVBKeys() )
{
    AQ_XLL_GUARD

    return returnValue( toExcelColumn( validation::tryAqSwapOisParRateLVBKeys() ) );
}
XLO_FUNC_END( aqSwapOisParRateLVBKeys )
    .help( L"The expected keys for an OIS par-rate label/value block, as a column." );
#endif


#if AQ_XLL_ENABLED(aqSwapOisParRate)
XLO_FUNC_START( aqSwapOisParRate(
    const ExcelObj& oisLVB,
    const ExcelObj& validateKeys ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqSwapOisParRate( toLabelValueBlock( oisLVB ), toBool( validateKeys, true ) ) );
}
XLO_FUNC_END( aqSwapOisParRate )
    .help( L"OIS par swap rate from a label/value block." )
    .arg( L"OisLVB",       L"The OIS definition as a label/value block" )
    .arg( L"ValidateKeys", L"Optional. Default TRUE. Check the LVB keys against aqSwapOisParRateLVBKeys" );
#endif


/* -------------------------------------------------------------------------
 *  Stateless swap leg pricing (a single-leg LVB in, value out)
 * ---------------------------------------------------------------------- */

#if AQ_XLL_ENABLED(aqSwapLegLVBKeys)
XLO_FUNC_START( aqSwapLegLVBKeys(
    const ExcelObj& legName ) )
{
    AQ_XLL_GUARD

    return returnValue( toExcelColumn( validation::tryAqSwapLegLVBKeys( toAQLString( legName ) ) ) );
}
XLO_FUNC_END( aqSwapLegLVBKeys )
    .help( L"The expected keys for a swap leg label/value block, as a column." )
    .arg( L"LegName", L"Leg type name, e.g. FIXED, FLOAT" );
#endif


#if AQ_XLL_ENABLED(aqSwapLegDisplay)
XLO_FUNC_START( aqSwapLegDisplay(
    const ExcelObj& legLVB,
    const ExcelObj& validateKeys,
    const ExcelObj& showColumnHeaders,
    const ExcelObj& columnList ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelMatrix( validation::tryAqSwapLegDisplay(
        toLabelValueBlock( legLVB ), toBool( validateKeys, true ),
        toBool( showColumnHeaders, true ), toStringVectorOr( columnList ) ) ) );
}
XLO_FUNC_END( aqSwapLegDisplay )
    .help( L"Display a swap leg's schedule and cashflows as a matrix." )
    .arg( L"LegLVB",            L"The leg definition as a label/value block" )
    .arg( L"ValidateKeys",      L"Optional. Default TRUE. Check the LVB keys" )
    .arg( L"ShowColumnHeaders", L"Optional. Default TRUE. Include a header row" )
    .arg( L"ColumnList",        L"Optional. Column names to include; default all columns" );
#endif


#if AQ_XLL_ENABLED(aqSwapLegPV)
XLO_FUNC_START( aqSwapLegPV(
    const ExcelObj& legLVB,
    const ExcelObj& validateKeys ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqSwapLegPV( toLabelValueBlock( legLVB ), toBool( validateKeys, true ) ) );
}
XLO_FUNC_END( aqSwapLegPV )
    .help( L"Swap leg PV from a label/value block." )
    .arg( L"LegLVB",       L"The leg definition as a label/value block" )
    .arg( L"ValidateKeys", L"Optional. Default TRUE. Check the LVB keys" );
#endif


#if AQ_XLL_ENABLED(aqSwapLegAnnuity)
XLO_FUNC_START( aqSwapLegAnnuity(
    const ExcelObj& legLVB,
    const ExcelObj& validateKeys ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqSwapLegAnnuity( toLabelValueBlock( legLVB ), toBool( validateKeys, true ) ) );
}
XLO_FUNC_END( aqSwapLegAnnuity )
    .help( L"Swap leg annuity from a label/value block." )
    .arg( L"LegLVB",       L"The leg definition as a label/value block" )
    .arg( L"ValidateKeys", L"Optional. Default TRUE. Check the LVB keys" );
#endif


/* -------------------------------------------------------------------------
 *  Stateless swap schedule (an LVB of schedule properties in, matrix out)
 * ---------------------------------------------------------------------- */

#if AQ_XLL_ENABLED(aqSwapScheduleLVBKeys)
XLO_FUNC_START( aqSwapScheduleLVBKeys() )
{
    AQ_XLL_GUARD

    return returnValue( toExcelColumn( validation::tryAqSwapScheduleLVBKeys() ) );
}
XLO_FUNC_END( aqSwapScheduleLVBKeys )
    .help( L"The expected keys for a swap schedule label/value block, as a column." );
#endif


#if AQ_XLL_ENABLED(aqSwapSchedule)
XLO_FUNC_START( aqSwapSchedule(
    const ExcelObj& showColumnHeaders,
    const ExcelObj& swapScheduleLVB,
    const ExcelObj& validateKeys,
    const ExcelObj& columnList,
    const ExcelObj& convertDatesToExcelFormat ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelMatrix( validation::tryAqSwapSchedule(
        toBool( showColumnHeaders, true ), toLabelValueBlock( swapScheduleLVB ), toBool( validateKeys, true ),
        toStringVectorOr( columnList ), toBool( convertDatesToExcelFormat, true ) ) ) );
}
XLO_FUNC_END( aqSwapSchedule )
    .help( L"Generate a swap's full (both legs) schedule from a label/value block of schedule properties." )
    .arg( L"ShowColumnHeaders",          L"Include a header row" )
    .arg( L"SwapScheduleLVB",            L"The schedule configuration as a label/value block" )
    .arg( L"ValidateKeys",               L"Optional. Default TRUE. Check the LVB keys" )
    .arg( L"ColumnList",                 L"Optional. Column names to include; default all columns" )
    .arg( L"ConvertDatesToExcelFormat",  L"Optional. Default TRUE" );
#endif


#if AQ_XLL_ENABLED(aqSwapScheduleFixed)
XLO_FUNC_START( aqSwapScheduleFixed(
    const ExcelObj& showColumnHeaders,
    const ExcelObj& swapScheduleLVB,
    const ExcelObj& validateKeys,
    const ExcelObj& columnList,
    const ExcelObj& convertDatesToExcelFormat ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelMatrix( validation::tryAqSwapScheduleFixed(
        toBool( showColumnHeaders, true ), toLabelValueBlock( swapScheduleLVB ), toBool( validateKeys, true ),
        toStringVectorOr( columnList ), toBool( convertDatesToExcelFormat, true ) ) ) );
}
XLO_FUNC_END( aqSwapScheduleFixed )
    .help( L"Generate just the fixed leg's schedule from a label/value block of schedule properties." )
    .arg( L"ShowColumnHeaders",          L"Include a header row" )
    .arg( L"SwapScheduleLVB",            L"The schedule configuration as a label/value block" )
    .arg( L"ValidateKeys",               L"Optional. Default TRUE. Check the LVB keys" )
    .arg( L"ColumnList",                 L"Optional. Column names to include; default all columns" )
    .arg( L"ConvertDatesToExcelFormat",  L"Optional. Default TRUE" );
#endif


#if AQ_XLL_ENABLED(aqSwapScheduleFloat)
XLO_FUNC_START( aqSwapScheduleFloat(
    const ExcelObj& showColumnHeaders,
    const ExcelObj& swapScheduleLVB,
    const ExcelObj& validateKeys,
    const ExcelObj& columnList,
    const ExcelObj& convertDatesToExcelFormat ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelMatrix( validation::tryAqSwapScheduleFloat(
        toBool( showColumnHeaders, true ), toLabelValueBlock( swapScheduleLVB ), toBool( validateKeys, true ),
        toStringVectorOr( columnList ), toBool( convertDatesToExcelFormat, true ) ) ) );
}
XLO_FUNC_END( aqSwapScheduleFloat )
    .help( L"Generate just the floating leg's schedule from a label/value block of schedule properties." )
    .arg( L"ShowColumnHeaders",          L"Include a header row" )
    .arg( L"SwapScheduleLVB",            L"The schedule configuration as a label/value block" )
    .arg( L"ValidateKeys",               L"Optional. Default TRUE. Check the LVB keys" )
    .arg( L"ColumnList",                 L"Optional. Column names to include; default all columns" )
    .arg( L"ConvertDatesToExcelFormat",  L"Optional. Default TRUE" );
#endif


/* -------------------------------------------------------------------------
 *  Swap object lifecycle / creation
 * ---------------------------------------------------------------------- */

#if AQ_XLL_ENABLED(aqSwapObjectLVBKeys)
XLO_FUNC_START( aqSwapObjectLVBKeys() )
{
    AQ_XLL_GUARD

    return returnValue( toExcelColumn( validation::tryAqSwapObjectLVBKeys() ) );
}
XLO_FUNC_END( aqSwapObjectLVBKeys )
    .help( L"The expected keys for a swap-level properties label/value block, as a column." );
#endif


#if AQ_XLL_ENABLED(aqSwapObjectCreateFromLegs)
XLO_FUNC_START( aqSwapObjectCreateFromLegs(
    const ExcelObj& swapName,
    const ExcelObj& legObjectNames,
    const ExcelObj& swapPropertiesLVB,
    const ExcelObj& isXccySwap,
    const ExcelObj& validateKeys ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const std::string name = decorateWithExcelLocation( toNarrowString( swapName ) );

    const std::string storedName = validation::tryAqSwapObjectCreateFromLegs(
        name, getNamesWithoutCounter( legObjectNames ), toLabelValueBlock( swapPropertiesLVB ),
        toBool( isXccySwap, false ), toBool( validateKeys, true ) );

    return returnValue( appendInstanceCounter( storedName ) );
}
XLO_FUNC_END( aqSwapObjectCreateFromLegs )
    .help( L"Create and store a swap from previously-cached leg objects; returns its handle." )
    .arg( L"SwapName",           L"Name for the swap object" )
    .arg( L"LegObjectNames",     L"Column of cached leg handles" )
    .arg( L"SwapPropertiesLVB",  L"Swap-level properties as a label/value block" )
    .arg( L"IsXccySwap",         L"Optional. Default FALSE. Enforce this is a cross-currency swap" )
    .arg( L"ValidateKeys",       L"Optional. Default TRUE. Check the LVB keys" );
#endif


#if AQ_XLL_ENABLED(aqSwapObjectCreateFromLegLVBs)
XLO_FUNC_START( aqSwapObjectCreateFromLegLVBs(
    const ExcelObj& swapName,
    const ExcelObj& leg1LVB,
    const ExcelObj& leg2LVB,
    const ExcelObj& swapPropertiesLVB,
    const ExcelObj& isXccySwap,
    const ExcelObj& validateKeys ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const std::string name = decorateWithExcelLocation( toNarrowString( swapName ) );

    const std::string storedName = validation::tryAqSwapObjectCreateFromLegLVBs(
        name, toLabelValueBlock( leg1LVB ), toLabelValueBlock( leg2LVB ), toLabelValueBlock( swapPropertiesLVB ),
        toBool( isXccySwap, false ), toBool( validateKeys, true ) );

    return returnValue( appendInstanceCounter( storedName ) );
}
XLO_FUNC_END( aqSwapObjectCreateFromLegLVBs )
    .help( L"Create and store a swap from two leg label/value blocks; returns its handle." )
    .arg( L"SwapName",           L"Name for the swap object" )
    .arg( L"Leg1LVB",            L"Leg 1 definition as a label/value block" )
    .arg( L"Leg2LVB",            L"Leg 2 definition as a label/value block" )
    .arg( L"SwapPropertiesLVB",  L"Swap-level properties as a label/value block" )
    .arg( L"IsXccySwap",         L"Optional. Default FALSE. Enforce this is a cross-currency swap" )
    .arg( L"ValidateKeys",       L"Optional. Default TRUE. Check the LVB keys" );
#endif


#if AQ_XLL_ENABLED(aqSwapObjectCreate)
XLO_FUNC_START( aqSwapObjectCreate(
    const ExcelObj& swapName,
    const ExcelObj& swapLVB,
    const ExcelObj& swapPropertiesLVB,
    const ExcelObj& isXccySwap,
    const ExcelObj& validateKeys ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const std::string name = decorateWithExcelLocation( toNarrowString( swapName ) );

    const std::string storedName = validation::tryAqSwapObjectCreate(
        name, toAQLStringMatrix( swapLVB ), toLabelValueBlock( swapPropertiesLVB ),
        toBool( isXccySwap, false ), toBool( validateKeys, true ) );

    return returnValue( appendInstanceCounter( storedName ) );
}
XLO_FUNC_END( aqSwapObjectCreate )
    .help( L"Create and store a swap from a combined two-leg label/value matrix; returns its handle." )
    .arg( L"SwapName",           L"Name for the swap object" )
    .arg( L"SwapLVB",            L"Swap label/value block with both legs" )
    .arg( L"SwapPropertiesLVB",  L"Swap-level properties as a label/value block" )
    .arg( L"IsXccySwap",         L"Optional. Default FALSE. Enforce this is a cross-currency swap" )
    .arg( L"ValidateKeys",       L"Optional. Default TRUE. Check the LVB keys" );
#endif


#if AQ_XLL_ENABLED(aqSwapObjectCreateFromSchedule)
XLO_FUNC_START( aqSwapObjectCreateFromSchedule(
    const ExcelObj& swapName,
    const ExcelObj& schedule1Name,
    const ExcelObj& schedule2Name,
    const ExcelObj& leg1LVB,
    const ExcelObj& leg2LVB,
    const ExcelObj& swapPropertiesLVB,
    const ExcelObj& isXccySwap,
    const ExcelObj& validateKeys ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const std::string name = decorateWithExcelLocation( toNarrowString( swapName ) );

    const std::string storedName = validation::tryAqSwapObjectCreateFromSchedule(
        name, getNameWithoutCounter( schedule1Name ), getNameWithoutCounter( schedule2Name ),
        toLabelValueBlock( leg1LVB ), toLabelValueBlock( leg2LVB ), toLabelValueBlock( swapPropertiesLVB ),
        toBool( isXccySwap, false ), toBool( validateKeys, true ) );

    return returnValue( appendInstanceCounter( storedName ) );
}
XLO_FUNC_END( aqSwapObjectCreateFromSchedule )
    .help( L"Create and store a swap from two cached schedules plus leg label/value blocks; returns its handle." )
    .arg( L"SwapName",           L"Name for the swap object" )
    .arg( L"Schedule1Name",      L"A cached schedule handle for leg 1" )
    .arg( L"Schedule2Name",      L"A cached schedule handle for leg 2" )
    .arg( L"Leg1LVB",            L"Leg 1 definition as a label/value block" )
    .arg( L"Leg2LVB",            L"Leg 2 definition as a label/value block" )
    .arg( L"SwapPropertiesLVB",  L"Swap-level properties as a label/value block" )
    .arg( L"IsXccySwap",         L"Optional. Default FALSE. Enforce this is a cross-currency swap" )
    .arg( L"ValidateKeys",       L"Optional. Default TRUE. Check the LVB keys" );
#endif


#if AQ_XLL_ENABLED(aqSwapObjectCreateBackToBack)
XLO_FUNC_START( aqSwapObjectCreateBackToBack(
    const ExcelObj& fromSwapName,
    const ExcelObj& toSwapName ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqSwapObjectCreateBackToBack(
        getNameWithoutCounter( fromSwapName ), getNameWithoutCounter( toSwapName ) ) );
}
XLO_FUNC_END( aqSwapObjectCreateBackToBack )
    .help( L"Create a back-to-back copy of a cached swap under a new name. Returns the new swap's name." )
    .arg( L"FromSwapName", L"The cached swap handle to copy from" )
    .arg( L"ToSwapName",   L"Name for the new swap" );
#endif


#if AQ_XLL_ENABLED(aqSwapObjectAddLeg)
XLO_FUNC_START( aqSwapObjectAddLeg(
    const ExcelObj& swapName,
    const ExcelObj& legObjectName ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqSwapObjectAddLeg(
        getNameWithoutCounter( swapName ), getNameWithoutCounter( legObjectName ) ) );
}
XLO_FUNC_END( aqSwapObjectAddLeg )
    .help( L"Add a cached leg to a cached swap. Returns the swap's name." )
    .arg( L"SwapName",      L"A cached swap handle" )
    .arg( L"LegObjectName", L"A cached leg handle to add" );
#endif


#if AQ_XLL_ENABLED(aqSwapObjectAddFee)
XLO_FUNC_START( aqSwapObjectAddFee(
    const ExcelObj& swapName,
    const ExcelObj& feeName ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqSwapObjectAddFee(
        getNameWithoutCounter( swapName ), getNameWithoutCounter( feeName ) ) );
}
XLO_FUNC_END( aqSwapObjectAddFee )
    .help( L"Add a cached fee leg to a cached swap. Returns the swap's name." )
    .arg( L"SwapName", L"A cached swap handle" )
    .arg( L"FeeName",  L"A cached fee-leg handle to add" );
#endif


#if AQ_XLL_ENABLED(aqSwapGeneratorCreate)
XLO_FUNC_START( aqSwapGeneratorCreate(
    const ExcelObj& swapGeneratorName,
    const ExcelObj& swapGeneratorLVB,
    const ExcelObj& validateKeys ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const std::string name = decorateWithExcelLocation( toNarrowString( swapGeneratorName ) );

    const std::string storedName = validation::tryAqSwapGeneratorCreate(
        name, toAQLStringMatrix( swapGeneratorLVB ), toBool( validateKeys, true ) );

    return returnValue( appendInstanceCounter( storedName ) );
}
XLO_FUNC_END( aqSwapGeneratorCreate )
    .help( L"Create and store a swap generator (conventions template); returns its handle." )
    .arg( L"SwapGeneratorName", L"Name for the swap-generator object" )
    .arg( L"SwapGeneratorLVB",  L"The generator's conventions as a label/value block" )
    .arg( L"ValidateKeys",      L"Optional. Default TRUE. Check the LVB keys" );
#endif


#if AQ_XLL_ENABLED(aqSwapObjectCreateFromGenerator)
XLO_FUNC_START( aqSwapObjectCreateFromGenerator(
    const ExcelObj& swapName,
    const ExcelObj& swapGeneratorName,
    const ExcelObj& expressionLVB,
    const ExcelObj& swapPropertiesLVB,
    const ExcelObj& isXccySwap,
    const ExcelObj& validateKeys ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const std::string name = decorateWithExcelLocation( toNarrowString( swapName ) );

    const std::string storedName = validation::tryAqSwapObjectCreateFromGenerator(
        name, getNameWithoutCounter( swapGeneratorName ), toLabelValueBlockOr( expressionLVB ),
        toLabelValueBlockOr( swapPropertiesLVB ), toBool( isXccySwap, false ), toBool( validateKeys, true ) );

    return returnValue( appendInstanceCounter( storedName ) );
}
XLO_FUNC_END( aqSwapObjectCreateFromGenerator )
    .help( L"Create and store a swap from a swap generator plus overrides; returns its handle." )
    .arg( L"SwapName",          L"Name for the swap object" )
    .arg( L"SwapGeneratorName", L"A cached swap-generator handle" )
    .arg( L"ExpressionLVB",     L"Optional. Overrides to customize the swap, as a label/value block" )
    .arg( L"SwapPropertiesLVB", L"Optional. Swap-level properties as a label/value block" )
    .arg( L"IsXccySwap",        L"Optional. Default FALSE. Enforce this is a cross-currency swap" )
    .arg( L"ValidateKeys",      L"Optional. Default TRUE. Check the LVB keys" );
#endif


#if AQ_XLL_ENABLED(aqSwapObjectDisplay)
XLO_FUNC_START( aqSwapObjectDisplay(
    const ExcelObj& swapName ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelStackedLegMatrices(
        validation::tryAqSwapObjectDisplay( getNameWithoutCounter( swapName ) ) ) );
}
XLO_FUNC_END( aqSwapObjectDisplay )
    .help( L"Display a cached swap's input parameters, one block per leg." )
    .arg( L"SwapName", L"A cached swap handle" );
#endif


#if AQ_XLL_ENABLED(aqSwapGeneratorDisplay)
XLO_FUNC_START( aqSwapGeneratorDisplay(
    const ExcelObj& swapGeneratorName ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelMatrix( validation::tryAqSwapGeneratorDisplay(
        getNameWithoutCounter( swapGeneratorName ) ) ) );
}
XLO_FUNC_END( aqSwapGeneratorDisplay )
    .help( L"Display a cached swap generator's configuration." )
    .arg( L"SwapGeneratorName", L"A cached swap-generator handle" );
#endif


/* -------------------------------------------------------------------------
 *  Swap leg object lifecycle
 * ---------------------------------------------------------------------- */

#if AQ_XLL_ENABLED(aqSwapObjectLegCreate)
XLO_FUNC_START( aqSwapObjectLegCreate(
    const ExcelObj& legObjectName,
    const ExcelObj& legLVB,
    const ExcelObj& validateKeys ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const std::string name = decorateWithExcelLocation( toNarrowString( legObjectName ) );

    const std::string storedName = validation::tryAqSwapObjectLegCreate(
        name, toLabelValueBlock( legLVB ), toBool( validateKeys, true ) );

    return returnValue( appendInstanceCounter( storedName ) );
}
XLO_FUNC_END( aqSwapObjectLegCreate )
    .help( L"Create and store a swap leg from a label/value block; returns its handle." )
    .arg( L"LegObjectName", L"Name for the leg object" )
    .arg( L"LegLVB",        L"The leg definition as a label/value block" )
    .arg( L"ValidateKeys",  L"Optional. Default TRUE. Check the LVB keys" );
#endif


#if AQ_XLL_ENABLED(aqSwapObjectLegCreateFromSchedule)
XLO_FUNC_START( aqSwapObjectLegCreateFromSchedule(
    const ExcelObj& legObjectName,
    const ExcelObj& scheduleName,
    const ExcelObj& legLVB,
    const ExcelObj& validateKeys ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const std::string name = decorateWithExcelLocation( toNarrowString( legObjectName ) );

    const std::string storedName = validation::tryAqSwapObjectLegCreateFromSchedule(
        name, getNameWithoutCounter( scheduleName ), toLabelValueBlock( legLVB ), toBool( validateKeys, true ) );

    return returnValue( appendInstanceCounter( storedName ) );
}
XLO_FUNC_END( aqSwapObjectLegCreateFromSchedule )
    .help( L"Create and store a swap leg from a cached schedule plus a label/value block; returns its handle." )
    .arg( L"LegObjectName", L"Name for the leg object" )
    .arg( L"ScheduleName",  L"A cached schedule handle" )
    .arg( L"LegLVB",        L"The leg definition as a label/value block" )
    .arg( L"ValidateKeys",  L"Optional. Default TRUE. Check the LVB keys" );
#endif


#if AQ_XLL_ENABLED(aqSwapObjectLegPV)
XLO_FUNC_START( aqSwapObjectLegPV(
    const ExcelObj& legObjectName,
    const ExcelObj& valuationSettingsLVB,
    const ExcelObj& fixingTableName ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqSwapObjectLegPV(
        getNameWithoutCounter( legObjectName ), toLabelValueBlock( valuationSettingsLVB ),
        toStrOr( fixingTableName, "" ) ) );
}
XLO_FUNC_END( aqSwapObjectLegPV )
    .help( L"PV of a cached swap leg." )
    .arg( L"LegObjectName",         L"A cached leg handle" )
    .arg( L"ValuationSettingsLVB",  L"ModelName, CurveCollection, ValuationDate etc, as a label/value block" )
    .arg( L"FixingTableName",       L"Optional. A cached fixing-table handle" );
#endif


#if AQ_XLL_ENABLED(aqSwapObjectLegDisplay)
XLO_FUNC_START( aqSwapObjectLegDisplay(
    const ExcelObj& legObjectName ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelMatrix( validation::tryAqSwapObjectLegDisplay(
        getNameWithoutCounter( legObjectName ) ) ) );
}
XLO_FUNC_END( aqSwapObjectLegDisplay )
    .help( L"Display a cached swap leg's input parameters." )
    .arg( L"LegObjectName", L"A cached leg handle" );
#endif


#if AQ_XLL_ENABLED(aqSwapObjectLegDisplayCashflows)
XLO_FUNC_START( aqSwapObjectLegDisplayCashflows(
    const ExcelObj& legObjectName,
    const ExcelObj& valuationSettingsLVB,
    const ExcelObj& fixingTableName,
    const ExcelObj& showColumnHeaders,
    const ExcelObj& columnList ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelMatrix( validation::tryAqSwapObjectLegDisplayCashflows(
        getNameWithoutCounter( legObjectName ), toLabelValueBlock( valuationSettingsLVB ),
        toStrOr( fixingTableName, "" ), toBool( showColumnHeaders, true ), toStringVectorOr( columnList ) ) ) );
}
XLO_FUNC_END( aqSwapObjectLegDisplayCashflows )
    .help( L"Display a cached swap leg's cashflows, priced under the given valuation settings." )
    .arg( L"LegObjectName",         L"A cached leg handle" )
    .arg( L"ValuationSettingsLVB",  L"ModelName, CurveCollection, ValuationDate etc, as a label/value block" )
    .arg( L"FixingTableName",       L"Optional. A cached fixing-table handle" )
    .arg( L"ShowColumnHeaders",     L"Optional. Default TRUE" )
    .arg( L"ColumnList",            L"Optional. Column names to include; default all columns" );
#endif


/* -------------------------------------------------------------------------
 *  Swap schedule object lifecycle
 * ---------------------------------------------------------------------- */

#if AQ_XLL_ENABLED(aqSwapObjectScheduleCreate)
XLO_FUNC_START( aqSwapObjectScheduleCreate(
    const ExcelObj& scheduleName,
    const ExcelObj& swapScheduleLVB,
    const ExcelObj& validateKeys ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const std::string name = decorateWithExcelLocation( toNarrowString( scheduleName ) );

    const std::string storedName = validation::tryAqSwapObjectScheduleCreate(
        name, toLabelValueBlock( swapScheduleLVB ), toBool( validateKeys, true ) );

    return returnValue( appendInstanceCounter( storedName ) );
}
XLO_FUNC_END( aqSwapObjectScheduleCreate )
    .help( L"Create and store a swap schedule from a label/value block; returns its handle." )
    .arg( L"ScheduleName",    L"Name for the schedule object" )
    .arg( L"SwapScheduleLVB", L"The schedule configuration as a label/value block" )
    .arg( L"ValidateKeys",    L"Optional. Default TRUE. Check the LVB keys" );
#endif


#if AQ_XLL_ENABLED(aqSwapObjectScheduleDisplay)
XLO_FUNC_START( aqSwapObjectScheduleDisplay(
    const ExcelObj& scheduleName,
    const ExcelObj& showBespokeProperties,
    const ExcelObj& showColumnHeaders,
    const ExcelObj& columnList ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelMatrix( validation::tryAqSwapObjectScheduleDisplay(
        getNameWithoutCounter( scheduleName ), toBool( showBespokeProperties, false ),
        toBool( showColumnHeaders, true ), toStringVectorOr( columnList ) ) ) );
}
XLO_FUNC_END( aqSwapObjectScheduleDisplay )
    .help( L"Display a cached swap schedule." )
    .arg( L"ScheduleName",           L"A cached schedule handle" )
    .arg( L"ShowBespokeProperties",  L"Optional. Default FALSE. Show bespoke schedule properties" )
    .arg( L"ShowColumnHeaders",      L"Optional. Default TRUE" )
    .arg( L"ColumnList",             L"Optional. Column names to include; default all columns" );
#endif


#if AQ_XLL_ENABLED(aqSwapObjectScheduleCreateBespoke)
XLO_FUNC_START( aqSwapObjectScheduleCreateBespoke(
    const ExcelObj& scheduleName,
    const ExcelObj& bespokeScheduleProperties,
    const ExcelObj& bespokeScheduleLVB,
    const ExcelObj& validateKeys ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const std::string name = decorateWithExcelLocation( toNarrowString( scheduleName ) );

    const std::string storedName = validation::tryAqSwapObjectScheduleCreateBespoke(
        name, toLabelValueBlock( bespokeScheduleProperties ), toAQLStringMatrix( bespokeScheduleLVB ),
        toBool( validateKeys, true ) );

    return returnValue( appendInstanceCounter( storedName ) );
}
XLO_FUNC_END( aqSwapObjectScheduleCreateBespoke )
    .help( L"Create and store a bespoke swap schedule from an explicit cashflow matrix; returns its handle." )
    .arg( L"ScheduleName",                L"Name for the bespoke schedule object" )
    .arg( L"BespokeScheduleProperties",   L"Bespoke schedule properties as a label/value block" )
    .arg( L"BespokeScheduleLVB",          L"The bespoke schedule cashflows" )
    .arg( L"ValidateKeys",                L"Optional. Default TRUE. Check BespokeScheduleProperties' keys" );
#endif


#if AQ_XLL_ENABLED(aqSwapObjectScheduleCreateBespokeFromCashflows)
XLO_FUNC_START( aqSwapObjectScheduleCreateBespokeFromCashflows(
    const ExcelObj& scheduleObjectName,
    const ExcelObj& bespokeScheduleProperties,
    const ExcelObj& bespokeCashflowsLVB,
    const ExcelObj& validateKeys ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const std::string name = decorateWithExcelLocation( toNarrowString( scheduleObjectName ) );

    const std::string storedName = validation::tryAqSwapObjectScheduleCreateBespokeFromCashflows(
        name, toLabelValueBlock( bespokeScheduleProperties ), toAQLStringMatrix( bespokeCashflowsLVB ),
        toBool( validateKeys, true ) );

    return returnValue( appendInstanceCounter( storedName ) );
}
XLO_FUNC_END( aqSwapObjectScheduleCreateBespokeFromCashflows )
    .help( L"Create and store a bespoke swap schedule from a cashflow matrix; returns its handle." )
    .arg( L"ScheduleObjectName",          L"Name for the bespoke schedule object" )
    .arg( L"BespokeScheduleProperties",   L"Bespoke schedule properties as a label/value block" )
    .arg( L"BespokeCashflowsLVB",         L"The bespoke cashflows matrix" )
    .arg( L"ValidateKeys",                L"Optional. Default TRUE. Check BespokeScheduleProperties' keys" );
#endif


/* -------------------------------------------------------------------------
 *  Swap object pricing
 * ---------------------------------------------------------------------- */

#if AQ_XLL_ENABLED(aqSwapObjectPV)
XLO_FUNC_START( aqSwapObjectPV(
    const ExcelObj& swapName,
    const ExcelObj& valuationSettingsLVB,
    const ExcelObj& legName,
    const ExcelObj& fixingTableNames ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqSwapObjectPV(
        getNameWithoutCounter( swapName ), toLabelValueBlock( valuationSettingsLVB ),
        toAQLString( legName ), toLabelValueBlockOr( fixingTableNames ) ) );
}
XLO_FUNC_END( aqSwapObjectPV )
    .help( L"PV of a cached swap (or one of its legs)." )
    .arg( L"SwapName",              L"A cached swap handle" )
    .arg( L"ValuationSettingsLVB",  L"ModelName, CurveCollection, ValuationDate etc, as a label/value block" )
    .arg( L"LegName",               L"Optional. Price just this leg; default prices the whole swap" )
    .arg( L"FixingTableNames",      L"Optional. Fixing-table name overrides as a label/value block" );
#endif


#if AQ_XLL_ENABLED(aqSwapObjectPV01)
XLO_FUNC_START( aqSwapObjectPV01(
    const ExcelObj& swapName,
    const ExcelObj& valuationSettingsLVB,
    const ExcelObj& fixingTableNames ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqSwapObjectPV01(
        getNameWithoutCounter( swapName ), toLabelValueBlock( valuationSettingsLVB ),
        toLabelValueBlockOr( fixingTableNames ) ) );
}
XLO_FUNC_END( aqSwapObjectPV01 )
    .help( L"PV01 of a cached swap." )
    .arg( L"SwapName",              L"A cached swap handle" )
    .arg( L"ValuationSettingsLVB",  L"ModelName, CurveCollection, ValuationDate etc, as a label/value block" )
    .arg( L"FixingTableNames",      L"Optional. Fixing-table name overrides as a label/value block" );
#endif


#if AQ_XLL_ENABLED(aqSwapObjectParRate)
XLO_FUNC_START( aqSwapObjectParRate(
    const ExcelObj& swapName,
    const ExcelObj& valuationSettingsLVB,
    const ExcelObj& fixingTableNames,
    const ExcelObj& legName ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqSwapObjectParRate(
        getNameWithoutCounter( swapName ), toLabelValueBlock( valuationSettingsLVB ),
        toLabelValueBlockOr( fixingTableNames ), toStrOr( legName, "" ) ) );
}
XLO_FUNC_END( aqSwapObjectParRate )
    .help( L"Par rate of a cached swap." )
    .arg( L"SwapName",              L"A cached swap handle" )
    .arg( L"ValuationSettingsLVB",  L"ModelName, CurveCollection, ValuationDate etc, as a label/value block" )
    .arg( L"FixingTableNames",      L"Optional. Fixing-table name overrides as a label/value block" )
    .arg( L"LegName",               L"Optional. Leg to solve the par rate on" );
#endif


#if AQ_XLL_ENABLED(aqSwapObjectAnnuity)
XLO_FUNC_START( aqSwapObjectAnnuity(
    const ExcelObj& swapName,
    const ExcelObj& valuationSettingsLVB,
    const ExcelObj& legName ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqSwapObjectAnnuity(
        getNameWithoutCounter( swapName ), toLabelValueBlock( valuationSettingsLVB ), toAQLString( legName ) ) );
}
XLO_FUNC_END( aqSwapObjectAnnuity )
    .help( L"Annuity of a cached swap's leg." )
    .arg( L"SwapName",              L"A cached swap handle" )
    .arg( L"ValuationSettingsLVB",  L"ModelName, CurveCollection, ValuationDate etc, as a label/value block" )
    .arg( L"LegName",               L"The leg to compute the annuity of" );
#endif


#if AQ_XLL_ENABLED(aqSwapObjectDisplayCashflows)
XLO_FUNC_START( aqSwapObjectDisplayCashflows(
    const ExcelObj& swapName,
    const ExcelObj& valuationSettingsLVB,
    const ExcelObj& legName,
    const ExcelObj& fixingTableNames,
    const ExcelObj& showColumnHeaders,
    const ExcelObj& columnList ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelStackedLegMatrices( validation::tryAqSwapObjectDisplayCashflows(
        getNameWithoutCounter( swapName ), toLabelValueBlock( valuationSettingsLVB ), toAQLString( legName ),
        toLabelValueBlockOr( fixingTableNames ), toBool( showColumnHeaders, true ), toStringVectorOr( columnList ) ) ) );
}
XLO_FUNC_END( aqSwapObjectDisplayCashflows )
    .help( L"Display a cached swap's cashflows, one block per leg (or just the named leg)." )
    .arg( L"SwapName",              L"A cached swap handle" )
    .arg( L"ValuationSettingsLVB",  L"ModelName, CurveCollection, ValuationDate etc, as a label/value block" )
    .arg( L"LegName",               L"Optional. Display just this leg; default displays every leg" )
    .arg( L"FixingTableNames",      L"Optional. Fixing-table name overrides as a label/value block" )
    .arg( L"ShowColumnHeaders",     L"Optional. Default TRUE" )
    .arg( L"ColumnList",            L"Optional. Column names to include; default all columns" );
#endif


#if AQ_XLL_ENABLED(aqSwapObjectSpread)
XLO_FUNC_START( aqSwapObjectSpread(
    const ExcelObj& swapName,
    const ExcelObj& valuationSettingsLVB,
    const ExcelObj& fixingTableNames,
    const ExcelObj& spreadLegName ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqSwapObjectSpread(
        getNameWithoutCounter( swapName ), toLabelValueBlock( valuationSettingsLVB ),
        toLabelValueBlockOr( fixingTableNames ), toStrOr( spreadLegName, "" ) ) );
}
XLO_FUNC_END( aqSwapObjectSpread )
    .help( L"PV of a cached swap given its legs' existing spreads." )
    .arg( L"SwapName",              L"A cached swap handle" )
    .arg( L"ValuationSettingsLVB",  L"ModelName, CurveCollection, ValuationDate etc, as a label/value block" )
    .arg( L"FixingTableNames",      L"Optional. Fixing-table name overrides as a label/value block" )
    .arg( L"SpreadLegName",         L"Optional. The leg the spread is quoted on" );
#endif


#if AQ_XLL_ENABLED(aqSwapObjectParSpread)
XLO_FUNC_START( aqSwapObjectParSpread(
    const ExcelObj& swapName,
    const ExcelObj& valuationSettingsLVB,
    const ExcelObj& fixingTableNames,
    const ExcelObj& spreadLegName ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqSwapObjectParSpread(
        getNameWithoutCounter( swapName ), toLabelValueBlock( valuationSettingsLVB ),
        toLabelValueBlockOr( fixingTableNames ), toStrOr( spreadLegName, "" ) ) );
}
XLO_FUNC_END( aqSwapObjectParSpread )
    .help( L"The spread that makes a cached swap's PV zero, ignoring the legs' existing spreads." )
    .arg( L"SwapName",              L"A cached swap handle" )
    .arg( L"ValuationSettingsLVB",  L"ModelName, CurveCollection, ValuationDate etc, as a label/value block" )
    .arg( L"FixingTableNames",      L"Optional. Fixing-table name overrides as a label/value block" )
    .arg( L"SpreadLegName",         L"Optional. The leg the spread is quoted on" );
#endif


#if AQ_XLL_ENABLED(aqSwapObjectAccruedInterest)
XLO_FUNC_START( aqSwapObjectAccruedInterest(
    const ExcelObj& swapName,
    const ExcelObj& valuationSettingsLVB,
    const ExcelObj& legName,
    const ExcelObj& fixingTableName ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqSwapObjectAccruedInterest(
        getNameWithoutCounter( swapName ), toLabelValueBlock( valuationSettingsLVB ),
        toAQLString( legName ), toLabelValueBlock( fixingTableName ) ) );
}
XLO_FUNC_END( aqSwapObjectAccruedInterest )
    .help( L"Accrued interest of a cached swap's leg." )
    .arg( L"SwapName",              L"A cached swap handle" )
    .arg( L"ValuationSettingsLVB",  L"ModelName, CurveCollection, ValuationDate etc, as a label/value block" )
    .arg( L"LegName",               L"The leg to compute accrued interest for" )
    .arg( L"FixingTableName",       L"Fixing-table name overrides as a label/value block" );
#endif


/* -------------------------------------------------------------------------
 *  Swap results / Jacobian risk store
 * ---------------------------------------------------------------------- */

#if AQ_XLL_ENABLED(aqSwapResultsEnable)
XLO_FUNC_START( aqSwapResultsEnable(
    const ExcelObj& enable ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqSwapResultsEnable( toBool( enable, true ) ) );
}
XLO_FUNC_END( aqSwapResultsEnable )
    .help( L"Enable or disable the swap results (Jacobian risk) store. Returns a status string." )
    .arg( L"Enable", L"TRUE to enable, FALSE to disable" );
#endif


#if AQ_XLL_ENABLED(aqSwapResultsIsEnabled)
XLO_FUNC_START( aqSwapResultsIsEnabled() )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqSwapResultsIsEnabled() );
}
XLO_FUNC_END( aqSwapResultsIsEnabled )
    .help( L"Whether the swap results (Jacobian risk) store is currently enabled." );
#endif


#if AQ_XLL_ENABLED(aqSwapResultsRiskUpdate)
XLO_FUNC_START( aqSwapResultsRiskUpdate(
    const ExcelObj& swapHandle,
    const ExcelObj& asOfDate,
    const ExcelObj& discountRiskLVB,
    const ExcelObj& forwardRiskLVB ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqSwapResultsRiskUpdate(
        getNameWithoutCounter( swapHandle ), toAQLDate( asOfDate ),
        toAQLStringMatrix( discountRiskLVB ), toAQLStringMatrix( forwardRiskLVB ) ) );
}
XLO_FUNC_END( aqSwapResultsRiskUpdate )
    .help( L"Update a swap's stored discount and forward-rate risk. Returns a status string." )
    .arg( L"SwapHandle",       L"A cached swap handle" )
    .arg( L"AsOfDate",         L"The as-of date for the risk being stored" )
    .arg( L"DiscountRiskLVB",  L"Table of payment dates and coupons" )
    .arg( L"ForwardRiskLVB",   L"Table of fixing dates and annuities" );
#endif


#if AQ_XLL_ENABLED(aqSwapResultsDiscountRiskUpdate)
XLO_FUNC_START( aqSwapResultsDiscountRiskUpdate(
    const ExcelObj& swapHandle,
    const ExcelObj& asOfDate,
    const ExcelObj& discountRiskLVB ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqSwapResultsDiscountRiskUpdate(
        getNameWithoutCounter( swapHandle ), toAQLDate( asOfDate ), toAQLStringMatrix( discountRiskLVB ) ) );
}
XLO_FUNC_END( aqSwapResultsDiscountRiskUpdate )
    .help( L"Update a swap's stored discount-rate risk. Returns a status string." )
    .arg( L"SwapHandle",       L"A cached swap handle" )
    .arg( L"AsOfDate",         L"The as-of date for the risk being stored" )
    .arg( L"DiscountRiskLVB",  L"Table of payment dates and coupons" );
#endif


#if AQ_XLL_ENABLED(aqSwapResultsForwardRiskUpdate)
XLO_FUNC_START( aqSwapResultsForwardRiskUpdate(
    const ExcelObj& swapHandle,
    const ExcelObj& asOfDate,
    const ExcelObj& forwardRiskLVB ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqSwapResultsForwardRiskUpdate(
        getNameWithoutCounter( swapHandle ), toAQLDate( asOfDate ), toAQLStringMatrix( forwardRiskLVB ) ) );
}
XLO_FUNC_END( aqSwapResultsForwardRiskUpdate )
    .help( L"Update a swap's stored forward-rate risk. Returns a status string." )
    .arg( L"SwapHandle",     L"A cached swap handle" )
    .arg( L"AsOfDate",       L"The as-of date for the risk being stored" )
    .arg( L"ForwardRiskLVB", L"Table of fixing dates and annuities" );
#endif


#if AQ_XLL_ENABLED(aqSwapResultsDelete)
XLO_FUNC_START( aqSwapResultsDelete(
    const ExcelObj& swapHandle ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqSwapResultsDelete( getNameWithoutCounter( swapHandle ) ) );
}
XLO_FUNC_END( aqSwapResultsDelete )
    .help( L"Delete one swap results object. Returns a status string." )
    .arg( L"SwapHandle", L"A cached swap handle" );
#endif


#if AQ_XLL_ENABLED(aqSwapResultsDeleteAll)
XLO_FUNC_START( aqSwapResultsDeleteAll() )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqSwapResultsDeleteAll() );
}
XLO_FUNC_END( aqSwapResultsDeleteAll )
    .help( L"Delete every swap results object. Returns a status string." );
#endif


#if AQ_XLL_ENABLED(aqSwapResultsRiskTotals)
XLO_FUNC_START( aqSwapResultsRiskTotals(
    const ExcelObj& swapHandle,
    const ExcelObj& riskType ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelDoubleColumn( validation::tryAqSwapResultsRiskTotals(
        getNameWithoutCounter( swapHandle ), etrading::toRiskTypeEnum( toNarrowString( riskType ) ) ) ) );
}
XLO_FUNC_END( aqSwapResultsRiskTotals )
    .help( L"Risk totals stored for a swap results object." )
    .arg( L"SwapHandle", L"A cached swap handle" )
    .arg( L"RiskType",   L"The risk type to total" );
#endif


#if AQ_XLL_ENABLED(aqSwapResultsDisplay)
XLO_FUNC_START( aqSwapResultsDisplay(
    const ExcelObj& swapHandle ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelMatrix( validation::tryAqSwapResultsDisplay(
        getNameWithoutCounter( swapHandle ) ) ) );
}
XLO_FUNC_END( aqSwapResultsDisplay )
    .help( L"Display a swap results object." )
    .arg( L"SwapHandle", L"A cached swap handle" );
#endif


/* -------------------------------------------------------------------------
 *  Swap risk (single-swap flat-shift delta; see the file header for the
 *  deferred multi-trade delta-ladder / stateless-Delta functions)
 * ---------------------------------------------------------------------- */

#if AQ_XLL_ENABLED(aqSwapObjectDelta)
XLO_FUNC_START( aqSwapObjectDelta(
    const ExcelObj& swapNames,
    const ExcelObj& curveCollectionNames,
    const ExcelObj& fixingTableNames,
    const ExcelObj& bumpSpreadInstruments,
    const ExcelObj& bumpSize,
    const ExcelObj& bumpMode,
    const ExcelObj& groupRiskBy,
    const ExcelObj& aggregateRisks,
    const ExcelObj& reportInLegCCY,
    const ExcelObj& xccyFXSpotRates ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    AQLStringVector positionIDs;
    DoubleVector    deltas;

    validation::tryAqSwapObjectDelta(
        positionIDs, deltas, toAQLStringVectorWithoutCounter( swapNames ),
        toAQLStringMatrix( curveCollectionNames ), toAQLStringMatrix( fixingTableNames ),
        toBool( bumpSpreadInstruments, true ), bumpSize.get<double>(), toAQLString( bumpMode ),
        toAQLString( groupRiskBy ), toBool( aggregateRisks, true ), toBool( reportInLegCCY, false ),
        toDoubleVectorOr( xccyFXSpotRates, "XccyFXSpotRates" ) );

    etrading::VariantMatrix result;
    etrading::VariantVector header;
    header.push_back( etrading::Variant( "PositionID" ) );
    header.push_back( etrading::Variant( "Delta" ) );
    result.push_back( header );

    for ( std::size_t i = 0; i < positionIDs.size(); ++i )
    {
        etrading::VariantVector row;
        row.push_back( etrading::Variant( positionIDs[i].getCString() ) );
        row.push_back( etrading::Variant( i < deltas.size() ? deltas[i] : 0.0 ) );
        result.push_back( row );
    }

    return returnValue( toExcelMatrix( result ) );
}
XLO_FUNC_END( aqSwapObjectDelta )
    .help( L"Flat-shift delta for a column of cached swaps, one row per position." )
    .arg( L"SwapNames",              L"Column of cached swap handles" )
    .arg( L"CurveCollectionNames",   L"Curve collection name(s) per swap leg" )
    .arg( L"FixingTableNames",       L"Fixing table name(s) per swap leg" )
    .arg( L"BumpSpreadInstruments",  L"Optional. Default TRUE. Bump LIBOR-OIS spread instruments in the OIS curve" )
    .arg( L"BumpSize",               L"Bump size, e.g. 0.0001 for 1bp" )
    .arg( L"BumpMode",               L"Up, Down, or Central bumping" )
    .arg( L"GroupRiskBy",            L"How to group the reported risk" )
    .arg( L"AggregateRisks",         L"Optional. Default TRUE. Aggregate risk against the same instrument across curves" )
    .arg( L"ReportInLegCCY",         L"Optional. Default FALSE. Report in each leg's own currency rather than valuation currency" )
    .arg( L"XccyFXSpotRates",        L"Optional. Column of cross-currency FX spot rates" );
#endif


/* -------------------------------------------------------------------------
 *  Multi-trade risk: delta ladders and the batch-PV overload. Deferred
 *  earlier this session pending a vector<LabelValueBlock> / multi-trade
 *  marshalling design - now built (see toLabelValueBlockVector and
 *  toExcelLabeledMatrix above).
 * ---------------------------------------------------------------------- */

// Batch PV across several cached swaps in one call (optimize=true uses OMP threading).
#if AQ_XLL_ENABLED(aqSwapObjectPVs)
XLO_FUNC_START( aqSwapObjectPVs(
    const ExcelObj& swapNames,
    const ExcelObj& valuationSettingsLVBs,
    const ExcelObj& legNames,
    const ExcelObj& fixingTableNamesLVBs,
    const ExcelObj& optimize ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    std::vector<std::string> legNamesVec = toStringVectorOr( legNames );
    std::vector<etrading::LabelValueBlock> fixingTables = toLabelValueBlockVector( fixingTableNamesLVBs );

    const std::vector<double> pvs = validation::tryAqSwapObjectPVs(
        getNamesWithoutCounter( swapNames ), toLabelValueBlockVector( valuationSettingsLVBs ),
        legNamesVec, fixingTables, toBool( optimize, false ) );

    return returnValue( toExcelDoubleColumn( pvs ) );
}
XLO_FUNC_END( aqSwapObjectPVs )
    .help( L"PV of several cached swaps in one call. ValuationSettingsLVBs/FixingTableNamesLVBs are tables: "
           L"row 1 is the shared key headers, one further row per swap (aligned with SwapNames)." )
    .arg( L"SwapNames",              L"Column of cached swap handles" )
    .arg( L"ValuationSettingsLVBs",  L"Valuation settings table: header row of keys, one row per swap" )
    .arg( L"LegNames",               L"Optional. Column of leg names to price, aligned with SwapNames; blank prices the whole swap" )
    .arg( L"FixingTableNamesLVBs",   L"Optional. Fixing-table overrides table: header row of keys, one row per swap" )
    .arg( L"Optimize",               L"Optional. Default FALSE. Use OMP threading" );
#endif


// Stateless flat-shift delta for several trades defined inline (no cached
// swap objects needed).
#if AQ_XLL_ENABLED(aqSwapDelta)
XLO_FUNC_START( aqSwapDelta(
    const ExcelObj& dealInfoLVBs,
    const ExcelObj& forecastCurveSet,
    const ExcelObj& curveCollection,
    const ExcelObj& bumpSize,
    const ExcelObj& bumpMode,
    const ExcelObj& deltaType,
    const ExcelObj& aggregateRisk ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    AQLStringVector pillarNames;
    AQLStringVector headers;
    DoubleMatrix    deltas;

    validation::tryAqSwapDelta(
        pillarNames, headers, deltas, etrading::getDataInstance(),
        toLabelValueBlockVector( dealInfoLVBs ), toAQLStringVector( forecastCurveSet ),
        toAQLString( curveCollection ), bumpSize.get<double>(), toAQLString( bumpMode ),
        toAQLString( deltaType ), toBool( aggregateRisk, true ) );

    return returnValue( toExcelLabeledMatrix( headers, pillarNames, deltas ) );
}
XLO_FUNC_END( aqSwapDelta )
    .help( L"Flat-shift delta for several trades defined inline. DealInfoLVBs is a table: header row of keys, "
           L"one row per trade. Returns a pillar x header delta matrix." )
    .arg( L"DealInfoLVBs",      L"Trade definitions table: header row of keys, one row per trade" )
    .arg( L"ForecastCurveSet",  L"Column of forecasting curve indices" )
    .arg( L"CurveCollection",   L"The curve collection" )
    .arg( L"BumpSize",          L"Bump size, e.g. 0.0001 for 1bp" )
    .arg( L"BumpMode",          L"Up, Down, or Central bumping" )
    .arg( L"DeltaType",         L"Flat-shift or ladder delta type" )
    .arg( L"AggregateRisk",     L"Optional. Default TRUE. Aggregate risk against the same instrument across curves" );
#endif


// Delta ladder (pillar-by-pillar risk) for a column of cached swaps.
#if AQ_XLL_ENABLED(aqSwapObjectDeltaLadder)
XLO_FUNC_START( aqSwapObjectDeltaLadder(
    const ExcelObj& swapNames,
    const ExcelObj& curveCollectionNames,
    const ExcelObj& fixingTableNames,
    const ExcelObj& bumpSpreadInstruments,
    const ExcelObj& bumpSize,
    const ExcelObj& bumpMode,
    const ExcelObj& aggregateRisk,
    const ExcelObj& reportInLegCCY,
    const ExcelObj& riskCutOffTenor,
    const ExcelObj& xccyFXSpotRates ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    AQLStringVector headers;
    AQLStringVector pillarNames;
    DoubleMatrix    deltas;

    validation::tryAqSwapObjectDeltaLadder(
        headers, pillarNames, deltas, toAQLStringVectorWithoutCounter( swapNames ),
        toAQLStringMatrix( curveCollectionNames ), toAQLStringMatrix( fixingTableNames ),
        toBool( bumpSpreadInstruments, true ), bumpSize.get<double>(), toAQLString( bumpMode ),
        toBool( aggregateRisk, true ), toBool( reportInLegCCY, false ), toNarrowString( riskCutOffTenor ),
        toDoubleVectorOr( xccyFXSpotRates, "XccyFXSpotRates" ) );

    return returnValue( toExcelLabeledMatrix( headers, pillarNames, deltas ) );
}
XLO_FUNC_END( aqSwapObjectDeltaLadder )
    .help( L"Pillar-by-pillar delta ladder for a column of cached swaps. Returns a pillar x curve delta matrix." )
    .arg( L"SwapNames",              L"Column of cached swap handles" )
    .arg( L"CurveCollectionNames",   L"Curve collection name(s) per swap leg" )
    .arg( L"FixingTableNames",       L"Fixing table name(s) per swap leg" )
    .arg( L"BumpSpreadInstruments",  L"Optional. Default TRUE. Bump LIBOR-OIS spread instruments in the OIS curve" )
    .arg( L"BumpSize",               L"Bump size, e.g. 0.0001 for 1bp" )
    .arg( L"BumpMode",               L"Up, Down, or Central bumping" )
    .arg( L"AggregateRisk",          L"Optional. Default TRUE. Aggregate risk against the same instrument across curves" )
    .arg( L"ReportInLegCCY",         L"Optional. Default FALSE. Report in each leg's own currency rather than valuation currency" )
    .arg( L"RiskCutOffTenor",        L"The maximum tenor (e.g. 10Y) beyond which curves are no longer bumped" )
    .arg( L"XccyFXSpotRates",        L"Optional. Column of cross-currency FX spot rates" );
#endif


// As aqSwapObjectDeltaLadder, with each curve's pillar/delta pair presented
// in its own two columns (a header row, then paired PillarName/Delta columns).
#if AQ_XLL_ENABLED(aqSwapObjectDeltaLadderHorizontally)
XLO_FUNC_START( aqSwapObjectDeltaLadderHorizontally(
    const ExcelObj& swapNames,
    const ExcelObj& curveCollectionNames,
    const ExcelObj& fixingTableNames,
    const ExcelObj& bumpSpreadInstruments,
    const ExcelObj& bumpSize,
    const ExcelObj& bumpMode,
    const ExcelObj& aggregateRisk,
    const ExcelObj& reportInLegCCY,
    const ExcelObj& riskCutOffTenor,
    const ExcelObj& xccyFXSpotRates ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    AQLStringVector                headers;
    std::vector<AQLStringVector>   pillarNames;
    std::vector<DoubleVector>      deltas;

    validation::tryAqSwapObjectDeltaLadderHorizontally(
        headers, pillarNames, deltas, toAQLStringVectorWithoutCounter( swapNames ),
        toAQLStringMatrix( curveCollectionNames ), toAQLStringMatrix( fixingTableNames ),
        toBool( bumpSpreadInstruments, true ), bumpSize.get<double>(), toAQLString( bumpMode ),
        toBool( aggregateRisk, true ), toBool( reportInLegCCY, false ), toNarrowString( riskCutOffTenor ),
        toDoubleVectorOr( xccyFXSpotRates, "XccyFXSpotRates" ) );

    std::size_t maxRows = 0;
    for ( const AQLStringVector& column : pillarNames )
    {
        maxRows = std::max( maxRows, column.size() );
    }

    etrading::VariantMatrix result;

    etrading::VariantVector headerRow;
    for ( std::size_t c = 0; c < headers.size(); ++c )
    {
        headerRow.push_back( etrading::Variant( headers[c].getCString() ) );
        headerRow.push_back( etrading::Variant( "" ) );
    }
    result.push_back( headerRow );

    for ( std::size_t r = 0; r < maxRows; ++r )
    {
        etrading::VariantVector row;
        for ( std::size_t c = 0; c < headers.size(); ++c )
        {
            if ( c < pillarNames.size() && r < pillarNames[c].size() )
            {
                row.push_back( etrading::Variant( pillarNames[c][r].getCString() ) );
                row.push_back( etrading::Variant( ( c < deltas.size() && r < deltas[c].size() ) ? deltas[c][r] : 0.0 ) );
            }
            else
            {
                row.push_back( etrading::Variant( "" ) );
                row.push_back( etrading::Variant( "" ) );
            }
        }
        result.push_back( row );
    }

    return returnValue( toExcelMatrix( result ) );
}
XLO_FUNC_END( aqSwapObjectDeltaLadderHorizontally )
    .help( L"As aqSwapObjectDeltaLadder, with each curve's pillar/delta pair in its own two columns." )
    .arg( L"SwapNames",              L"Column of cached swap handles" )
    .arg( L"CurveCollectionNames",   L"Curve collection name(s) per swap leg" )
    .arg( L"FixingTableNames",       L"Fixing table name(s) per swap leg" )
    .arg( L"BumpSpreadInstruments",  L"Optional. Default TRUE. Bump LIBOR-OIS spread instruments in the OIS curve" )
    .arg( L"BumpSize",               L"Bump size, e.g. 0.0001 for 1bp" )
    .arg( L"BumpMode",               L"Up, Down, or Central bumping" )
    .arg( L"AggregateRisk",          L"Optional. Default TRUE. Aggregate risk against the same instrument across curves" )
    .arg( L"ReportInLegCCY",         L"Optional. Default FALSE. Report in each leg's own currency rather than valuation currency" )
    .arg( L"RiskCutOffTenor",        L"The maximum tenor (e.g. 10Y) beyond which curves are no longer bumped" )
    .arg( L"XccyFXSpotRates",        L"Optional. Column of cross-currency FX spot rates" );
#endif
