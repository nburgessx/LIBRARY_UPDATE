#include "tryMeCurveDisplay.h"
#include "LACurveCalibrationHelpers.h"
#include "CreateDataFile.h"
#include "CurveValidation.h"
#include "LabelValueBlockValidation.h"
#include "ScheduleValidation.h"
#include "SwapValidation.h"
#include "ParameterValidation.h"
#include "StructuredExceptionHandler.h"
#include "LWOUtilities.h"

using etrading::CreateDataFile;
using etrading::decorateCurvename;

namespace validation_api
{

    /* @brief			validation interface for the meCurveDisplay method
    *  @param [in]		curveCollection		ID of the curve set
    *  @param [in]		curveIndex			Index of the curve set
    *  @return			Array of terms and corresponding discount factors from the yield curve
    */
    DoubleArray tryMeCurveDisplay( const LAString& curveCollection, const LAString& curveIndex )

    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateCurvename( "tryMeCurveDisplay_inputs", curveCollection, curveIndex ) );
            file.write( "generatorFunction", "tryMeCurveDisplay" );
            file.write( "curveCollection", curveCollection );
            file.write( "curveIndex", curveIndex );
        }

        LAString curIndex( etrading::getDefaultValueForEmptyString( curveIndex, "STD" ) );

        //Throw exception if the curve has not been built.
        etrading::getCurveStaticDataTableName( curveCollection, curveIndex );

        DoubleArray ret = etrading::LACurveCalibrationHelpers::outPutCurveFromYieldData( etrading::getDataInstance(), curveCollection, curIndex );

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateCurvename( "tryMeCurveDisplay_outputs", curveCollection, curIndex ) );
            file.write( "output", ret );
        }

        return ret;

        VALID_EXCEPTION_END
    };


}




