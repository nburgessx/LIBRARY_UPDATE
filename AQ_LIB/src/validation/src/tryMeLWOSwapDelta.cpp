#include "tryMeLWOSwapDelta.h"

#include "CreateDataFile.h"
#include "Swap.h"
#include "DeltaRiskGenerator.h"
#include "MultiCurveDeltaRiskGenerator.h"
#include "LWOUtilities.h"
#include "DateUtilities.h"
#include "StructuredExceptionHandler.h"
#include "TypeHelpers.h"
#include "RecordMacros.h"
#include "tryMeUtilityClean.h" // Data Trimming Methods

using etrading::CreateDataFile;
using etrading::decorateFilename;
using etrading::MultiCurveDeltaGenerator;


namespace
{
	// Helper function to calculate a unique set of currencies from a vector of currencies
	// Use the property of the std::set where each key appears just once
	std::set<LAString> calcUniqueCCYs(const LAStringVector& deltaCCYs)
	{
		std::set<LAString> uniqueCCYs;
		for (auto it=deltaCCYs.begin(); it != deltaCCYs.end(); ++it)
		{
			uniqueCCYs.insert( *it );
		}
		return uniqueCCYs;
	}
}

namespace validation
{
   /* @brief			Validation interface for the meSwapDeltaLadder method. This method calculates a Delta Ladder for a vector of swaps
	*  @param [out]		headers						Headers of the delta report
    *  @param [out]		pillarNames					The final list of pillar names
    *  @param [out]		deltas						All the deltas
    *  @param [in]		swapNames					A vector of strings representing the LWO Swap names
	*  @param [in]		curveCollectionNames		A vector of strings representing the CurveCollection names for each leg of swap
    *  @param [in]		fixingTableNames			A vector of strings represening the fixing table names for each leg of swap
	*  @param [in]		bumpSpreadInstruments		Whether to bump the LIBOR_OIS spread instruments in the OIS curve. Default is TRUE.
    *  @param [in]		bumpSize					Bumping size. Default to 0.01 (basis point).
    *  @param [in]		bumpMode					Bumping mode. Up, down, or central bumping.
    *  @param [in]		aggregateRisk				Whether to aggregate risks against the same market instruments from different curves
	*  @param [in]		reportInLegCCY				Whether to report the risk in LegCCY (if true) or valuationCCY (if false)
	*  @param [in]		riskCutOffTenor	    		The maximum tenor in years (e.g. 10Y) beyond which the curves are no longer bumped
    *  @param [in]		xccyFXSpotRates			        A vector of doubles representing Xccy FX Spot Rates
    */
	void tryMeLWOSwapDeltaLadder( LAStringVector& headers,
								  LAStringVector& pillarNames,
								  DoubleMatrix& deltas,
								  const LAStringVector& swapNames,
								  const LAStringMatrix& curveCollectionNames,
								  const LAStringMatrix& fixingTableNames,
								  const bool bumpSpreadInstruments,
								  const double bumpSize,
								  const LAString& bumpMode,
								  const bool aggregateRisks,
								  const bool reportInLegCCY,
								  const std::string& riskCutOffTenor,
                                  const DoubleVector& xccyFXSpotRates )
    {
        VALID_EXCEPTION_START

		RECORD_INPUTS( swapNames, curveCollectionNames, fixingTableNames, bumpSpreadInstruments, bumpSize, bumpMode, aggregateRisks, reportInLegCCY, riskCutOffTenor, xccyFXSpotRates );

        // Trim Inputs then Check if they are empty
        LAStringVector trimmedSwapNames = validation::trimLAStringVector( swapNames );
        LAStringMatrix trimmedCurveCollectionsNames = validation::trimLAStringMatrix( curveCollectionNames );
        LAStringMatrix trimmedFixingTableNames = validation::trimLAStringMatrix( fixingTableNames );
        
        // Appears to be already trimmed in the XLLPlusTips.cpp
        // How would we trim double vectors?, blank excel input mostly treated as zero, perhaps in XLOper layer
        // DoubleVector trimmedXccyFXSpotRates = validation::trimDoubleVector( xccyFXSpotRates );
        DoubleVector trimmedXccyFXSpotRates = xccyFXSpotRates; 

        const size_t nSwaps           = trimmedSwapNames.size();
        const size_t nCurves          = trimmedCurveCollectionsNames.size();
        const size_t nFixingTables    = trimmedFixingTableNames.size();
        const size_t nFXSpotRates     = trimmedXccyFXSpotRates.size();

		AQ_REQUIRE( !trimmedSwapNames.empty(), "No swap names have been provided" )
        AQ_REQUIRE( nSwaps == nCurves, "Inconsistent Data: Number of Swaps and Curve Collections do not match. There are " + AQ_TO_STRING_FROM_SIZE_T(nSwaps) + " Swaps and " + AQ_TO_STRING_FROM_SIZE_T(nCurves) + " Curves." )
        
        // Optional Fixing Tables
        if ( nFixingTables > 0 )
        {
            AQ_REQUIRE( nSwaps == nFixingTables, "Inconsistent Data: Number of Swaps and Fixing Tables does not match. There are " + AQ_TO_STRING_FROM_SIZE_T(nSwaps) + " Swaps and " + AQ_TO_STRING_FROM_SIZE_T(nFixingTables) + " Fixing Tables." )
        }

        // Optional Xccy FX Spot Rates
        if ( nFXSpotRates > 0 )
        {
            AQ_REQUIRE( nSwaps == nFXSpotRates, "Inconsistent Data: Number of Swaps and Xccy FXSpot Rates does not match. There are " + AQ_TO_STRING_FROM_SIZE_T(nSwaps) + " Swaps and " + AQ_TO_STRING_FROM_SIZE_T(nFXSpotRates) + " Xccy FXSpot Rates." )
        }

        //----------------------------------------------------------------------------------
        // Risk

		// Verify that the riskCutOffTenor can be parsed, if provided.
		const unsigned int riskCutOffTenorYears = etrading::parseTenorYears( riskCutOffTenor, true /* throw on failure */ );

		LAString uppercaseBumpMode( bumpMode );
		uppercaseBumpMode.toUpper();
		MultiCurveDeltaGenerator riskGen( trimmedSwapNames, trimmedCurveCollectionsNames, fixingTableNames, trimmedXccyFXSpotRates, bumpSpreadInstruments, bumpSize, uppercaseBumpMode, aggregateRisks, reportInLegCCY, riskCutOffTenor );

        headers.clear();
		pillarNames.clear();
        deltas.clear();

		/* Here we calculate the delta ladder for the given swapNames.
		 * Because we wish to re-format the output and include a total-per-CCY column
		 * we invoke the underlying delta ladder calculation with temporary variables prior
		 * to reformatting into the output variables.
		 */
		LAStringVector deltaCCYs;
		LAStringVector tmpHeaders;
		DoubleMatrix tmpDeltas;
        riskGen.deltaLadder( pillarNames, tmpHeaders, deltaCCYs, tmpDeltas );

		/*
		 * The rows in the output matrix "tmpDeltas" correspond to the  pillar names
		 * and there is one column value for each swap leg. Each column value is labelled with a currency (deltaCCYs)
		 *
		 * 1. First, we augment the column headers.
		 */
		headers.push_back( "Pillars" );

		std::set<LAString> uniqueCCYs = calcUniqueCCYs( deltaCCYs );
		// Add a header for each Total by CCY
		for ( auto ccy = uniqueCCYs.begin(); ccy != uniqueCCYs.end(); ++ccy )
		{
			LAString columnHeading = *ccy + " : Total"; 
			headers.push_back( columnHeading );
		}

		// The results we receive back from the deltaLadder() method contain a delta column per swap leg in the portfolio.
		// Here we prefix the header with the currency
		const size_t nDeltaColumns = deltaCCYs.size();
		for ( size_t i = 0; i < nDeltaColumns; ++i )
		{
			LAString columnHeading = deltaCCYs[i] + " : " + tmpHeaders[i];
			headers.push_back( columnHeading );
		}

		/*
		 * 2. For each pillarName, we accumulate the total delta by CCY.
		 * i.e. For each row (pillarName), we construct a map:  CCY -> double
		 */
		for (size_t i=0; i < pillarNames.size(); i++)
		{
			// Fetch the row of delta values for this pillarName
			const std::vector<double>& allDeltasUnderSinglePillarName = tmpDeltas[i];

			// For a particular pillar, we construct a map to hold the total delta per currrency
			std::map<LAString, double> totalDeltaPerCCY;
			for (size_t j = 0; j < nDeltaColumns; j++)
			{
				const LAString& ccy = deltaCCYs[j];
				double& total = totalDeltaPerCCY[ccy];   
				total += allDeltasUnderSinglePillarName[j];
			}

			// Output the total delta per currency for this pillar
			DoubleVector outputDeltasForPillar;
			for ( auto it = uniqueCCYs.begin(); it != uniqueCCYs.end(); it++)
			{
				double totalDelta = totalDeltaPerCCY[ *it ];
				outputDeltasForPillar.push_back( totalDelta );
			}
			// Now copy in the deltas for each swap leg
			outputDeltasForPillar.insert( outputDeltasForPillar.end(), allDeltasUnderSinglePillarName.begin(), allDeltasUnderSinglePillarName.end() );

			// Add the row for this pillar to our matrix of results
			deltas.push_back( outputDeltasForPillar );
		}

        // Record results
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateFilename( "tryMeLWOSwapDeltaLadder_outputs") );
			
