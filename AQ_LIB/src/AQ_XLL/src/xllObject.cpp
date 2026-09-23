/*
 * Object lifecycle worksheet functions. These act on the object cache rather
 * than a product, so they take the aqObject<Lifecycle> form with no category
 * word (aqObjectExists / aqObjectType / aqObjectList / aqObjectLoad /
 * aqObjectSave / aqObjectDelete / aqObjectDeleteAll / aqObjectLoadFromString /
 * aqObjectQuickLoad / aqObjectQuickSave / aqObjectClearCache) and route through
 * the matching validation wrappers.
 *
 * aqObjectDecorateNames is the exception: it controls the AQObj handle-naming
 * switches themselves (instance counter / Excel-address decoration / unique-
 * ID vs address), which are pure AQ_XLL-layer state (aq_xll::setInstanceCount
 * Names et al. in aqXllTools) - there is nothing in etrading for `validation`
 * to validate, so it has no tryAq* wrapper, the same disposition as
 * aqToolEcho / aqToolBuildTime / aqToolSEH in xllTool.cpp. Ported from the
 * legacy meUtilityLWODecorateNames (.APPLES\...\meUtilities.cpp).
 */

#include <xllMain.h>

#include <cstddef>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>
#include <sstream>

#include <xllSupport.h>
#include <tryAqObject.h>      // validation::tryAqObject{Exists,Type,List,Load,Save,Delete,DeleteAll,QuickLoad,QuickSave,LoadFromString}
#include <tryAqToolSetup.h>   // validation::tryAqObjectClearCache

using namespace aq_xll;


#if AQ_XLL_ENABLED(aqObjectExists)
XLO_FUNC_START( aqObjectExists(
    const ExcelObj& objectName,
    const ExcelObj& objectType ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    // Names are stored upper-cased and without the recalculation counter.
    const std::string name = etrading::trim_to_upper( getNameWithoutCounter( objectName ) );
    const std::string type = etrading::trim_to_upper( toNarrowString( objectType ) );

    const bool exists = validation::tryAqObjectExists( type, name );

    return returnValue( exists );
}
XLO_FUNC_END( aqObjectExists )
    .help( L"TRUE if an object of the given type and name exists in the cache." )
    .arg( L"ObjectName", L"Object name or a handle returned by an aq*ObjectCreate function" )
    .arg( L"ObjectType", L"Object type to check, e.g. BOND, CURVE, SWAP" );
#endif


#if AQ_XLL_ENABLED(aqObjectLoad)
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

    const std::string objectName = validation::tryAqObjectLoad( filePath );

    // Decorated handle - the counter suffix makes Excel re-fire dependents.
    return returnValue( appendInstanceCounter( objectName ) );
}
XLO_FUNC_END( aqObjectLoad )
    .help( L"Load a single AQObj object from a JSON file and return its handle." )
    .arg( L"FileNameJSON", L"Full path to the .json file written by aqObjectSave" );
#endif


/*
 * Save a single object to a JSON file. Returns the object name by default;
 * ShowArrayOutputs=TRUE returns a 3-row column: result message, file path,
 * object name. If FullFilePath is omitted the file defaults to
 * C:\Temp\<ObjectName>.json. The path is not upper-cased (only the object name
 * and type are), so it is safe on case-sensitive filesystems and on Linux.
 */
#if AQ_XLL_ENABLED(aqObjectSave)
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

    const std::string result = validation::tryAqObjectSave( name, type, filePath );

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
#endif


/*
 * List the object types held in the cache under a given name. A name can exist
 * under more than one type, so the result is a column.
 */
