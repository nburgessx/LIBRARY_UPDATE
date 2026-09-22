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
