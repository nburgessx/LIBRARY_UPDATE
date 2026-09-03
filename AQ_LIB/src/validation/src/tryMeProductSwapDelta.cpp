#include "tryMeProductSwapDelta.h"

#include "CreateDataFile.h"
#include "VanillaInterestRateSwap.h"
#include "OISSwap.h"
#include "MultiCurveDeltaRiskGenerator.h"
#include "CurveValidation.h"
#include "LabelValueBlockValidation.h"
#include "ScheduleValidation.h"
#include "SwapValidation.h"
#include "ParameterValidation.h"
#include "StructuredExceptionHandler.h"
#include "TypeHelpers.h"
#include "CommonConstants.h"
#include "TradeCollection.h"
#include <numeric>


using etrading::CreateDataFile;
using etrading::decorateFilename;
using etrading::BaseInstrument;
using etrading::VanillaInterestRateSwap;
using etrading::OISSwap;
using etrading::MultiCurveDeltaGenerator;
using etrading::BaseInstrumentPtr;
using etrading::TradeCollection;

namespace validation
{

    /* @brief			validation interface for the meSwapDelta method
    *  @param [out]		pillarNames		    The final list of pillar names
    *  @param [out]		headers			    Headers of the delta report
    *  @param [out]		deltas			    All the deltas
    *  @param [in]		dataInstance			    Pointer to the object pool
    *  @param [in]		dealInfo		    A vector of label value blocks each defining a swap trade
    *  @param [in]		ForecastCurveSet	A collection of yield curves
    *  @param [in]		CurveCollection	    ID of the curve set
    *  @param [in]		bumpSize		    Bumping size. Default to 0.01 (basis point).
    *  @param [in]		bumpMode		    Bumping mode. Up, down, or central bumping.
    *  @param [in]		deltaType		    Type of deltas. Flat shift or ladder.
    *  @param [in]		aggregateRisk	    Whether to aggregate risks against the same market instruments from different curves
    *  @param [in]		xccyFXSpotRates	        Xccy FX Spot Rates
    */
    void tryMeProductSwapDelta(
        LAStringVector& pillarNames,
        LAStringVector& headers,
        DoubleMatrix& deltas,
        LADataInstance* dataInstance,
        const std::vector<LabelValueBlock>& dealInfo,
        const LAStringVector& forecastCurveSet,
        const LAString& curveCollection,
        double bumpSize,
        const LAString& bumpMode,
        const LAString& deltaType,
        bool aggregateRisks )
    {
        VALID_EXCEPTION_START

		LAString uppercaseBumpMode( bumpMode );
		uppercaseBumpMode.toUpper();

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateFilename( "tryMeProductSwapDelta_inputs", curveCollection ) );
            file.write( "generatorFunction", "tryMeProductSwapDelta" );

            for ( size_t i = 0; i < dealInfo.size(); ++i )
            {
                LAString name = "dealInfo_" + LAString( etrading::NumberToString<size_t>( i ).c_str() );
                file.write( name.getCString(), dealInfo[i] );
            }

            file.write( "curveCollection", curveCollection );
            file.write( "forecastCurveSet", forecastCurveSet );
            file.write( "bumpSize", bumpSize );
            file.write( "bumpMode", uppercaseBumpMode );
            file.write( "deltaType", deltaType );
            file.write( "aggregateRisks", aggregateRisks );
        }

        //----------------------------------------------------------------------------------
        // Risk

		bool bumpSpreadInstruments = true;
		std::string riskCutOffTenor = "";
        MultiCurveDeltaGenerator riskGen( dealInfo, curveCollection, bumpSpreadInstruments, bumpSize, uppercaseBumpMode, aggregateRisks, riskCutOffTenor );
        riskGen.setCurves( curveCollection, forecastCurveSet );

        pillarNames.clear();
        headers.clear();
        deltas.clear();

		/* Here we calculate the delta ladder for the given dealinfo swaps.
		 * Because we wish to re-format the output and include a total column
		 * we invoke the underlying delta ladder calculation with temporary variables prior
		 * to reformatting into the output variables.
		 */
		LAStringVector deltaCCYs;
		LAStringVector tmpHeaders;
		DoubleMatrix tmpDeltas;
        riskGen.deltaLadder( pillarNames, tmpHeaders, deltaCCYs, tmpDeltas );

		/* 1. Insert a "Total" column at the beginning of the returned delta matrix
		 * This "BaseInstrument" version of the deltaLadder assumes all risks are in the same CCY.
		 */
		headers.push_back( "Pillars" );
		headers.push_back( "Total" );
		headers.insert( headers.end(), tmpHeaders.begin(), tmpHeaders.end() );

		/* 2. Calculate a total delta for each pillar
		 */
		for (size_t i=0; i<pillarNames.size(); i++)
		{
			// Fetch the row of delta values for this pillarName
			const std::vector<double>& allDeltasUnderSinglePillarName = tmpDeltas[i];

			// Compute and store the total delta for this row
			double total = std::accumulate( allDeltasUnderSinglePillarName.begin(), allDeltasUnderSinglePillarName.end(), 0.0);
			DoubleVector outputDeltasForPillar;
			outputDeltasForPillar.push_back( total );

			// Store the individual deltas for each instrument, for this pillar
			outputDeltasForPillar.insert( outputDeltasForPillar.end(), allDeltasUnderSinglePillarName.begin(), allDeltasUnderSinglePillarName.end() );
			deltas.push_back( outputDeltasForPillar );
		}

        // Record results
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateFilename( "tryMeProductSwapDelta_outputs", curveCollection ) );
            for ( size_t i = 0; i < pillarNames.size(); ++i )
            {
                // 12 is the number of decimal points required
                file.write( pillarNames[i], deltas[i], 12 );
            }
        }

        VALID_EXCEPTION_END
    }


}