#if AQ_XLL_ENABLED(aqObjectType)
XLO_FUNC_START( aqObjectType(
    const ExcelObj& objectName ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const std::string name = etrading::trim_to_upper( getNameWithoutCounter( objectName ) );

    const std::vector<std::string> types = validation::tryAqObjectType( name );

    return returnValue( toExcelColumn( types ) );
}
XLO_FUNC_END( aqObjectType )
    .help( L"The object type(s) stored in the cache under the given name, as a column." )
    .arg( L"ObjectName", L"Object name or a handle returned by an aq*ObjectCreate function" );
#endif


/*
 * List the object names held in the cache for a given type.
 */
#if AQ_XLL_ENABLED(aqObjectList)
XLO_FUNC_START( aqObjectList(
    const ExcelObj& objectType ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const std::string type = etrading::trim_to_upper( toNarrowString( objectType ) );

    const std::vector<std::string> names = validation::tryAqObjectList( type );

    return returnValue( toExcelColumn( names ) );
}
XLO_FUNC_END( aqObjectList )
    .help( L"The names of every cached object of the given type, as a column." )
    .arg( L"ObjectType", L"Object type to list, e.g. BOND, CURVE, SWAP" );
#endif


/*
 * Delete one object of a given type and name from the cache. Returns a message.
 * On a successful delete the instance counter for that name is also stopped so
 * a later re-create starts from a fresh handle.
 */
#if AQ_XLL_ENABLED(aqObjectDelete)
XLO_FUNC_START( aqObjectDelete(
    const ExcelObj& objectName,
    const ExcelObj& objectType ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const std::string name = etrading::trim_to_upper( getNameWithoutCounter( objectName ) );
    const std::string type = etrading::trim_to_upper( toNarrowString( objectType ) );

    if ( !validation::tryAqObjectExists( type, name ) )
    {
        std::ostringstream msg;
        msg << "No object named " << name << " of type " << type << " currently exists";
        return returnValue( msg.str() );
    }

    const bool stillExists = validation::tryAqObjectDelete( type, name );
    if ( stillExists )
    {
        std::ostringstream msg;
        msg << "Unable to delete object " << name << " of type " << type << "; use aqObjectType / aqObjectList to check available types and names";
        return returnValue( msg.str() );
    }

    stopCountingName( name );
    std::ostringstream msg;
    msg << "Object " << name << " of type " << type << " was successfully deleted";
    return returnValue( msg.str() );
}
XLO_FUNC_END( aqObjectDelete )
    .help( L"Delete one cached object of the given type and name. Returns a status message." )
    .arg( L"ObjectName", L"Object name or handle to delete" )
    .arg( L"ObjectType", L"Type of the object to delete, e.g. BOND, CURVE, SWAP" );
#endif


/*
 * Delete every cached object of a given type, or every cached object of
 * every type if ObjectType is omitted. Returns a count message and stops
 * the instance counter for each deleted name.
 */
#if AQ_XLL_ENABLED(aqObjectDeleteAll)
XLO_FUNC_START( aqObjectDeleteAll(
    const ExcelObj& objectType ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    if ( objectType.isMissing() || !objectType.isNonEmpty() )
    {
        const std::vector<std::string> deletedNames = validation::tryAqObjectList();
        const int deletedCount = validation::tryAqObjectDeleteAll();

        if ( deletedCount > 0 )
        {
            for ( const std::string& name : deletedNames )
            {
                stopCountingName( name );
            }
        }

        std::ostringstream msg;
        msg << deletedCount << " objects of all types were successfully deleted";
        return returnValue( msg.str() );
    }

    const std::string type = etrading::trim_to_upper( toNarrowString( objectType ) );

    const std::vector<std::string> deletedNames = validation::tryAqObjectList( type );
    const int deletedCount = validation::tryAqObjectDeleteAll( type );

    if ( deletedCount > 0 )
    {
        for ( const std::string& name : deletedNames )
        {
            stopCountingName( name );
        }
    }

    std::ostringstream msg;
    msg << deletedCount << " objects of type " << type << " were successfully deleted";
    return returnValue( msg.str() );
}
XLO_FUNC_END( aqObjectDeleteAll )
    .help( L"Delete every cached object of the given type, or every cached object of every type if ObjectType is omitted. Returns a count message." )
    .arg( L"ObjectType", L"Optional. Type of the objects to delete, e.g. BOND, CURVE, SWAP; omit to delete every object of every type" );
#endif


/*
 * Read a single AQObj object from a JSON string (rather than a file) and return
 * its decorated handle.
 */
#if AQ_XLL_ENABLED(aqObjectLoadFromString)
XLO_FUNC_START( aqObjectLoadFromString(
    const ExcelObj& jsonString ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const std::string json = toNarrowString( jsonString );
    if ( json.empty() )
    {
        throw std::runtime_error( "Unable to load object: no JSON string provided" );
    }

    const std::string objectName = std::get<0>( validation::tryAqObjectLoadFromString( json ) );

    return returnValue( appendInstanceCounter( objectName ) );
}
XLO_FUNC_END( aqObjectLoadFromString )
    .help( L"Load a single AQObj object from a JSON string and return its handle." )
    .arg( L"JSONString", L"The JSON text of an object, as written by aqObjectSave" );
#endif


/*
 * Load a group of objects of the same type by name from a folder. Each input
 * name is undecorated before the load; each returned name that came back
 * unchanged (i.e. loaded) is re-decorated with a fresh instance counter.
 */
#if AQ_XLL_ENABLED(aqObjectQuickLoad)
XLO_FUNC_START( aqObjectQuickLoad(
    const ExcelObj& objectNames,
    const ExcelObj& folderName ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    std::vector<std::string> names = getNamesWithoutCounter( objectNames );
    const std::string folder = toNarrowString( folderName );

    std::vector<std::string> results = validation::tryAqObjectQuickLoad( names, folder );

    for ( std::size_t i = 0; i < results.size() && i < names.size(); ++i )
    {
        if ( results[i] == names[i] )
        {
            results[i] = appendInstanceCounter( results[i] );
        }
    }

    return returnValue( toExcelColumn( results ) );
}
XLO_FUNC_END( aqObjectQuickLoad )
    .help( L"Load several objects of the same type by name from a folder. Returns the loaded handles as a column." )
    .arg( L"ObjectNames", L"Column of object names to load" )
    .arg( L"FolderName",  L"Folder holding the <ObjectName>.json files" );
#endif


/*
 * Save a group of cached objects by name to a folder. Names are undecorated
 * before the save; the per-name result strings are returned as a column.
 */
#if AQ_XLL_ENABLED(aqObjectQuickSave)
XLO_FUNC_START( aqObjectQuickSave(
    const ExcelObj& objectNames,
    const ExcelObj& folderName ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    std::vector<std::string> names = getNamesWithoutCounter( objectNames );
    const std::string folder = toNarrowString( folderName );

    const std::vector<std::string> results = validation::tryAqObjectQuickSave( names, folder );

    return returnValue( toExcelColumn( results ) );
}
XLO_FUNC_END( aqObjectQuickSave )
    .help( L"Save several cached objects by name to a folder. Returns the per-object result strings as a column." )
    .arg( L"ObjectNames", L"Column of object names to save" )
    .arg( L"FolderName",  L"Target folder for the <ObjectName>.json files" );
#endif


/*
 * Clear the whole AQObj object cache. validation::tryAqObjectClearCache
 * already deletes every cached object of every type (etrading::deleteAllObjects
 * loops every CachedObjectEnum - see tryAqToolSetup.cpp), plus the curve/swap/
 * credit results containers, the entity pool and reloads the config files.
 * All objects from every category need to be deleted here too.
 */
#if AQ_XLL_ENABLED(aqObjectClearCache)
XLO_FUNC_START( aqObjectClearCache() )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const std::string info = validation::tryAqObjectClearCache().getCString();

    // Every cached object is now gone - the AQ_XLL handle-name counters
    // tracking them would otherwise keep stale entries around.
    clearAllInstanceCounters();

    return returnValue( info );
}
XLO_FUNC_END( aqObjectClearCache )
    .help( L"Clear the entire AQObj object cache - every cached object of every category, plus curve/swap/credit "
           L"results and the entity pool. Returns a summary of what was removed." );
#endif


// The single object type this name resolves to (the first match, where
// aqObjectType returns every possible match as a column).
#if AQ_XLL_ENABLED(aqObjectTypeAsString)
XLO_FUNC_START( aqObjectTypeAsString(
    const ExcelObj& objectName ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqObjectTypeAsString( getNameWithoutCounter( objectName ) ) );
}
XLO_FUNC_END( aqObjectTypeAsString )
    .help( L"The object type this name resolves to (the first match)." )
    .arg( L"ObjectName", L"Object name or a handle returned by an aq*ObjectCreate function" );
#endif


// Load a single AQObj object and report both its handle and its cached type.
#if AQ_XLL_ENABLED(aqObjectLoadAndReturnTupleResults)
XLO_FUNC_START( aqObjectLoadAndReturnTupleResults(
    const ExcelObj& fileNameJson ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const std::string filePath = toNarrowString( fileNameJson );
    if ( filePath.empty() )
    {
        throw std::runtime_error( "Unable to load object: no file path provided" );
    }

    const std::tuple<std::string, etrading::CachedObjectEnum> loaded =
        validation::tryAqObjectLoadAndReturnTupleResults( filePath );

    std::vector<std::string> result;
    result.push_back( appendInstanceCounter( std::get<0>( loaded ) ) );
    result.push_back( etrading::toString( std::get<1>( loaded ) ) );

    return returnValue( toExcelColumn( result ) );
}
XLO_FUNC_END( aqObjectLoadAndReturnTupleResults )
    .help( L"Load a single AQObj object from a JSON file; returns a 2-row column [handle, cached object type]." )
    .arg( L"FileNameJSON", L"Full path to the .json file written by aqObjectSave" );
#endif


// Control AQObj handle-name decoration for the rest of this Excel session.
// No tryAq* wrapper - see the file header comment.
#if AQ_XLL_ENABLED(aqObjectDecorateNames)
XLO_FUNC_START( aqObjectDecorateNames(
    const ExcelObj& enableCounter,
    const ExcelObj& appendLocation,
    const ExcelObj& showExcelCellAddress ) )
{
    AQ_XLL_GUARD

    setInstanceCountNames( toBool( enableCounter, true ) );
    setDecorateNamesWithExcelAddress( toBool( appendLocation, true ) );
    setConvertExcelAddressToUniqueID( !toBool( showExcelCellAddress, false ) );

    std::ostringstream resultStream;
    resultStream << "AQObj Names: Instance Counting is " << ( instanceCountNames() ? "ON" : "OFF" )
                 << ", Append Excel Cell Location is " << ( decorateNamesWithExcelAddress() ? "ON" : "OFF" )
                 << ", Showing Excel Location as " << ( convertExcelAddressToUniqueID() ? "UNIQUE ID" : "EXCEL ADDRESS" );

    return returnValue( resultStream.str() );
}
XLO_FUNC_END( aqObjectDecorateNames )
    .help( L"Control AQObj handle-name decoration (instance counter, Excel-address suffix) for this session." )
    .arg( L"EnableCounter",         L"Optional. Default TRUE. Append an instance counter so recalculation fires dependents" )
    .arg( L"AppendLocation",        L"Optional. Default TRUE. Append the calling cell's location to the handle" )
    .arg( L"ShowExcelCellAddress",  L"Optional. Default FALSE. TRUE appends the real cell address; FALSE appends a short unique ID instead" );
#endif
