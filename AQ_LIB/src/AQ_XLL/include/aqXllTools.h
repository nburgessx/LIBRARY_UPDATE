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

#include <string>
#include <vector>

#include "AQLDate.h"
#include "AQLString.h"
#include "AQLCoreTemplateType.h"   // DateVector
#include "CoreEnumerations.h"       // etrading::CachedObjectEnum

namespace aq_xll
{
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

    // ---------------------------------------------------------------------
    //  Marshalling: AQ  ->  Excel
    // ---------------------------------------------------------------------

    /* @brief   Convert an AQLDate to an Excel date serial. */
    double toExcelDate( const AQLDate& date );

    /* @brief   Convert a vector of AQLDates to a column of Excel date serials. */
    xloil::ExcelObj toExcelDateColumn( const DateVector& dates );

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
