/* 
 * @brief			Swig interface to Java for meProductSwapSchedule method
 * @Created:		03 June 2016
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#include "LACoreTemplateType.h"
#include "TypeUtilities.h"

#include "meProductSwapSchedule.h"
#include "tryMeProductSwapSchedule.h"
#include "APISetUp.h"               // MLIB_API_START and MLIB_API_END Macros


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
    MLIB_API_START

    // marshall all inputs
	LAStringMatrix swapScheduleParams;
	swig::buildStringMatrix( swapScheduleParams, swapScheduleLVB );
	
	const bool validateKeys = true;
	const std::vector<std::string> columnList;
	const bool convertDatesToExcelFormat = false;
	LAStringMatrix tempResult = validation_api::tryMeProductSwapSchedule( showColumnHeaders, swapScheduleParams, validateKeys, columnList, convertDatesToExcelFormat );

	etrading::VariantMatrix variantMatrix;
	swig::buildVariantMatrix( variantMatrix, tempResult );

	SWIG_STRINGMATRIX ret = swig::fromVariantMatrixToMatrixOfString( variantMatrix );

	return ret;
    MLIB_API_END
}

/* @brief			swig interface for meProductSwapScheduleFixed
*  @param [in]		showColumnHeaders	A boolean to specify whether to include the column headers in the output
*  @param [in]		swapScheduleLVB		A label value block defining the swap schedule
*  @return			A matrix containing the swap schedule
*/
SWIG_STRINGMATRIX meProductSwapScheduleFixed( bool showColumnHeaders, 
											  const SWIG_STRINGMATRIX& swapScheduleLVB )
{
    MLIB_API_START

    // marshall all inputs
	LAStringMatrix swapScheduleParams;
	swig::buildStringMatrix( swapScheduleParams, swapScheduleLVB );
	
	const bool validateKeys = true;
	const std::vector<std::string> columnList;
	const bool convertDatesToExcelFormat = false;
	LAStringMatrix tempResult = validation_api::tryMeProductSwapScheduleFixed( showColumnHeaders, swapScheduleParams, validateKeys, columnList, convertDatesToExcelFormat );

	etrading::VariantMatrix variantMatrix;
	swig::buildVariantMatrix( variantMatrix, tempResult );

	SWIG_STRINGMATRIX ret = swig::fromVariantMatrixToMatrixOfString( variantMatrix );

	return ret;
    MLIB_API_END
}

/* @brief			swig interface for meProductSwapScheduleFloat
*  @param [in]		showColumnHeaders	A boolean to specify whether to include the column headers in the output
*  @param [in]		swapScheduleLVB		A label value block defining the swap schedule
*  @return			A matrix containing the swap schedule
*/
SWIG_STRINGMATRIX meProductSwapScheduleFloat( bool showColumnHeaders, 
											  const SWIG_STRINGMATRIX& swapScheduleLVB )
{
    MLIB_API_START

    // marshall all inputs
	LAStringMatrix swapScheduleParams;
	swig::buildStringMatrix( swapScheduleParams, swapScheduleLVB );
	
	const bool validateKeys = true;
	const std::vector<std::string> columnList;
	const bool convertDatesToExcelFormat = false;
	LAStringMatrix tempResult = validation_api::tryMeProductSwapScheduleFloat( showColumnHeaders, swapScheduleParams, validateKeys, columnList, convertDatesToExcelFormat );

	etrading::VariantMatrix variantMatrix;
	swig::buildVariantMatrix( variantMatrix, tempResult );

	SWIG_STRINGMATRIX ret = swig::fromVariantMatrixToMatrixOfString( variantMatrix );

	return ret;
    MLIB_API_END
}


/* @brief			return a set of expected keys for swap leg schedule label value block
*  @return			expected keys
*/
std::vector<std::string> meProductSwapScheduleLVBKeys()
{
    MLIB_API_START
	std::vector<std::string> ret;
    ret = validation_api::tryMeProductSwapScheduleLVBKeys();
    return ret;
    MLIB_API_END
}
