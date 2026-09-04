// aqObjSwapsCreate.h

/* 
 * @brief			Swig interface for aqObjBondsCreate... functions
 */

#pragma once

#include "SwigTypes.h"

#include <string>
#include <vector>

/* @brief			swig interface for aqObjSwapsCreate function
*  @param [in]		swapObjectName		Swap Object Name
*  @param [in]		swapLVB             Swap Definition Label Value Block (LVB) - A key-value matrix representing the swap definition
*  @param [in]		xccyPropertiesLVB   Xccy Swap Properties Label Value Block (LVB) - A key-value matrix representing the Xccy Swap Properties
*  @param [in]		isXccySwap          isXccySwap: True or False
*  @param [in]		validateKeys	    Validate Bond LVB keys: true or false - true will throw if a key is misspecified
*  @param [out]		Returns the swap object handle name
*/
const std::string aqObjSwapsCreate( const std::string& swapObjectName,
                                   const SWIG_STRINGMATRIX & swapLVB,
                                   const SWIG_STRINGMATRIX & xccyPropertiesLVB,
                                   const bool isXccySwap,
                                   const bool validateKeys );

/* @brief			swig interface for aqObjSwapsCreateFromGenerator function
*  @param [in]		swapObjectName		Swap Object Name
*  @param [in]		swapGeneratorName   Swap Generator Object Name
*  @param [in]		swapLVB	            Swap Definition Label Value Block (LVB) - A key-value matrix representing the swap definition
*  @param [in]		xccyPropertiesLVB   Xccy Swap Properties Label Value Block (LVB) - A key-value matrix representing the Xccy Swap Properties
*  @param [in]		isXccySwap          isXccySwap: True or False
*  @param [in]		validateKeys	    Validate Bond LVB keys: true or false - true will throw if a key is misspecified
*  @param [out]		Returns the swap object handle name
*/
const std::string aqObjSwapsCreateFromGenerator( const std::string& swapObjectName,
                                                const std::string& swapGeneratorName,
                                                const SWIG_STRINGMATRIX & swapLVB,
                                                const SWIG_STRINGMATRIX & xccyPropertiesLVB,
                                                const bool isXccySwap,
                                                const bool validateKeys );
