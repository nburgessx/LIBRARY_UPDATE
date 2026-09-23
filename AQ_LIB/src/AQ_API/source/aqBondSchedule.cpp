// aqBondSchedule.cpp

/*
 * @brief			Swig interface for the stateless aqBondSchedule function - builds a bond's cashflow schedule from a label/value block
 */

#include "aqBondSchedule.h"
#include "AQLCoreTemplateType.h"
#include "TypeUtilities.h"
#include "tryAqBondSchedule.h"
#include "APISetUp.h"               // AQ_API_START and AQ_API_END Macros


/* @brief			swig interface for aqBondScheduleLVBKeys function
*  @param [out]		Returns the expected keys for a bond leg schedule label/value block
*/
std::vector<std::string> aqBondScheduleLVBKeys()
{
    AQ_API_START

    // Call validation method
    std::vector<std::string> result = validation::tryAqBondScheduleLVBKeys();
    return result;

    AQ_API_END
}

/* @brief			swig interface for aqBondSchedule function
*  @param [in]		bondScheduleLVB		A label value block defining the bond
*  @param [in]		validateKeys		True to validate the keys provided are valid
*  @param [in]		showColumnHeaders	True to include the column headers in the output
*  @param [out]		Returns a matrix of the bond's cashflow schedule
*/
SWIG_STRINGMATRIX aqBondSchedule( const SWIG_STRINGMATRIX& bondScheduleLVB, const bool validateKeys, const bool showColumnHeaders )
{
    AQ_API_START

    // Marshall Inputs
    AQLStringMatrix bondScheduleLVBAsStringMatrix;
    swig::buildStringMatrix( bondScheduleLVBAsStringMatrix, bondScheduleLVB );
    LabelValueBlock bondScheduleLVBAsLabelValueBlock( bondScheduleLVBAsStringMatrix );

    // Call validation method
    AQLStringMatrix result = validation::tryAqBondSchedule( bondScheduleLVBAsLabelValueBlock, validateKeys, showColumnHeaders );

    // Marshall Output(s)
    SWIG_STRINGMATRIX resultsStringMatrix = swig::fromStringMatrixToMatrixOfString( result );
    return resultsStringMatrix;

    AQ_API_END
}
