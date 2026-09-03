/*
 * @brief			validation interface for the tryMeCurveGetInterpolationJoinDate function
 * @Created:		18 Nov 2017
 * @Author:			Joseph Ye
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#include "tryMeCurveGetInterpolationJoinDate.h"
#include "StructuredExceptionHandler.h"
#include "CurveUtilities.h"
#include "CreateDataFile.h"
#include "LADate.h"

using etrading::CreateDataFile;
using etrading::decorateCurvename;

namespace validation_api
{
    /* @brief			validation interface for the tryMeCurveGetInterpolationJoinDate function, to clear the object pool
    *  @param [in]		curveCollection		ID of the curve set
    *  @param [in]		curveIndex			Index of the curve set
	*  @param [in]		interpolation		Interpolation type
	*  @return			Interpolation join date
    */
    LADate tryMeCurveGetInterpolationJoinDate(const LAString& curveCollection, const LAString& curveIndex, const LAString& interpolation)
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateCurvename( "tryMeCurveGetInterpolationJoinDate", curveCollection, curveIndex) );
            file.write( "generatorFunction", "tryMeCurveGetInterpolationJoinDate" );
            file.write( "curveCollection", curveCollection );
            file.write( "curveIndex", curveIndex );
			file.write( "interpolation", interpolation );
        }

		LADate joinDate;
		if (interpolation.size() == 0 || LAString(interpolation) == "LINEARSPLINE")
		{
			joinDate = etrading::getLinearSplineJoinDate(curveCollection, curveIndex);
		}
		else
		{
			throw LACoreInvalidData( "#Error: Unsupported interpolation method", __FILE__, __LINE__ );
		}

		if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateCurvename( "tryMeCurveGetInterpolationJoinDate", curveCollection, curveIndex ) );
            file.write( "output", joinDate );
        }

		return joinDate;

        VALID_EXCEPTION_END
    }

}