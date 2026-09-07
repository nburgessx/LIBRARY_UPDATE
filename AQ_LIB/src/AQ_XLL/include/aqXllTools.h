#pragma once

/*
 * @brief   Excel-side helpers for the AQ xlOil add-in.
 *
 *          Two responsibilities:
 *
 *          1. Marshalling  - convert between xlOil ExcelObj values and the
 *                            AQ types the validation layer expects.
 *
 *          2. AQObj handles - the instance-counter decoration that makes the
 *                            object framework work inside Excel. See below.
 *
 *          This replaces XllPlusTips.cpp / XllPlusTipsForETrading.cpp from the
 *          legacy XLL+ add-in. Those files were NOT copied across: they are
 *          built on the XLL+ framework types (CXlOper, COper, CXlStringArg),
 *          which is the paid third-party framework xlOil replaces. Only the
 *          behaviour we actually need has been ported, expressed in xlOil
 *          primitives.
 */

#include <xloil/xlOil.h>
#include <xloil/Caller.h>

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "AQLDate.h"
#include "AQLString.h"
#include "AQLCoreTemplateType.h"   // DateVector, AQLStringMatrix, AnyTypeMatrix
#include "CoreEnumerations.h"       // etrading::CachedObjectEnum
#include "StructuredExceptionHandler.h"
#include "InitializeETrading.h"     // etrading::InitializeETrading
#include "LabelValueBlock.h"        // etrading::LabelValueBlock

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
    //  check - the analogue of checkIfStaticDataLoaded() inside the legacy
    //  MLIB_START_AND_CHECK_LICENCE macro. InitializeETrading::instance() with
    //  no arguments is idempotent: once the singleton exists it is a mutex
    //  lock plus a null check, and it does NOT reload. Add it after
    //  AQ_XLL_GUARD in any function that reaches calendar or static data and
    //  might plausibly be called before xlAutoOpen has completed.

    #define AQ_INITIALIZE  ::etrading::InitializeETrading::instance();

    // ---------------------------------------------------------------------
    //  Array-vs-scalar output
    // ---------------------------------------------------------------------
    //
    //  AQ_IS_ARRAY_OUTPUT is true when the calling formula occupies more than one
    //  cell (the user pressed Ctrl+Shift+Enter over a range). Use it to return
    //  just the headline value on a plain Enter and the full array on CSE:
    //
    //      if ( !AQ_IS_ARRAY_OUTPUT )
    //          return returnValue( summary );
    //      return returnValue( toExcelColumn( allRows ) );

    #define AQ_IS_ARRAY_OUTPUT  ( ::aq_xll::isArrayOutput() )

    // ---------------------------------------------------------------------
    //  Marshalling: Excel  ->  AQ
    // ---------------------------------------------------------------------

    /* @brief   Convert an Excel wide string to the narrow strings the AQ API uses.
    *           Safe for the ASCII keywords the API accepts (Call, Put, Following, ...).
    */
    std::string toNarrowString( const xloil::ExcelObj& obj );

    /* @brief   Convert an Excel value to an AQLString. */
    AQLString toAQLString( const xloil::ExcelObj& obj );

    /* @brief   Convert a single Excel cell to an AQLDate.
    *           Accepts an Excel date serial (a number) or any date string the
    *           library's date parser understands (YYYYMMDD, DD-MMM-YYYY, ...).
    */
    AQLDate toAQLDate( const xloil::ExcelObj& obj );

    /* @brief   Convert an Excel cell or range to a vector of AQLDates.
    *  @param [in] obj                  A single cell or a one/two dimensional range.
    *  @param [in] skipTrailingBlanks   Drop empty trailing cells, so a user can
    *                                   select a whole column without padding the result.
    *  @param [in] nameOfVariable       Argument name, used in the error message.
    */
    DateVector toDateVector( const xloil::ExcelObj& obj,
                             bool skipTrailingBlanks = true,
                             const std::string& nameOfVariable = "date" );

    /* @brief   Read an optional boolean argument.
    *  @param [in] obj            The Excel cell. Missing / empty / blank yields the default.
    *  @param [in] defaultValue   Returned when the cell carries no value.
    *           TRUE/FALSE, a non-zero number, and the words "true"/"false" are all accepted.
    */
    bool toBool( const xloil::ExcelObj& obj, bool defaultValue );

    /* @brief   Convert an Excel cell or range to an AQLStringMatrix (every cell
    *           stringified). A single cell becomes a 1x1 matrix. Fully-blank
    *           trailing rows are dropped so a user can over-select a block.
    */
    AQLStringMatrix toAQLStringMatrix( const xloil::ExcelObj& obj );

    /* @brief   Build a LabelValueBlock from a two-column (key, value) Excel range.
    *           Thin wrapper over toAQLStringMatrix + the LabelValueBlock ctor;
    *           this is how every AQObj *Create function takes its LVB arguments.
    */
    etrading::LabelValueBlock toLabelValueBlock( const xloil::ExcelObj& obj );

    // ---------------------------------------------------------------------
    //  Marshalling: AQ  ->  Excel  (matrices)
    // ---------------------------------------------------------------------

    /* @brief   Convert an AnyTypeMatrix (the type every AQObj *Display function
    *           returns) to an Excel array, preserving numbers, booleans and
    *           strings as their native Excel types. An empty matrix returns #N/A.
    */
    xloil::ExcelObj toExcelMatrix( const AnyTypeMatrix& matrix );

    /* @brief   Reshape a range into numRows x numCols, row by row. Short cells
    *           are blank-filled, surplus source cells are dropped. Backs
    *           aqToolsResize.
    */
    xloil::ExcelObj reshapeToSize( const xloil::ExcelObj& obj,
                                   uint32_t numRows,
                                   uint32_t numCols );

    // ---------------------------------------------------------------------
    //  Marshalling: AQ  ->  Excel
    // ---------------------------------------------------------------------

    /* @brief   Convert an AQLDate to an Excel date serial. */
    double toExcelDate( const AQLDate& date );

    /* @brief   Convert a vector of AQLDates to a column of Excel date serials. */
    xloil::ExcelObj toExcelDateColumn( const DateVector& dates );

    /* @brief   Convert a list of strings to a single Excel column. Empty -> #N/A. */
    xloil::ExcelObj toExcelColumn( const std::vector<std::string>& values );

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
    //  increment, so a handle is always visibly "used".
    //
    //  Ported from XllPlusTipsForETrading; behaviour is unchanged. The one
    //  addition is a mutex, because xlOil can register thread-safe functions
    //  and the legacy map was not guarded.

    /* @brief   Append (and by default advance) the instance counter for an object.
    *  @param [in] objectName     The undecorated object name.
    *  @param [in] updateCounter  False to read the current counter without advancing it.
    *  @return  The decorated handle, or objectName unchanged if counting is disabled.
    */
    std::string appendInstanceCounter( const std::string& objectName, bool updateCounter = true );
    std::string appendInstanceCounter( const AQLString& objectName, bool updateCounter = true );

    /* @brief   The current counter suffix for an object, or "" if it is not counted. */
    std::string getInstanceCounterAsString( const std::string& objectName );

    /* @brief   Stop counting an object, e.g. when it is deleted.
    *  @return  True if the name was being counted, false if it was not known.
    */
    bool stopCountingName( const std::string& objectName );

    /* @brief   Strip the instance counter from a handle, giving the object name. */
    std::string getNameWithoutCounter( const std::string& handle );
    std::string getNameWithoutCounter( const xloil::ExcelObj& handle );

    /* @brief   Strip the instance counter from every handle in a range. */
    std::vector<std::string> getNamesWithoutCounter( const xloil::ExcelObj& handles );

    /* @brief   The calling cell as "[Book]Sheet!$A$1", upper-cased.
    *           Used to give an object a name unique to the cell that created it.
    *           Returns "" when address decoration is disabled or the caller is
    *           not a worksheet cell (a macro or a VBA call, for instance).
    */
    std::string getExcelLocationAsString();

    /* @brief   Rows x columns the calling formula occupies.
    *           {1, 1} for a normal single-cell entry (and whenever the caller
    *           is not a worksheet cell). Larger when the user committed the
    *           formula over a multi-cell selection with Ctrl+Shift+Enter.
    *           Lets a function return a scalar on Enter and a full array on CSE.
    */
    std::pair<uint32_t, uint32_t> callerRangeSize();

    /* @brief   True when the calling formula spans more than one cell, i.e. the
    *           user wants an array back (Ctrl+Shift+Enter). Use via the
    *           IS_ARRAY_OUTPUT macro below.
    */
    bool isArrayOutput();

    /* @brief   Make an object name unique to the calling cell.
    *           When address decoration is enabled, returns
    *           "<objectName>@<cell>"; otherwise returns objectName unchanged.
    *           This is the analogue of the legacy appendExcelLocation and is
    *           the first step of every AQObj *Create function.
    */
    std::string decorateWithExcelLocation( const std::string& objectName );

    /* @brief   Guard for create-vs-modify behaviour.
    *           When allowUpdate is false and the object already exists, this
    *           returns false and writes the existing decorated handle into
    *           result, so the caller can return it unchanged.
    *  @return  True if the caller may go ahead and create or overwrite.
    */
    bool allowAQObjUpdates( bool allowUpdate,
                            std::string& result,
                            const std::string& objectName,
                            const etrading::CachedObjectEnum objectType );

    // ---------------------------------------------------------------------
    //  Handle behaviour switches
    // ---------------------------------------------------------------------

    /* @brief   Whether handles carry an instance counter. On by default;
    *           turning it off makes handles stable but breaks recalculation
    *           of dependent cells.
    */
    void setInstanceCountNames( bool on );
    bool instanceCountNames();

    /* @brief   Whether object names are decorated with the caller's cell address. */
    void setDecorateNamesWithExcelAddress( bool on );
    bool decorateNamesWithExcelAddress();
}