			file.write( "headers", headers );

            for ( size_t i = 0; i < pillarNames.size(); ++i )
            {
                // 12 is the number of decimal points required
                file.write( pillarNames[i], deltas[i], 12 );
            }
        }

        VALID_EXCEPTION_END
    }

   /* @brief			Validation interface for the meSwapDeltaLadder method. This method calculates a Delta Ladder for a vector of swaps
    *                   This version presents the delta buckets for each curve in separate columns. Each consecutive pairs of columns
	*                   (pillarName, delta) are aligned horizontally.
	*  @param [out]		headers						Headers of the delta report
    *  @param [out]		pillarNames					A vector of LAStringVector, with each LAStringVector representing a column of pillarNames for a curve
    *  @param [out]		deltas						A vector of DoubleVector, with each DoubleVector representing a column of delta sensitivities for a curve
    *  @param [in]		swapNames					A vector of strings representing the LWO Swap names
	*  @param [in]		curveCollectionNames		A vector of strings representing the CurveCollection names for each leg of swap
    *  @param [in]		fixingTableNames			A vector of strings represening the fixing table names for each leg of swap
	*  @param [in]		bumpSpreadInstruments		Whether to bump the LIBOR_OIS spread instruments in the OIS curve. Default is TRUE.
    *  @param [in]		bumpSize					Bumping size. Default to 0.01 (basis point).
    *  @param [in]		bumpMode					Bumping mode. Up, down, or central bumping.
    *  @param [in]		aggregateRisk				Whether to aggregate risks against the same market instruments from different curves
	*  @param [in]		reportInLegCCY				Whether to report the risk in LegCCY (if true) or valuationCCY (if false)
	*  @param [in]		riskCutOffTenor	    		The maximum tenor in years (e.g. 10Y) beyond which the curves are no longer bumped
    *  @param [in]		xccyFXSpotRates			        A vector of doubles representing Xccy FX Spot Rates
    */
	void tryMeLWOSwapDeltaLadderHorizontally( LAStringVector& headers,
											  std::vector<LAStringVector>& pillarNames,
											  std::vector<DoubleVector>& deltas,
											  const LAStringVector& swapNames,
											  const LAStringMatrix& curveCollectionNames,
											  const LAStringMatrix& fixingTableNames,
											  const bool bumpSpreadInstruments,
											  const double bumpSize,
											  const LAString& bumpMode,
											  const bool aggregateRisks,
											  const bool reportInLegCCY,
											  const std::string& riskCutOffTenor,
                                              const DoubleVector& xccyFXSpotRates )
	{
        VALID_EXCEPTION_START

		RECORD_INPUTS( swapNames, curveCollectionNames, fixingTableNames, bumpSpreadInstruments, bumpSize, bumpMode, aggregateRisks, reportInLegCCY, riskCutOffTenor, xccyFXSpotRates );

        // Trim Inputs then Check if they are empty
        LAStringVector trimmedSwapNames = validation::trimLAStringVector( swapNames );
        LAStringMatrix trimmedCurveCollectionsNames = validation::trimLAStringMatrix( curveCollectionNames );
        LAStringMatrix trimmedFixingTableNames = validation::trimLAStringMatrix( fixingTableNames );
        
        // Appears to be already trimmed in the XLLPlusTips.cpp
        // How would we trim double vectors?, blank excel input mostly treated as zero, perhaps in XLOper layer
        // DoubleVector trimmedXccyFXSpotRates = validation::trimDoubleVector( xccyFXSpotRates );
        DoubleVector trimmedXccyFXSpotRates = xccyFXSpotRates; 

        const size_t nSwaps           = trimmedSwapNames.size();
        const size_t nCurves          = trimmedCurveCollectionsNames.size();
        const size_t nFixingTables    = trimmedFixingTableNames.size();
        const size_t nFXSpotRates     = trimmedXccyFXSpotRates.size();

		AQ_REQUIRE( !trimmedSwapNames.empty(), "No swap names have been provided" )
        AQ_REQUIRE( nSwaps == nCurves, "Inconsistent Data: Number of Swaps and Curve Collections do not match. There are " + AQ_TO_STRING_FROM_SIZE_T(nSwaps) + " Swaps and " + AQ_TO_STRING_FROM_SIZE_T(nCurves) + " Curves." )
        
        // Optional Fixing Tables
        if ( nFixingTables > 0 )
        {
            AQ_REQUIRE( nSwaps == nFixingTables, "Inconsistent Data: Number of Swaps and Fixing Tables does not match. There are " + AQ_TO_STRING_FROM_SIZE_T(nSwaps) + " Swaps and " + AQ_TO_STRING_FROM_SIZE_T(nFixingTables) + " Fixing Tables." )
        }

        // Optional Xccy FX Spot Rates
        if ( nFXSpotRates > 0 )
        {
            AQ_REQUIRE( nSwaps == nFXSpotRates, "Inconsistent Data: Number of Swaps and Xccy FXSpot Rates does not match. There are " + AQ_TO_STRING_FROM_SIZE_T(nSwaps) + " Swaps and " + AQ_TO_STRING_FROM_SIZE_T(nFXSpotRates) + " Xccy FXSpot Rates." )
        }

		// Clear the output parameters
		headers.clear();
		pillarNames.clear();
		deltas.clear();

        //----------------------------------------------------------------------------------
        // Risk
		// Verify that the riskCutOffTenor can be parsed, if provided.
		const unsigned int riskCutOffTenorYears = etrading::parseTenorYears( riskCutOffTenor, true /* throw on failure */ );

		// Initialise the MultiCurveDeltaGenerator with the portfolio of swapNames
		LAString uppercaseBumpMode( bumpMode );
		uppercaseBumpMode.toUpper();
		MultiCurveDeltaGenerator riskGen( trimmedSwapNames, trimmedCurveCollectionsNames, trimmedFixingTableNames, xccyFXSpotRates, bumpSpreadInstruments, bumpSize, uppercaseBumpMode, aggregateRisks, reportInLegCCY, riskCutOffTenor );

		/* Here we calculate the delta ladder for the given swap names.
		 * Because we wish to re-format the output and return the ladder grouped by curve horizontally,
		 * we invoke the underlying delta ladder calculation with temporary variables prior
		 * to reformatting into the output variables.
		 */
		LAStringVector tmpHeaders;
		LAStringVector deltaCCYs;
		LAStringVector tmpPillarNames;
		DoubleMatrix tmpDeltas;
		riskGen.deltaLadder( tmpPillarNames, tmpHeaders, deltaCCYs, tmpDeltas );

		/*
		 * The rows in the output matrix "tmpDeltas" correspond to the  pillar names (tmpPillarNames)
		 * and there is one column value for each swap leg. Each column value is labelled with a currency (deltaCCYs)
		 *
		 * 1. First, for each pillarName, we accumulate the total delta by CCY.
		 * i.e. For each row (pillarName), we construct a map:  CCY -> double
		 *
		 * At the end, the following vector will contain the same number of elements as there are pillarNames.
		 */

		std::vector<std::map<LAString, double> >totalDeltaPerPillarPerCCY;
		const size_t nPillars = tmpPillarNames.size();

		// There is a delta column per swap leg in the portfolio
		const size_t nDeltaColumns = deltaCCYs.size();

		for (size_t i=0; i<nPillars; i++)
		{
			// Fetch the row of delta values for this pillarName
			const std::vector<double>& allDeltasUnderSinglePillarName = tmpDeltas[i];

			// For a particular pillar, we construct a map to hold the total delta per currrency
			std::map<LAString, double> totalDeltaPerCCY;
			for (size_t j=0; j<nDeltaColumns; j++)
			{
				const LAString& ccy = deltaCCYs[j];
				double& total = totalDeltaPerCCY[ccy];   
				total += allDeltasUnderSinglePillarName[j];
			}
			totalDeltaPerPillarPerCCY.push_back( totalDeltaPerCCY );
		}

		/* 2. Next we group the delta results by COLLECTION_CURVENAME, so that the pillarNames belonging to
		 * different curves are returned in separate columns.
		 *
		 * The pillarNames are given in the format:  CURVECOLLECTION.CURVENAME.INSTRUMENT.TENOR
		 * where '.' represents PILLAR_DELIMITER
		 *
		 * Group the results by:  CURVECOLLECTION_CURVENAME
		 * i.e.  for each currency, construct a map: CURVECOLLECTION_CURVENAME ->  vector ( PILLARNAME, TOTALDELTA )
		 */
		std::set<LAString> uniqueCCYs = calcUniqueCCYs( deltaCCYs );
		for (auto it=uniqueCCYs.begin(); it != uniqueCCYs.end(); ++it)
		{
			const LAString& ccy = *it;
			std::map<LAString, std::vector<std::pair<LAString, double> > > groupedResults;
			
			for (size_t i = 0; i < tmpPillarNames.size(); i++)
			{
				// Construct a key from CURVECOLLECTION and CURVENAME
				const LAString& pillarName = tmpPillarNames[i];
				LAStringVector tokens = pillarName.toToken( etrading::PILLAR_DELIMITER );
				assert ( tokens.size() >= 3 );
				LAString key = tokens[0] + etrading::PILLAR_DELIMITER + tokens[1];

				// Append this current PillarName and delta bucket in the vector of results for the curvename specified by "key"
				std::vector<std::pair<LAString, double> >& bucketData = groupedResults[ key ];

				std::map<LAString, double> totalDeltaPerCCY = totalDeltaPerPillarPerCCY[i];
				double totalDelta = totalDeltaPerCCY[ccy];

				bucketData.push_back( std::make_pair( pillarName, totalDelta ) );
			}

			// Now format the results into columns:
			// Each item in groupedResults corresponds to a vector of pillars for a given curve
			for (auto it=groupedResults.begin(); it != groupedResults.end(); ++it)
			{
				std::vector<std::pair<LAString, double> >& bucketData = it->second;

				// Decide whether to include this column of deltas in the output:
				// If the portfolio of swaps is sensitive to any point in this curve, include it
				bool includeCurveInOutput = false;
				for (size_t j=0; j < bucketData.size(); j++)
				{
					double totalDelta = bucketData[j].second;
					if ( fabs( totalDelta ) > 0.0 )
					{
						includeCurveInOutput = true;
						break;
					}
				}

				if ( includeCurveInOutput )
				{
					headers.push_back( ccy + " : Pillars" );
					headers.push_back( ccy + " : Total" );

					LAStringVector pillarColumn;
					DoubleVector deltaColumn;
					for (size_t j=0; j < bucketData.size(); j++)
					{
						LAString& pillarName = bucketData[j].first;
						pillarColumn.push_back( pillarName );

						double totalDelta = bucketData[j].second;
						deltaColumn.push_back( totalDelta );
					}
					pillarNames.push_back( pillarColumn );
					deltas.push_back( deltaColumn );
				}
			}
		 }

        // Record results
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateFilename( "tryMeLWOSwapDeltaLadderHorizontal_outputs") );

			file.write( "headers", headers );

            for ( size_t i = 0; i < pillarNames.size(); ++i )
            {
                // 12 is the number of decimal points required
                file.write( tmpPillarNames[i], tmpDeltas[i], 12 );
            }
        }
		
        VALID_EXCEPTION_END
    }

   /* @brief			validation interface for the meSwapDelta method. This method calculates a Flat-Shift Delta for a vector of swaps
	*  @param [out]		positionIDs					The vector of positions for which the flat-shift delta is calculated
    *  @param [out]		deltas						The delta value for each SwapLeg
    *  @param [in]		swapNames					A vector of strings representing the LWO Swap names
	*  @param [in]		CurveCollectionNamesLeg1	A vector of strings representing the CurveCollection names for Leg1 of each swap
	*  @param [in]		CurveCollectionNamesLeg2	A vector of strings representing the CurveCollection names for Leg2 of each swap
    *  @param [in]		fixingTableNamesLeg1		A vector of strings represening the fixing table names for Leg1 of each swap
	*  @param [in]		fixingTableNamesLeg2		A vector of strings represening the fixing table names for Leg2 of each swap
	*  @param [in]		bumpSpreadInstruments		Whether to bump the LIBOR_OIS spread instruments in the OIS curve. Default is TRUE.
    *  @param [in]		bumpSize					Bumping size. Default to 0.01 (basis point).
    *  @param [in]		bumpMode					Bumping mode. Up, down, or central bumping.
    *  @param [in]		aggregateRisk				Whether to aggregate risks against the same market instruments from different curves
	*  @param [in]		reportInLegCCY				Whether to report the risk in LegCCY (if true) or valuationCCY (if false)
    *  @param [in]		xccyFXSpotRates			        A vector of doubles representing Xccy FX Spot Rates
    */
	void tryMeLWOSwapDelta( LAStringVector& positionIDs,
							DoubleVector& deltas,
							const LAStringVector& swapNames,
							const LAStringMatrix& curveCollectionNames,
							const LAStringMatrix& fixingTableNames,
							const bool bumpSpreadInstruments,
							const double bumpSize,
							const LAString& bumpMode,
							const LAString& groupRiskBy,
							const bool aggregateRisks,
							const bool reportInLegCCY,
                            const DoubleVector& xccyFXSpotRates)
    {
        VALID_EXCEPTION_START

		RECORD_INPUTS( swapNames, curveCollectionNames, fixingTableNames, bumpSpreadInstruments, bumpSize, bumpMode, groupRiskBy, aggregateRisks, reportInLegCCY, xccyFXSpotRates );

		// Trim Inputs then Check if they are empty
        LAStringVector trimmedSwapNames = validation::trimLAStringVector( swapNames );
        LAStringMatrix trimmedCurveCollectionsNames = validation::trimLAStringMatrix( curveCollectionNames );
        LAStringMatrix trimmedFixingTableNames = validation::trimLAStringMatrix( fixingTableNames );
        
        // Appears to be already trimmed in the XLLPlusTips.cpp
        // How would we trim double vectors?, blank excel input mostly treated as zero, perhaps in XLOper layer
        // DoubleVector trimmedXccyFXSpotRates = validation::trimDoubleVector( xccyFXSpotRates );
        DoubleVector trimmedXccyFXSpotRates = xccyFXSpotRates; 

        const size_t nSwaps           = trimmedSwapNames.size();
        const size_t nCurves          = trimmedCurveCollectionsNames.size();
        const size_t nFixingTables    = trimmedFixingTableNames.size();
        const size_t nFXSpotRates     = trimmedXccyFXSpotRates.size();

		AQ_REQUIRE( !trimmedSwapNames.empty(), "No swap names have been provided" )
        AQ_REQUIRE( nSwaps == nCurves, "Inconsistent Data: Number of Swaps and Curve Collections do not match. There are " + AQ_TO_STRING_FROM_SIZE_T(nSwaps) + " Swaps and " + AQ_TO_STRING_FROM_SIZE_T(nCurves) + " Curves." )
        
        // Optional Fixing Tables
        if ( nFixingTables > 0 )
        {
            AQ_REQUIRE( nSwaps == nFixingTables, "Inconsistent Data: Number of Swaps and Fixing Tables does not match. There are " + AQ_TO_STRING_FROM_SIZE_T(nSwaps) + " Swaps and " + AQ_TO_STRING_FROM_SIZE_T(nFixingTables) + " Fixing Tables." )
        }

        // Optional Xccy FX Spot Rates
        if ( nFXSpotRates > 0 )
        {
            AQ_REQUIRE( nSwaps == nFXSpotRates, "Inconsistent Data: Number of Swaps and Xccy FXSpot Rates does not match. There are " + AQ_TO_STRING_FROM_SIZE_T(nSwaps) + " Swaps and " + AQ_TO_STRING_FROM_SIZE_T(nFXSpotRates) + " Xccy FXSpot Rates." )
        }

        //----------------------------------------------------------------------------------
        // Risk

		LAString uppercaseBumpMode( bumpMode );
		uppercaseBumpMode.toUpper();

		LAString uppercaseGroupRiskBy( groupRiskBy );
		uppercaseGroupRiskBy.toUpper();

		// The flat-shift delta risk calculation is relatively fast, compared to the delta-ladder calculation
		// Therefore currently no need to truncate the curve bumping with a riskCutoffTenor
		std::string riskCutOffTenor = "";
        MultiCurveDeltaGenerator riskGen( trimmedSwapNames, trimmedCurveCollectionsNames, trimmedFixingTableNames, xccyFXSpotRates, bumpSpreadInstruments, bumpSize, uppercaseBumpMode, aggregateRisks, reportInLegCCY, riskCutOffTenor );

		positionIDs.clear();
        deltas.clear();
        riskGen.flatShiftDelta( positionIDs, deltas, uppercaseGroupRiskBy);

        // Record results
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateFilename( "tryMeLWOSwapDelta_outputs") );
            
			for ( size_t i = 0; i < positionIDs.size(); ++i )
			{
				// 12 is the number of decimal points required
				file.write( positionIDs[i], deltas[i], 12 );
			}
        }

        VALID_EXCEPTION_END
    }

}


