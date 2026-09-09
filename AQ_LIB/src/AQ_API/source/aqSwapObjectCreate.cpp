// aqSwapObjectCreate.cpp

/* 
 * @brief			Swig interface for aqBondObjectCreate... functions
 */

#include "aqSwapObjectCreate.h"
#include "AQLCoreTemplateType.h"
#include "TypeUtilities.h"
#include "tryAqSwapObjectCreation.h"
#include "APISetUp.h"               // AQ_API_START and AQ_API_END Macros


/* @brief			swig interface for aqSwapObjectCreate function
*  @param [in]		swapObjectName		Swap Object Name
*  @param [in]		swapLVB             Swap Definition Label Value Block (LVB) - A key-value matrix representing the swap definition
*  @param [in]		xccyPropertiesLVB   Xccy Swap Properties Label Value Block (LVB) - A key-value matrix representing the Xccy Swap Properties
*  @param [in]		isXccySwap          isXccySwap: True or False
*  @param [in]		validateKeys	    Validate Bond LVB keys: true or false - true will throw if a key is misspecified
*  @param [out]		Returns the swap object handle name
*/
const std::string aqSwapObjectCreate( const std::string& swapObjectName,
                                   const SWIG_STRINGMATRIX & swapLVB,
                                   const SWIG_STRINGMATRIX & xccyPropertiesLVB,
                                   const bool isXccySwap,
                                   const bool validateKeys )
{
    AQ_API_START
    
    // Marshall Inputs
	AQLStringMatrix swapLVBAsStringMatrix;
	swig::buildStringMatrix( swapLVBAsStringMatrix, swapLVB );

    AQLStringMatrix xccyPropertiesLVBAsStringMatrix;
	swig::buildStringMatrix( xccyPropertiesLVBAsStringMatrix, xccyPropertiesLVB );

    // Call validation method
    std::string result = validation::tryAqSwapObjectCreate( swapObjectName, swapLVBAsStringMatrix, xccyPropertiesLVBAsStringMatrix, isXccySwap, validateKeys );
    return result;

    AQ_API_END
}

/* @brief			swig interface for aqSwapObjectCreateFromGenerator function
*  @param [in]		swapObjectName		Swap Object Name
*  @param [in]		swapGeneratorName   Swap Generator Object Name
*  @param [in]		swapLVB	            Swap Definition Label Value Block (LVB) - A key-value matrix representing the swap definition
*  @param [in]		xccyPropertiesLVB   Xccy Swap Properties Label Value Block (LVB) - A key-value matrix representing the Xccy Swap Properties
*  @param [in]		isXccySwap          isXccySwap: True or False
*  @param [in]		validateKeys	    Validate Bond LVB keys: true or false - true will throw if a key is misspecified
*  @param [out]		Returns the swap object handle name
*/
const std::string aqSwapObjectCreateFromGenerator( const std::string& swapObjectName,
                                                const std::string& swapGeneratorName,
                                                const SWIG_STRINGMATRIX & swapLVB,
                                                const SWIG_STRINGMATRIX & xccyPropertiesLVB,
                                                const bool isXccySwap,
                                                const bool validateKeys )
{
    AQ_API_START
    
    // Marshall Inputs
	AQLStringMatrix swapLVBAsStringMatrix;
	swig::buildStringMatrix( swapLVBAsStringMatrix, swapLVB );

    AQLStringMatrix xccyPropertiesLVBAsStringMatrix;
	swig::buildStringMatrix( xccyPropertiesLVBAsStringMatrix, xccyPropertiesLVB );

    // Call validation method
    std::string result = validation::tryAqSwapObjectCreateFromGenerator( swapObjectName, swapGeneratorName, swapLVBAsStringMatrix, xccyPropertiesLVBAsStringMatrix, isXccySwap, validateKeys );
    return result;
    
    AQ_API_END
}
