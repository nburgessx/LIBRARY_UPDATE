/*
 * @brief			validation interface for the meCurveCalibrateBasis method
 * @Created:		19 April 2016
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#include <memory>

#include "tryMeCurveCalibrateBasis.h"

#include "LAUpdateStaticDataManager.h"
#include "CreateDataFile.h"
#include "StructuredExceptionHandler.h"
#include "CurveValidation.h"
#include "ParameterValidation.h"
#include "CurveResultsContainer.h"          // CurveResultsContainer - Singleton object for storage of curve results

using etrading::CreateDataFile;
using etrading::decorateCurvename;

namespace validation_api
{
	/* @brief			validation interface for the meCurveCalibrateBasis method
    *  @param [in]		curveCollection		The curve collection ID
    *  @param [in]		staticDataTable		Name of the curve constructed by this method
    *  @param [in]		curveIndex			Equivalent names of the curve being built
    *  @param [in]		curveConv			General curve properties such as asofdate, ccy, interp, etc
    *  @param [in]		basisConv			Basis swap conventions
    *  @param [in]		basisRates			Basis swap market rates
    *  @param [in]		fxFwdConv			FX forward market convention
    *  @param [in]		fxFwdRates			Forward FX rates
    *  @param [in]		spotFxRates			Spot FX rates
	*  @param [in]		fraConv				FRA conventions
	*  @param [in]		fraRates			FRA market data
	*  @param [in]		liborConv			Libor instrument conventions
	*  @param [in]		liborRates			Libor market data
    */
    const LAString tryMeCurveCalibrateBasis( const LAString& curveCollection,
											 const LAString& staticDataTable,
											 const LAString& curveIndexInput,
											 const LAStringMatrix& curveConv,
											 const LAStringMatrix& basisConv,
											 const LAStringMatrix& basisRates,
											 const LAStringMatrix& fxFwdConv,
											 const LAStringMatrix& fxFwdRates,
											 const LAStringMatrix& spotFxRates,
											 const LAStringMatrix& fraConv,
											 const LAStringMatrix& fraRates,
											 const LAStringMatrix& liborConv,
											 const LAStringMatrix& liborRates)
    {
        VALID_EXCEPTION_START

        // LWO Single Curves Populate Curve Results Objects that Conflict with Other Curve Types, so we must clear the Curve Results Cache
        MLIB_CLEAR_CURVE_RESULTS_CACHE

        // Prefix the staticDataTable to the curveIndex Name Set, ensuring to use the ':' delimiter
        std::string curveIndexStdStr =  curveIndexInput.getCString();
        curveIndexStdStr = etrading::addPrefixStringAndCheckForDuplicates( curveIndexStdStr, std::string(staticDataTable.getCString()) );
        LAString curveIndex  = curveIndexStdStr.c_str();

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateCurvename( "tryMeCurveCalibrateBasis_inputs", curveCollection, staticDataTable ) );
            file.write( "generatorFunction", "tryMeCurveCalibrateBasis" );
            file.write( "curveCollection", curveCollection );
            file.write( "staticDataTable", staticDataTable );
            file.write( "curveIndex", curveIndex );
            file.write( "curveConv", curveConv );
            file.write( "basisConv", basisConv );
            file.write( "basisRates", basisRates );
            file.write( "fxFwdConv", fxFwdConv );
            file.write( "fxFwdRates", fxFwdRates );
            file.write( "spotFxRates", spotFxRates );
			file.write( "fraConv", fraConv );
			file.write( "fraRates", fraRates );
			file.write( "liborConv", liborConv );
			file.write( "liborRates", liborRates );
        }

        if( ( fxFwdRates.empty() && basisRates.empty() ) || basisConv.empty() || curveConv.empty() )
        {
            throw LACoreInvalidData( "Input Matrix is empty", __FILE__, __LINE__ );
            MLIB_THROW("Invalid Data: Input data is missing")
        }

        if( ( fxFwdRates.empty() && 2 > basisRates[0].size() ) || 2 > basisConv[0].size() || 2 > curveConv[0].size() )
        {
            MLIB_THROW("Invalid Data: Input matix data must have column size 2")
        }

        if ( !fxFwdRates.empty() && fxFwdRates[0].size() < 2 )
        {
            MLIB_THROW("Invalid Data: Input matix data must have column size 2")
        }

        if ( !fxFwdConv.empty() && fxFwdConv[0].size() < 2 )
        {
            MLIB_THROW("Invalid Data: Input matix data must have column size 2")
        }

        if ( !spotFxRates.empty() && spotFxRates[0].size() < 3 )
        {
            MLIB_THROW("Invalid Data: Input matix data must have column size 3")
        }

        LAStringMatrix moneyConv = LAStringMatrix( 0 );

        etrading::LAUpdateStaticDataManager::setUpBasisCurve( etrading::getDataInstance(),
                                                              curveCollection,
                                                              staticDataTable,
                                                              basisRates,
                                                              basisConv,
                                                              fxFwdRates,
                                                              fxFwdConv,
                                                              spotFxRates,
                                                              curveConv,
                                                              moneyConv,
                                                              curveIndex,
										                      fraConv,
										                      fraRates,
										                      liborConv,
										                      liborRates );

        // note that curveIndexCopy is actually the staticDataTable ...
        LAString curveIndexCopy( etrading::getDefaultValueForEmptyString( staticDataTable, "3M6MBasis" ) );
        //Throw exception if the curve has not been built.
        etrading::getCurveStaticDataTableName( curveCollection, curveIndexCopy );
        /*
        	********** We display a message indicating the success of the curve build.
        	********** The long term and more appropriate plan is to display an object handle rather than simply a message
        */

        LAString ret( curveCollection + " " + staticDataTable + " Curve has been set" );

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateCurvename( "tryMeCurveCalibrateBasis_outputs", curveCollection, staticDataTable ) );
            file.write( "output", ret );
        }

        return ret.getCString();

        VALID_EXCEPTION_END
    };

}
