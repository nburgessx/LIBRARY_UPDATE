// aqSwapObjectCreate.h

/* 
 * @brief			Swig interface for aqBondObjectCreate... functions
 */

#pragma once

#include "SwigTypes.h"

#include <string>
#include <vector>

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
                                   const bool validateKeys );

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
                                                const bool validateKeys );

/* @brief			return a set of expected keys for a swap-level properties label value block
*  @return			expected keys
*/
std::vector<std::string> aqSwapObjectLVBKeys();

/* @brief			swig interface for the aqSwapObjectCreateFromLegs function
*  @param [in]		swapName		    Swap name to be cached
*  @param [in]		legObjectNames	    Cached leg object names
*  @param [in]		swapPropertiesLVB	Swap level properties label value block
*  @param [in]		isXccySwap	        True to enforce it is a cross currency swap
*  @param [in]		validateKeys	    Validate LVB keys: true or false - true will throw if a key is misspecified
*  @param [out]		Returns the swap object handle name
*/
std::string aqSwapObjectCreateFromLegs( const std::string& swapName,
                                     const std::vector<std::string>& legObjectNames,
                                     const SWIG_STRINGMATRIX & swapPropertiesLVB,
                                     const bool isXccySwap,
                                     const bool validateKeys );

/* @brief			swig interface for the aqSwapObjectCreateFromLegLVBs function
*  @param [in]		swapName		    Swap name
*  @param [in]		leg1LVB			    Leg1 label value block
*  @param [in]		leg2LVB			    Leg2 label value block
*  @param [in]		swapPropertiesLVB	Swap level properties label value block
*  @param [in]		isXccySwap	        True to enforce it is a cross currency swap
*  @param [in]		validateKeys	    Validate LVB keys: true or false - true will throw if a key is misspecified
*  @param [out]		Returns the swap object handle name
*/
std::string aqSwapObjectCreateFromLegLVBs( const std::string& swapName,
                                        const SWIG_STRINGMATRIX & leg1LVB,
                                        const SWIG_STRINGMATRIX & leg2LVB,
                                        const SWIG_STRINGMATRIX & swapPropertiesLVB,
                                        const bool isXccySwap,
                                        const bool validateKeys );

/* @brief			swig interface for the aqSwapObjectCreateFromSchedule function
*  @param [in]		swapName		    Swap name
*  @param [in]		schedule1Name	    Cached schedule name for leg 1
*  @param [in]		schedule2Name	    Cached schedule name for leg 2
*  @param [in]		leg1LVB			    Leg1 label value block
*  @param [in]		leg2LVB			    Leg2 label value block
*  @param [in]		swapPropertiesLVB	Swap level properties label value block
*  @param [in]		isXccySwap	        True to enforce it is a cross currency swap
*  @param [in]		validateKeys	    Validate LVB keys: true or false - true will throw if a key is misspecified
*  @param [out]		Returns the swap object handle name
*/
std::string aqSwapObjectCreateFromSchedule( const std::string& swapName,
                                         const std::string& schedule1Name,
                                         const std::string& schedule2Name,
                                         const SWIG_STRINGMATRIX & leg1LVB,
                                         const SWIG_STRINGMATRIX & leg2LVB,
                                         const SWIG_STRINGMATRIX & swapPropertiesLVB,
                                         const bool isXccySwap,
                                         const bool validateKeys );

/* @brief			swig interface for the aqSwapObjectCreateBackToBack function
*  @param [in]		fromSwapName	    The cached swap handle to copy from
*  @param [in]		toSwapName		    Name for the new swap
*  @param [out]		Returns the new swap object handle name
*/
std::string aqSwapObjectCreateBackToBack( const std::string& fromSwapName,
                                       const std::string& toSwapName );

/* @brief			swig interface for the aqSwapObjectAddLeg function
*  @param [in]		swapName		    The cached swap name
*  @param [in]		legObjectName	    The cached leg name to add
*  @param [out]		Returns the swap object handle name
*/
std::string aqSwapObjectAddLeg( const std::string& swapName,
                             const std::string& legObjectName );

/* @brief			swig interface for the aqSwapObjectAddFee function
*  @param [in]		swapName		    The cached swap name
*  @param [in]		feeName		        The cached fee leg name to add
*  @param [out]		Returns the swap object handle name
*/
std::string aqSwapObjectAddFee( const std::string& swapName,
                             const std::string& feeName );

/* @brief			swig interface for the aqSwapGeneratorCreate function
*  @param [in]		swapGeneratorName	Swap generator name
*  @param [in]		swapGeneratorLVB	Swap generator label value block (conventions template)
*  @param [in]		validateKeys	    Validate LVB keys: true or false - true will throw if a key is misspecified
*  @param [out]		Returns the swap generator object handle name
*/
std::string aqSwapGeneratorCreate( const std::string& swapGeneratorName,
                                const SWIG_STRINGMATRIX & swapGeneratorLVB,
                                const bool validateKeys );

/* @brief			swig interface for the aqSwapObjectDisplay function, displays the INPUT parameters of the cached swap
*  @param [in]		swapName		    Swap name
*  @param [out]		Swap display of the input parameters, one block per leg
*/
SWIG_STRINGMATRIX aqSwapObjectDisplay( const std::string& swapName );

/* @brief			swig interface for the aqSwapGeneratorDisplay function, displays the INPUT parameters of the cached swap generator
*  @param [in]		swapGeneratorName	Swap generator name
*  @param [out]		Swap generator display of the input parameters
*/
SWIG_STRINGMATRIX aqSwapGeneratorDisplay( const std::string& swapGeneratorName );
