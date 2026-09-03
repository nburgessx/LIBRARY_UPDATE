#include "tryMirSetUpOISCurve.h"

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

    /* @brief			validation interface for the SetUpOISCurve method
    *  @param [in]		dataInstance			Pointer to the object pool
    *  @param [in]		curveID			The curve collection ID
    *  @param [in]		marketName		Name of the curve constructed by this method
    *  @param [in]		generalProps	General curve properties such as asofdate, ccy, interp, etc
    *  @param [in]		oisConv			The OIS curve configuration info
    *  @param [in]		oisRates		Constituent OIS instrument rates
    *  @param [in]		curveNames		Equivalent names of the curve being built
    *  @param [in]		oisHistRates	Historical OIS fixings
    *  @param [in]		loBasisRates	Libor-OIS basis spreads
    *  @param [in]		loBasisConv		Libor-OIS swap conventions
    *  @param [in]		swapRates		Libor swap market rates
    *  @param [in]		swapConv		Libor swap conventions
    */
    const LAString tryMirSetUpOISCurve( LADataInstance* dataInstance,
                                        const LAString& curveID,
                                        const LAString& marketName,
                                        const LAStringMatrix& generalProps,
                                        const LAStringMatrix& oisRates,
                                        const LAStringMatrix& oisConv,
                                        const LAString& curveNames,
                                        const LAStringMatrix& oisHistRates,
                                        const LAStringMatrix& loBasisRates,
                                        const LAStringMatrix& loBasisConv,
                                        const LAStringMatrix& swapRates,
                                        const LAStringMatrix& swapConv )
    {
        VALID_EXCEPTION_START
        
        // LWO Single Curves Populate Curve Results Objects that Conflict with Other Curve Types, so we must clear the Curve Results Cache
        AQ_CLEAR_CURVE_RESULTS_CACHE

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateCurvename( "tryMirSetUpOISCurve_inputs", curveID, marketName ) );
            file.write( "generatorFunction", "tryMirSetUpOISCurve" );
            file.write( "curveID", curveID );
            file.write( "marketName", marketName );
            file.write( "generalProps", generalProps );
            file.write( "oisRates", oisRates );
            file.write( "oisConv", oisConv );
            file.write( "curveNames", curveNames );
            file.write( "oisHistRates", oisHistRates );
            file.write( "loBasisRates", loBasisRates );
            file.write( "loBasisConv", loBasisConv );
            file.write( "swapRates", swapRates );
            file.write( "swapConv", swapConv );
        }

        if ( !generalProps.empty() && generalProps[0].size() < 2 )
        {
            AQ_THROW("Invalid Data: Input matix data must have column size 2")
        }

        if ( !oisConv.empty() && oisConv[0].size() < 2 )
        {
            AQ_THROW("Invalid Data: Input matix data must have column size 2")
        }

        if ( !oisRates.empty() && oisRates[0].size() < 2 )
        {
            AQ_THROW("Invalid Data: Input matix data must have column size 2")
        }

        etrading::LAUpdateStaticDataManager::setUpOISCurve( dataInstance,
                                                        curveID,
                                                        marketName,
                                                        generalProps,
                                                        oisRates,
                                                        oisConv,
                                                        curveNames,
                                                        oisHistRates,
                                                        loBasisRates,
                                                        loBasisConv,
                                                        swapRates,
                                                        swapConv );

        //Throw exception if the curve has not been built.
        etrading::checkIfCurveExists( dataInstance, curveID );

        /*
        	********** We display a message indicating the success of the curve build.
        	********** The long term and more appropriate plan is to display an object handle rather than simply a message
        */
        LAString ret( marketName + " Curve has been set" );

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateCurvename( "tryMirSetUpOISCurve_outputs", curveID, marketName ) );
            file.write( "output", ret );
        }

        return ret.getCString();

        VALID_EXCEPTION_END
    }

}
