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

/* @brief			return a set of expected keys for a swap-level properties label value block
*  @return			expected keys
*/
std::vector<std::string> aqSwapObjectLVBKeys()
{
    AQ_API_START

    std::vector<std::string> result = validation::tryAqSwapObjectLVBKeys();
    return result;

    AQ_API_END
}

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
                                     const bool validateKeys )
{
    AQ_API_START

    // Marshall Inputs
    AQLStringMatrix swapPropertiesAsStringMatrix;
    swig::buildStringMatrix( swapPropertiesAsStringMatrix, swapPropertiesLVB );

    // Call validation method
    std::string result = validation::tryAqSwapObjectCreateFromLegs( swapName, legObjectNames, swapPropertiesAsStringMatrix, isXccySwap, validateKeys );
    return result;

    AQ_API_END
}

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
                                        const bool validateKeys )
{
    AQ_API_START

    // Marshall Inputs
    AQLStringMatrix leg1AsStringMatrix;
    swig::buildStringMatrix( leg1AsStringMatrix, leg1LVB );

    AQLStringMatrix leg2AsStringMatrix;
    swig::buildStringMatrix( leg2AsStringMatrix, leg2LVB );

    AQLStringMatrix swapPropertiesAsStringMatrix;
    swig::buildStringMatrix( swapPropertiesAsStringMatrix, swapPropertiesLVB );

    // Call validation method
    std::string result = validation::tryAqSwapObjectCreateFromLegLVBs( swapName, leg1AsStringMatrix, leg2AsStringMatrix, swapPropertiesAsStringMatrix, isXccySwap, validateKeys );
    return result;

    AQ_API_END
}

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
                                         const bool validateKeys )
{
    AQ_API_START

    // Marshall Inputs
    AQLStringMatrix leg1AsStringMatrix;
    swig::buildStringMatrix( leg1AsStringMatrix, leg1LVB );

    AQLStringMatrix leg2AsStringMatrix;
    swig::buildStringMatrix( leg2AsStringMatrix, leg2LVB );

    AQLStringMatrix swapPropertiesAsStringMatrix;
    swig::buildStringMatrix( swapPropertiesAsStringMatrix, swapPropertiesLVB );

    // Call validation method
    std::string result = validation::tryAqSwapObjectCreateFromSchedule( swapName, schedule1Name, schedule2Name, leg1AsStringMatrix, leg2AsStringMatrix, swapPropertiesAsStringMatrix, isXccySwap, validateKeys );
    return result;

    AQ_API_END
}

/* @brief			swig interface for the aqSwapObjectCreateBackToBack function
*  @param [in]		fromSwapName	    The cached swap handle to copy from
*  @param [in]		toSwapName		    Name for the new swap
*  @param [out]		Returns the new swap object handle name
*/
std::string aqSwapObjectCreateBackToBack( const std::string& fromSwapName,
                                       const std::string& toSwapName )
{
    AQ_API_START

    std::string result = validation::tryAqSwapObjectCreateBackToBack( fromSwapName, toSwapName );
    return result;

    AQ_API_END
}

/* @brief			swig interface for the aqSwapObjectAddLeg function
*  @param [in]		swapName		    The cached swap name
*  @param [in]		legObjectName	    The cached leg name to add
*  @param [out]		Returns the swap object handle name
*/
std::string aqSwapObjectAddLeg( const std::string& swapName,
                             const std::string& legObjectName )
{
    AQ_API_START

    std::string result = validation::tryAqSwapObjectAddLeg( swapName, legObjectName );
    return result;

    AQ_API_END
}

/* @brief			swig interface for the aqSwapObjectAddFee function
*  @param [in]		swapName		    The cached swap name
*  @param [in]		feeName		        The cached fee leg name to add
*  @param [out]		Returns the swap object handle name
*/
std::string aqSwapObjectAddFee( const std::string& swapName,
                             const std::string& feeName )
{
    AQ_API_START

    std::string result = validation::tryAqSwapObjectAddFee( swapName, feeName );
    return result;

    AQ_API_END
}

/* @brief			swig interface for the aqSwapGeneratorCreate function
*  @param [in]		swapGeneratorName	Swap generator name
*  @param [in]		swapGeneratorLVB	Swap generator label value block (conventions template)
*  @param [in]		validateKeys	    Validate LVB keys: true or false - true will throw if a key is misspecified
*  @param [out]		Returns the swap generator object handle name
*/
std::string aqSwapGeneratorCreate( const std::string& swapGeneratorName,
                                const SWIG_STRINGMATRIX & swapGeneratorLVB,
                                const bool validateKeys )
{
    AQ_API_START

    // Marshall Inputs
    AQLStringMatrix swapGeneratorLVBAsStringMatrix;
    swig::buildStringMatrix( swapGeneratorLVBAsStringMatrix, swapGeneratorLVB );

    // Call validation method
    std::string result = validation::tryAqSwapGeneratorCreate( swapGeneratorName, swapGeneratorLVBAsStringMatrix, validateKeys );
    return result;

    AQ_API_END
}

/* @brief			swig interface for the aqSwapObjectDisplay function, displays the INPUT parameters of the cached swap
*  @param [in]		swapName		    Swap name
*  @param [out]		Swap display of the input parameters, one block per leg
*/
SWIG_STRINGMATRIX aqSwapObjectDisplay( const std::string& swapName )
{
    AQ_API_START

    std::vector<AnyTypeMatrix> result = validation::tryAqSwapObjectDisplay( swapName );

    // Marshall Output to Standard String Matrix
    SWIG_STRINGMATRIX resultsStringMatrix = swig::fromVectorOfAnyTypeMatrixToMatrixOfString( result );
    return resultsStringMatrix;

    AQ_API_END
}

/* @brief			swig interface for the aqSwapGeneratorDisplay function, displays the INPUT parameters of the cached swap generator
*  @param [in]		swapGeneratorName	Swap generator name
*  @param [out]		Swap generator display of the input parameters
*/
SWIG_STRINGMATRIX aqSwapGeneratorDisplay( const std::string& swapGeneratorName )
{
    AQ_API_START

    AQLStringMatrix result = validation::tryAqSwapGeneratorDisplay( swapGeneratorName );

    // Marshall Output to Standard String Matrix
    SWIG_STRINGMATRIX resultsStringMatrix = swig::fromStringMatrixToMatrixOfString( result );
    return resultsStringMatrix;

    AQ_API_END
}
