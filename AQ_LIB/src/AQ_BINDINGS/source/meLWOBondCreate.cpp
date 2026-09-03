// meLWOBondCreate.cpp

/* 
 * @brief			Swig interface for meLWOBondCreate... functions
 * @Created:		25th June 2018
 * @Author:			Nicholas Burgess
 * @Department:		Quant Research and Analytics
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#include "meLWOBondCreate.h"
#include "LACoreTemplateType.h"
#include "TypeUtilities.h"
#include "tryMeLWOBond.h"
#include "APISetUp.h"               // MLIB_API_START and MLIB_API_END Macros


/* @brief			swig interface for meLWOBondCreate function
*  @param [in]		bondObjectName		Bond Object Name
*  @param [in]		bondLVB	            Bond Definition Label Value Block (LVB) - A key-value matrix representing the bond definition
*  @param [in]		scheduleLVB     	Schedule Definition Label Value Block (LVB) - A key-value matrix representing the schedule definition
*  @param [in]		validateKeys	    Validate Bond LVB keys: true or false - true will throw if a key is misspecified
*  @param [out]		Returns the bond object handle name
*/
const std::string meLWOBondCreate( const std::string& bondObjectName,
                                   const SWIG_STRINGMATRIX & bondLVB,
                                   const SWIG_STRINGMATRIX & scheduleLVB,
                                   const bool validateKeys )
{
    MLIB_API_START

    // Marshall Inputs
	LAStringMatrix bondLVBAsStringMatrix;
	swig::buildStringMatrix( bondLVBAsStringMatrix, bondLVB );

    LAStringMatrix scheduleLVBAsStringMatrix;
	swig::buildStringMatrix( scheduleLVBAsStringMatrix, scheduleLVB );

    // Call validation_api method
    std::string result = validation_api::tryMeLWOBondCreate( bondObjectName, bondLVBAsStringMatrix, scheduleLVBAsStringMatrix, validateKeys );
    return result;
    
    MLIB_API_END
}

/* @brief			swig interface for meLWOBondCreateFromGenerator function
*  @param [in]		bondObjectName		Bond Object Name
*  @param [in]		bondGeneratorName   Bond Generator Name
*  @param [in]		bondExpressionLVB	Bond Expression Label Value Block (LVB) - A key-value matrix representing the bond definition
*  @param [in]		validateKeys	    Validate Bond LVB keys: true or false - true will throw if a key is misspecified
*  @param [out]		Returns the bond object handle name
*/
const std::string meLWOBondCreateFromGenerator( const std::string& bondObjectName,
                                                const std::string& bondGeneratorName,
                                                const SWIG_STRINGMATRIX & bondExpressionLVB,
                                                const bool validateKeys )
{
    MLIB_API_START

    // Marshall Inputs
    LAStringMatrix bondExpressionLVBAsStringMatrix;
	swig::buildStringMatrix( bondExpressionLVBAsStringMatrix, bondExpressionLVB );

    // Call validation_api method
    std::string result = validation_api::tryMeLWOBondCreateFromGenerator( bondObjectName, bondGeneratorName, bondExpressionLVBAsStringMatrix, validateKeys );
    return result;

    MLIB_API_END
}
