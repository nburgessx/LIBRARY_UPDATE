#include <aqMain.h>

#include <algorithm>
#include <cstdio>
#include <ctime>
#include <string>

#include <vector>

#include <aqXllTools.h>
#include <FolderConfig.h>          // etrading::FolderConfig
#include <AQLString.h>
#include <AQLCoreTemplateType.h>   // AQLStringVector, AQLStringMatrix, DoubleMatrix, StandardStringMatrix
#include <Variant.h>               // etrading::VariantMatrix, etrading::VariantVector
#include <tryAqToolSetup.h>        // validation::tryAqTool{Version,ClearEntityPool,LoadStaticData,LoadCalendarFile,LoadConfigurationFiles,ParallelModeEnable,ParallelModeStatus}
#include <tryAqToolRecord.h>       // validation::tryAqToolRecord
#include <tryAqToolReplay.h>       // validation::tryAqToolReplay
#include <tryAqToolLVB.h>          // validation::tryAqToolLVB*
#include <tryAqToolClean.h>        // validation::tryAqToolClean
#include <tryAqToolAppend.h>       // validation::tryAqToolAppend
#include <tryAqToolDataFilter.h>   // validation::tryAqToolDataFilter
#include <tryAqToolValuationSettings.h>  // validation::tryAqToolValuationSettingsDisplay
#include <tryAqToolDate.h>         // validation::tryAqToolTermsToDates / tryAqToolDatesToTerms
#include <tryAqToolGrid.h>         // validation::tryAqToolObjectGrid*
#include <tryAqToolMultiGrid.h>   // validation::tryAqToolObjectMultiGrid*
#include <tryAqToolEchoDouble.h>  // validation::tryAqToolEchoDouble
#include <tryAqBondObject.h>      // validation::tryAqToolBondAverageYield / tryAqToolBondYieldFromFuturePrice (filed under Bond, golden-named Tool)
#include <tryAqSwapObjectSchedule.h>  // validation::tryAqToolSwapScheduleTemplate (filed under Swap, golden-named Tool)

using namespace aq_xll;

namespace
{
    // A (data, column-names) pair as an Excel array: the names become a header
    // row above the data. Backs the grid Display functions.
    xloil::ExcelObj flexibleDataToExcel( const std::pair<const validation::FlexibleData, std::vector<std::string>>& result )
    {
        const validation::FlexibleData& data = result.first;
        const std::vector<std::string>& names = result.second;

        etrading::VariantMatrix matrix;
        if ( !names.empty() )
        {
            etrading::VariantVector header;
            header.reserve( names.size() );
            for ( const std::string& name : names )
            {
                header.push_back( etrading::Variant( name.c_str() ) );
            }
            matrix.push_back( header );
        }
        for ( const auto& row : data )
        {
            matrix.push_back( row );
        }
        return toExcelMatrix( matrix );
    }
}

namespace
{
    // The library builds config paths with forward slashes so they also work on
    // Linux (see FolderConfig.cpp). On Windows that leaves a mixed
    // "C:\dir\sub/resources/config/..." string. Normalise to backslashes for
    // display only - it does not affect how the file is opened.
    std::string toNativeSeparators( std::string path )
    {
        std::replace( path.begin(), path.end(), '/', '\\' );
        return path;
    }

    // Optional integer worksheet argument: missing / empty / blank yields
    // defaultValue; a number or a numeric string is truncated to int.
    int toIntOr( const xloil::ExcelObj& obj, int defaultValue )
    {
        if ( obj.isMissing() || !obj.isNonEmpty() )
        {
            return defaultValue;
        }
        return static_cast< int >( obj.get<double>() );
    }

    // Append a non-empty LVB block to the group vector; blank arguments are
    // skipped so aqToolLVBGroup can be called with only the first few filled in.
    void addLvbBlock( std::vector<AQLStringMatrix>& blocks, const xloil::ExcelObj& obj )
    {
        if ( obj.isMissing() || !obj.isNonEmpty() )
        {
            return;
        }
        AQLStringMatrix block = toAQLStringMatrix( obj );
        if ( !block.empty() )
        {
            blocks.push_back( std::move( block ) );
        }
    }

    // Append a non-empty key / value column, optionally prefixing every key.
    void addStringColumn( std::vector<AQLStringVector>& columns,
                          const xloil::ExcelObj& obj,
                          const std::string& keyPrefix = std::string() )
    {
        if ( obj.isMissing() || !obj.isNonEmpty() )
        {
            return;
        }
        AQLStringVector column = toAQLStringVector( obj, true );
        if ( !keyPrefix.empty() )
        {
            for ( AQLString& key : column )
            {
                key = AQLString( ( keyPrefix + key.getCString() ).c_str() );
            }
        }
        columns.push_back( std::move( column ) );
    }
}


/*
 * Load and verify the AlgoQuantLib configuration (holiday calendars, IR static
 * data, optional startup config). The add-in already does this when Excel opens
 * it, so this function is only for testing and diagnosis: it re-runs the setup
 * with the loud checks enabled - a missing or wrong config folder throws here
 * and the error lands in the calling cell - and on success reports which
 * Calendar.csv the library resolved to.
 *
 * Resolution order for the config folder:
 *   1. .\config\                     (Excel's working directory)
 *   2. %AQ%\resources\config\        (the AQ environment variable)
 *   3. <folder of the .xll>\config\  (business-user layout)
 */
