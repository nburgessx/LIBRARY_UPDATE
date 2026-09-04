#include "tryAqCurvesDisplay.h"
#include "AQLCurveCalibrationHelpers.h"
#include "CreateDataFile.h"
#include "CurveValidation.h"
#include "LabelValueBlockValidation.h"
#include "ScheduleValidation.h"
#include "SwapValidation.h"
#include "ParameterValidation.h"
#include "StructuredExceptionHandler.h"
#include "AQOUtilities.h"

using etrading::CreateDataFile;
using etrading::decorateCurvename;

namespace validation
{

    /* @brief			validation interface for the aqCurvesDisplay method
    *  @param [in]		curveCollection		ID of the curve set
    *  @param [in]		curveIndex			Index of the curve set
    *  @return			Array of terms and corresponding discount factors from the yield curve
    */
    DoubleArray tryAqCurvesDisplay( const AQLString& curveCollection, const AQLString& curveIndex )

    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateCurvename( "tryAqCurvesDisplay_inputs", curveCollection, curveIndex ) );
            file.write( "generatorFunction", "tryAqCurvesDisplay" );
            file.write( "curveCollection", curveCollection );
            file.write( "curveIndex", curveIndex );
        }

        AQLString curIndex( etrading::getDefaultValueForEmptyString( curveIndex, "STD" ) );

        //Throw exception if the curve has not been built.
        etrading::getCurveStaticDataTableName( curveCollection, curveIndex );

        DoubleArray ret = etrading::AQLCurveCalibrationHelpers::outPutCurveFromYieldData( etrading::getDataInstance(), curveCollection, curIndex );

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateCurvename( "tryAqCurvesDisplay_outputs", curveCollection, curIndex ) );
            file.write( "output", ret );
        }

        return ret;

        VALID_EXCEPTION_END
    };


}




