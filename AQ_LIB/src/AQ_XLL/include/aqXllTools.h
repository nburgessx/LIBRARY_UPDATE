#pragma once

/*
 * Excel-side helpers for the AQ xlOil add-in. Two jobs:
 *   1. Marshalling  - convert between xlOil ExcelObj values and the AQ types the
 *                     validation layer expects.
 *   2. AQObj handles - the instance-counter decoration that makes the object
 *                     framework work inside Excel (see below).
 */

#include <xloil/xlOil.h>
#include <xloil/Caller.h>

#include <cstdint>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include <boost/date_time/gregorian/gregorian_types.hpp>

#include <AQLDate.h>
#include <AQLString.h>
#include <AQLCoreTemplateType.h>   // DateVector, AQLStringMatrix, AnyTypeMatrix
#include <CoreEnumerations.h>       // etrading::CachedObjectEnum
#include <StructuredExceptionHandler.h>
#include <InitializeETrading.h>     // etrading::InitializeETrading
#include <LabelValueBlock.h>        // etrading::LabelValueBlock
#include <Variant.h>                // etrading::Variant, VariantMatrix, ContainedTypeEnum

namespace aq_xll
{
    // ---------------------------------------------------------------------
    //  Structured exception handling
    // ---------------------------------------------------------------------
    //
    //  An access violation, stack overflow or divide-by-zero inside a worksheet
    //  function is a Windows structured exception, not a C++ one. Left alone it
    //  unwinds straight through Excel and takes the process down with it.
    //
    //  etrading::StructuredExceptionHandler installs a _set_se_translator for
    //  the lifetime of the object, turning those into C++ exceptions that
    //  xlOil's XLO_FUNC_START/END can catch and return to the cell as an error
    //  string. It is per-thread and needs /EHa, which every project already
    //  uses (<ExceptionHandling>Async).
    //
    //  validation already does this via VALID_EXCEPTION_START, so anything
    //  inside a tryAq* call is covered. This guard extends the same protection
    //  over the MARSHALLING either side of that call - reading a bad Excel
    //  range, or building the result array - which validation never sees.
    //
    //  Put AQ_XLL_GUARD as the first line of every AQ_XLL worksheet function.

    #define AQ_XLL_GUARD  etrading::StructuredExceptionHandler aqXllSehGuard_;

    // ---------------------------------------------------------------------
    //  Library initialisation guard
    // ---------------------------------------------------------------------
    //
    //  The add-in brings the library up once in the AlgoQuantLib constructor
    //  (xlAutoOpen). AQ_INITIALIZE is a cheap per-function belt-and-braces
    //  check: InitializeETrading::instance() with no arguments is idempotent -
    //  once the singleton exists it is a mutex lock plus a null check, and it
    //  does NOT reload. Add it after AQ_XLL_GUARD in any function that reaches
    //  calendar or static data and might be called before xlAutoOpen completes.

    #define AQ_INITIALIZE  ::etrading::InitializeETrading::instance();

    // ---------------------------------------------------------------------
    //  Marshalling: Excel  ->  AQ
    // ---------------------------------------------------------------------

    std::string toNarrowString( const xloil::ExcelObj& obj );

    AQLString toAQLString( const xloil::ExcelObj& obj );

    // Accepts an Excel date serial (a number) or any date string the library's
    // date parser understands (YYYYMMDD, DD-MMM-YYYY, ...).
    AQLDate toAQLDate( const xloil::ExcelObj& obj );

    // A single cell or a range -> a vector. skipTrailingBlanks drops empty
    // trailing cells so a user can select a whole column; nameOfVariable is used
    // in the error message. An error cell anywhere is a hard failure.
    DateVector toDateVector( const xloil::ExcelObj& obj,
                             bool skipTrailingBlanks = true,
                             const std::string& nameOfVariable = "date" );

    // A handful of validation wrappers (Rate's fixing table, most of Curve's
    // discount-factor / forward-rate family) take boost::gregorian::date
    // rather than AQLDate - these bridge via etrading::toGregorianDateFromAQLDate.
    boost::gregorian::date toGregorian( const AQLDate& date );
    std::vector<boost::gregorian::date> toGregorianVector( const xloil::ExcelObj& obj,
                                                            bool skipTrailingBlanks = true,
                                                            const std::string& nameOfVariable = "date" );

