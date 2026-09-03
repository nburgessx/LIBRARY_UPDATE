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

    /* @brief			swig interface for mirSetUpOISCurve
    *  @param [in]		dataInstance				Pointer to the object pool
    *  @param [in]		curveID				The curve collection ID
    *  @param [in]		marketName			Name of the curve constructed by this method
    *  @param [in]		generalProps		General yield curve data
    *  @param [in]		moneyConv			Money market conventions
    *  @param [in]		liborRates			Libor market data
    *  @param [in]		liborConv			Libor market conventions
    *  @param [in]		swapConv			Swap market conventions
    *  @param [in]		swapConv			Swap market data
    *  @param [in]		FRAData				FRA market conventions
    *  @param [in]		fra3mRates			3M FRA market data
    *  @param [in]		fra6mRates			6M FRA market data
    *  @param [in]		futureRates			Futures market data
    *  @param [in]		futureConv			Futures market conventions
    *  @param [in]		adjustSwapConv		Basis swaps market conventions
    *  @param [in]		adjustSwapRates		Basis swaps market data
    *  @param [in]		curveNames			Equivalent names of the curve being built
    *  @param [in]		curveName_DF2		Trade discount curve
    */
    const AQLString tryMirSetUpSwapCurve( AQLDataInstance* dataInstance,
                                         const AQLString& curveID,
                                         const AQLString& marketName,
                                         const AQLStringMatrix& generateProp,
                                         const AQLStringMatrix& moneyConv,
                                         const AQLStringMatrix& liborRates,
                                         const AQLStringMatrix& liborConv,
                                         const AQLStringMatrix& swapRates,
                                         const AQLStringMatrix& swapConv,
                                         const AQLStringMatrix& fra3mRates,
                                         const AQLStringMatrix& fra6mRates,
                                         const AQLStringMatrix& fraConv,
                                         const AQLStringMatrix& futureRates,
                                         const AQLStringMatrix& futureConv,
                                         const AQLStringMatrix& adjustSwapConv,
                                         const AQLStringMatrix& adjustSwapRates,
                                         const AQLString& curveNames,
                                         const AQLString& curveName_DF2 )
    {
        VALID_EXCEPTION_START
        
        // LWO Single Curves Populate Curve Results Objects that Conflict with Other Curve Types, so we must clear the Curve Results Cache
        AQ_CLEAR_CURVE_RESULTS_CACHE

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateCurvename( "tryMirSetUpSwapCurve_inputs", curveID, marketName ) );
            file.write( "generatorFunction", "tryMirSetUpSwapCurve" );
            file.write( "curveID", curveID );
            file.write( "marketName", marketName );
            file.write( "generalProps", generateProp );
            file.write( "moneyConv", moneyConv );
            file.write( "liborRates", liborRates );
            file.write( "liborConv", liborConv );
            file.write( "swapRates", swapRates );
            file.write( "swapConv", swapConv );
            file.write( "fra3mRates", fra3mRates );
            file.write( "fra6mRates", fra6mRates );
            file.write( "fraConv", fraConv );
            file.write( "futureRates", futureRates );
            file.write( "futureConv", futureConv );
            file.write( "adjustSwapConv", adjustSwapConv );
            file.write( "adjustSwapRates", adjustSwapRates );
            file.write( "curveNames", curveNames );
            file.write( "curveName_DF2", curveName_DF2 );
        }

        if ( !generateProp.empty() && generateProp[0].size() < 2 )
        {
            AQ_THROW("Invalid Data: Input matix data must have column size 2")
        }

        if ( !moneyConv.empty() && moneyConv[0].size() < 2 )
        {
            AQ_THROW("Invalid Data: Input matix data must have column size 2")
        }

        if ( !liborConv.empty() && liborConv[0].size() < 2 )
        {
            AQ_THROW("Invalid Data: Input matix data must have column size 2")
        }

        if ( !liborRates.empty() && liborRates[0].size() < 2 )
        {
            AQ_THROW("Invalid Data: Input matix data must have column size 2")
        }

        if ( !swapConv.empty() && swapConv[0].size() < 2 )
        {
            AQ_THROW("Invalid Data: Input matix data must have column size 2")
        }

        if ( !swapRates.empty() && swapRates[0].size() < 2 )
        {
            AQ_THROW("Invalid Data: Input matix data must have column size 2")
        }

        if ( !fraConv.empty() && fraConv[0].size() < 2 )
        {
            AQ_THROW("Invalid Data: Input matix data must have column size 2")
        }

        if ( !fra3mRates.empty() && fra3mRates[0].size() < 2 )
        {
            AQ_THROW("Invalid Data: Input matix data must have column size 2")
        }

        if ( !fra6mRates.empty() && fra6mRates[0].size() < 2 )
        {
            AQ_THROW("Invalid Data: Input matix data must have column size 2")
        }

        if ( !futureConv.empty() && futureConv[0].size() < 2 )
        {
            AQ_THROW("Invalid Data: Input matix data must have column size 2")
        }

        if ( !futureRates.empty() && futureRates[0].size() < 3 )
        {
            throw AQLCoreInvalidData( "Matrix column size must be greater or equal to 3", __FILE__, __LINE__ );
        }

        if ( !adjustSwapConv.empty() && adjustSwapConv[0].size() < 2 )
        {
            AQ_THROW("Invalid Data: Input matix data must have column size 2")
        }

        if ( !adjustSwapRates.empty() && adjustSwapRates[0].size() < 2 )
        {
            AQ_THROW("Invalid Data: Input matix data must have column size 2")
        }

        etrading::LAUpdateStaticDataManager::setUpSwapCurve( dataInstance,
                                                         curveID,
                                                         marketName,
                                                         generateProp,
                                                         moneyConv,
                                                         liborRates,
                                                         liborConv,
                                                         swapRates,
                                                         swapConv,
                                                         fra3mRates,
                                                         fra6mRates,
                                                         fraConv,
                                                         futureRates,
                                                         futureConv,
                                                         adjustSwapConv,
                                                         adjustSwapRates,
                                                         curveNames,
                                                         curveName_DF2 );


        //Throw exception if the curve has not been built.
        etrading::checkIfCurveExists( dataInstance, curveID );

        /*
        	********** We display a message indicating the success of the curve build.
        	********** The long term and more appropriate plan is to display an object handle rather than simply a message
        */
        AQLString ret( marketName + " Curve has been set" );

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateCurvename( "tryMirSetUpSwapCurve_outputs", curveID, marketName ) );
            file.write( "output", ret );
        }

        return ret.getCString();

        VALID_EXCEPTION_END
    }

}
