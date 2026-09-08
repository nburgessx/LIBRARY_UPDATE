#include <aqMain.h>

#include <algorithm>

#include <aqXllTools.h>
#include <FolderConfig.h>          // etrading::FolderConfig
#include <AQLString.h>

using namespace aq_xll;

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