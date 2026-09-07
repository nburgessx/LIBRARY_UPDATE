/*
 * @brief   Bonds category - AQObj handle worksheet functions for the AQ xlOil
 *          add-in.
 *
 *          Ported from .APPLES\APPLE\src\MLIBQ_ADDIN\src\meLWO.cpp
 *          (meLWOBondCreate, meLWOBondDisplay). The XLL+ registration blob and
 *          the _4/_12 export wrappers are gone - xlOil generates those from
 *          XLO_FUNC_START/END - and CXlOper / CXlStringArg / LAStringMatrix
 *          marshalling is replaced by the aq_xll helpers in aqXllTools.h. The
 *          call into validation is unchanged in spirit: tryAqObjBonds*.
 *
 *          These two are the template for every AQObj handle pair:
 *
 *            aqObj<Category>Create   data + LVBs in  ->  a decorated handle out
 *            aqObj<Category>Display  a handle in     ->  a matrix out
 *
 *          Create:
 *            1. decorateWithExcelLocation - make the name unique to the cell
 *            2. allowAQObjUpdates         - honour AllowUpdates / create-vs-modify
 *            3. tryAqObj<Category>Create  - the validation-layer call
 *            4. appendInstanceCounter     - bump the suffix so Excel's
 *                                           dependency tree re-fires downstream
 *          Display:
 *            1. getNameWithoutCounter     - strip the "<name>:<counter>" suffix
 *            2. tryAqObj<Category>Display - returns an AnyTypeMatrix
 *            3. toExcelMatrix             - back to a native Excel array
 *
 *          Naming: the locked category is "Bonds" (plural), so the functions are
 *          aqObjBondsCreate / aqObjBondsDisplay - matching the tryAqObjBonds*
 *          wrappers and the aq* <-> tryAq* CI pairing check.
 */

#include <aqMain.h>

#include <string>

#include "aqXllTools.h"
#include "tryAqObjBonds.h"      // validation::tryAqObjBondsCreate / ...Display

using namespace aq_xll;


/* @brief   Create a bond object from a bond LVB and a schedule LVB. */
XLO_FUNC_START( aqObjBondsCreate(
    const ExcelObj& bondName,
    const ExcelObj& bondLVB,
    const ExcelObj& scheduleLVB,
    const ExcelObj& validateKeys,
    const ExcelObj& allowUpdates ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const std::string objectName =
        decorateWithExcelLocation( toNarrowString( bondName ) );

    // Create-vs-modify guard. When the object already exists and AllowUpdates is
    // FALSE, hand back the existing handle unchanged and do not rebuild it.
    std::string existingHandle;
    if ( !allowAQObjUpdates( toBool( allowUpdates, true ),
                             existingHandle,
                             objectName,
                             etrading::BOND ) )
    {
        return returnValue( existingHandle );
    }

    const etrading::LabelValueBlock bondBlock     = toLabelValueBlock( bondLVB );
    const etrading::LabelValueBlock scheduleBlock = toLabelValueBlock( scheduleLVB );

    const std::string storedName =
        validation::tryAqObjBondsCreate( objectName,
                                         bondBlock,
                                         scheduleBlock,
                                         toBool( validateKeys, true ) );

    // Return the decorated handle - the counter suffix changes on every
    // recalculation so dependent cells recompute.
    return returnValue( appendInstanceCounter( storedName ) );
}
XLO_FUNC_END( aqObjBondsCreate )
    .help( L"Create a bond object and return its handle. Enter the bond and schedule as "
           L"two-column (key, value) blocks." )
    .arg( L"BondName",     L"Name for the bond object; the returned handle is this name plus a counter" )
    .arg( L"BondLVB",      L"Bond parameters as a two-column key/value range" )
    .arg( L"ScheduleLVB",  L"Schedule parameters as a two-column key/value range" )
    .arg( L"ValidateKeys", L"Optional. TRUE (default) rejects unknown keys" )
    .arg( L"AllowUpdates", L"Optional. FALSE returns the existing object unchanged if the name is in use; TRUE (default) rebuilds it" );


/* @brief   Display a bond object's input parameters as a key/value matrix. */
XLO_FUNC_START( aqObjBondsDisplay(
    const ExcelObj& bondName ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const std::string objectName = getNameWithoutCounter( bondName );

    const AnyTypeMatrix result =
        validation::tryAqObjBondsDisplay( objectName );

    return returnValue( toExcelMatrix( result ) );
}
XLO_FUNC_END( aqObjBondsDisplay )
    .help( L"Return a bond object's stored parameters as a key/value block." )
    .arg( L"BondName", L"A bond handle returned by aqObjBondsCreate" );
