#include "AQLCoreTemplateType.h"
#include "TypeUtilities.h"

#include "meProductSwapSchedule.h"
#include "tryMeProductSwapSchedule.h"
#include "APISetUp.h"               // AQ_API_START and AQ_API_END Macros


/* @brief			swig interface for meProductSwapSchedule
*  @param [in]		showColumnHeaders	A boolean to specify whether to include the column headers in the output
*  @param [in]		swapScheduleLVB		A label value block defining the swap schedule
*  @param [in]		validateKeys		True to validate the all keys provided are valid. Default to True
*  @return			A matrix containing the swap schedule
*/
SWIG_STRINGMATRIX meProductSwapSchedule(bool showColumnHeaders, 
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
	AQLStringMatrix tempResult = validation::tryMeProductSwapSchedule( showColumnHeaders, swapScheduleParams, validateKeys, columnList, convertDatesToExcelFormat );

	etrading::VariantMatrix variantMatrix;
	swig::buildVariantMatrix( variantMatrix, tempResult );

	SWIG_STRINGMATRIX ret = swig::fromVariantMatrixToMatrixOfString( variantMatrix );

	return ret;
    AQ_API_END
}

/* @brief			swig interface for meProductSwapScheduleFixed
*  @param [in]		showColumnHeaders	A boolean to specify whether to include the column headers in the output
*  @param [in]		swapScheduleLVB		A label value block defining the swap schedule
*  @return			A matrix containing the swap schedule
*/
SWIG_STRINGMATRIX meProductSwapScheduleFixed( bool showColumnHeaders, 
											  const SWIG_STRINGMATRIX& swapScheduleLVB )
{
    AQ_API_START

    // marshall all inputs
	AQLStringMatrix swapScheduleParams;
	swig::buildStringMatrix( swapScheduleParams, swapScheduleLVB );
	
	const bool validateKeys = true;
	const std::vector<std::string> columnList;
	const bool convertDatesToExcelFormat = false;
	AQLStringMatrix tempResult = validation::tryMeProductSwapScheduleFixed( showColumnHeaders, swapScheduleParams, validateKeys, columnList, convertDatesToExcelFormat );

	etrading::VariantMatrix variantMatrix;
	swig::buildVariantMatrix( variantMatrix, tempResult );

	SWIG_STRINGMATRIX ret = swig::fromVariantMatrixToMatrixOfString( variantMatrix );

	return ret;
    AQ_API_END
}

/* @brief			swig interface for meProductSwapScheduleFloat
*  @param [in]		showColumnHeaders	A boolean to specify whether to include the column headers in the output
*  @param [in]		swapScheduleLVB		A label value block defining the swap schedule
*  @return			A matrix containing the swap schedule
*/
SWIG_STRINGMATRIX meProductSwapScheduleFloat( bool showColumnHeaders, 
											  const SWIG_STRINGMATRIX& swapScheduleLVB )
{
    AQ_API_START

    // marshall all inputs
	AQLStringMatrix swapScheduleParams;
	swig::buildStringMatrix( swapScheduleParams, swapScheduleLVB );
	
	const bool validateKeys = true;
	const std::vector<std::string> columnList;
	const bool convertDatesToExcelFormat = false;
	AQLStringMatrix tempResult = validation::tryMeProductSwapScheduleFloat( showColumnHeaders, swapScheduleParams, validateKeys, columnList, convertDatesToExcelFormat );

	etrading::VariantMatrix variantMatrix;
	swig::buildVariantMatrix( variantMatrix, tempResult );

	SWIG_STRINGMATRIX ret = swig::fromVariantMatrixToMatrixOfString( variantMatrix );

	return ret;
    AQ_API_END
}


/* @brief			return a set of expected keys for swap leg schedule label value block
*  @return			expected keys
*/
std::vector<std::string> meProductSwapScheduleLVBKeys()
{
    AQ_API_START
	std::vector<std::string> ret;
    ret = validation::tryMeProductSwapScheduleLVBKeys();
    return ret;
    AQ_API_END
}