XLO_FUNC_START( aqToolInitialize() )
{
    AQ_XLL_GUARD

    // checkStaticDataLoaded = true, checkIfCalendarLoaded = true -> throw, with
    // the offending path, if the calendars or static data did not load.
    etrading::InitializeETrading::instance( true, true );

    const AQLString* calendarPath = etrading::FolderConfig::calendar_path();

    std::string message = "AlgoQuantLib initialised. Calendar file: ";
    message += ( calendarPath != nullptr && calendarPath->size() != 0 )
                   ? toNativeSeparators( calendarPath->getCString() )
                   : std::string( "<not resolved>" );

    return returnValue( message );
}
XLO_FUNC_END( aqToolInitialize )
    .help( L"Load and verify the AlgoQuantLib configuration (holiday calendars, IR static data). "
           L"Runs automatically when the add-in opens; call it manually to re-check the setup "
           L"or to see which config path was used." );


// Test Function
XLO_FUNC_START(aqToolEcho(const ExcelObj* arg))
{
    return returnValue(arg->toString());
}
XLO_FUNC_END(aqToolEcho).threadsafe()
.help(L"Returns the argument provided")
.arg(L"Value", L"Any value");


// Build Version Time Stamp Method
XLO_FUNC_START(aqToolBuildTime())
{
    return returnValue(L"AQ_ADDIN built " L"" __DATE__ L" " __TIME__);
}
XLO_FUNC_END(aqToolBuildTime)
.help(L"Build date and time of the loaded add-in.");


/*
 * Reshape a range into NumRows x NumCols. Input cells are read row by row and
 * laid back out row by row into the requested shape; cells beyond the source
 * are filled with blanks and any surplus source cells are dropped. Enter as an
 * array formula (Ctrl+Shift+Enter) over the target block, or rely on
 * dynamic-array spill.
 */
XLO_FUNC_START( aqToolResize(
    const ExcelObj& inputArray,
    const ExcelObj& numRows,
    const ExcelObj& numCols ) )
{
    AQ_XLL_GUARD

    const uint32_t rows = static_cast< uint32_t >( numRows.get<double>() );
    const uint32_t cols = static_cast< uint32_t >( numCols.get<double>() );

    return returnValue( aq_xll::reshapeToSize( inputArray, rows, cols ) );
}
XLO_FUNC_END( aqToolResize )
    .help( L"Reshape a range into NumRows x NumCols, reading and writing row by row. "
           L"Short cells are blank-filled; surplus input cells are dropped." )
    .arg( L"InputArray", L"The range to reshape" )
    .arg( L"NumRows",    L"Number of rows in the result" )
    .arg( L"NumCols",    L"Number of columns in the result" );


/*
 * Report the library version, and optionally the licence expiry.
 */
XLO_FUNC_START( aqToolVersion(
    const ExcelObj& showLicenceExpiry ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    int expiryMonth = 0;
    int expiryYear  = 0;
    const bool showExpiry = toBool( showLicenceExpiry, false );

    return returnValue( validation::tryAqToolVersion( expiryMonth, expiryYear, showExpiry ) );
}
XLO_FUNC_END( aqToolVersion )
    .help( L"The AlgoQuantLib version string. Pass ShowLicenceExpiry=TRUE to include the licence expiry." )
    .arg( L"ShowLicenceExpiry", L"Optional. Default FALSE. TRUE appends the licence expiry month/year" );


/*
 * Clear the entity pool (cached market data, curves and instruments held by the
 * pricing environment). Returns an information string.
 */
XLO_FUNC_START( aqToolClearEntityPool() )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( std::string( validation::tryAqToolClearEntityPool().getCString() ) );
}
XLO_FUNC_END( aqToolClearEntityPool )
    .help( L"Clear the entity pool (cached market data, curves and instruments). Returns a summary." );


/*
 * Load an interest-rate static-data (properties) file at runtime. Returns an
 * information string.
 */