    // As toDateVector; a text cell that reads cleanly as a number is accepted.
    std::vector<double> toDoubleVector( const xloil::ExcelObj& obj,
                                        bool skipTrailingBlanks = true,
                                        const std::string& nameOfVariable = "value" );

    // As toDateVector, one string per cell. Does NOT strip an AQObj instance
    // counter (getNamesWithoutCounter does) - use it for plain string lists.
    std::vector<std::string> toStringVector( const xloil::ExcelObj& obj,
                                             bool skipTrailingBlanks = true );

    // As toStringVector, returned as AQLStrings - the Tool LVB wrappers take
    // their key / value columns this way.
    AQLStringVector toAQLStringVector( const xloil::ExcelObj& obj,
                                      bool skipTrailingBlanks = true );

    // A rectangular range -> a row-major matrix of doubles. Blank cells read as
    // 0.0; a non-numeric cell is a hard failure. Backs aqMathPCA.
    DoubleMatrix toDoubleMatrix( const xloil::ExcelObj& obj );

    // A rectangular range -> a row-major matrix of std::string, every cell
    // stringified. Fully-blank trailing rows are dropped. Backs
    // aqToolValuationSettingsDisplay.
    StandardStringMatrix toStandardStringMatrix( const xloil::ExcelObj& obj );

    // A rectangular range -> a row-major VariantMatrix that keeps each cell's
    // native type (number, boolean, text, blank). An error cell is carried
    // through as its text ("#REF!", ...) rather than throwing. Back the
    // aqToolClean / aqToolAppend data-shaping wrappers.
    etrading::VariantMatrix toVariantMatrix( const xloil::ExcelObj& obj );

    // As toVariantMatrix, flattened row by row into a single vector. Backs
    // aqToolDataFilter.
    etrading::VariantVector toVariantVector( const xloil::ExcelObj& obj );

    // Optional boolean: missing / empty / blank yields defaultValue. TRUE/FALSE,
    // a non-zero number, and the words "true"/"false" are all accepted.
    bool toBool( const xloil::ExcelObj& obj, bool defaultValue );

    // Every cell stringified. A single cell becomes a 1x1 matrix; fully-blank
    // trailing rows are dropped so a user can over-select a block.
    AQLStringMatrix toAQLStringMatrix( const xloil::ExcelObj& obj );

    // Build a LabelValueBlock from a two-column (key, value) range. This is how
    // every AQObj *Create function takes its LVB arguments.
    etrading::LabelValueBlock toLabelValueBlock( const xloil::ExcelObj& obj );

    // As above, choosing orientation: keysAreVertical TRUE reads two columns
    // (key, value) per row; FALSE reads two rows (keys, then values) and
    // transposes before building the block.
    etrading::LabelValueBlock toLabelValueBlock( const xloil::ExcelObj& obj, bool keysAreVertical );

    // Read a rectangular range into the tuple the generator / market-data Create
    // functions expect: column names (COL_1, COL_2, ...), column types
    // (Variant::getContainedTypeInfo), and a column-major VariantMatrix of
    // string Variants.
    std::tuple< std::vector<std::string>,
                std::vector<etrading::ContainedTypeEnum>,
                etrading::VariantMatrix >
        toTableInfo( const xloil::ExcelObj& obj );

    // ---------------------------------------------------------------------
    //  Marshalling: AQ  ->  Excel  (matrices)
    // ---------------------------------------------------------------------

    // An AnyTypeMatrix (what every AQObj *Display function returns) -> an Excel
    // array, keeping numbers, booleans and strings as their native Excel types.
    // An empty matrix returns #N/A.
    xloil::ExcelObj toExcelMatrix( const AnyTypeMatrix& matrix );

    // An AQLStringMatrix -> an Excel array. Each cell that reads cleanly as a
    // number is returned as a real (formattable) number; everything else stays
    // text. An empty matrix returns #N/A.
    xloil::ExcelObj toExcelMatrix( const AQLStringMatrix& matrix );

