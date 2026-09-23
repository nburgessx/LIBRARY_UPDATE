// aqToolLVB.h

/* 
 * @brief			Swig interface for aqToolLVB
 */

#pragma once

#include <string>
#include <vector>

#include "SwigTypes.h"	// For SWIG_STRINGMATRIX which manages the AQLStringMatrix idiosyncrasies type for R API

typedef std::vector<std::string> STDStringVector;
typedef std::vector<std::vector<std::string> > STDStringMatrix;
    
/* @brief			create a string matrix
*  @param [in]		keys			A vector of keys from user input
*  @param [in]		values			A vector of values from user input
*  @return			A string matrix of keys and values as a single data block
*/
SWIG_STRINGMATRIX aqToolLVBCreate( const std::vector<std::string>& keys,
                                      const std::vector<std::string>& values );

/* @brief			create a string matrix
*  @param [in]		keys			A vector of keys from user input
*  @param [in]		values1			A vector of values1 from user input
*  @param [in]		values2			A vector of values2 from user input
*  @return			A string matrix of keys and values as a single data block
*/

#if (!defined(SWIG_R)) && (!defined(SWIGR))
SWIG_STRINGMATRIX aqToolLVBCreate( const std::vector<std::string>& keys,
                                      const std::vector<std::string>& values1,
                                      const std::vector<std::string>& values2 );
#endif								  


/* @brief			Add a key and value to an existing Label Value Block (LVB)
*  @param [in]		LVB                 The original string matrix Label Value Block (LVB)
*  @param [in]		key                 The new key to add to the original string matrix
*  @param [in]		value               The new value to add to the original string matrix
*  @return			Returns the augmented LVB
*/
std::vector<std::vector<std::string> > aqToolsLVBAppend( const std::vector<std::vector<std::string> >& LVB,
                                                                    const std::string& key,
                                                                    const std::string& value );

/* @brief			Add a key and value to an existing Label Value Block (LVB)
*  @param [in]		LVB                 The original string matrix Label Value Block (LVB)
*  @param [in]		key                 The new key to add to the original string matrix
*  @param [in]		value1              The new value to add to the original string matrix
*  @param [in]		value2              The new value to add to the original string matrix
*  @return			Returns the augmented LVB
*/
std::vector<std::vector<std::string> > aqToolsLVBAppend( const std::vector<std::vector<std::string> >& LVB,
                                                                    const std::string& key,
                                                                    const std::string& value1,
                                                                    const std::string& value2 );

/* @brief			Add a key and value to an existing Label Value Block (LVB)
*  @param [in]		LVB                 The original string matrix Label Value Block (LVB)
*  @param [in]		key                 The new key to add to the original string matrix
*  @param [in]		value               The new value to add to the original string matrix
*  @return			Returns the augmented LVB
*/
void aqToolLVBAdd( std::vector<std::vector<std::string> >& LVB,
                      const std::string& key,
                      const std::string& value );

/* @brief			Add a key and value to an existing Label Value Block (LVB)
*  @param [in]		LVB                 The original string matrix Label Value Block (LVB)
*  @param [in]		key                 The new key to add to the original string matrix
*  @param [in]		value1              The new value to add to the original string matrix
*  @param [in]		value2              The new value to add to the original string matrix
*  @return			Returns the augmented LVB
*/
void aqToolLVBAdd( std::vector<std::vector<std::string> >& LVB,
                      const std::string& key,
                      const std::string& value1,
                      const std::string& value2 );

/* @brief			swig interface for aqToolLVBFromKeysValues. Build a two-column label-value block from a keys column and a values column.
*  @param [in]		keys			A list of keys from user input
*  @param [in]		values			A list of values from user input, aligned with keys
*  @param [in]		keyPrefix		Optional. Prefix prepended to every key
*  @return			A string matrix representing the label-value block
*/
SWIG_STRINGMATRIX aqToolLVBFromKeysValues( const std::vector<std::string>& keys,
                                            const std::vector<std::string>& values,
                                            const std::string& keyPrefix = "" );

/* @brief			swig interface for aqToolLVB. Build a label-value block from a key/value matrix and a list of key prefixes.
*  @param [in]		keyValueMatrix	The key/value matrix
*  @param [in]		keyPrefixes		List of key prefixes, one per key block
*  @param [in]		verticalKeys	True if the keys are vertical
*  @return			A string matrix representing the label-value block
*/
SWIG_STRINGMATRIX aqToolLVB( const SWIG_STRINGMATRIX& keyValueMatrix,
                             const std::vector<std::string>& keyPrefixes,
                             const bool verticalKeys = true );

/* @brief			swig interface for aqToolLVBGroup. Concatenate several label-value blocks into one.
*  @param [in]		lvbs		A list of string matrices, each representing a label-value block
*  @return			The concatenated label-value block
*/
SWIG_STRINGMATRIX aqToolLVBGroup( const std::vector<SWIG_STRINGMATRIX>& lvbs );

/* @brief			swig interface for aqToolLVBFromMultipleKeysValues. Build a label-value block from multiple (keys, values) pairs.
*  @param [in]		keys			Multiple lists of keys from user input
*  @param [in]		values			Multiple lists of values from user input
*  @return			The concatenated label-value block
*/
SWIG_STRINGMATRIX aqToolLVBFromMultipleKeysValues( const std::vector<std::vector<std::string> >& keys,
                                                    const std::vector<std::vector<std::string> >& values );

/* @brief			swig interface for aqToolLVBFromKeysAndMultipleValues. Build a label-value block from one shared key column and multiple value columns.
*  @param [in]		commonKeys		A list of keys shared by every value column
*  @param [in]		multiValues		Multiple lists of values from user input
*  @return			A vertical string matrix with a key column and multiple value columns
*/
SWIG_STRINGMATRIX aqToolLVBFromKeysAndMultipleValues( const std::vector<std::string>& commonKeys,
                                                       const std::vector<std::vector<std::string> >& multiValues );

