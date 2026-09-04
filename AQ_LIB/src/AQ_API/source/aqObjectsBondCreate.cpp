// aqObjectsBondCreate.cpp

/* 
 * @brief			Swig interface for aqObjectsBondCreate... functions
 * @Created:		25th June 2018
 * @Author:			Nicholas Burgess
 * @Department:		Quant Research and Analytics
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#include "aqObjectsBondCreate.h"
#include "AQLCoreTemplateType.h"
#include "TypeUtilities.h"
#include "tryAqObjectsBond.h"
#include "APISetUp.h"               // AQ_API_START and AQ_API_END Macros


/* @brief			swig interface for aqObjectsBondCreate function
*  @param [in]		bondObjectName		Bond Object Name
*  @param [in]		bondLVB	            Bond Definition Label Value Block (LVB) - A key-value matrix representing the bond definition
*  @param [in]		scheduleLVB     	Schedule Definition Label Value Block (LVB) - A key-value matrix representing the schedule definition
*  @param [in]		validateKeys	    Validate Bond LVB keys: true or false - true will throw if a key is misspecified
*  @param [out]		Returns the bond object handle name
*/
const std::string aqObjectsBondCreate( const std::string& bondObjectName,
                                   const SWIG_STRINGMATRIX & bondLVB,
                                   const SWIG_STRINGMATRIX & scheduleLVB,
                                   const bool validateKeys )
{
    AQ_API_START

    // Marshall Inputs
	AQLStringMatrix bondLVBAsStringMatrix;
	swig::buildStringMatrix( bondLVBAsStringMatrix, bondLVB );

    AQLStringMatrix scheduleLVBAsStringMatrix;
	swig::buildStringMatrix( scheduleLVBAsStringMatrix, scheduleLVB );

    // Call validation method
    std::string result = validation::tryAqObjectsBondCreate( bondObjectName, bondLVBAsStringMatrix, scheduleLVBAsStringMatrix, validateKeys );
    return result;
    
    AQ_API_END
}

/* @brief			swig interface for aqObjectsBondCreateFromGenerator function
*  @param [in]		bondObjectName		Bond Object Name
*  @param [in]		bondGeneratorName   Bond Generator Name
*  @param [in]		bondExpressionLVB	Bond Expression Label Value Block (LVB) - A key-value matrix representing the bond definition
*  @param [in]		validateKeys	    Validate Bond LVB keys: true or false - true will throw if a key is misspecified
*  @param [out]		Returns the bond object handle name
*/
const std::string aqObjectsBondCreateFromGenerator( const std::string& bondObjectName,
                                                const std::string& bondGeneratorName,
                                                const SWIG_STRINGMATRIX & bondExpressionLVB,
                                                const bool validateKeys )
{
    AQ_API_START

    // Marshall Inputs
    AQLStringMatrix bondExpressionLVBAsStringMatrix;
	swig::buildStringMatrix( bondExpressionLVBAsStringMatrix, bondExpressionLVB );

    // Call validation method
    std::string result = validation::tryAqObjectsBondCreateFromGenerator( bondObjectName, bondGeneratorName, bondExpressionLVBAsStringMatrix, validateKeys );
    return result;

    AQ_API_END
}
