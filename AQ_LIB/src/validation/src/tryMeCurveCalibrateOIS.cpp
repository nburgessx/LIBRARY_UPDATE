/*
 * @brief			validation interface for the meCurveCalibrateOIS method
 * @Created:		19 April 2016
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#include "tryMeCurveCalibrateOIS.h"
#include "LAUpdateStaticDataManager.h"
#include "CreateDataFile.h"
#include "StructuredExceptionHandler.h"
#include "CurveValidation.h"
#include "ParameterValidation.h"
#include "CurveResultsContainer.h"          // CurveResultsContainer - Singleton object for storage of curve results
#include "LWOUtilities.h"
#include "CurveUtilities.h"

using etrading::CreateDataFile;
using etrading::decorateCurvename;

namespace validation_api
{

    /* @brief			validation interface for the meCurveCalibrateOIS method
    *  @param [in]		curveCollection		The curve collection ID
    *  @param [in]		staticDataTable		Name of the curve constructed by this method
    *  @param [in]		curveIndex			Equivalent names of the curve being built
    *  @param [in]		curveConv			General curve properties such as asofdate, ccy, interp, etc
    *  @param [in]		oisConv				The OIS curve configuration info
    *  @param [in]		oisRates			Constituent OIS instrument rates
    *  @param [in]		oisHistoricalRates	Historical OIS fixings
    *  @param [in]		liborOisBasisConv	Libor-OIS swap conventions
    *  @param [in]		liborOisBasisRates	Libor-OIS basis spreads
    *  @param [in]		swapConv			Libor swap conventions
    *  @param [in]		swapRates			Libor swap market rates
    */
    const LAString tryMeCurveCalibrateOIS( const LAString& curveCollection,
                                           const LAString& staticDataTable,
                                           const LAString& curveIndexInput,
                                           const LAStringMatrix& curveConv,
                                           const LAStringMatrix& oisConv,
                                           const LAStringMatrix& oisRates,
                                           const LAStringMatrix& oisHistoricalRates,
                                           const LAStringMatrix& liborOisBasisConv,
                                           const LAStringMatrix& liborOisBasisRates,
                                           const LAStringMatrix& swapConv,
                                           const LAStringMatrix& swapRates )
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
            CreateDataFile file( decorateCurvename( "tryMeCurveCalibrateOIS_inputs", curveCollection, staticDataTable ) );
            file.write( "generatorFunction", "tryMeCurveCalibrateOIS" );
            file.write( "curveCollection", curveCollection );
            file.write( "staticDataTable", staticDataTable );
            file.write( "curveIndex", curveIndex );
            file.write( "curveConv", curveConv );
            file.write( "oisConv", oisConv );
            file.write( "oisRates", oisRates );
            file.write( "oisHistoricalRates", oisHistoricalRates );
            file.write( "liborOisBasisConv", liborOisBasisConv );
            file.write( "liborOisBasisRates", liborOisBasisRates );
            file.write( "swapConv", swapConv );
            file.write( "swapRates", swapRates );
        }

        if ( !curveConv.empty() && curveConv[0].size() < 2 )
        {
            MLIB_THROW("Invalid Data: Input matix data must have column size 2")
        }

        if ( !oisConv.empty() && oisConv[0].size() < 2 )
        {
            MLIB_THROW("Invalid Data: Input matix data must have column size 2")
        }

        if ( !oisRates.empty() && oisRates[0].size() < 2 )
        {
            MLIB_THROW("Invalid Data: Input matix data must have column size 2")
        }

		auto fixingTableDateRates = etrading::retrieveFixingTableMatrix(oisHistoricalRates);

        etrading::LAUpdateStaticDataManager::setUpOISCurve( etrading::getDataInstance(),
                                                            curveCollection,
                                                            staticDataTable,
                                                            curveConv,
                                                            oisRates,
                                                            oisConv,
                                                            curveIndex,
															fixingTableDateRates,
                                                            liborOisBasisRates,
                                                            liborOisBasisConv,
                                                            swapRates,
                                                            swapConv );

        // note that curveIndexCopy is actually the staticDataTable ...
        LAString curveIndexCopy( etrading::getDefaultValueForEmptyString( staticDataTable, "OIS" ) );
        //Throw exception if the curve has not been built.
        etrading::getCurveStaticDataTableName( curveCollection, curveIndexCopy );

        LAString ret( curveCollection + " " + staticDataTable + " Curve has been set" );

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateCurvename( "tryMeCurveCalibrateOIS_outputs", curveCollection, staticDataTable ) );
            file.write( "output", ret );
        }

        return ret.getCString();

        VALID_EXCEPTION_END

    };

}