    // A VariantMatrix -> an Excel array, keeping each cell's native type
    // (number, boolean, text, blank). An empty matrix returns #N/A.
    xloil::ExcelObj toExcelMatrix( const etrading::VariantMatrix& matrix );

    // Reshape a range into numRows x numCols, row by row. Short cells are
    // blank-filled, surplus source cells are dropped. Backs aqToolResize.
    xloil::ExcelObj reshapeToSize( const xloil::ExcelObj& obj,
                                   uint32_t numRows,
                                   uint32_t numCols );

    // ---------------------------------------------------------------------
    //  Marshalling: AQ  ->  Excel
    // ---------------------------------------------------------------------

    double toExcelDate( const AQLDate& date );

    xloil::ExcelObj toExcelDateColumn( const DateVector& dates );

    xloil::ExcelObj toExcelColumn( const std::vector<std::string>& values );

    // A vector of doubles / ints -> a single Excel column. One value is returned
    // as a scalar so it needs no array entry; an empty vector returns #N/A.
    xloil::ExcelObj toExcelDoubleColumn( const std::vector<double>& values );
    xloil::ExcelObj toExcelIntColumn( const std::vector<int>& values );

    // ---------------------------------------------------------------------
    //  AQObj handles - the instance counter
    // ---------------------------------------------------------------------
    //
    //  An AQObj handle looks like  <objectName><delimiter><counter>, e.g.
    //  "USD_OIS_CURVE:12". The counter is bumped every time the creating
    //  function recalculates, which changes the handle string and so makes
    //  Excel's dependency tree fire the downstream functions. Without it,
    //  a recalculated object would return an unchanged string and dependent
    //  cells would keep stale values.
    //
    //  The counter wraps at 100 and never returns to 0 after the first
    //  increment, so a handle is always visibly "used". The map is mutex-guarded
    //  because xlOil can register thread-safe functions.

    // Append (and by default advance) the instance counter for an object.
    // updateCounter=false reads the current counter without advancing it.
    // Returns the decorated handle, or objectName unchanged if counting is off.
    std::string appendInstanceCounter( const std::string& objectName, bool updateCounter = true );
    std::string appendInstanceCounter( const AQLString& objectName, bool updateCounter = true );

    std::string getInstanceCounterAsString( const std::string& objectName );

    // Stop counting an object, e.g. when it is deleted. Returns true if the name
    // was being counted.
    bool stopCountingName( const std::string& objectName );

    std::string getNameWithoutCounter( const std::string& handle );
    std::string getNameWithoutCounter( const xloil::ExcelObj& handle );

    std::vector<std::string> getNamesWithoutCounter( const xloil::ExcelObj& handles );

    // The calling cell as "[Book]Sheet!$A$1", upper-cased. Returns "" when
    // address decoration is disabled or the caller is not a worksheet cell.
    std::string getExcelLocationAsString();

    // Make an object name unique to the calling cell: "<objectName>@<cell>" when
    // address decoration is enabled, otherwise objectName unchanged. The first
    // step of every AQObj *Create function.
    std::string decorateWithExcelLocation( const std::string& objectName );

    // Create-vs-modify guard. When allowUpdate is false and the object already
    // exists, returns false and writes the existing decorated handle into
    // result so the caller can return it unchanged; otherwise returns true.
    bool allowAQObjUpdates( bool allowUpdate,
                            std::string& result,
                            const std::string& objectName,
                            const etrading::CachedObjectEnum objectType );

    // ---------------------------------------------------------------------
    //  Handle behaviour switches
    // ---------------------------------------------------------------------

    // Whether handles carry an instance counter. On by default; turning it off
    // makes handles stable but breaks recalculation of dependent cells.
    void setInstanceCountNames( bool on );
    bool instanceCountNames();

    void setDecorateNamesWithExcelAddress( bool on );
    bool decorateNamesWithExcelAddress();

    // When decoration is on: FALSE (default) appends the real Excel cell
    // address (e.g. "@[BOOK]SHEET!$A$1"); TRUE appends a short numeric ID
    // standing in for it instead (stable within this session only).
    void setConvertExcelAddressToUniqueID( bool on );
    bool convertExcelAddressToUniqueID();
}
