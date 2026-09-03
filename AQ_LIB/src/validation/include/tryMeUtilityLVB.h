#pragma once

#include "LACoreTemplateType.h"
#include "LabelValueBlock.h"

using etrading::LabelValueBlock;

typedef std::vector<std::string> STDStringVector;
typedef std::vector<std::vector<std::string > > STDStringMatrix;

namespace validation_api
{

    /* @brief			validation interface for meUtilityLVBFromKeysValues method
    *  @param [in]		keys			A list of keys from user input
    *  @param [in]		values			A list of values from user input
    *  @param [in]		keyPrefix		Prefix added to for keys
    *  @return			a LAStringMatrix representing LabelValueBlock, i.e. first columns are keys, second columns are values
    */
    LAStringMatrix tryMeUtilityLVBFromKeysValues( const LAStringVector& keys, const LAStringVector& values, const LAString& keyPrefix );

    /* @brief			validation interface for meUtilityLVB method
    *  @param [in]		sMatrix			A input LAStringMatrix with common key column/row and mutltiple value columns/rows
    *  @param [in]		keyPrefix		Prefix added to for keys
    *  @param [in]		verticalKeys	True if the keys are vertical
    *  @return			a LAStringMatrix representing a LabelValueBlock, i.e. first columns are keys, second columns are values
    */
    LAStringMatrix tryMeUtilityLVB( const LAStringMatrix& sMatrix, LAStringVector& keyPrefixes, bool verticalKeys );

    /* @brief			validation interface for meUtilityLVBGroup method
    *  @param [in]		lvbs		A list of LAStringMatrix objects representing label value blocks
    *  @return			A LAStringMatrix representing the concatenated LabelValueBlock
    */
    LAStringMatrix tryMeUtilityLVBGroup( const std::vector<LAStringMatrix>& lvbs );

    /* @brief			validation interface for meUtilityLVBFromMultipleKeysValues method
    *  @param [in]		keys			Multiple lists of keys from user input
    *  @param [in]		values			Multiple lists of values from user input
    *  @return			a LAStringMatrix representing LabelValueBlock, i.e. first columns are keys, second columns are values
    */
    LAStringMatrix tryMeUtilityLVBFromMultipleKeysValues( const std::vector<LAStringVector>& keys, const std::vector<LAStringVector>& values );

    /* @brief			validation interface for meUtilityLVBFromKeysAndMultipleValues method
    *  @param [in]		commonKeys		A lists of keys from user input
    *  @param [in]		multiValues		Multiple lists of values from user input
    *  @return			a vertial LAStringMatrix with a key column and multiple values columns
    */
    LAStringMatrix tryMeUtilityLVBFromKeysAndMultipleValues( const LAStringVector& commonKeys, const std::vector<LAStringVector>& multiValues);

    /* @brief			create a string matrix
    *  @param [in]		keys			A vector of keys from user input
    *  @param [in]		values			A vector of values from user input
    *  @return			A string matrix of keys and values as a single data block
    */
    STDStringMatrix tryMeUtilityLVBCreate( const STDStringVector& keys, const STDStringVector& values );

    /* @brief			create a string matrix
    *  @param [in]		keys			A vector of keys from user input
    *  @param [in]		values1			A vector of values1 from user input
    *  @param [in]		values2			A vector of values2 from user input
    *  @return			A string matrix of keys and values as a single data block
    */
    STDStringMatrix tryMeUtilityLVBCreate( const STDStringVector& keys, const STDStringVector& values1, const STDStringVector& values2 );

    /* @brief			Add a key and value to an existing Label Value Block (LVB)
    *  @param [in]		LVB                 The original string matrix Label Value Block (LVB)
    *  @param [in]		key                 The new key to add to the original string matrix
    *  @param [in]		value               The new value to add to the original string matrix
    *  @return			The input LVB is appended with the key and value
    */
    void tryMeUtilityLVBAdd( STDStringMatrix& LVB, const std::string& key, const std::string& value );

    /* @brief			Add a key and value to an existing Label Value Block (LVB)
    *  @param [in]		LVB                 The original string matrix Label Value Block (LVB)
    *  @param [in]		key                 The new key to add to the original string matrix
    *  @param [in]		value1              The new value to add to the original string matrix
    *  @param [in]		value2              The new value to add to the original string matrix
    *  @return			The input LVB is appended with the key, value1 and value2
    */
    void tryMeUtilityLVBAdd( STDStringMatrix& LVB, const std::string& key, const std::string& value1, const std::string& value2 );

}
