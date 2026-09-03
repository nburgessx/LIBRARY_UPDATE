#include "tryMeCurveInterpolation.h"
#include "StructuredExceptionHandler.h"
#include "CurveUtilities.h"
#include "CreateDataFile.h"
#include "LADate.h"
#include "RecordMacros.h"			// Record Macros
#include "CoreEnumerations.h"		// isHybridInterpolation method

using etrading::CreateDataFile;
using etrading::decorateCurvename;

namespace validation_api
{
    /* @brief			validation interface for the tryMeCurveInterpolationJoinDate function, to clear the object pool
    *  @param [in]		curveCollection		ID of the curve set
    *  @param [in]		curveIndex			Index of the curve set
	*  @param [in]		interpolation		Interpolation type
	*  @return			Interpolation join date
    */
    LADate tryMeCurveInterpolationJoinDate( const LAString& curveCollection, const LAString& curveIndex, const LAString& interpolation )
    {
        VALID_EXCEPTION_START

		RECORD_INPUTS( curveCollection, curveIndex, interpolation )
		
		AQ_REQUIRE( etrading::isHybridInterpolation( interpolation.c_str() ), "Invalid Interpolation Method: '" + interpolation + "' is not a hybrid/mixed interpolation method" )
		
		const LADate joinDate = etrading::getLinearSplineJoinDate(curveCollection, curveIndex);

		RECORD_OUTPUTS_AND_RETURN_RESULT( joinDate )
		
        VALID_EXCEPTION_END
    }

}