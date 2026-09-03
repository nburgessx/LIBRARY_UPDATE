// meLWOSwapCreate.cpp

/* 
 * @brief			Swig interface for meLWOBondCreate... functions
 * @Created:		25th June 2018
 * @Author:			Nicholas Burgess
 * @Department:		Quant Research and Analytics
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#include "meLWOSwapCreate.h"
#include "LACoreTemplateType.h"
#include "TypeUtilities.h"
#include "tryMeLWOSwapCreation.h"
#include "APISetUp.h"               // MLIB_API_START and MLIB_API_END Macros


/* @brief			swig interface for meLWOSwapCreate function
*  @param [in]		swapObjectName		Swap Object Name
*  @param [in]		swapLVB             Swap Definition Label Value Block (LVB) - A key-value matrix representing the swap definition
*  @param [in]		xccyPropertiesLVB   Xccy Swap Properties Label Value Block (LVB) - A key-value matrix representing the Xccy Swap Properties
*  @param [in]		isXccySwap          isXccySwap: True or False
*  @param [in]		validateKeys	    Validate Bond LVB keys: true or false - true will throw if a key is misspecified
*  @param [out]		Returns the swap object handle name
*/
const std::string meLWOSwapCreate( const std::string& swapObjectName,
                                   const SWIG_STRINGMATRIX & swapLVB,
                                   const SWIG_STRINGMATRIX & xccyPropertiesLVB,
                                   const bool isXccySwap,
                                   const bool validateKeys )
{
    MLIB_API_START
    
    // Marshall Inputs
	LAStringMatrix swapLVBAsStringMatrix;
	swig::buildStringMatrix( swapLVBAsStringMatrix, swapLVB );

    LAStringMatrix xccyPropertiesLVBAsStringMatrix;
	swig::buildStringMatrix( xccyPropertiesLVBAsStringMatrix, xccyPropertiesLVB );

    // Call validation_api method
    std::string result = validation_api::tryMeLWOSwapCreate( swapObjectName, swapLVBAsStringMatrix, xccyPropertiesLVBAsStringMatrix, isXccySwap, validateKeys );
    return result;

    MLIB_API_END
}

/* @brief			swig interface for meLWOSwapCreateFromGenerator function
*  @param [in]		swapObjectName		Swap Object Name
*  @param [in]		swapGeneratorName   Swap Generator Object Name
*  @param [in]		swapLVB	            Swap Definition Label Value Block (LVB) - A key-value matrix representing the swap definition
*  @param [in]		xccyPropertiesLVB   Xccy Swap Properties Label Value Block (LVB) - A key-value matrix representing the Xccy Swap Properties
*  @param [in]		isXccySwap          isXccySwap: True or False
*  @param [in]		validateKeys	    Validate Bond LVB keys: true or false - true will throw if a key is misspecified
*  @param [out]		Returns the swap object handle name
*/
const std::string meLWOSwapCreateFromGenerator( const std::string& swapObjectName,
                                                const std::string& swapGeneratorName,
                                                const SWIG_STRINGMATRIX & swapLVB,
                                                const SWIG_STRINGMATRIX & xccyPropertiesLVB,
                                                const bool isXccySwap,
                                                const bool validateKeys )
{
    MLIB_API_START
    
    // Marshall Inputs
	LAStringMatrix swapLVBAsStringMatrix;
	swig::buildStringMatrix( swapLVBAsStringMatrix, swapLVB );

    LAStringMatrix xccyPropertiesLVBAsStringMatrix;
	swig::buildStringMatrix( xccyPropertiesLVBAsStringMatrix, xccyPropertiesLVB );

    // Call validation_api method
    std::string result = validation_api::tryMeLWOSwapCreateFromGenerator( swapObjectName, swapGeneratorName, swapLVBAsStringMatrix, xccyPropertiesLVBAsStringMatrix, isXccySwap, validateKeys );
    return result;
    
    MLIB_API_END
}
