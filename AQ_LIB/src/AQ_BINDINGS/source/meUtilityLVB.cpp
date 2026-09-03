// meUtilityLVB.cpp

/* 
 * @brief			Swig interface for meUtilityLVB
 * @Created:		27th June 2016
 * @Author:			Nicholas Burgess
 * @Department:		MHI Quant Research and Analytics
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#include "meUtilityLVB.h"
#include "tryMeUtilityLVB.h"
#include "LACoreTemplateType.h"
#include "TypeUtilities.h"
#include "APISetUp.h"               // MLIB_API_START and MLIB_API_END Macros


/* @brief			create a string matrix
*  @param [in]		keys			A vector of keys from user input
*  @param [in]		values			A vector of values from user input
*  @return			A string matrix of keys and values as a single data block
*/
SWIG_STRINGMATRIX meUtilityLVBCreate( const std::vector<std::string>& keys,
                                      const std::vector<std::string>& values )
{
    MLIB_API_START
        
    // Call validation_api method
    std::vector<std::vector<std::string> > result = validation_api::tryMeUtilityLVBCreate( keys, values );

#if defined(SWIG_R) || defined(SWIGR)
	etrading::VariantMatrix variantMatrix;
	swig::buildVariantMatrix( variantMatrix, result );
	SWIG_STRINGMATRIX swigMatrix = swig::fromVariantMatrixToMatrixOfString( variantMatrix );
	return swigMatrix;
#else
	return result;
#endif

    MLIB_API_END
}

/* @brief			create a string matrix
*  @param [in]		keys			A vector of keys from user input
*  @param [in]		values1			A vector of values1 from user input
*  @param [in]		values2			A vector of values2 from user input
*  @return			A string matrix of keys and values as a single data block
*/

#if (!defined(SWIG_R)) && (!defined(SWIGR))
SWIG_STRINGMATRIX meUtilityLVBCreate( const std::vector<std::string>& keys,
                                      const std::vector<std::string>& values1, 
                                      const std::vector<std::string>& values2 )
{
    MLIB_API_START
    
    // Call validation_api method
    std::vector<std::vector<std::string> > result = validation_api::tryMeUtilityLVBCreate( keys, values1, values2 );
	return result;
    MLIB_API_END
}
#endif


/* @brief			Add a key and value to an existing Label Value Block (LVB)
*  @param [in]		LVB                 The original string matrix Label Value Block (LVB)
*  @param [in]		key                 The new key to add to the original string matrix
*  @param [in]		value               The new value to add to the original string matrix
*  @return			Returns the augmented LVB
*/
std::vector<std::vector<std::string> > meUtilityLVBAppendAndCreate( const std::vector<std::vector<std::string> >& LVB,
                                                                    const std::string& key,
                                                                    const std::string& value )
{
    MLIB_API_START
    
    // Call validation_api method
    std::vector<std::vector<std::string> > resultLVB = LVB;
    validation_api::tryMeUtilityLVBAdd( resultLVB, key, value );
    return resultLVB;
    
    MLIB_API_END
}

/* @brief			Add a key and value to an existing Label Value Block (LVB)
*  @param [in]		LVB                 The original string matrix Label Value Block (LVB)
*  @param [in]		key                 The new key to add to the original string matrix
*  @param [in]		value1              The new value to add to the original string matrix
*  @param [in]		value2              The new value to add to the original string matrix
*  @return			Returns the augmented LVB
*/
std::vector<std::vector<std::string> > meUtilityLVBAppendAndCreate( const std::vector<std::vector<std::string> >& LVB,
                                                                    const std::string& key,
                                                                    const std::string& value1,
                                                                    const std::string& value2 )
{
    MLIB_API_START
    
    // Call validation_api method
    std::vector<std::vector<std::string> > resultLVB = LVB;
    validation_api::tryMeUtilityLVBAdd( resultLVB, key, value1, value2 );
    return resultLVB;
    
    MLIB_API_END
}

/* @brief			Add a key and value to an existing Label Value Block (LVB)
*  @param [in]		LVB                 The original string matrix Label Value Block (LVB)
*  @param [in]		key                 The new key to add to the original string matrix
*  @param [in]		value               The new value to add to the original string matrix
*  @return			Returns the augmented LVB
*/
void meUtilityLVBAdd( std::vector<std::vector<std::string> >& LVB,
                      const std::string& key,
                      const std::string& value )
{
    MLIB_API_START
    
    // Call validation_api method
    validation_api::tryMeUtilityLVBAdd( LVB, key, value );
    return;
    
    MLIB_API_END
}

/* @brief			Add a key and value to an existing Label Value Block (LVB)
*  @param [in]		LVB                 The original string matrix Label Value Block (LVB)
*  @param [in]		key                 The new key to add to the original string matrix
*  @param [in]		value1              The new value to add to the original string matrix
*  @param [in]		value2              The new value to add to the original string matrix
*  @return			Returns the augmented LVB
*/
void meUtilityLVBAdd( std::vector<std::vector<std::string> >& LVB,
                      const std::string& key,
                      const std::string& value1,
                      const std::string& value2 )
{
    MLIB_API_START
    
    // Call validation_api method
    validation_api::tryMeUtilityLVBAdd( LVB, key, value1, value2 );
    return;
    
    MLIB_API_END
}
