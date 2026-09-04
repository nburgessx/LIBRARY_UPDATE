#include "tryAqCurvesGetInterpolationJoinDate.h"
#include "StructuredExceptionHandler.h"
#include "CurveUtilities.h"
#include "CreateDataFile.h"
#include "AQLDate.h"

using etrading::CreateDataFile;
using etrading::decorateCurvename;

namespace validation
{
    /* @brief			validation interface for the tryAqCurvesGetInterpolationJoinDate function, to clear the object pool
    *  @param [in]		curveCollection		ID of the curve set
    *  @param [in]		curveIndex			Index of the curve set
	*  @param [in]		interpolation		Interpolation type
	*  @return			Interpolation join date
    */
    AQLDate tryAqCurvesGetInterpolationJoinDate(const AQLString& curveCollection, const AQLString& curveIndex, const AQLString& interpolation)
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateCurvename( "tryAqCurvesGetInterpolationJoinDate", curveCollection, curveIndex) );
            file.write( "generatorFunction", "tryAqCurvesGetInterpolationJoinDate" );
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
			throw AQLCoreInvalidData( "#Error: Unsupported interpolation method", __FILE__, __LINE__ );
		}

		if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateCurvename( "tryAqCurvesGetInterpolationJoinDate", curveCollection, curveIndex ) );
            file.write( "output", joinDate );
        }

		return joinDate;

        VALID_EXCEPTION_END
    }

}