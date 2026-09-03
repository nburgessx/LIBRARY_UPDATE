#include "tryMirSetUpBasisSwapCurve.h"

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

namespace validation_api
{

    /* @brief			validation interface for the SetUpBasisSwapCurve method
    *  @param [in]		dataInstance			Pointer to the object pool
    *  @param [in]		curveID			The curve collection ID
    *  @param [in]		marketName		Name of the curve constructed by this method
    *  @param [in]		generalProps	General curve properties such as asofdate, ccy, interp, etc
    *  @param [in]		basisConv		Basis swap conventions
    *  @param [in]		basisRates		Basis swap market rates
    *  @param [in]		curveNames		Equivalent names of the curve being built
    *  @param [in]		fwdConv			FX forward market convention
    *  @param [in]		fwdFXs			Forward FX rates
    *  @param [in]		spotFXs			Spot FX rates
	*  @param [in]		fraConv			FRA Conventions
	*  @param [in]		fraRates		FRA market quotes
    */
    const LAString tryMirSetUpBasisSwapCurve( LADataInstance* dataInstance,
                                              const LAString& curveID,
                                              const LAString& marketName,
                                              const LAStringMatrix& generalProps,
                                              const LAStringMatrix& basisConv,
                                              const LAStringMatrix& basisRates,
                                              const LAString& curveNames,
                                              const LAStringMatrix& fwdConv,
                                              const LAStringMatrix& fwdFXs,
                                              const LAStringMatrix& spotFXs,
			                                  const LAStringMatrix& fraConv,
			                                  const LAStringMatrix& fraRates)
    {
        VALID_EXCEPTION_START
        
        // LWO Single Curves Populate Curve Results Objects that Conflict with Other Curve Types, so we must clear the Curve Results Cache
        AQ_CLEAR_CURVE_RESULTS_CACHE

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateCurvename( "tryMirSetUpBasisSwapCurve_inputs", curveID, marketName ) );
            file.write( "generatorFunction", "tryMirSetUpBasisSwapCurve" );
            file.write( "curveID", curveID );
            file.write( "marketName", marketName );
            file.write( "generalProps", generalProps );
            file.write( "basisConv", basisConv );
            file.write( "basisRates", basisRates );
            file.write( "curveNames", curveNames );
            file.write( "fwdConv", fwdConv );
            file.write( "fwdFXs", fwdFXs );
            file.write( "spotFXs", spotFXs );
			file.write( "fraConv", fraConv );
			file.write( "fraRates", fraRates );
        }

        if( ( fwdFXs.empty() && basisRates.empty() ) || basisConv.empty() || generalProps.empty() )
        {
            throw LACoreInvalidData( "Input Matrix 'fwdFXs' is empty", __FILE__, __LINE__ );
        }

        if( ( fwdFXs.empty() && 2 > basisRates[0].size() ) || 2 > basisConv[0].size() || 2 > generalProps[0].size() )
        {
            throw LACoreInvalidData( "Matrix column size for 'fwdFXs' must be 2", __FILE__, __LINE__ );
        }

        if ( !fwdFXs.empty() && fwdFXs[0].size() < 2 )
        {
            throw LACoreInvalidData( "Matrix column size for 'fwdFXs' must be 2", __FILE__, __LINE__ );
        }

        if ( !fwdConv.empty() && fwdConv[0].size() < 2 )
        {
            throw LACoreInvalidData( "Matrix column size for 'fwdConv' must be 2", __FILE__, __LINE__ );
        }

        if ( !spotFXs.empty() && spotFXs[0].size() < 3 )
        {
            throw LACoreInvalidData( "Matrix column size for 'spotFXs' must be 3", __FILE__, __LINE__ );
        }

		if ( !fraConv.empty() && fraConv[0].size() < 2 )
        {
            throw LACoreInvalidData( "Matrix column size for 'fraConv' must be at least 2", __FILE__, __LINE__ );
        }

		if ( !fraRates.empty() && fraRates[0].size() < 2 )
        {
            throw LACoreInvalidData( "Matrix column size for 'fraRates' must be at least 2", __FILE__, __LINE__ );
        }

        LAStringMatrix moneyConv = LAStringMatrix( 0 );

        etrading::LAUpdateStaticDataManager::setUpBasisCurve( dataInstance,
                                                          curveID,
                                                          marketName,
                                                          basisRates,
                                                          basisConv,
                                                          fwdFXs,
                                                          fwdConv,
                                                          spotFXs,
                                                          generalProps,
                                                          moneyConv,
                                                          curveNames,
										                  fraConv,
										                  fraRates);


        //Throw exception if the curve has not been built.
        etrading::checkIfCurveExists( dataInstance, curveID );

        /*
        	********** We display a message indicating the success of the curve build.
        	********** The long term and more appropriate plan is to display an object handle rather than simply a message
        */
        LAString ret( marketName + " Curve has been set" );

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateCurvename( "tryMirSetUpBasisSwapCurve_outputs", curveID, marketName ) );
            file.write( "output", ret );
        }

        return ret.getCString();

        VALID_EXCEPTION_END
    }

}
