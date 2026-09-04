#include "tryMeCurveCalibrateSwap.h"
#include "AQLUpdateStaticDataManager.h"
#include "CreateDataFile.h"
#include "StructuredExceptionHandler.h"
#include "CurveValidation.h"
#include "ParameterValidation.h"
#include "CurveResultsContainer.h"          // CurveResultsContainer - Singleton object for storage of curve results

using etrading::CreateDataFile;
using etrading::decorateCurvename;

namespace validation
{

    /* @brief			validation interface for meCurveCalibrateSwap
    *  @param [in]		curveCollection		The curve collection ID
    *  @param [in]		staticDataTable		Name of the curve constructed by this method
    *  @param [in]		curveIndex			Equivalent names of the curve being built
    *  @param [in]		curveConv			General yield curve data
    *  @param [in]		moneyMarketConv			Money market conventions
    *  @param [in]		liborConv			Libor market conventions
    *  @param [in]		liborRates			Libor market data
    *  @param [in]		swapConv			Swap market conventions
    *  @param [in]		swapRates			Swap market data
    *  @param [in]		fraConv				FRA market conventions
    *  @param [in]		fra3mRates			3M FRA market data
    *  @param [in]		fra6mRates			6M FRA market data
    *  @param [in]		futureConv			Futures market conventions
    *  @param [in]		futureRates			Futures market data
    *  @param [in]		tenorBasisConv	    Tenor Basis market conventions
    *  @param [in]		tenorBasisRates 	Tenor Basis market data
    */
    const AQLString tryMeCurveCalibrateSwap( const AQLString& curveCollection,
                                            const AQLString& staticDataTable,
                                            const AQLString& curveIndexInput,
                                            const AQLStringMatrix& curveConv,
                                            const AQLStringMatrix& moneyMarketConv,
                                            const AQLStringMatrix& liborConv,
                                            const AQLStringMatrix& liborRates,
                                            const AQLStringMatrix& swapConv,
                                            const AQLStringMatrix& swapRates,
                                            const AQLStringMatrix& fraConv,
                                            const AQLStringMatrix& fra3mRates,
                                            const AQLStringMatrix& fra6mRates,
                                            const AQLStringMatrix& futureConv,
                                            const AQLStringMatrix& futureRates,
                                            const AQLStringMatrix& tenorBasisConv,
                                            const AQLStringMatrix& tenorBasisRates )
    {
        VALID_EXCEPTION_START
        
        // LWO Single Curves Populate Curve Results Objects that Conflict with Other Curve Types, so we must clear the Curve Results Cache
        AQ_CLEAR_CURVE_RESULTS_CACHE

        // Prefix the staticDataTable onto the curveIndex Name Set, using the ':' delimiter by default
        std::string curveIndexStdStr =  curveIndexInput.getCString();
        curveIndexStdStr = etrading::addPrefixStringAndCheckForDuplicates( curveIndexStdStr, std::string(staticDataTable.getCString()) );
        AQLString curveIndex  = curveIndexStdStr.c_str();

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateCurvename( "tryMeCurveCalibrateSwap_inputs", curveCollection, staticDataTable ) );
            file.write( "generatorFunction", "tryMeCurveCalibrateSwap" );
            file.write( "curveCollection", curveCollection );
            file.write( "staticDataTable", staticDataTable );
            file.write( "curveIndex", curveIndex );
            file.write( "curveConv", curveConv );
            file.write( "moneyMarketConv", moneyMarketConv );
            file.write( "liborConv", liborConv );
            file.write( "liborRates", liborRates );
            file.write( "swapConv", swapConv );
            file.write( "swapRates", swapRates );
            file.write( "fraConv", fraConv );
            file.write( "fra3mRates", fra3mRates );
            file.write( "fra6mRates", fra6mRates );
            file.write( "futureConv", futureConv );
            file.write( "futureRates", futureRates );
            file.write( "convexityAdjConv", tenorBasisConv );       // Note convexity adj was the INCORRECT previous name for this tenor basis parameter
            file.write( "convexityAdjRates", tenorBasisRates );     // Note convexity adj was the INCORRECT previous name for this tenor basis parameter
        }

        if ( !curveConv.empty() && curveConv[0].size() < 2 )
        {
            throw AQLCoreInvalidData( "#Error: Curve Conventions column size must be 2", __FILE__, __LINE__ );
        }

        if ( !moneyMarketConv.empty() && moneyMarketConv[0].size() < 2 )
        {
            throw AQLCoreInvalidData( "#Error: Money Market Conventions column size must be 2", __FILE__, __LINE__ );
        }

        if ( !liborConv.empty() && liborConv[0].size() < 2 )
        {
            throw AQLCoreInvalidData( "#Error: Libor Conventions column size must be 2", __FILE__, __LINE__ );
        }

        if ( !liborRates.empty() && liborRates[0].size() < 2 )
        {
            throw AQLCoreInvalidData( "#Error: Libor Rates column size must be 2", __FILE__, __LINE__ );
        }

        if ( !swapConv.empty() && swapConv[0].size() < 2 )
        {
            throw AQLCoreInvalidData( "#Error: Swap Conventions column size must be 2", __FILE__, __LINE__ );
        }

        if ( !swapRates.empty() && swapRates[0].size() < 2 )
        {
            throw AQLCoreInvalidData( "#Error: Swap Rates column size must be 2", __FILE__, __LINE__ );
        }

        if ( !fraConv.empty() && fraConv[0].size() < 2 )
        {
            throw AQLCoreInvalidData( "#Error: FRA Conventions column size must be 2", __FILE__, __LINE__ );
        }

        if ( !fra3mRates.empty() && fra3mRates[0].size() < 2 )
        {
            throw AQLCoreInvalidData( "#Error: FRA 3M Rates column size must be 2", __FILE__, __LINE__ );
        }

        if ( !fra6mRates.empty() && fra6mRates[0].size() < 2 )
        {
            throw AQLCoreInvalidData( "#Error: FRA 6M Rates column size must be 2", __FILE__, __LINE__ );
        }

        if ( !futureConv.empty() && futureConv[0].size() < 2 )
        {
            throw AQLCoreInvalidData( "#Error: Futures Conventions column size must be 2", __FILE__, __LINE__ );
        }

        if ( !futureRates.empty() && futureRates[0].size() < 3 )
        {
            throw AQLCoreInvalidData( "#Error: Futures Rates column size must be greater or equal to 3", __FILE__, __LINE__ );
        }

        if ( !tenorBasisConv.empty() && tenorBasisConv[0].size() < 2 )
        {
            throw AQLCoreInvalidData( "#Error: Tenor Basis Adjustment Conventions column size must be 2", __FILE__, __LINE__ );
        }

        if ( !tenorBasisRates.empty() && tenorBasisRates[0].size() < 2 )
        {
            throw AQLCoreInvalidData( "#Error: Tenor Basis Adjustment Rates column size must be 2", __FILE__, __LINE__ );
        }

        etrading::AQLUpdateStaticDataManager::setUpSwapCurve( etrading::getDataInstance(),
                                                                  curveCollection,
                                                                  staticDataTable,
                                                                  curveConv,
                                                                  moneyMarketConv,
                                                                  liborRates,
                                                                  liborConv,
                                                                  swapRates,
                                                                  swapConv,
                                                                  fra3mRates,
                                                                  fra6mRates,
                                                                  fraConv,
                                                                  futureRates,
                                                                  futureConv,
                                                                  tenorBasisConv,
                                                                  tenorBasisRates,
                                                                  curveIndex,
                                                                  AQLString( "" ) );

        //Throw exception if the curve has not been built.
        etrading::getCurveStaticDataTableName( curveCollection, curveIndex );

        // note that curveIndexCopy is actually the staticDataTable ...
        AQLString curveIndexCopy( etrading::getDefaultValueForEmptyString( staticDataTable, "STD" ) );
        
        //Throw exception if the curve has not been built.
        etrading::getCurveStaticDataTableName( curveCollection, curveIndexCopy );

        AQLString ret( curveCollection + " " + staticDataTable + " Curve has been set" );

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateCurvename( "tryMeCurveCalibrateSwap_outputs", curveCollection, staticDataTable ) );
            file.write( "output", ret );
        }

        return ret.getCString();

        return ret;

        VALID_EXCEPTION_END

    };

}
