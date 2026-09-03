#include "tryMeUtilityInterpolation.h"
#include "CurveUtilities.h"
#include "CreateDataFile.h"
#include "StructuredExceptionHandler.h"

using etrading::CreateDataFile;

namespace validation
{

    /* @brief			validation interface for meUtilityInterpolation
    *  @param [in]		xValues			X-axis values of the data
    *  @param [in]		yValues			Y-axis values of the data
    *  @param [in]		xPoint			X-axis point
    *  @param [in]		interpolation	Interpolation method
	*  @param [in]		joinXValue		A joinXValue as a double value used in hybrid interpolation
    *  @return			the Y-axis interpolated value
    */
    double tryMeUtilityInterpolation( const DoubleVector& xValues,
                                      const DoubleVector& yValues,
                                      double xPoint,
                                      const AQLString& interpolation,
									  const double joinXValue)

    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( "tryMeUtilityInterpolation_inputs" );
            file.write( "generatorFunction", "tryMeUtilityInterpolation" );
            file.write( "xValues", xValues );
            file.write( "yValues", yValues );
            file.write( "xPoint",	xPoint );
            file.write( "interpolation", interpolation );
			file.write( "joinXValue", joinXValue);
        }

        const double ret = etrading::interpolate( xValues, yValues, xPoint, etrading::toInterpolationEnum( interpolation.c_str() ), joinXValue);

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( "tryMeUtilityInterpolation_outputs" );
            file.write( "output", ret );
        }

        return ret;

        VALID_EXCEPTION_END
    }


}
