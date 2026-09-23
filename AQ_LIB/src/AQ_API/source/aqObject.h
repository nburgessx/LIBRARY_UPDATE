#pragma once
#include <string>
#include <vector>


/* @brief			swig interface for aqObjectSave function
*  @param [in]		objectName		    Object Name
*  @param [in]		objectType		    Object Type
*  @param [in]		filePath		    The object file path
*/
const std::string aqObjectSave( const std::string& objectName, const std::string& objectType, const std::string& filePath );

/* @brief			swig interface for aqObjectLoad function
*  @param [in]		filePath		    The object file path
*/
const std::string aqObjectLoad( const std::string& filePath );

/* @brief			swig interface for the aqObjectExists method
*  @param [in]		typeAsString		Object type to check, e.g. BOND, CURVE, SWAP
*  @param [in]		objectName			Object name to check
*  @returns			TRUE if an object of the given type and name exists in the cache
*/
bool aqObjectExists( const std::string& typeAsString, const std::string& objectName );

/* @brief			swig interface for the aqObjectTypeAsString method
*  @param [in]		objectName			Object name
*  @returns			The object type this name resolves to (the first match, where aqObjectType returns every match)
*/
std::string aqObjectTypeAsString( const std::string& objectName );

/* @brief			swig interface for the aqObjectType method
*  @param [in]		objectName			Object name
*  @returns			Every object type stored in the cache under the given name
*/
std::vector<std::string> aqObjectType( const std::string& objectName );

/* @brief			swig interface for the aqObjectList method
*  @param [in]		typeAsString		Optional. Object type to list, e.g. BOND, CURVE, SWAP; omit to list every cached object of every type
*  @returns			The names of every matching cached object
*/
std::vector<std::string> aqObjectList( const std::string& typeAsString = "" );

/* @brief			swig interface for the aqObjectDelete method
*  @param [in]		typeAsString		Type of the object to delete, e.g. BOND, CURVE, SWAP
*  @param [in]		objectName			Object name to delete
*  @returns			TRUE if the object was successfully deleted
*/
bool aqObjectDelete( const std::string& typeAsString, const std::string& objectName );

/* @brief			swig interface for the aqObjectDeleteAll method
*  @param [in]		typeAsString		Optional. Type of the objects to delete, e.g. BOND, CURVE, SWAP; omit to delete every cached object of every type
*  @returns			The number of objects deleted
*/
int aqObjectDeleteAll( const std::string& typeAsString = "" );

/* @brief			swig interface for the aqObjectLoadAndReturnTupleResults method.
*					Load a single AQObj object from a JSON file and report both its handle and its cached type.
*  @param [in]		fileName			Full path to the .json file written by aqObjectSave
*  @returns			A 2-element vector: [object handle, cached object type]
*/
std::vector<std::string> aqObjectLoadAndReturnTupleResults( const std::string& fileName );

/* @brief			swig interface for the aqObjectLoadFromString method.
*					Read a single AQObj object from a JSON string (rather than a file).
*  @param [in]		jsonString			The JSON text of an object, as written by aqObjectSave
*  @returns			The object handle
*/
std::string aqObjectLoadFromString( const std::string& jsonString );

/* @brief			swig interface for the aqObjectQuickLoad method.
*					Load several objects of the same type by name from a folder.
*  @param [in]		objectNames			Object names to load
*  @param [in]		folder				Folder holding the <ObjectName>.json files
*  @returns			The loaded object handles
*/
std::vector<std::string> aqObjectQuickLoad( const std::vector<std::string>& objectNames, const std::string& folder );

/* @brief			swig interface for the aqObjectQuickSave method.
*					Save several cached objects by name to a folder.
*  @param [in]		objectNames			Object names to save
*  @param [in]		folder				Target folder for the <ObjectName>.json files
*  @returns			The per-object result strings
*/
std::vector<std::string> aqObjectQuickSave( const std::vector<std::string>& objectNames, const std::string& folder );
