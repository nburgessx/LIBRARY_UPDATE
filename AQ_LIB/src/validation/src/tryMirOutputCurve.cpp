#include "tryMirOutputCurve.h"
#include "LACurveCalibrationHelpers.h"
#include "CreateDataFile.h"
#include "CurveValidation.h"
#include "LabelValueBlockValidation.h"
#include "ScheduleValidation.h"
#include "SwapValidation.h"
#include "ParameterValidation.h"
#include "StructuredExceptionHandler.h"

using etrading::CreateDataFile;
using etrading::decorateCurvename;

namespace validation_api
{

    /* @brief			validation interface for the tryMirOutputCurve method
    *  @param [in]		dataInstance			Pointer to the object pool
    *  @param [in]		curveId			ID of the curve set
    *  @param [in]		curveName		Name of the curve set
    *  @return			Array of terms and corresponding discount factors from the yield curve
    */
    DoubleArray tryMirOutputCurve( LADataInstance* dataInstance,
                                   const LAString& curveID,
                                   const LAString& curveName )

    {
        VALID_EXCEPTION_START


        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateCurvename( "tryMirOutputCurve_inputs", curveID, curveName ) );
            file.write( "generatorFunction", "tryMirOutputCurve" );
            file.write( "curveId", curveID );
            file.write( "curveName", curveName );
        }

        LAString curName( etrading::getDefaultValueForEmptyString( curveName, LAString( "STD" ) ) );

        DoubleArray ret = etrading::LACurveCalibrationHelpers::outPutCurveFromYieldData( dataInstance, curveID, curName );

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateCurvename( "tryMirOutputCurve_outputs", curveID, curveName ) );
            file.write( "output", ret );
        }

        return ret;

        VALID_EXCEPTION_END
    }

}
