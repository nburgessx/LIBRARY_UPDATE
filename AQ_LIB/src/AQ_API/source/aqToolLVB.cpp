// aqToolLVB.cpp

/* 
 * @brief			Swig interface for aqToolLVB
 */

#include "aqToolLVB.h"
#include "tryAqToolLVB.h"
#include "AQLCoreTemplateType.h"
#include "TypeUtilities.h"
#include "APISetUp.h"               // AQ_API_START and AQ_API_END Macros


/* @brief			create a string matrix
*  @param [in]		keys			A vector of keys from user input
*  @param [in]		values			A vector of values from user input
*  @return			A string matrix of keys and values as a single data block
*/
SWIG_STRINGMATRIX aqToolLVBCreate( const std::vector<std::string>& keys,
                                      const std::vector<std::string>& values )
{
    AQ_API_START
        
    // Call validation method
    std::vector<std::vector<std::string> > result = validation::tryAqToolLVBCreate( keys, values );

#if defined(SWIG_R) || defined(SWIGR)
	etrading::VariantMatrix variantMatrix;
	swig::buildVariantMatrix( variantMatrix, result );
	SWIG_STRINGMATRIX swigMatrix = swig::fromVariantMatrixToMatrixOfString( variantMatrix );
	return swigMatrix;
#else
	return result;
#endif

    AQ_API_END
}

/* @brief			create a string matrix
*  @param [in]		keys			A vector of keys from user input
*  @param [in]		values1			A vector of values1 from user input
*  @param [in]		values2			A vector of values2 from user input
*  @return			A string matrix of keys and values as a single data block
*/

#if (!defined(SWIG_R)) && (!defined(SWIGR))
SWIG_STRINGMATRIX aqToolLVBCreate( const std::vector<std::string>& keys,
                                      const std::vector<std::string>& values1, 
                                      const std::vector<std::string>& values2 )
{
    AQ_API_START
    
    // Call validation method
    std::vector<std::vector<std::string> > result = validation::tryAqToolLVBCreate( keys, values1, values2 );
	return result;
    AQ_API_END
}
#endif


/* @brief			Add a key and value to an existing Label Value Block (LVB)
*  @param [in]		LVB                 The original string matrix Label Value Block (LVB)
*  @param [in]		key                 The new key to add to the original string matrix
*  @param [in]		value               The new value to add to the original string matrix
*  @return			Returns the augmented LVB
*/
std::vector<std::vector<std::string> > aqToolsLVBAppend( const std::vector<std::vector<std::string> >& LVB,
                                                                    const std::string& key,
                                                                    const std::string& value )
{
    AQ_API_START
    
    // Call validation method
    std::vector<std::vector<std::string> > resultLVB = LVB;
    validation::tryAqToolLVBAdd( resultLVB, key, value );
    return resultLVB;
    
    AQ_API_END
}

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
                                                                    const std::string& value2 )
{
    AQ_API_START
    
    // Call validation method
    std::vector<std::vector<std::string> > resultLVB = LVB;
    validation::tryAqToolLVBAdd( resultLVB, key, value1, value2 );
    return resultLVB;
    
    AQ_API_END
}

/* @brief			Add a key and value to an existing Label Value Block (LVB)
*  @param [in]		LVB                 The original string matrix Label Value Block (LVB)
*  @param [in]		key                 The new key to add to the original string matrix
*  @param [in]		value               The new value to add to the original string matrix
*  @return			Returns the augmented LVB
*/
void aqToolLVBAdd( std::vector<std::vector<std::string> >& LVB,
                      const std::string& key,
                      const std::string& value )
{
    AQ_API_START
    
    // Call validation method
    validation::tryAqToolLVBAdd( LVB, key, value );
    return;
    
    AQ_API_END
}

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
                      const std::string& value2 )
{
    AQ_API_START

    // Call validation method
    validation::tryAqToolLVBAdd( LVB, key, value1, value2 );
    return;

    AQ_API_END
}

/* @brief			swig interface for aqToolLVBFromKeysValues. Build a two-column label-value block from a keys column and a values column.
*  @param [in]		keys			A list of keys from user input
*  @param [in]		values			A list of values from user input, aligned with keys
*  @param [in]		keyPrefix		Optional. Prefix prepended to every key
*  @return			A string matrix representing the label-value block
*/
SWIG_STRINGMATRIX aqToolLVBFromKeysValues( const std::vector<std::string>& keys,
                                            const std::vector<std::string>& values,
                                            const std::string& keyPrefix )
{
    AQ_API_START

    // Marshall Inputs
    AQLStringVector keys_;
    swig::buildStringVector( keys_, keys );
    AQLStringVector values_;
    swig::buildStringVector( values_, values );
    AQLString keyPrefix_( keyPrefix.c_str() );

    // Call Function and Return Result
    AQLStringMatrix result = validation::tryAqToolLVBFromKeysValues( keys_, values_, keyPrefix_ );
    return swig::fromStringMatrixToMatrixOfString( result );

    AQ_API_END
}

