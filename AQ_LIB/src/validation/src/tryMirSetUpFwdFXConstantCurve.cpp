#include "tryMirSetUpFwdFXConstantCurve.h"

#include "LAUpdateStaticDataManager.h"
#include "CreateDataFile.h"
#include "StructuredExceptionHandler.h"
#include "CurveValidation.h"
#include "LabelValueBlockValidation.h"
#include "ScheduleValidation.h"
#include "SwapValidation.h"
#include "ParameterValidation.h"
#include "CurveResultsContainer.h"          // CurveResultsContainer - Singleton object for storage of curve results

using etrading::CreateDataFile;
using etrading::decorateCurvename;

namespace validation
{

    /* @brief			validation method for mirSetUpFwdFXConstantCurve
    *  @param [in]		dataInstance				Pointer to the object pool
    *  @param [in]		curveID				The curve collection ID
    *  @param [in]		marketName			Name of the curve constructed by this method
    *  @param [in]		generalProps		General curve properties such as asofdate, ccy, interp, etc
    *  @param [in]		fwdFXConstantConv	FX forward conventions
    *  @param [in]		curveNames			Equivalent names of the curve being built
    */
    LAString tryMirSetUpFwdFXConstantCurve( LADataInstance* dataInstance,
                                            const LAString& curveID,
                                            const LAString& marketName,
                                            const LAStringMatrix& generalProps,
                                            const LAStringMatrix& fwdFXConstantConv,
                                            const LAString& curveNames )
    {
        VALID_EXCEPTION_START
        
        // LWO Single Curves Populate Curve Results Objects that Conflict with Other Curve Types, so we must clear the Curve Results Cache
        AQ_CLEAR_CURVE_RESULTS_CACHE

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateCurvename( "tryMirSetUpFwdFXConstantCurve_inputs", curveID, marketName ) );
            file.write( "generatorFunction", "tryMirSetUpFwdFXConstantCurve" );
            file.write( "curveID", curveID );
            file.write( "marketName", marketName );
            file.write( "generalProps", generalProps );
            file.write( "fwdFXConstantConv", fwdFXConstantConv );
            file.write( "curveNames", curveNames );
        }

        if( fwdFXConstantConv.empty() )
        {
            throw LACoreInvalidData( "Input Matrix is empty", __FILE__, __LINE__ );
        }

        if( fwdFXConstantConv[0].size() < 2 )
        {
            AQ_THROW("Invalid Data: Input matix data must have column size 2")
        }

        etrading::LAUpdateStaticDataManager::setUpFwdFXConstantCurve( dataInstance, curveID, marketName, fwdFXConstantConv, generalProps, curveNames );

        //Throw exception if the curve has not been built.
        etrading::checkIfCurveExists( dataInstance, curveID );

        LAString ret( marketName + " Curve has been set" );

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateCurvename( "tryMirSetUpFwdFXConstantCurve_outputs", curveID, marketName ) );
            file.write( "output", ret );
        }

        return ret;

        VALID_EXCEPTION_END
    }

}


