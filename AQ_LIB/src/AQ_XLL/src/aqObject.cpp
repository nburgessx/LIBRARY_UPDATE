/*
 * Object lifecycle worksheet functions. These act on the object cache rather
 * than a product, so they take the aqObject<Lifecycle> form with no category
 * word (aqObjectExists / aqObjectLoad / aqObjectSave) and route through the
 * matching validation wrappers.
 */

#include <aqMain.h>

#include <stdexcept>
#include <string>
#include <vector>

#include <aqXllTools.h>
#include <tryAqObjects.h>      // validation::tryAqObjExists / tryAqObjLoad / tryAqObjSave

using namespace aq_xll;


XLO_FUNC_START( aqObjectExists(
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
XLO_FUNC_END( aqObjectExists )
    .help( L"TRUE if an object of the given type and name exists in the cache." )
    .arg( L"ObjectName", L"Object name or a handle returned by an aq*ObjectCreate function" )
    .arg( L"ObjectType", L"Object type to check, e.g. BOND, CURVE, SWAP" );


XLO_FUNC_START( aqObjectLoad(
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
XLO_FUNC_END( aqObjectLoad )
    .help( L"Load a single AQObj object from a JSON file and return its handle." )
    .arg( L"FileNameJSON", L"Full path to the .json file written by aqObjectSave" );


/*
 * Save a single object to a JSON file. Returns the object name by default;
 * ShowArrayOutputs=TRUE returns a 3-row column: result message, file path,
 * object name. If FullFilePath is omitted the file defaults to
 * C:\Temp\<ObjectName>.json. The path is not upper-cased (only the object name
 * and type are), so it is safe on case-sensitive filesystems and on Linux.
 */
XLO_FUNC_START( aqObjectSave(
    const ExcelObj& objectName,
    const ExcelObj& objectType,
    const ExcelObj& fullFilePath,
    const ExcelObj& showArrayOutputs ) )
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

    // Default: return just the object name. ShowArrayOutputs=TRUE returns a
    // 3-row column: result message, file path, object name.
    if ( toBool( showArrayOutputs, false ) )
    {
        return returnValue( toExcelColumn( { result, filePath, name } ) );
    }

    return returnValue( name );

}
XLO_FUNC_END( aqObjectSave )
    .help( L"Save a single AQObj object to a JSON file. Returns the object name; "
           L"pass ShowArrayOutputs=TRUE for a result / file path / object name column." )
    .arg( L"ObjectName",       L"Object name or handle to save" )
    .arg( L"ObjectType",       L"Optional. Object type; disambiguates when a name exists under several types" )
    .arg( L"FullFilePath",     L"Optional. Target .json path; defaults to C:\\Temp\\<ObjectName>.json" )
    .arg( L"ShowArrayOutputs", L"Optional. Default FALSE returns just the object name. "
                              L"TRUE returns a 3-row column: result message, file path, object name" );
