#include "AQLCoreTemplateType.h"
#include "TypeUtilities.h"

#include "aqSwapSchedule.h"
#include "tryAqSwapSchedule.h"
#include "tryAqSwapObjectSchedule.h"
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

/* @brief			swig interface for the aqSwapObjectScheduleCreate function
*  @param [in]		scheduleName		Schedule name
*  @param [in]		swapScheduleLVB		Schedule label value block
*  @param [in]		validateKeys		True to validate the all keys provided are valid. Default to True
*  @return			The scheduleName showing the schedule has been cached
*/
std::string aqSwapObjectScheduleCreate( const std::string& scheduleName,
                                     const SWIG_STRINGMATRIX & swapScheduleLVB,
                                     bool validateKeys )
{
    AQ_API_START

    AQLStringMatrix swapScheduleLVBAsStringMatrix;
    swig::buildStringMatrix( swapScheduleLVBAsStringMatrix, swapScheduleLVB );

    std::string result = validation::tryAqSwapObjectScheduleCreate( scheduleName, swapScheduleLVBAsStringMatrix, validateKeys );
    return result;

    AQ_API_END
}

/* @brief			swig interface for the aqSwapObjectScheduleDisplay function
*  @param [in]		scheduleName			Schedule name
*  @param [in]		showBespokeProperties	True to show bespoke schedule properties, default to false
*  @param [in]		showColumnHeaders		True to show column headers, default to true
*  @param [in]		columnList				Column header names to show specified columns. Default to empty list showing all columns
*  @return			Schedule display
*/
SWIG_STRINGMATRIX aqSwapObjectScheduleDisplay( const std::string& scheduleName,
                                            bool showBespokeProperties,
                                            bool showColumnHeaders,
                                            const std::vector<std::string>& columnList )
{
    AQ_API_START

    AnyTypeMatrix result = validation::tryAqSwapObjectScheduleDisplay( scheduleName, showBespokeProperties, showColumnHeaders, columnList );

    SWIG_STRINGMATRIX resultsStringMatrix = swig::fromAnyTypeMatrixToMatrixOfString( result );
    return resultsStringMatrix;

    AQ_API_END
}

/* @brief			swig interface for the aqSwapObjectScheduleCreateBespoke function
*  @param [in]		scheduleName				Bespoke schedule name
*  @param [in]		bespokeScheduleProperties	Bespoke schedule properties label value block
*  @param [in]		bespokeScheduleLVB			Bespoke schedule cashflow label value block
*  @param [in]		validateKeys				True to validate the all keys provided are valid for the bespokeScheduleProperties. Default to True
*  @return			The schedule name
*/
std::string aqSwapObjectScheduleCreateBespoke( const std::string& scheduleName,
                                            const SWIG_STRINGMATRIX & bespokeScheduleProperties,
                                            const SWIG_STRINGMATRIX & bespokeScheduleLVB,
                                            bool validateKeys )
{
    AQ_API_START

    AQLStringMatrix bespokeSchedulePropertiesAsStringMatrix;
    swig::buildStringMatrix( bespokeSchedulePropertiesAsStringMatrix, bespokeScheduleProperties );

    AQLStringMatrix bespokeScheduleLVBAsStringMatrix;
    swig::buildStringMatrix( bespokeScheduleLVBAsStringMatrix, bespokeScheduleLVB );

    std::string result = validation::tryAqSwapObjectScheduleCreateBespoke( scheduleName, bespokeSchedulePropertiesAsStringMatrix, bespokeScheduleLVBAsStringMatrix, validateKeys );
    return result;

    AQ_API_END
}

/* @brief			swig interface for the aqSwapObjectScheduleCreateBespokeFromCashflows function
*  @param [in]		scheduleObjectName			Bespoke schedule name
*  @param [in]		bespokeScheduleProperties	Bespoke schedule properties label value block
*  @param [in]		bespokeCashflowsLVB			Bespoke schedule cashflow label value block matrix
*  @param [in]		validateKeys				True to validate the all keys provided are valid for the bespokeScheduleProperties. Default to True
*  @return			The schedule name
*/
std::string aqSwapObjectScheduleCreateBespokeFromCashflows( const std::string& scheduleObjectName,
                                                          const SWIG_STRINGMATRIX & bespokeScheduleProperties,
                                                          const SWIG_STRINGMATRIX & bespokeCashflowsLVB,
                                                          bool validateKeys )
{
    AQ_API_START

    AQLStringMatrix bespokeSchedulePropertiesAsStringMatrix;
    swig::buildStringMatrix( bespokeSchedulePropertiesAsStringMatrix, bespokeScheduleProperties );

    AQLStringMatrix bespokeCashflowsLVBAsStringMatrix;
    swig::buildStringMatrix( bespokeCashflowsLVBAsStringMatrix, bespokeCashflowsLVB );

    std::string result = validation::tryAqSwapObjectScheduleCreateBespokeFromCashflows( scheduleObjectName, bespokeSchedulePropertiesAsStringMatrix, bespokeCashflowsLVBAsStringMatrix, validateKeys );
    return result;

    AQ_API_END
}
