// meLWOBondCreate.h

/* 
 * @brief			Swig interface for meLWOBondCreate... functions
 * @Created:		25th June 2018
 * @Author:			Nicholas Burgess
 * @Department:		Quant Research and Analytics
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#pragma once

#include "SwigTypes.h"

#include <string>
#include <vector>

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
                                   const bool validateKeys );

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
                                                const bool validateKeys );
