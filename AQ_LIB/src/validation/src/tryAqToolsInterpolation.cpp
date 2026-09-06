#include "tryAqToolsInterpolation.h"
#include "CurveUtilities.h"
#include "CreateDataFile.h"
#include "RecordMacros.h"
#include "StructuredExceptionHandler.h"

using etrading::CreateDataFile;

namespace validation
{

    /* @brief			validation interface for aqToolsInterpolation
    *  @param [in]		xValues			X-axis values of the data
    *  @param [in]		yValues			Y-axis values of the data
    *  @param [in]		xPoint			X-axis point
    *  @param [in]		interpolation	Interpolation method
	*  @param [in]		joinXValue		A joinXValue as a double value used in hybrid interpolation
    *  @return			the Y-axis interpolated value
    */
    double tryAqToolsInterpolation( const DoubleVector& xValues,
                                      const DoubleVector& yValues,
                                      double xPoint,
                                      const AQLString& interpolation,
									  const double joinXValue)

    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        AQ_RECORD_INPUTS( xValues, yValues, xPoint, interpolation, joinXValue );

        const double ret = etrading::interpolate( xValues, yValues, xPoint, etrading::toInterpolationEnum( interpolation.c_str() ), joinXValue);

        AQ_RECORD_OUTPUTS( ret );

        return ret;

        VALID_EXCEPTION_END
    }


}