XLO_FUNC_START( aqToolLoadStaticData(
    const ExcelObj& filePath ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( std::string( validation::tryAqToolLoadStaticData( toAQLString( filePath ) ).getCString() ) );
}
XLO_FUNC_END( aqToolLoadStaticData )
    .help( L"Load an interest-rate static-data (properties) file at runtime. Returns a status string." )
    .arg( L"FilePath", L"Full path to the static-data properties file" );


/*
 * Load a holiday-calendar file at runtime. Returns an information string.
 */
XLO_FUNC_START( aqToolLoadCalendarFile(
    const ExcelObj& filePath ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( std::string( validation::tryAqToolLoadCalendarFile( toAQLString( filePath ) ).getCString() ) );
}
XLO_FUNC_END( aqToolLoadCalendarFile )
    .help( L"Load a holiday-calendar file at runtime. Returns a status string." )
    .arg( L"FilePath", L"Full path to the calendar file" );


/*
 * Reload the configuration files (calendars, static data, startup config) from
 * the resolved config folder. Returns a status string with the elapsed time.
 */
XLO_FUNC_START( aqToolLoadConfigurationFiles() )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const std::clock_t start = std::clock();
    std::string result = validation::tryAqToolLoadConfigurationFiles().getCString();
    const double seconds = static_cast< double >( std::clock() - start ) / CLOCKS_PER_SEC;

    char elapsed[ 32 ] = { 0 };
    std::snprintf( elapsed, sizeof( elapsed ), " (%.5f sec)", seconds );
    result += elapsed;

    return returnValue( result );
}
XLO_FUNC_END( aqToolLoadConfigurationFiles )
    .help( L"Reload the configuration files (calendars, static data, startup config). Returns a status string." );


/*
 * Enable or disable parallel pricing mode. Returns the resulting status string.
 */
XLO_FUNC_START( aqToolParallelModeEnable(
    const ExcelObj& enable ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqToolParallelModeEnable( toBool( enable, false ) ) );
}
XLO_FUNC_END( aqToolParallelModeEnable )
    .help( L"Enable or disable parallel pricing mode. Returns the resulting status." )
    .arg( L"Enable", L"TRUE to enable parallel mode, FALSE to disable it" );


/*
 * Report whether parallel pricing mode is currently enabled.
 */
XLO_FUNC_START( aqToolParallelModeStatus() )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqToolParallelModeStatus() );
}
XLO_FUNC_END( aqToolParallelModeStatus )
    .help( L"The current parallel-pricing-mode status." );


/*
 * Enable or disable capture of validation-layer inputs and outputs (the
 * recording that generates GoogleTest cases). Returns a status message.
 */
