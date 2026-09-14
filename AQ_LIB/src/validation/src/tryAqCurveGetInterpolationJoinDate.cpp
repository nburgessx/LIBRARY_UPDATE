#include "tryAqCurveGetInterpolationJoinDate.h"
#include "StructuredExceptionHandler.h"
#include "CurveUtilities.h"
#include "CreateDataFile.h"
#include "AQLDate.h"

using etrading::CreateDataFile;
using etrading::decorateCurvename;

namespace validation
{
    /* @brief			validation interface for the tryAqCurveGetInterpolationJoinDate function, to clear the object pool
    *  @param [in]		curveCollection		ID of the curve set
    *  @param [in]		curveIndex			Index of the curve set
	*  @param [in]		interpolation		Interpolation type
	*  @return			Interpolation join date
    */
    AQLDate tryAqCurveGetInterpolationJoinDate(const AQLString& curveCollection, const AQLString& curveIndex, const AQLString& interpolation)
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateCurvename( "tryAqCurveGetInterpolationJoinDate", curveCollection, curveIndex) );
            file.write( "generatorFunction", "tryAqCurveGetInterpolationJoinDate" );
            file.write( "curveCollection", curveCollection );
            file.write( "curveIndex", curveIndex );
			file.write( "interpolation", interpolation );
        }

		AQLDate joinDate;
		if (interpolation.size() == 0 || AQLString(interpolation) == "LINEARSPLINE")
		{
			joinDate = etrading::getLinearSplineJoinDate(curveCollection, curveIndex);
		}
		else
		{
			AQ_THROW( "Unsupported interpolation method" );
		}

		if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateCurvename( "tryAqCurveGetInterpolationJoinDate", curveCollection, curveIndex ) );
            file.write( "output", joinDate );
        }

		return joinDate;

        VALID_EXCEPTION_END
    }

}