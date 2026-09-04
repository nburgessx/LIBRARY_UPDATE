#include "AQLCoreTemplateType.h"
#include "TypeUtilities.h"

#include "aqSwapSchedule.h"
#include "tryAqSwapSchedule.h"
#include "APISetUp.h"               // AQ_API_START and AQ_API_END Macros


/* @brief			swig interface for aqSwapSchedule
*  @param [in]		showColumnHeaders	A boolean to specify whether to include the column headers in the output
*  @param [in]		swapScheduleLVB		A label value block defining the swap schedule
*  @param [in]		validateKeys		True to validate the all keys provided are valid. Default to True
*  @return			A matrix containing the swap schedule
*/
SWIG_STRINGMATRIX aqSwapSchedule(bool showColumnHeaders, 
							            const SWIG_STRINGMATRIX & swapScheduleLVB, 
							            bool validateKeys)
{
    AQ_API_START

    // marshall all inputs
	AQLStringMatrix swapScheduleParams;
	swig::buildStringMatrix( swapScheduleParams, swapScheduleLVB );
	
	const bool validateKeys = true;
	const std::vector<std::string> columnList;
	const bool convertDatesToExcelFormat = false;
	AQLStringMatrix tempResult = validation::tryAqSwapSchedule( showColumnHeaders, swapScheduleParams, validateKeys, columnList, convertDatesToExcelFormat );

	etrading::VariantMatrix variantMatrix;
	swig::buildVariantMatrix( variantMatrix, tempResult );

	SWIG_STRINGMATRIX ret = swig::fromVariantMatrixToMatrixOfString( variantMatrix );

	return ret;
    AQ_API_END
}

/* @brief			swig interface for aqSwapScheduleFixed
*  @param [in]		showColumnHeaders	A boolean to specify whether to include the column headers in the output
*  @param [in]		swapScheduleLVB		A label value block defining the swap schedule
*  @return			A matrix containing the swap schedule
*/
SWIG_STRINGMATRIX aqSwapScheduleFixed( bool showColumnHeaders, 
											  const SWIG_STRINGMATRIX& swapScheduleLVB )
{
    AQ_API_START

    // marshall all inputs
	AQLStringMatrix swapScheduleParams;
	swig::buildStringMatrix( swapScheduleParams, swapScheduleLVB );
	
	const bool validateKeys = true;
	const std::vector<std::string> columnList;
	const bool convertDatesToExcelFormat = false;
	AQLStringMatrix tempResult = validation::tryAqSwapScheduleFixed( showColumnHeaders, swapScheduleParams, validateKeys, columnList, convertDatesToExcelFormat );

	etrading::VariantMatrix variantMatrix;
	swig::buildVariantMatrix( variantMatrix, tempResult );

	SWIG_STRINGMATRIX ret = swig::fromVariantMatrixToMatrixOfString( variantMatrix );

	return ret;
    AQ_API_END
}

/* @brief			swig interface for aqSwapScheduleFloat
*  @param [in]		showColumnHeaders	A boolean to specify whether to include the column headers in the output
*  @param [in]		swapScheduleLVB		A label value block defining the swap schedule
*  @return			A matrix containing the swap schedule
*/
SWIG_STRINGMATRIX aqSwapScheduleFloat( bool showColumnHeaders, 
											  const SWIG_STRINGMATRIX& swapScheduleLVB )
{
    AQ_API_START

    // marshall all inputs
	AQLStringMatrix swapScheduleParams;
	swig::buildStringMatrix( swapScheduleParams, swapScheduleLVB );
	
	const bool validateKeys = true;
	const std::vector<std::string> columnList;
	const bool convertDatesToExcelFormat = false;
	AQLStringMatrix tempResult = validation::tryAqSwapScheduleFloat( showColumnHeaders, swapScheduleParams, validateKeys, columnList, convertDatesToExcelFormat );

	etrading::VariantMatrix variantMatrix;
	swig::buildVariantMatrix( variantMatrix, tempResult );

	SWIG_STRINGMATRIX ret = swig::fromVariantMatrixToMatrixOfString( variantMatrix );

	return ret;
    AQ_API_END
}


/* @brief			return a set of expected keys for swap leg schedule label value block
*  @return			expected keys
*/
std::vector<std::string> aqSwapScheduleLVBKeys()
{
    AQ_API_START
	std::vector<std::string> ret;
    ret = validation::tryAqSwapScheduleLVBKeys();
    return ret;
    AQ_API_END
}
