#include "tryMeCurveDualBootstrap.h"
#include "LAUpdateStaticDataManager.h"
#include "CreateDataFile.h"
#include "StructuredExceptionHandler.h"
#include "CurveValidation.h"
#include "ParameterValidation.h"
#include "CurveResultsContainer.h"

using etrading::CreateDataFile;
using etrading::decorateCurvename;

namespace validation_api
{

    /* @brief			validation interface for curve calibration method using dual-bootstrapping
    *  @param [in]		curveCollection		The curve collection ID
    *  @param [in]		curveName_db		Name of the dual-bootstrapping curve object
    *  @param [in]		curveName_OIS		Name of the OIS curve
    *  @param [in]		curveName_swap		Name of the swap curve
    *  @param [in]		curveIndex_OIS		Curve index of OIS curve
    *  @param [in]		curveIndex_swap		Curve index of swap curve
	*  @param [in]		commonParams		Common parameters across all curves
    *  @param [in]		generateProp_OIS	Curve build properties of OIS curve
    *  @param [in]		oisRates_OIS		OIS rates
    *  @param [in]		oisConv_OIS			OIS conventions
    *  @param [in]		histRates_OIS		Historical OIS rates
    *  @param [in]		lobasisRates_OIS	Libor OIS basis rates
    *  @param [in]		lobasisConv_OIS		Libor OIS basis conventions
    *  @param [in]		swapConv_OIS		Swap conventions
    *  @param [in]		generateProp_swap	Curve build properties of swap curve
    *  @param [in]		moneyConv_swap	    Money market conventions
    *  @param [in]		liborRates_swap 	Libor rates
	*  @param [in]		liborConv_swap 		Libor conventions
	*  @param [in]		swapRates_swap 		Swap rates in swap curve
	*  @param [in]		swapConv_swap 		Swap conventions in swap curve
	*  @param [in]		fra3mRates_swap 	3M FRA rates
	*  @param [in]		fra6mRates_swap 	6M FRA rates
	*  @param [in]		fraConv_swap 		FRA conventions
	*  @param [in]		futureRates_swap 	Futures rates
	*  @param [in]		futureConv_swap 	Futures convention
	*  @param [in]		adjustSwapConv_swap 	Tenor Basis market data
	*  @param [in]		adjustSwapRates_swap 	Tenor Basis convention
    */
    const LAString tryMeCurveDualBootstrap(const LAString& curveCollection,
											const LAString& curveName_db,
											const LAString& curveName_OIS,
											const LAString& curveName_swap,
											const LAString& curveIndex_OIS,
											const LAString& curveIndex_swap,
											const LAStringMatrix& commonParams,
											const LAStringMatrix& generateProp_OIS,
											const LAStringMatrix& oisRates_OIS,
											const LAStringMatrix& oisConv_OIS,
											const LAStringMatrix& histRates_OIS,
											const LAStringMatrix& lobasisRates_OIS,
											const LAStringMatrix& lobasisConv_OIS,
											const LAStringMatrix& swapConv_OIS,
											const LAStringMatrix& generateProp_swap,
											const LAStringMatrix& moneyConv_swap,
											const LAStringMatrix& liborRates_swap,
											const LAStringMatrix& liborConv_swap,
											const LAStringMatrix& swapRates_swap,
											const LAStringMatrix& swapConv_swap,
											const LAStringMatrix& fra3mRates_swap,
											const LAStringMatrix& fra6mRates_swap,
											const LAStringMatrix& fraConv_swap,
											const LAStringMatrix& futureRates_swap,
											const LAStringMatrix& futureConv_swap,
											const LAStringMatrix& tenorBasisConv_swap,
											const LAStringMatrix& tenorBasisRates_swap)
    {
        VALID_EXCEPTION_START
        
        // LWO Single Curves Populate Curve Results Objects that Conflict with Other Curve Types, so we must clear the Curve Results Cache
        MLIB_CLEAR_CURVE_RESULTS_CACHE

        // Prefix the staticDataTable onto the curveIndex Name Set, using the ':' delimiter by default
        std::string curveIndexStdStr_swap = curveIndex_swap.getCString();
		curveIndexStdStr_swap = etrading::addPrefixStringAndCheckForDuplicates(curveIndexStdStr_swap, std::string(curveName_swap.getCString()) );

		std::string curveIndexStdStr_OIS = curveIndex_OIS.getCString();
		curveIndexStdStr_OIS = etrading::addPrefixStringAndCheckForDuplicates(curveIndexStdStr_OIS, std::string(curveName_OIS.getCString()));
        
        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateCurvename( "tryMeCurveDualBootstrap_inputs", curveCollection, curveName_db) );
            file.write( "generatorFunction", "tryMeCurveDualBootstrap" );
            file.write( "curveCollection", curveCollection );
            file.write( "curveName_db", curveName_db);
            file.write( "curveName_OIS", curveName_OIS);
            file.write( "curveName_swap", curveName_swap);
			file.write( "curveIndex_OIS", curveIndex_OIS);
			file.write( "curveIndex_swap", curveIndex_swap);
			file.write( "commonParams", commonParams);
            file.write( "generateProp_OIS", generateProp_OIS);
            file.write( "oisRates_OIS", oisRates_OIS);
            file.write( "oisConv_OIS", oisConv_OIS);
            file.write( "histRates_OIS", histRates_OIS);
            file.write( "lobasisRates_OIS", lobasisRates_OIS);
            file.write( "lobasisConv_OIS", lobasisConv_OIS);
            file.write( "swapConv_OIS", swapConv_OIS);
            file.write( "generateProp_swap", generateProp_swap);
            file.write( "moneyConv_swap", moneyConv_swap);
            file.write( "liborRates_swap", liborRates_swap);
			file.write("liborConv_swap", liborConv_swap);
			file.write("swapRates_swap", swapRates_swap);
			file.write("swapConv_swap", swapConv_swap);
			file.write("fra3mRates_swap", fra3mRates_swap);
			file.write("fra6mRates_swap", fra6mRates_swap);
			file.write("fraConv_swap", fraConv_swap);
			file.write("futureConv_swap", futureConv_swap);
			file.write("futureRates_swap", futureRates_swap);
			file.write("tenorBasisConv_swap", tenorBasisConv_swap);
			file.write("tenorBasisRates_swap", tenorBasisRates_swap);
        }

        if ( !oisRates_OIS.empty() && oisRates_OIS[0].size() < 2 )
        {
            throw LACoreInvalidData( "#Error: OIS Rates column size must be 2", __FILE__, __LINE__ );
        }

        if ( !oisConv_OIS.empty() && oisConv_OIS[0].size() < 2 )
        {
            throw LACoreInvalidData( "#Error: OIS Convention column size must be 2", __FILE__, __LINE__ );
        }

        if ( !histRates_OIS.empty() && histRates_OIS[0].size() < 2 )
        {
            throw LACoreInvalidData( "#Error: OIS Historical Rates column size must be 2", __FILE__, __LINE__ );
        }

        if ( !lobasisRates_OIS.empty() && lobasisRates_OIS[0].size() < 2 )
        {
            throw LACoreInvalidData( "#Error: Libor-OIS Basis Rates column size must be 2", __FILE__, __LINE__ );
        }

        if ( !lobasisConv_OIS.empty() && lobasisConv_OIS[0].size() < 2 )
        {
            throw LACoreInvalidData( "#Error: Libor-OIS Basis Conventions column size must be 2", __FILE__, __LINE__ );
        }

        if ( !swapConv_OIS.empty() && swapConv_OIS[0].size() < 2 )
        {
            throw LACoreInvalidData( "#Error: OIS curve's Swap Convention column size must be 2", __FILE__, __LINE__ );
        }

        if ( !moneyConv_swap.empty() && moneyConv_swap[0].size() < 2 )
        {
            throw LACoreInvalidData( "#Error: Swap curve Money Market Conventions column size must be 2", __FILE__, __LINE__ );
        }

        if ( !liborRates_swap.empty() && liborRates_swap[0].size() < 2 )
        {
            throw LACoreInvalidData( "#Error: Swap curve Libor Rates column size must be 2", __FILE__, __LINE__ );
        }

        if ( !liborConv_swap.empty() && liborConv_swap[0].size() < 2 )
        {
            throw LACoreInvalidData( "#Error: Swap curve Libor Convention column size must be 2", __FILE__, __LINE__ );
        }

        if ( !swapRates_swap.empty() && swapRates_swap[0].size() < 2 )
        {
            throw LACoreInvalidData( "#Error: Swap Rates column size must be 2", __FILE__, __LINE__ );
        }

        if ( !swapConv_swap.empty() && swapConv_swap[0].size() < 3 )
        {
            throw LACoreInvalidData( "#Error: Swap curve Swap Convention column size must be greater or equal to 3", __FILE__, __LINE__ );
        }
        
		if (!fra3mRates_swap.empty() && fra3mRates_swap[0].size() < 2)
		{
			throw LACoreInvalidData("#Error: 3M FRA Rates column size must be 2", __FILE__, __LINE__);
		}

		if (!fra6mRates_swap.empty() && fra6mRates_swap[0].size() < 2)
		{
			throw LACoreInvalidData("#Error: 6M FRA Rates column size must be 2", __FILE__, __LINE__);
		}

		if (!fraConv_swap.empty() && fraConv_swap[0].size() < 2)
		{
			throw LACoreInvalidData("#Error: FRA Conventions column size must be 2", __FILE__, __LINE__);
		}

		if (!futureRates_swap.empty() && futureRates_swap[0].size() < 2)
		{
			throw LACoreInvalidData("#Error: Future Rates column size must be 2", __FILE__, __LINE__);
		}

		if (!futureConv_swap.empty() && futureConv_swap[0].size() < 2)
		{
			throw LACoreInvalidData("#Error: Future Convention column size must be 2", __FILE__, __LINE__);
		}

		if (!tenorBasisConv_swap.empty() && tenorBasisConv_swap[0].size() < 2)
		{
			throw LACoreInvalidData("#Error: Tenor Basis Adjustment Conventions column size must be 2", __FILE__, __LINE__);
		}

		if (!tenorBasisRates_swap.empty() && tenorBasisRates_swap[0].size() < 2)
		{
			throw LACoreInvalidData("#Error: Tenor Basis Adjustment Rates column size must be 2", __FILE__, __LINE__);
		}

		etrading::LAUpdateStaticDataManager::setUpDualBootstrapCurve(etrading::getDataInstance(),
													                      curveCollection,
													                      curveName_db,
													                      curveName_swap,
													                      curveName_OIS,
													                      curveIndex_OIS,
													                      curveIndex_swap,
													                      commonParams,
													                      generateProp_OIS,
													                      oisRates_OIS,
													                      oisConv_OIS,
													                      histRates_OIS,
													                      lobasisRates_OIS,
													                      lobasisConv_OIS,
													                      swapConv_OIS,
													                      generateProp_swap,
													                      moneyConv_swap,
													                      liborRates_swap,
													                      liborConv_swap,
													                      swapRates_swap,
													                      swapConv_swap,
													                      fra3mRates_swap,
													                      fra6mRates_swap,
													                      fraConv_swap,
													                      futureRates_swap,
													                      futureConv_swap,
													                      tenorBasisConv_swap,
													                      tenorBasisRates_swap);

        //Throw exception if the curves have not been built.
        etrading::getCurveStaticDataTableName( curveCollection, curveName_swap);
		etrading::getCurveStaticDataTableName(curveCollection, curveName_OIS);
		
        LAString ret( curveCollection + ": Curves" + curveName_swap + " and " + curveName_OIS + " have been set" );

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateCurvename( "tryMeCurveDualBootstrap_outputs", curveCollection, curveName_db ) );
            file.write( "output", ret );
        }

        return ret;

        VALID_EXCEPTION_END

    };

}
