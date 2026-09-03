#include "tryMeCurveFrequency.h"
#include "CreateDataFile.h"
#include "StructuredExceptionHandler.h"
#include "CurveValidation.h"
#include "LabelValueBlockValidation.h"
#include "ScheduleValidation.h"
#include "SwapValidation.h"
#include "ParameterValidation.h"

using etrading::CreateDataFile;
using etrading::decorateCurvename;

namespace validation
{

    /* @brief			Returns the curveFrequency i.e. the float index frequency
     *  @param [in]		curveCollection		The curve collection ID
     *  @param [in]		curveIndex			Equivalent names of the curve being built
     *  @param [out]     curveFrequency      The curve frequency i.e the float index frequency
     */
    const AQLString tryMeCurveFrequency( const AQLString& curveCollection,
                                        const AQLString& curveIndex )
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( "tryMeCurveFrequency_inputs" );
            file.write( "generatorFunction", "tryMeCurveFrequency" );
            file.write( "curveCollection", curveCollection );
            file.write( "curveIndex", curveIndex );
        }

        AQLString curveFrequency = etrading::validateCurveAndGetCurveFrequency( curveCollection, curveIndex );

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( "tryMeCurveFrequency_outputs" );
            file.write( "output", curveFrequency );
        }

        return curveFrequency.getCString();

        VALID_EXCEPTION_END
    }

}
