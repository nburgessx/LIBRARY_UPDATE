/*
 * @brief   AQObj handle lifecycle worksheet functions for the AQ xlOil add-in.
 *
 *          Ported from .APPLES\APPLE\src\MLIBQ_ADDIN\src\meUtilities.cpp
 *          (meLWOExists, meLWOLoad, meLWOSave). These are lifecycle operations
 *          on the object cache, not product functions, so per the naming
 *          convention they take the aqObj<Lifecycle> form with no category
 *          word - aqObjExists / aqObjLoad / aqObjSave - matching the
 *          tryAqObj* wrappers in validation/tryAqObjects.h.
 *
 *          xlOil replaces the XLL+ registration blob, the _4/_12 exports and
 *          the CXlOper marshalling; the call into validation is unchanged.
 */

#include <aqMain.h>

#include <stdexcept>
#include <string>
#include <vector>

#include "aqXllTools.h"
#include "tryAqObjects.h"      // validation::tryAqObjExists / tryAqObjLoad / tryAqObjSave

using namespace aq_xll;


/* @brief   TRUE if an object of the given type and name is in the cache. */
XLO_FUNC_START( aqObjExists(
    const ExcelObj& objectName,
    const ExcelObj& objectType ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    // Names are stored upper-cased and without the recalculation counter.
    const std::string name = etrading::trim_to_upper( getNameWithoutCounter( objectName ) );
    const std::string type = etrading::trim_to_upper( toNarrowString( objectType ) );

    const bool exists = validation::tryAqObjExists( type, name );

    return returnValue( exists );
}
XLO_FUNC_END( aqObjExists )
    .help( L"TRUE if an object of the given type and name exists in the cache." )
    .arg( L"ObjectName", L"Object name or a handle returned by an aqObj*Create function" )
    .arg( L"ObjectType", L"Object type to check, e.g. BOND, CURVE, SWAP" );


/* @brief   Load a single object from a JSON file; returns its handle. */
XLO_FUNC_START( aqObjLoad(
    const ExcelObj& fileNameJson ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const std::string filePath = toNarrowString( fileNameJson );
    if ( filePath.empty() )
    {
        throw std::runtime_error( "Unable to load object: no file path provided" );
    }

    const std::string objectName = validation::tryAqObjLoad( filePath );

    // Decorated handle - the counter suffix makes Excel re-fire dependents.
    return returnValue( appendInstanceCounter( objectName ) );
}
XLO_FUNC_END( aqObjLoad )
    .help( L"Load a single AQObj object from a JSON file and return its handle." )
    .arg( L"FileNameJSON", L"Full path to the .json file written by aqObjSave" );


/* @brief   Save a single object to a JSON file.
*
*           Returns a 3-row column: the result message, the file path written,
*           and the object name - the same shape meLWOSave returned.
*
*           If FullFilePath is omitted the file defaults to
*           C:\Temp\<ObjectName>.json. Unlike the legacy function the path is
*           NOT upper-cased (that broke case-sensitive filesystems and the
*           Linux build); only the object name and type are.
*/
XLO_FUNC_START( aqObjSave(
    const ExcelObj& objectName,
    const ExcelObj& objectType,
    const ExcelObj& fullFilePath ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const std::string name = etrading::trim_to_upper( getNameWithoutCounter( objectName ) );

    const std::string type = objectType.isMissing()
                                 ? std::string()
                                 : etrading::trim_to_upper( toNarrowString( objectType ) );

    std::string filePath =
        ( fullFilePath.isMissing() || !fullFilePath.isNonEmpty() )
            ? std::string()
            : toNarrowString( fullFilePath );

    if ( filePath.empty() )
    {
        filePath = "C:\\Temp\\" + name + ".json";
    }

    const std::string result = validation::tryAqObjSave( name, type, filePath );

    // Entered normally (single cell): return just the status message. Entered as
    // a multi-cell array with Ctrl+Shift+Enter: return the full 3-row column
    // { result, filePath, name }.
	if ( !AQ_IS_ARRAY_OUTPUT )
    {
        return returnValue( result );
    }

    const std::vector<std::string> resultColumn = { result, filePath, name };
    return returnValue( toExcelColumn( resultColumn ) );
}
XLO_FUNC_END( aqObjSave )
    .help( L"Save a single AQObj object to a JSON file. Returns result, file path and object name." )
    .arg( L"ObjectName",   L"Object name or handle to save" )
    .arg( L"ObjectType",   L"Optional. Object type; disambiguates when a name exists under several types" )
    .arg( L"FullFilePath", L"Optional. Target .json path; defaults to C:\\Temp\\<ObjectName>.json" );
