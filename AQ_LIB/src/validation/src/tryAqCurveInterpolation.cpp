#include "tryAqCurveInterpolation.h"
#include "StructuredExceptionHandler.h"
#include "CurveUtilities.h"
#include "CreateDataFile.h"
#include "AQLDate.h"
#include "RecordMacros.h"			// Record Macros
#include "CoreEnumerations.h"		// isHybridInterpolation method

using etrading::CreateDataFile;
using etrading::decorateCurvename;

namespace validation
{
    /* @brief			validation interface for the tryAqCurveInterpolationJoinDate function, to clear the object pool
    *  @param [in]		curveCollection		ID of the curve set
    *  @param [in]		curveIndex			Index of the curve set
	*  @param [in]		interpolation		Interpolation type
	*  @return			Interpolation join date
    */
    AQLDate tryAqCurveInterpolationJoinDate( const AQLString& curveCollection, const AQLString& curveIndex, const AQLString& interpolation )
    {
        VALID_EXCEPTION_START

		AQ_RECORD_INPUTS( curveCollection, curveIndex, interpolation )
		
		AQ_REQUIRE( etrading::isHybridInterpolation( interpolation.c_str() ), "Invalid Interpolation Method: '" + interpolation + "' is not a hybrid/mixed interpolation method" )
		
		const AQLDate joinDate = etrading::getLinearSplineJoinDate(curveCollection, curveIndex);

		AQ_RECORD_OUTPUTS_AND_RETURN_RESULT( joinDate )
		
        VALID_EXCEPTION_END
    }

}