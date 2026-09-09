#include "tryAqCurveDelete.h"
#include "CreateDataFile.h"
#include "ParameterValidation.h"
#include "StructuredExceptionHandler.h"
#include "EntityPoolUtilities.h"

using etrading::CreateDataFile;
using etrading::decorateCurvename;

namespace validation
{

    /* @brief			validation interface for tryAqCurveDelete
    *  @param [in]		curveCollection		Name of curve collection
    *  @param [in]		curveIndex			Name of the curve to be removed from object pool
    *  @return	A message showing the output of the action
    */
	AQLString tryAqCurveDelete(const AQLString& curveCollection, const AQLString& curveIndex)
	{
		VALID_EXCEPTION_START

		// Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateCurvename( "tryAqCurveDelete", curveCollection, curveIndex ) );
            file.write( "curveCollection",	curveCollection );
            file.write( "curveIndex",		curveIndex );
        }

		AQLString ret = etrading::removeCurveFromEntityPool(curveCollection, curveIndex);
		return ret;

		VALID_EXCEPTION_END
	}


}




