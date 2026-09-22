/*
 * Generator category worksheet functions - introspection over the Swap/Bond/
 * Curve generator templates shipped under resources\config\<TYPE>_GENERATOR,
 * not construction (that stays in each asset category, e.g. aqSwapGenerator*,
 * aqBondGenerator*, aqCurveGenerator*). See CLAUDE.md Section 5.1.
 */

#include <xllMain.h>

#include <string>
#include <vector>

#include <xllSupport.h>
#include <CoreEnumerations.h>   // etrading::trim_to_upper
#include <Variant.h>            // etrading::VariantMatrix
#include <tryAqGenerator.h>     // validation::tryAqGenerator{List,Display,Validate}

using namespace aq_xll;


#if AQ_XLL_ENABLED(aqGeneratorList)
XLO_FUNC_START( aqGeneratorList(
    const ExcelObj& generatorType ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const std::string type = etrading::trim_to_upper( toNarrowString( generatorType ) );

    const std::vector<std::string> names = validation::tryAqGeneratorList( type );

    return returnValue( toExcelColumn( names ) );
}
XLO_FUNC_END( aqGeneratorList )
    .help( L"The generator names available on disk for a generator type, as a column." )
    .arg( L"GeneratorType", L"SWAP_GENERATOR, BOND_GENERATOR or CURVE_GENERATOR" );
#endif


#if AQ_XLL_ENABLED(aqGeneratorDisplay)
XLO_FUNC_START( aqGeneratorDisplay(
    const ExcelObj& generatorType,
    const ExcelObj& generatorName ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const std::string type = etrading::trim_to_upper( toNarrowString( generatorType ) );
    const std::string name = etrading::trim_to_upper( toNarrowString( generatorName ) );

    const etrading::VariantMatrix properties = validation::tryAqGeneratorDisplay( type, name );

    // Transposed: the underlying block is column-major (one column per
    // key/value pair); Excel users expect key/value rows instead - same
    // convention as aqBondGeneratorDisplay.
    return returnValue( toExcelMatrix( etrading::toAQLStringMatrixFromVariantMatrix( properties, true ) ) );
}
XLO_FUNC_END( aqGeneratorDisplay )
    .help( L"The properties of one generator, read directly from its JSON file on disk." )
    .arg( L"GeneratorType", L"SWAP_GENERATOR, BOND_GENERATOR or CURVE_GENERATOR" )
    .arg( L"GeneratorName", L"The generator name" );
#endif


#if AQ_XLL_ENABLED(aqGeneratorValidate)
XLO_FUNC_START( aqGeneratorValidate(
    const ExcelObj& generatorType,
    const ExcelObj& generatorName ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const std::string type = etrading::trim_to_upper( toNarrowString( generatorType ) );
    const std::string name = etrading::trim_to_upper( toNarrowString( generatorName ) );

    const std::string result = validation::tryAqGeneratorValidate( type, name );

    return returnValue( result );
}
XLO_FUNC_END( aqGeneratorValidate )
    .help( L"Checks that a generator's JSON file parses and satisfies its schema. Returns OK, or the reason it failed." )
    .arg( L"GeneratorType", L"SWAP_GENERATOR, BOND_GENERATOR or CURVE_GENERATOR" )
    .arg( L"GeneratorName", L"The generator name" );
#endif
