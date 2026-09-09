#include "tryAqCurveCalibrateOIS.h"
#include "AQLUpdateStaticDataManager.h"
#include "CreateDataFile.h"
#include "StructuredExceptionHandler.h"
#include "CurveValidation.h"
#include "ParameterValidation.h"
#include "CurveResultsContainer.h"          // CurveResultsContainer - Singleton object for storage of curve results
#include "AQObjUtilities.h"
#include "CurveUtilities.h"

using etrading::CreateDataFile;
using etrading::decorateCurvename;

namespace validation
{

    /* @brief			validation interface for the aqCurveCalibrateOIS method
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
    const AQLString tryAqCurveCalibrateOIS( const AQLString& curveCollection,
                                           const AQLString& staticDataTable,
                                           const AQLString& curveIndexInput,
                                           const AQLStringMatrix& curveConv,
                                           const AQLStringMatrix& oisConv,
                                           const AQLStringMatrix& oisRates,
                                           const AQLStringMatrix& oisHistoricalRates,
                                           const AQLStringMatrix& liborOisBasisConv,
                                           const AQLStringMatrix& liborOisBasisRates,
                                           const AQLStringMatrix& swapConv,
                                           const AQLStringMatrix& swapRates )
    {
        VALID_EXCEPTION_START
        
        // AQObj Single Curves Populate Curve Results Objects that Conflict with Other Curve Types, so we must clear the Curve Results Cache
        AQ_CLEAR_CURVE_RESULTS_CACHE

        // Prefix the staticDataTable to the curveIndex Name Set, ensuring to use the ':' delimiter
        std::string curveIndexStdStr =  curveIndexInput.getCString();
        curveIndexStdStr = etrading::addPrefixStringAndCheckForDuplicates( curveIndexStdStr, std::string(staticDataTable.getCString()) );
        AQLString curveIndex  = curveIndexStdStr.c_str();

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateCurvename( "tryAqCurveCalibrateOIS_inputs", curveCollection, staticDataTable ) );
            file.write( "generatorFunction", "tryAqCurveCalibrateOIS" );
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

		auto fixingTableDateRates = etrading::retrieveFixingTableMatrix(oisHistoricalRates);

        etrading::AQLUpdateStaticDataManager::setUpOISCurve( etrading::getDataInstance(),
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
        AQLString curveIndexCopy( etrading::getDefaultValueForEmptyString( staticDataTable, "OIS" ) );
        //Throw exception if the curve has not been built.
        etrading::getCurveStaticDataTableName( curveCollection, curveIndexCopy );

        AQLString ret( curveCollection + " " + staticDataTable + " Curve has been set" );

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateCurvename( "tryAqCurveCalibrateOIS_outputs", curveCollection, staticDataTable ) );
            file.write( "output", ret );
        }

        return ret.getCString();

        VALID_EXCEPTION_END

    };

}