XLO_FUNC_START( aqToolRecord(
    const ExcelObj& enable,
    const ExcelObj& folder,
    const ExcelObj& repeat,
    const ExcelObj& startIndex,
    const ExcelObj& maxIndex ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const bool enableRecording = toBool( enable, false );
    const bool repeatTests     = toBool( repeat, false );
    const int  firstIndex      = toIntOr( startIndex, 0 );
    const int  lastIndex       = toIntOr( maxIndex, 0 );

    return returnValue(
        std::string( validation::tryAqToolRecord( enableRecording, toAQLString( folder ),
                                                  repeatTests, firstIndex, lastIndex ).getCString() ) );
}
XLO_FUNC_END( aqToolRecord )
    .help( L"Enable or disable capture of validation-layer inputs/outputs. Returns a status message." )
    .arg( L"Enable",     L"TRUE to start recording, FALSE to stop" )
    .arg( L"Folder",     L"Output folder for the recorded input/output files" )
    .arg( L"Repeat",     L"Optional. TRUE allows repeated tests. Default FALSE" )
    .arg( L"StartIndex", L"Optional. Start index for repeated tests. Default 0" )
    .arg( L"MaxIndex",   L"Optional. Maximum index for repeated tests. Default 0" );


/*
 * Replay a recorded input file back through the validation layer and return the
 * result string.
 */
XLO_FUNC_START( aqToolReplay(
    const ExcelObj& filePath ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( std::string( validation::tryAqToolReplay( toAQLString( filePath ) ).getCString() ) );
}
XLO_FUNC_END( aqToolReplay )
    .help( L"Replay a recorded input file through the validation layer. Returns the result string." )
    .arg( L"FilePath", L"Full path to the recorded input .csv file" );


/* -------------------------------------------------------------------------
 *  Label-value-block builders
 * ---------------------------------------------------------------------- */

/*
 * Build a two-column label-value block from a column of keys and a column of
 * values, optionally prefixing every key.
 */
XLO_FUNC_START( aqToolLVBFromKeysValues(
    const ExcelObj& keys,
    const ExcelObj& values,
    const ExcelObj& keyPrefix ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelMatrix(
        validation::tryAqToolLVBFromKeysValues( toAQLStringVector( keys ),
                                                toAQLStringVector( values ),
                                                toAQLString( keyPrefix ) ) ) );
}
XLO_FUNC_END( aqToolLVBFromKeysValues )
    .help( L"Build a two-column (key, value) label-value block from a keys column and a values column." )
    .arg( L"Keys",      L"Column of keys" )
    .arg( L"Values",    L"Column of values, aligned with Keys" )
    .arg( L"KeyPrefix", L"Optional. Text prepended to every key" );


/*
 * Build a label-value block from a key/value matrix and a list of key prefixes.
 */
XLO_FUNC_START( aqToolLVB(
    const ExcelObj& keyValueMatrix,
    const ExcelObj& keyPrefixes,
    const ExcelObj& verticalKeys ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const AQLStringMatrix sMatrix = toAQLStringMatrix( keyValueMatrix );
    AQLStringVector prefixes = toAQLStringVector( keyPrefixes, false );
    const bool verticalKeysFlag = toBool( verticalKeys, true );

    return returnValue( toExcelMatrix(
        validation::tryAqToolLVB( sMatrix, prefixes, verticalKeysFlag ) ) );
}
XLO_FUNC_END( aqToolLVB )
    .help( L"Build a label-value block from a key/value matrix and a list of key prefixes." )
    .arg( L"KeyValueMatrix", L"The key/value matrix" )
    .arg( L"KeyPrefixes",    L"List of key prefixes, one per key block" )
    .arg( L"VerticalKeys",   L"Optional. Default TRUE. FALSE reads keys then values as rows" );


/*
 * Concatenate up to ten label-value blocks into one. Blank arguments are
 * skipped.
 */
XLO_FUNC_START( aqToolLVBGroup(
    const ExcelObj& lvb1,  const ExcelObj& lvb2,  const ExcelObj& lvb3,
    const ExcelObj& lvb4,  const ExcelObj& lvb5,  const ExcelObj& lvb6,
    const ExcelObj& lvb7,  const ExcelObj& lvb8,  const ExcelObj& lvb9,
    const ExcelObj& lvb10 ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    std::vector<AQLStringMatrix> blocks;
    addLvbBlock( blocks, lvb1 );  addLvbBlock( blocks, lvb2 );  addLvbBlock( blocks, lvb3 );
    addLvbBlock( blocks, lvb4 );  addLvbBlock( blocks, lvb5 );  addLvbBlock( blocks, lvb6 );
    addLvbBlock( blocks, lvb7 );  addLvbBlock( blocks, lvb8 );  addLvbBlock( blocks, lvb9 );
    addLvbBlock( blocks, lvb10 );

    return returnValue( toExcelMatrix( validation::tryAqToolLVBGroup( blocks ) ) );
}
XLO_FUNC_END( aqToolLVBGroup )
    .help( L"Concatenate up to ten label-value blocks into one. Blank arguments are skipped." )
    .arg( L"LVB1",  L"Label-value block 1" )
    .arg( L"LVB2",  L"Optional. Label-value block 2" )
    .arg( L"LVB3",  L"Optional. Label-value block 3" )
    .arg( L"LVB4",  L"Optional. Label-value block 4" )
    .arg( L"LVB5",  L"Optional. Label-value block 5" )
    .arg( L"LVB6",  L"Optional. Label-value block 6" )
    .arg( L"LVB7",  L"Optional. Label-value block 7" )
    .arg( L"LVB8",  L"Optional. Label-value block 8" )
    .arg( L"LVB9",  L"Optional. Label-value block 9" )
    .arg( L"LVB10", L"Optional. Label-value block 10" );


/*
 * Build a label-value block from up to five (keys, values, key-prefix) triples.
 * Blank key/value columns are skipped.
 */
XLO_FUNC_START( aqToolLVBFromMultipleKeysValues(
    const ExcelObj& keys1, const ExcelObj& values1, const ExcelObj& keyPrefix1,
    const ExcelObj& keys2, const ExcelObj& values2, const ExcelObj& keyPrefix2,
    const ExcelObj& keys3, const ExcelObj& values3, const ExcelObj& keyPrefix3,
    const ExcelObj& keys4, const ExcelObj& values4, const ExcelObj& keyPrefix4,
    const ExcelObj& keys5, const ExcelObj& values5, const ExcelObj& keyPrefix5 ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    std::vector<AQLStringVector> keys;
    addStringColumn( keys, keys1, toNarrowString( keyPrefix1 ) );
    addStringColumn( keys, keys2, toNarrowString( keyPrefix2 ) );
    addStringColumn( keys, keys3, toNarrowString( keyPrefix3 ) );
    addStringColumn( keys, keys4, toNarrowString( keyPrefix4 ) );
    addStringColumn( keys, keys5, toNarrowString( keyPrefix5 ) );

    std::vector<AQLStringVector> values;
    addStringColumn( values, values1 );
    addStringColumn( values, values2 );
    addStringColumn( values, values3 );
    addStringColumn( values, values4 );
    addStringColumn( values, values5 );

    return returnValue( toExcelMatrix(
        validation::tryAqToolLVBFromMultipleKeysValues( keys, values ) ) );
}
XLO_FUNC_END( aqToolLVBFromMultipleKeysValues )
    .help( L"Build a label-value block from up to five (keys, values, key-prefix) triples." )
    .arg( L"Keys1",      L"Keys column 1" )
    .arg( L"Values1",    L"Values column 1" )
    .arg( L"KeyPrefix1", L"Optional. Prefix for keys column 1" )
    .arg( L"Keys2",      L"Optional. Keys column 2" )
    .arg( L"Values2",    L"Optional. Values column 2" )
    .arg( L"KeyPrefix2", L"Optional. Prefix for keys column 2" )
    .arg( L"Keys3",      L"Optional. Keys column 3" )
    .arg( L"Values3",    L"Optional. Values column 3" )
    .arg( L"KeyPrefix3", L"Optional. Prefix for keys column 3" )
    .arg( L"Keys4",      L"Optional. Keys column 4" )
    .arg( L"Values4",    L"Optional. Values column 4" )
    .arg( L"KeyPrefix4", L"Optional. Prefix for keys column 4" )
    .arg( L"Keys5",      L"Optional. Keys column 5" )
    .arg( L"Values5",    L"Optional. Values column 5" )
    .arg( L"KeyPrefix5", L"Optional. Prefix for keys column 5" );


/*
 * Build a label-value block from one shared key column and up to five value
 * columns.
 */
XLO_FUNC_START( aqToolLVBFromKeysAndMultipleValues(
    const ExcelObj& keys,
    const ExcelObj& values1, const ExcelObj& values2, const ExcelObj& values3,
    const ExcelObj& values4, const ExcelObj& values5 ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const AQLStringVector commonKeys = toAQLStringVector( keys, false );

    std::vector<AQLStringVector> values;
    addStringColumn( values, values1 );
    addStringColumn( values, values2 );
    addStringColumn( values, values3 );
    addStringColumn( values, values4 );
    addStringColumn( values, values5 );

    return returnValue( toExcelMatrix(
        validation::tryAqToolLVBFromKeysAndMultipleValues( commonKeys, values ) ) );
}
XLO_FUNC_END( aqToolLVBFromKeysAndMultipleValues )
    .help( L"Build a label-value block from one shared key column and up to five value columns." )
    .arg( L"Keys",    L"Shared keys column" )
    .arg( L"Values1", L"Values column 1" )
    .arg( L"Values2", L"Optional. Values column 2" )
    .arg( L"Values3", L"Optional. Values column 3" )
    .arg( L"Values4", L"Optional. Values column 4" )
    .arg( L"Values5", L"Optional. Values column 5" );


/* -------------------------------------------------------------------------
 *  Data reshaping
 * ---------------------------------------------------------------------- */

/*
 * Remove blank and/or error cells from a range, row by row or column by column.
 * Cell types (number, boolean, text) are preserved.
 */
XLO_FUNC_START( aqToolClean(
    const ExcelObj& inputMatrix,
    const ExcelObj& cleanByRow,
    const ExcelObj& checkRowColumnNumber,
    const ExcelObj& removeBlanks,
    const ExcelObj& removeErrors ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const etrading::VariantMatrix result = validation::tryAqToolClean(
        toVariantMatrix( inputMatrix ),
        toBool( cleanByRow, true ),
        toIntOr( checkRowColumnNumber, 0 ),
        toBool( removeBlanks, true ),
        toBool( removeErrors, true ) );

    return returnValue( toExcelMatrix( result ) );
}
XLO_FUNC_END( aqToolClean )
    .help( L"Remove blank and/or error cells from a range, row by row or column by column." )
    .arg( L"InputMatrix",          L"The range to clean" )
    .arg( L"CleanByRow",           L"Optional. Default TRUE. FALSE cleans column by column" )
    .arg( L"CheckRowColumnNumber", L"Optional. Row/column index whose blanks/errors drive the removal. Default 0" )
    .arg( L"RemoveBlanks",         L"Optional. Default TRUE. Remove blank cells" )
    .arg( L"RemoveErrors",         L"Optional. Default TRUE. Remove error cells" );


/*
 * Append up to ten ranges into one, stacking by row or by column. Cell types
 * are preserved.
 */
XLO_FUNC_START( aqToolAppend(
    const ExcelObj& appendByRow,
    const ExcelObj& matrix1,  const ExcelObj& matrix2,  const ExcelObj& matrix3,
    const ExcelObj& matrix4,  const ExcelObj& matrix5,  const ExcelObj& matrix6,
    const ExcelObj& matrix7,  const ExcelObj& matrix8,  const ExcelObj& matrix9,
    const ExcelObj& matrix10 ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const etrading::VariantMatrix result = validation::tryAqToolAppend(
        toBool( appendByRow, true ),
        toVariantMatrix( matrix1 ), toVariantMatrix( matrix2 ), toVariantMatrix( matrix3 ),
        toVariantMatrix( matrix4 ), toVariantMatrix( matrix5 ), toVariantMatrix( matrix6 ),
        toVariantMatrix( matrix7 ), toVariantMatrix( matrix8 ), toVariantMatrix( matrix9 ),
        toVariantMatrix( matrix10 ) );

    return returnValue( toExcelMatrix( result ) );
}
XLO_FUNC_END( aqToolAppend )
    .help( L"Append up to ten ranges into one, stacking by row or by column." )
    .arg( L"AppendByRow", L"TRUE stacks the ranges vertically, FALSE horizontally" )
    .arg( L"Matrix1",     L"Range 1" )
    .arg( L"Matrix2",     L"Optional. Range 2" )
    .arg( L"Matrix3",     L"Optional. Range 3" )
    .arg( L"Matrix4",     L"Optional. Range 4" )
    .arg( L"Matrix5",     L"Optional. Range 5" )
    .arg( L"Matrix6",     L"Optional. Range 6" )
    .arg( L"Matrix7",     L"Optional. Range 7" )
    .arg( L"Matrix8",     L"Optional. Range 8" )
    .arg( L"Matrix9",     L"Optional. Range 9" )
    .arg( L"Matrix10",    L"Optional. Range 10" );


/*
 * Split a single vector of mixed data into typed columns.
 */
XLO_FUNC_START( aqToolDataFilter(
    const ExcelObj& dataVector,
    const ExcelObj& displayByRow ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const etrading::VariantMatrix result =
        validation::tryAqToolDataFilter( toVariantVector( dataVector ), toBool( displayByRow, false ) );

    return returnValue( toExcelMatrix( result ) );
}
XLO_FUNC_END( aqToolDataFilter )
    .help( L"Split a single vector of mixed data into typed columns." )
    .arg( L"DataVector",   L"The vector of mixed values" )
    .arg( L"DisplayByRow", L"Optional. Default FALSE. TRUE lays the result out by row" );


/*
 * Resolve a raw valuation-settings block (curve collection, fixing tables and
 * so on) into the settings actually used for pricing.
 */
XLO_FUNC_START( aqToolValuationSettingsDisplay(
    const ExcelObj& rawInput ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const StandardStringMatrix result =
        validation::tryAqToolValuationSettingsDisplay( toStandardStringMatrix( rawInput ) );

    AQLStringMatrix asAql;
    asAql.reserve( result.size() );
    for ( const StandardStringVector& row : result )
    {
        AQLStringVector aqlRow;
        aqlRow.reserve( row.size() );
        for ( const std::string& cell : row )
        {
            aqlRow.push_back( AQLString( cell.c_str() ) );
        }
        asAql.push_back( std::move( aqlRow ) );
    }

    return returnValue( toExcelMatrix( asAql ) );
}
XLO_FUNC_END( aqToolValuationSettingsDisplay )
    .help( L"Resolve a raw valuation-settings block into the settings used for pricing. "
           L"Input is read as rows of (key, value) pairs." )
    .arg( L"RawInput", L"The raw valuation-settings block, rows of (key, value)" );


/* -------------------------------------------------------------------------
 *  Term / date conversions
 * ---------------------------------------------------------------------- */

/*
 * Convert a list of year-fraction terms, measured from AsOfDate, into dates.
 */
XLO_FUNC_START( aqToolTermsToDates(
    const ExcelObj& asOfDate,
    const ExcelObj& terms ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const DateVector dates = validation::tryAqToolTermsToDates( toAQLDate( asOfDate ), toDoubleVector( terms ) );

    return returnValue( toExcelDateColumn( dates ) );
}
XLO_FUNC_END( aqToolTermsToDates )
    .help( L"Convert a list of year-fraction terms, measured from AsOfDate, into dates." )
    .arg( L"AsOfDate", L"The anchor date" )
    .arg( L"Terms",    L"Column of year-fraction terms" );


/*
 * Convert a list of dates into year-fraction terms measured from AsOfDate.
 */
XLO_FUNC_START( aqToolDatesToTerms(
    const ExcelObj& asOfDate,
    const ExcelObj& paymentDates ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const DoubleVector terms = validation::tryAqToolDatesToTerms( toAQLDate( asOfDate ), toDateVector( paymentDates ) );

    return returnValue( toExcelDoubleColumn( terms ) );
}
XLO_FUNC_END( aqToolDatesToTerms )
    .help( L"Convert a list of dates into year-fraction terms measured from AsOfDate." )
    .arg( L"AsOfDate",     L"The anchor date" )
    .arg( L"PaymentDates", L"Column of dates" );


/* -------------------------------------------------------------------------
 *  Label-value-block: matrix form
 * ---------------------------------------------------------------------- */

// Build a label/value block from a keys column and one or two values columns.
XLO_FUNC_START( aqToolLVBCreate(
    const ExcelObj& keys,
    const ExcelObj& values1,
    const ExcelObj& values2 ) )
{
    AQ_XLL_GUARD

    const std::vector<std::string> keyVec = toStringVector( keys, true );
    const std::vector<std::string> val1   = toStringVector( values1, true );

    StandardStringMatrix result;
    if ( !values2.isMissing() && values2.isNonEmpty() )
    {
        result = validation::tryAqToolLVBCreate( keyVec, val1, toStringVector( values2, true ) );
    }
    else
    {
        result = validation::tryAqToolLVBCreate( keyVec, val1 );
    }

    AQLStringMatrix asAql;
    asAql.reserve( result.size() );
    for ( const StandardStringVector& row : result )
    {
        AQLStringVector aqlRow;
        aqlRow.reserve( row.size() );
        for ( const std::string& cell : row )
        {
            aqlRow.push_back( AQLString( cell.c_str() ) );
        }
        asAql.push_back( std::move( aqlRow ) );
    }
    return returnValue( toExcelMatrix( asAql ) );
}
XLO_FUNC_END( aqToolLVBCreate )
    .help( L"Build a label/value block from a keys column and one or two values columns." )
    .arg( L"Keys",    L"Column of keys" )
    .arg( L"Values1", L"Column of values, aligned with Keys" )
    .arg( L"Values2", L"Optional. A second column of values" );


/* -------------------------------------------------------------------------
 *  Object grid - a cached rectangular data block
 * ---------------------------------------------------------------------- */

// Create and store an object grid from a range.
XLO_FUNC_START( aqToolObjectGridCreate(
    const ExcelObj& objectName,
    const ExcelObj& data,
    const ExcelObj& allowJaggedData ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const std::string name = decorateWithExcelLocation( toNarrowString( objectName ) );

    const std::string storedName =
        validation::tryAqToolObjectGridCreate( name, toTableInfo( data ), toBool( allowJaggedData, false ) );

    return returnValue( appendInstanceCounter( storedName ) );
}
XLO_FUNC_END( aqToolObjectGridCreate )
    .help( L"Create and store an object grid (a cached rectangular data block); returns its handle." )
    .arg( L"ObjectName",      L"Name for the grid object" )
    .arg( L"Data",            L"The range to store" )
    .arg( L"AllowJaggedData", L"Optional. Default FALSE. Allow columns of differing length" );


// Save an object grid to a file.
XLO_FUNC_START( aqToolObjectGridSave(
    const ExcelObj& objectName,
    const ExcelObj& fileNameToWriteTo ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqToolObjectGridSave(
        getNameWithoutCounter( objectName ), toNarrowString( fileNameToWriteTo ) ) );
}
XLO_FUNC_END( aqToolObjectGridSave )
    .help( L"Save an object grid to a file. Returns a status string." )
    .arg( L"ObjectName",        L"A grid handle" )
    .arg( L"FileNameToWriteTo", L"Full path to write the grid to" );


// Load an object grid from a file.
XLO_FUNC_START( aqToolObjectGridLoad(
    const ExcelObj& fileName ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqToolObjectGridLoad( toNarrowString( fileName ) ).second );
}
XLO_FUNC_END( aqToolObjectGridLoad )
    .help( L"Load an object grid from a file. Returns a status string." )
    .arg( L"FileName", L"Full path to the grid file" );


// Display an object grid as a matrix, column names as the header row.
XLO_FUNC_START( aqToolObjectGridDisplay(
    const ExcelObj& objectName ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( flexibleDataToExcel(
        validation::tryAqToolObjectGridDisplay( getNameWithoutCounter( objectName ) ) ) );
}
XLO_FUNC_END( aqToolObjectGridDisplay )
    .help( L"Display an object grid as a matrix with the column names as a header row." )
    .arg( L"ObjectName", L"A grid handle" );


// The names of every cached object grid.
XLO_FUNC_START( aqToolObjectGridObjectNames() )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelColumn( validation::tryAqToolObjectGridObjectNames() ) );
}
XLO_FUNC_END( aqToolObjectGridObjectNames )
    .help( L"The names of every cached object grid, as a column." );


// Remove one object grid from the cache.
XLO_FUNC_START( aqToolObjectGridClearOne(
    const ExcelObj& objectName ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqToolObjectGridClearOne( getNameWithoutCounter( objectName ) ) );
}
XLO_FUNC_END( aqToolObjectGridClearOne )
    .help( L"Remove one object grid from the cache. Returns TRUE on success." )
    .arg( L"ObjectName", L"A grid handle" );


// Remove every object grid from the cache.
XLO_FUNC_START( aqToolObjectGridClearAll() )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqToolObjectGridClearAll() );
}
XLO_FUNC_END( aqToolObjectGridClearAll )
    .help( L"Remove every object grid from the cache. Returns TRUE on success." );


/* -------------------------------------------------------------------------
 *  Object multi-grid - a cached set of named grids
 * ---------------------------------------------------------------------- */

// Create and store a multi-grid from up to three named grids.
XLO_FUNC_START( aqToolObjectMultiGridCreate(
    const ExcelObj& objectName,
    const ExcelObj& gridName1,
    const ExcelObj& grid1,
    const ExcelObj& gridName2,
    const ExcelObj& grid2,
    const ExcelObj& gridName3,
    const ExcelObj& grid3,
    const ExcelObj& allowJaggedData ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const std::string name = decorateWithExcelLocation( toNarrowString( objectName ) );

    std::vector<std::string>           gridNames;
    std::vector<validation::TableInfo> infoBlocks;

    gridNames.push_back( etrading::trim_to_upper( toNarrowString( gridName1 ) ) );
    infoBlocks.push_back( toTableInfo( grid1 ) );

    if ( !grid2.isMissing() && grid2.isNonEmpty() )
    {
        gridNames.push_back( etrading::trim_to_upper( toNarrowString( gridName2 ) ) );
        infoBlocks.push_back( toTableInfo( grid2 ) );
    }
    if ( !grid3.isMissing() && grid3.isNonEmpty() )
    {
        gridNames.push_back( etrading::trim_to_upper( toNarrowString( gridName3 ) ) );
        infoBlocks.push_back( toTableInfo( grid3 ) );
    }

    const std::string storedName = validation::tryAqToolObjectMultiGridCreate(
        name, gridNames, infoBlocks, toBool( allowJaggedData, false ) );

    return returnValue( appendInstanceCounter( storedName ) );
}
XLO_FUNC_END( aqToolObjectMultiGridCreate )
    .help( L"Create and store a multi-grid from up to three named grids; returns its handle." )
    .arg( L"ObjectName",      L"Name for the multi-grid object" )
    .arg( L"GridName1",       L"Name of the first grid" )
    .arg( L"Grid1",           L"First grid, as a range" )
    .arg( L"GridName2",       L"Optional. Name of the second grid" )
    .arg( L"Grid2",           L"Optional. Second grid, as a range" )
    .arg( L"GridName3",       L"Optional. Name of the third grid" )
    .arg( L"Grid3",           L"Optional. Third grid, as a range" )
    .arg( L"AllowJaggedData", L"Optional. Default FALSE. Allow columns of differing length" );


// Display one named grid of a multi-grid as a matrix.
XLO_FUNC_START( aqToolObjectMultiGridDisplay(
    const ExcelObj& objectName,
    const ExcelObj& gridName ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( flexibleDataToExcel( validation::tryAqToolObjectMultiGridDisplay(
        getNameWithoutCounter( objectName ), toNarrowString( gridName ) ) ) );
}
XLO_FUNC_END( aqToolObjectMultiGridDisplay )
    .help( L"Display one named grid of a multi-grid as a matrix with a header row." )
    .arg( L"ObjectName", L"A multi-grid handle" )
    .arg( L"GridName",   L"The grid within the multi-grid to display" );


// The sub-grid names held by a multi-grid.
XLO_FUNC_START( aqToolObjectMultiGridSubNames(
    const ExcelObj& objectName ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelColumn(
        validation::tryAqToolObjectMultiGridSubNames( getNameWithoutCounter( objectName ) ) ) );
}
XLO_FUNC_END( aqToolObjectMultiGridSubNames )
    .help( L"The sub-grid names held by a multi-grid, as a column." )
    .arg( L"ObjectName", L"A multi-grid handle" );


/* -------------------------------------------------------------------------
 *  Diagnostics
 * ---------------------------------------------------------------------- */

// Deliberately triggers a structured exception (out-of-bounds access on an
// empty vector, undefined behaviour - typically an access violation) to
// exercise AQ_XLL_GUARD / etrading::StructuredExceptionHandler. No tryAq*
// wrapper - this is an AQ_XLL-only diagnostic, like aqToolEcho / aqToolBuildTime.
XLO_FUNC_START( aqToolSEH() )
{
    AQ_XLL_GUARD

    std::vector<int> empty;
    return returnValue( static_cast<double>( empty.front() ) );  // UB: no bounds check, nothing to return
}
XLO_FUNC_END( aqToolSEH )
    .help( L"Diagnostic: deliberately triggers a structured exception (out-of-bounds access) "
           L"to test whether AQ_XLL_GUARD catches it or the add-in crashes." );


// Echo a double straight back through the validation layer. Diagnostic for
// confirming a numeric argument round-trips validation unchanged.
XLO_FUNC_START( aqToolEchoDouble(
    const ExcelObj& value ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqToolEchoDouble( value.get<double>() ) );
}
XLO_FUNC_END( aqToolEchoDouble )
    .help( L"Echo a double straight back through the validation layer." )
    .arg( L"Value", L"Any number" );


// The average of several underlying bond yields (filed under Bond, golden-named Tool).
XLO_FUNC_START( aqToolBondAverageYield(
    const ExcelObj& underlyingBondYields ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqToolBondAverageYield(
        toDoubleVector( underlyingBondYields, true, "UnderlyingBondYields" ) ) );
}
XLO_FUNC_END( aqToolBondAverageYield )
    .help( L"The average of several underlying bond yields." )
    .arg( L"UnderlyingBondYields", L"Column of bond yields to average" );


// A bond yield implied by a bond-future price (filed under Bond, golden-named Tool).
XLO_FUNC_START( aqToolBondYieldFromFuturePrice(
    const ExcelObj& futurePrice ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqToolBondYieldFromFuturePrice( futurePrice.get<double>() ) );
}
XLO_FUNC_END( aqToolBondYieldFromFuturePrice )
    .help( L"A bond yield implied by a bond-future price." )
    .arg( L"FuturePrice", L"The bond-future price" );


// A template of a swap's floating/fixing leg schedules (filed under Swap, golden-named Tool).
XLO_FUNC_START( aqToolSwapScheduleTemplate(
    const ExcelObj& showColumnHeaders,
    const ExcelObj& swapScheduleLVB,
    const ExcelObj& validateKeys,
    const ExcelObj& columnList ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const std::vector<std::string> columns =
        ( columnList.isMissing() || !columnList.isNonEmpty() ) ? std::vector<std::string>() : toStringVector( columnList, true );

    return returnValue( toExcelMatrix( validation::tryAqToolSwapScheduleTemplate(
        toBool( showColumnHeaders, true ), toLabelValueBlock( swapScheduleLVB ), toBool( validateKeys, true ), columns ) ) );
}
XLO_FUNC_END( aqToolSwapScheduleTemplate )
    .help( L"A template of a swap's floating/fixing leg schedules, from a label/value block of schedule properties." )
    .arg( L"ShowColumnHeaders", L"Include a header row" )
    .arg( L"SwapScheduleLVB",   L"The schedule configuration as a label/value block" )
    .arg( L"ValidateKeys",      L"Optional. Default TRUE. Check the LVB keys" )
    .arg( L"ColumnList",        L"Optional. Column names to include; default all columns" );