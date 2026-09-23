// aqToolDiagnostics.cpp

/*
 * @brief			Swig interface for small Tool-category utility functions: aqToolEchoDouble,
 *					aqToolBondAverageYield, aqToolBondYieldFromFuturePrice and aqToolSwapScheduleTemplate.
 */

#include "aqToolDiagnostics.h"
#include "tryAqToolEchoDouble.h"
#include "tryAqBondObject.h"
#include "tryAqSwapObjectSchedule.h"

#include "AQLCoreTemplateType.h"
#include "TypeUtilities.h"          // Swig Marshalling Helper Methods
#include "APISetUp.h"               // AQ_API_START and AQ_API_END Macros

/* @brief			swig interface for aqToolEchoDouble. Echo a double straight back through the validation layer.
*  @param [in]		value		Any number
*  @return			The same number
*/
double aqToolEchoDouble( const double& value )
{
    AQ_API_START

    double result = validation::tryAqToolEchoDouble( value );
    return result;

    AQ_API_END
}

/* @brief			swig interface for aqToolBondAverageYield. The average of several underlying bond yields.
*  @param [in]		underlyingBondYields	A vector of bond yields to average
*  @return			The average yield
*/
double aqToolBondAverageYield( const std::vector<double>& underlyingBondYields )
{
    AQ_API_START

    double result = validation::tryAqToolBondAverageYield( underlyingBondYields );
    return result;

    AQ_API_END
}

/* @brief			swig interface for aqToolBondYieldFromFuturePrice. A bond yield implied by a bond-future price.
*  @param [in]		futurePrice		The bond-future price
*  @return			The implied bond yield
*/
double aqToolBondYieldFromFuturePrice( const double& futurePrice )
{
    AQ_API_START

    double result = validation::tryAqToolBondYieldFromFuturePrice( futurePrice );
    return result;

    AQ_API_END
}

/* @brief			swig interface for aqToolSwapScheduleTemplate. A template of a swap's floating/fixing leg
*                   schedules, from a label/value block of schedule properties.
*  @param [in]		showColumnHeaders	Include a header row
*  @param [in]		swapScheduleLVB		The schedule configuration as a label/value block
*  @param [in]		validateKeys		Optional. Default TRUE. Check the LVB keys
*  @param [in]		columnList			Optional. Column names to include; default all columns
*  @return			The swap schedule template
*/
SWIG_STRINGMATRIX aqToolSwapScheduleTemplate( const bool showColumnHeaders,
                                               const SWIG_STRINGMATRIX& swapScheduleLVB,
                                               const bool validateKeys,
                                               const std::vector<std::string>& columnList )
{
    AQ_API_START

    // Marshall Inputs
    LabelValueBlock swapScheduleLVB_ = swig::buildSingleLabelValueBlock( swapScheduleLVB );

    // Call the Function
    AnyTypeMatrix result = validation::tryAqToolSwapScheduleTemplate( showColumnHeaders, swapScheduleLVB_, validateKeys, columnList );

    // Marshall Output(s)
    return swig::fromAnyTypeMatrixToMatrixOfString( result );

    AQ_API_END
}
