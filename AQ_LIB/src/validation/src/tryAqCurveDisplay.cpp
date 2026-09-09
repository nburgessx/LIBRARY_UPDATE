#include "tryAqCurveDisplay.h"
#include "AQLCurveCalibrationHelpers.h"
#include "CreateDataFile.h"
#include "CurveValidation.h"
#include "LabelValueBlockValidation.h"
#include "ScheduleValidation.h"
#include "SwapValidation.h"
#include "ParameterValidation.h"
#include "StructuredExceptionHandler.h"
#include "AQObjUtilities.h"

using etrading::CreateDataFile;
using etrading::decorateCurvename;

namespace validation
{

    /* @brief			validation interface for the aqCurveDisplay method
    *  @param [in]		curveCollection		ID of the curve set
    *  @param [in]		curveIndex			Index of the curve set
    *  @return			Array of terms and corresponding discount factors from the yield curve
    */
    DoubleArray tryAqCurveDisplay( const AQLString& curveCollection, const AQLString& curveIndex )

    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateCurvename( "tryAqCurveDisplay_inputs", curveCollection, curveIndex ) );
            file.write( "generatorFunction", "tryAqCurveDisplay" );
            file.write( "curveCollection", curveCollection );
            file.write( "curveIndex", curveIndex );
        }

        AQLString curIndex( etrading::getDefaultValueForEmptyString( curveIndex, "STD" ) );

        //Throw exception if the curve has not been built.
        etrading::getCurveStaticDataTableName( curveCollection, curveIndex );

        DoubleArray ret = etrading::AQLCurveCalibrationHelpers::outPutCurveFromYieldData( etrading::getDataInstance(), curveCollection, curIndex );

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateCurvename( "tryAqCurveDisplay_outputs", curveCollection, curIndex ) );
            file.write( "output", ret );
        }

        return ret;

        VALID_EXCEPTION_END
    };


}




