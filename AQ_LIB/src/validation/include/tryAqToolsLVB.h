#pragma once

#include "AQLCoreTemplateType.h"
#include "LabelValueBlock.h"

using etrading::LabelValueBlock;

typedef std::vector<std::string> STDStringVector;
typedef std::vector<std::vector<std::string > > STDStringMatrix;

namespace validation
{

    /* @brief			validation interface for aqToolsLVBFromKeysValues method
    *  @param [in]		keys			A list of keys from user input
    *  @param [in]		values			A list of values from user input
    *  @param [in]		keyPrefix		Prefix added to for keys
    *  @return			a AQLStringMatrix representing LabelValueBlock, i.e. first columns are keys, second columns are values
    */
    AQLStringMatrix tryAqToolsLVBFromKeysValues( const AQLStringVector& keys, const AQLStringVector& values, const AQLString& keyPrefix );

    /* @brief			validation interface for aqToolsLVB method
    *  @param [in]		sMatrix			A input AQLStringMatrix with common key column/row and mutltiple value columns/rows
    *  @param [in]		keyPrefix		Prefix added to for keys
    *  @param [in]		verticalKeys	True if the keys are vertical
    *  @return			a AQLStringMatrix representing a LabelValueBlock, i.e. first columns are keys, second columns are values
    */
    AQLStringMatrix tryAqToolsLVB( const AQLStringMatrix& sMatrix, AQLStringVector& keyPrefixes, bool verticalKeys );

    /* @brief			validation interface for aqToolsLVBGroup method
    *  @param [in]		lvbs		A list of AQLStringMatrix objects representing label value blocks
    *  @return			A AQLStringMatrix representing the concatenated LabelValueBlock
    */
    AQLStringMatrix tryAqToolsLVBGroup( const std::vector<AQLStringMatrix>& lvbs );

    /* @brief			validation interface for aqToolsLVBFromMultipleKeysValues method
    *  @param [in]		keys			Multiple lists of keys from user input
    *  @param [in]		values			Multiple lists of values from user input
    *  @return			a AQLStringMatrix representing LabelValueBlock, i.e. first columns are keys, second columns are values
    */
    AQLStringMatrix tryAqToolsLVBFromMultipleKeysValues( const std::vector<AQLStringVector>& keys, const std::vector<AQLStringVector>& values );

    /* @brief			validation interface for aqToolsLVBFromKeysAndMultipleValues method
    *  @param [in]		commonKeys		A lists of keys from user input
    *  @param [in]		multiValues		Multiple lists of values from user input
    *  @return			a vertial AQLStringMatrix with a key column and multiple values columns
    */
    AQLStringMatrix tryAqToolsLVBFromKeysAndMultipleValues( const AQLStringVector& commonKeys, const std::vector<AQLStringVector>& multiValues);

    /* @brief			create a string matrix
    *  @param [in]		keys			A vector of keys from user input
    *  @param [in]		values			A vector of values from user input
    *  @return			A string matrix of keys and values as a single data block
    */
    STDStringMatrix tryAqToolsLVBCreate( const STDStringVector& keys, const STDStringVector& values );

    /* @brief			create a string matrix
    *  @param [in]		keys			A vector of keys from user input
    *  @param [in]		values1			A vector of values1 from user input
    *  @param [in]		values2			A vector of values2 from user input
    *  @return			A string matrix of keys and values as a single data block
    */
    STDStringMatrix tryAqToolsLVBCreate( const STDStringVector& keys, const STDStringVector& values1, const STDStringVector& values2 );

    /* @brief			Add a key and value to an existing Label Value Block (LVB)
    *  @param [in]		LVB                 The original string matrix Label Value Block (LVB)
    *  @param [in]		key                 The new key to add to the original string matrix
    *  @param [in]		value               The new value to add to the original string matrix
    *  @return			The input LVB is appended with the key and value
    */
    void tryAqToolsLVBAdd( STDStringMatrix& LVB, const std::string& key, const std::string& value );

    /* @brief			Add a key and value to an existing Label Value Block (LVB)
    *  @param [in]		LVB                 The original string matrix Label Value Block (LVB)
    *  @param [in]		key                 The new key to add to the original string matrix
    *  @param [in]		value1              The new value to add to the original string matrix
    *  @param [in]		value2              The new value to add to the original string matrix
    *  @return			The input LVB is appended with the key, value1 and value2
    */
    void tryAqToolsLVBAdd( STDStringMatrix& LVB, const std::string& key, const std::string& value1, const std::string& value2 );

}
