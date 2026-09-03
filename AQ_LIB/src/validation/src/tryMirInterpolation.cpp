/*
 * @brief			validation interface for mirInterpolation method
 * @Created:		24 March 2016
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#include "tryMirInterpolation.h"
#include "LAMathInterpolationUtilities.h"
#include "CreateDataFile.h"
#include "CurveValidation.h"
#include "LabelValueBlockValidation.h"
#include "ScheduleValidation.h"
#include "SwapValidation.h"
#include "ParameterValidation.h"
#include "StructuredExceptionHandler.h"
#include "LAFunctionUtilities.h"
#include "RecordMacros.h"

using etrading::CreateDataFile;

namespace validation_api
{

    /* @brief			swig interface for mirInterpolation
    *  @param [in]		array1			X-axis values of the data to be interpolated
    *  @param [in]		array2			Y-axis values of the data to be interpolated
    *  @param [in]		point			X-axis point to interpolate
    *  @param [in]		interpolation	Interpolation method of the yield curve
    *  @return			the Y-axis interpolated value
    */
    double tryMirInterpolation(	const DoubleVector& array1,
                                const DoubleVector& array2,
                                double point,
                                const LAString& interpolation )

    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        RECORD_INPUTS( array1, array2, point, interpolation )

        // Validate parameters
        MLIB_REQUIRE( array1.size() == array2.size(), "Invalid Interpolation Data: xValues and yValues must be the same size")
		MLIB_REQUIRE( array1.size() == 0, "Invalid Interpolation Data: Interpolation data is empty")
        
		const double result = LAMathInterpolationUtilities::interpolate( array1, array2, point, interpolation.c_str() );
		RECORD_OUTPUTS_AND_RETURN_RESULT( result );
        
        VALID_EXCEPTION_END
    }


}