/* @brief			swig interface for aqToolLVB. Build a label-value block from a key/value matrix and a list of key prefixes.
*  @param [in]		keyValueMatrix	The key/value matrix
*  @param [in]		keyPrefixes		List of key prefixes, one per key block
*  @param [in]		verticalKeys	True if the keys are vertical
*  @return			A string matrix representing the label-value block
*/
SWIG_STRINGMATRIX aqToolLVB( const SWIG_STRINGMATRIX& keyValueMatrix,
                             const std::vector<std::string>& keyPrefixes,
                             const bool verticalKeys )
{
    AQ_API_START

    // Marshall Inputs
    AQLStringMatrix keyValueMatrix_;
    swig::buildStringMatrix( keyValueMatrix_, keyValueMatrix );
    AQLStringVector keyPrefixes_;
    swig::buildStringVector( keyPrefixes_, keyPrefixes );

    // Call Function and Return Result (tryAqToolLVB mutates keyPrefixes_ - not surfaced back to the caller, matching the AQ_XLL binding)
    AQLStringMatrix result = validation::tryAqToolLVB( keyValueMatrix_, keyPrefixes_, verticalKeys );
    return swig::fromStringMatrixToMatrixOfString( result );

    AQ_API_END
}

/* @brief			swig interface for aqToolLVBGroup. Concatenate several label-value blocks into one.
*  @param [in]		lvbs		A list of string matrices, each representing a label-value block
*  @return			The concatenated label-value block
*/
SWIG_STRINGMATRIX aqToolLVBGroup( const std::vector<SWIG_STRINGMATRIX>& lvbs )
{
    AQ_API_START

    // Marshall Inputs
    std::vector<AQLStringMatrix> blocks;
    blocks.reserve( lvbs.size() );
    for ( const SWIG_STRINGMATRIX& lvb : lvbs )
    {
        AQLStringMatrix block;
        swig::buildStringMatrix( block, lvb );
        blocks.push_back( block );
    }

    // Call Function and Return Result
    AQLStringMatrix result = validation::tryAqToolLVBGroup( blocks );
    return swig::fromStringMatrixToMatrixOfString( result );

    AQ_API_END
}

/* @brief			swig interface for aqToolLVBFromMultipleKeysValues. Build a label-value block from multiple (keys, values) pairs.
*  @param [in]		keys			Multiple lists of keys from user input
*  @param [in]		values			Multiple lists of values from user input
*  @return			The concatenated label-value block
*/
SWIG_STRINGMATRIX aqToolLVBFromMultipleKeysValues( const std::vector<std::vector<std::string> >& keys,
                                                    const std::vector<std::vector<std::string> >& values )
{
    AQ_API_START

    // Marshall Inputs
    std::vector<AQLStringVector> keys_;
    keys_.reserve( keys.size() );
    for ( const std::vector<std::string>& keyBlock : keys )
    {
        AQLStringVector block;
        swig::buildStringVector( block, keyBlock );
        keys_.push_back( block );
    }

    std::vector<AQLStringVector> values_;
    values_.reserve( values.size() );
    for ( const std::vector<std::string>& valueBlock : values )
    {
        AQLStringVector block;
        swig::buildStringVector( block, valueBlock );
        values_.push_back( block );
    }

    // Call Function and Return Result
    AQLStringMatrix result = validation::tryAqToolLVBFromMultipleKeysValues( keys_, values_ );
    return swig::fromStringMatrixToMatrixOfString( result );

    AQ_API_END
}

/* @brief			swig interface for aqToolLVBFromKeysAndMultipleValues. Build a label-value block from one shared key column and multiple value columns.
*  @param [in]		commonKeys		A list of keys shared by every value column
*  @param [in]		multiValues		Multiple lists of values from user input
*  @return			A vertical string matrix with a key column and multiple value columns
*/
SWIG_STRINGMATRIX aqToolLVBFromKeysAndMultipleValues( const std::vector<std::string>& commonKeys,
                                                       const std::vector<std::vector<std::string> >& multiValues )
{
    AQ_API_START

    // Marshall Inputs
    AQLStringVector commonKeys_;
    swig::buildStringVector( commonKeys_, commonKeys );

    std::vector<AQLStringVector> multiValues_;
    multiValues_.reserve( multiValues.size() );
    for ( const std::vector<std::string>& valueBlock : multiValues )
    {
        AQLStringVector block;
        swig::buildStringVector( block, valueBlock );
        multiValues_.push_back( block );
    }

    // Call Function and Return Result
    AQLStringMatrix result = validation::tryAqToolLVBFromKeysAndMultipleValues( commonKeys_, multiValues_ );
    return swig::fromStringMatrixToMatrixOfString( result );

    AQ_API_END
}
