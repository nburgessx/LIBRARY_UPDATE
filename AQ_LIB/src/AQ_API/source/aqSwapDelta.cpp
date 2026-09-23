// aqSwapDelta.cpp

/*
 * @brief			Swig interface for aqSwapDelta / aqSwapObjectDelta... risk functions
 */

#include "aqSwapDelta.h"
#include "AQLCoreTemplateType.h"
#include "TypeUtilities.h"
#include "tryAqSwapDelta.h"
#include "tryAqSwapObjectDelta.h"
#include "ParameterValidation.h"    // etrading::getDataInstance
#include "LabelValueBlock.h"        // etrading::buildMultiLabelValueBlock
#include "APISetUp.h"               // AQ_API_START and AQ_API_END Macros

namespace
{
    // A DoubleMatrix + row/column labels -> a single VariantMatrix: a header
    // row ("" then each column header) followed by one row per pillar (pillar
    // name then its values). Mirrors AQ_XLL's xllSwap.cpp toExcelLabeledMatrix,
    // which backs the same tryAqSwapObjectDeltaLadder[Horizontally] outputs.
    etrading::VariantMatrix buildLabeledDeltaMatrix( const AQLStringVector& columnHeaders,
                                                      const AQLStringVector& rowLabels,
                                                      const DoubleMatrix& values )
    {
        etrading::VariantMatrix result;

        etrading::VariantVector header;
        header.push_back( etrading::Variant( "" ) );
        for ( const AQLString& columnHeader : columnHeaders )
        {
            header.push_back( etrading::Variant( columnHeader.getCString() ) );
        }
        result.push_back( header );

        for ( std::size_t r = 0; r < rowLabels.size(); ++r )
        {
            etrading::VariantVector row;
            row.push_back( etrading::Variant( rowLabels[r].getCString() ) );
            if ( r < values.size() )
            {
                for ( double v : values[r] )
                {
                    row.push_back( etrading::Variant( v ) );
                }
            }
            result.push_back( row );
        }

        return result;
    }
}

/* @brief			swig interface for the aqSwapDelta function - flat-shift delta for several trades defined inline
*  @param [in]		dealInfoLVBs		Trade definitions table: header row of keys, one row per trade
*  @param [in]		forecastCurveSet	Column of forecasting curve indices
*  @param [in]		curveCollection		The curve collection
*  @param [in]		bumpSize			Bump size, e.g. 0.0001 for 1bp
*  @param [in]		bumpMode			Up, Down, or Central bumping
*  @param [in]		deltaType			Flat-shift or ladder delta type
*  @param [in]		aggregateRisk		Whether to aggregate risk against the same instrument across curves
*  @return			A pillar x header delta matrix (header row, then one row per pillar)
*/
SWIG_STRINGMATRIX aqSwapDelta( const SWIG_STRINGMATRIX & dealInfoLVBs,
                            const std::vector<std::string>& forecastCurveSet,
                            const std::string& curveCollection,
                            double bumpSize,
                            const std::string& bumpMode,
                            const std::string& deltaType,
                            bool aggregateRisk )
{
    AQ_API_START

    AQLStringMatrix dealInfoAsStringMatrix;
    swig::buildStringMatrix( dealInfoAsStringMatrix, dealInfoLVBs );
    std::vector<LabelValueBlock> dealInfo = etrading::buildMultiLabelValueBlock( dealInfoAsStringMatrix );

    AQLStringVector forecastCurveSetAsAQLStringVector;
    swig::buildStringVector( forecastCurveSetAsAQLStringVector, forecastCurveSet );

    AQLString curveCollectionAsAQLString( curveCollection.c_str() );
    AQLString bumpModeAsAQLString( bumpMode.c_str() );
    AQLString deltaTypeAsAQLString( deltaType.c_str() );

    AQLStringVector pillarNames;
    AQLStringVector headers;
    DoubleMatrix    deltas;

    validation::tryAqSwapDelta( pillarNames, headers, deltas, etrading::getDataInstance(),
                                 dealInfo, forecastCurveSetAsAQLStringVector, curveCollectionAsAQLString,
                                 bumpSize, bumpModeAsAQLString, deltaTypeAsAQLString, aggregateRisk );

    etrading::VariantMatrix result = buildLabeledDeltaMatrix( headers, pillarNames, deltas );

    return swig::fromVariantMatrixToMatrixOfString( result );

    AQ_API_END
}

/* @brief			swig interface for the aqSwapObjectDelta function - flat-shift delta for a column of cached swaps
*  @param [in]		swapNames				Column of cached swap handles
*  @param [in]		curveCollectionNames	Curve collection name(s) per swap leg
*  @param [in]		fixingTableNames		Fixing table name(s) per swap leg
*  @param [in]		bumpSpreadInstruments	Whether to bump the LIBOR-OIS spread instruments in the OIS curve
*  @param [in]		bumpSize				Bump size, e.g. 0.0001 for 1bp
*  @param [in]		bumpMode				Up, Down, or Central bumping
*  @param [in]		groupRiskBy				How to group the reported risk
*  @param [in]		aggregateRisks			Whether to aggregate risk against the same instrument across curves
*  @param [in]		reportInLegCCY			Whether to report the risk in LegCCY (if true) or valuationCCY (if false)
*  @param [in]		xccyFXSpotRates			Optional. Column of cross-currency FX spot rates
*  @return			A two-column (PositionID, Delta) matrix, one row per position
*/
SWIG_STRINGMATRIX aqSwapObjectDelta( const std::vector<std::string>& swapNames,
                                  const SWIG_STRINGMATRIX & curveCollectionNames,
                                  const SWIG_STRINGMATRIX & fixingTableNames,
                                  bool bumpSpreadInstruments,
                                  double bumpSize,
                                  const std::string& bumpMode,
                                  const std::string& groupRiskBy,
                                  bool aggregateRisks,
                                  bool reportInLegCCY,
                                  const std::vector<double>& xccyFXSpotRates )
{
    AQ_API_START

    AQLStringVector swapNamesAsAQLStringVector;
    swig::buildStringVector( swapNamesAsAQLStringVector, swapNames );

    AQLStringMatrix curveCollectionNamesAsStringMatrix;
    swig::buildStringMatrix( curveCollectionNamesAsStringMatrix, curveCollectionNames );

    AQLStringMatrix fixingTableNamesAsStringMatrix;
    swig::buildStringMatrix( fixingTableNamesAsStringMatrix, fixingTableNames );

    AQLString bumpModeAsAQLString( bumpMode.c_str() );
    AQLString groupRiskByAsAQLString( groupRiskBy.c_str() );

    AQLStringVector positionIDs;
    DoubleVector    deltas;

    validation::tryAqSwapObjectDelta( positionIDs, deltas, swapNamesAsAQLStringVector,
                                       curveCollectionNamesAsStringMatrix, fixingTableNamesAsStringMatrix,
                                       bumpSpreadInstruments, bumpSize, bumpModeAsAQLString, groupRiskByAsAQLString,
                                       aggregateRisks, reportInLegCCY, xccyFXSpotRates );

    etrading::VariantMatrix result;
    etrading::VariantVector header;
    header.push_back( etrading::Variant( "PositionID" ) );
    header.push_back( etrading::Variant( "Delta" ) );
    result.push_back( header );

    for ( std::size_t i = 0; i < positionIDs.size(); ++i )
    {
        etrading::VariantVector row;
        row.push_back( etrading::Variant( positionIDs[i].getCString() ) );
        row.push_back( etrading::Variant( i < deltas.size() ? deltas[i] : 0.0 ) );
        result.push_back( row );
    }

    return swig::fromVariantMatrixToMatrixOfString( result );

    AQ_API_END
}

/* @brief			swig interface for the aqSwapObjectDeltaLadder function - pillar-by-pillar delta ladder for a column of cached swaps
*  @param [in]		swapNames				Column of cached swap handles
*  @param [in]		curveCollectionNames	Curve collection name(s) per swap leg
*  @param [in]		fixingTableNames		Fixing table name(s) per swap leg
*  @param [in]		bumpSpreadInstruments	Whether to bump the LIBOR-OIS spread instruments in the OIS curve
*  @param [in]		bumpSize				Bump size, e.g. 0.0001 for 1bp
*  @param [in]		bumpMode				Up, Down, or Central bumping
*  @param [in]		aggregateRisk			Whether to aggregate risk against the same instrument across curves
*  @param [in]		reportInLegCCY			Whether to report the risk in LegCCY (if true) or valuationCCY (if false)
*  @param [in]		riskCutOffTenor			The maximum tenor (e.g. 10Y) beyond which curves are no longer bumped
*  @param [in]		xccyFXSpotRates			Optional. Column of cross-currency FX spot rates
*  @return			A pillar x curve delta matrix (header row, then one row per pillar)
*/
SWIG_STRINGMATRIX aqSwapObjectDeltaLadder( const std::vector<std::string>& swapNames,
                                        const SWIG_STRINGMATRIX & curveCollectionNames,
                                        const SWIG_STRINGMATRIX & fixingTableNames,
                                        bool bumpSpreadInstruments,
                                        double bumpSize,
                                        const std::string& bumpMode,
                                        bool aggregateRisk,
                                        bool reportInLegCCY,
                                        const std::string& riskCutOffTenor,
                                        const std::vector<double>& xccyFXSpotRates )
{
    AQ_API_START

    AQLStringVector swapNamesAsAQLStringVector;
    swig::buildStringVector( swapNamesAsAQLStringVector, swapNames );

    AQLStringMatrix curveCollectionNamesAsStringMatrix;
    swig::buildStringMatrix( curveCollectionNamesAsStringMatrix, curveCollectionNames );

    AQLStringMatrix fixingTableNamesAsStringMatrix;
    swig::buildStringMatrix( fixingTableNamesAsStringMatrix, fixingTableNames );

    AQLString bumpModeAsAQLString( bumpMode.c_str() );

    AQLStringVector headers;
    AQLStringVector pillarNames;
    DoubleMatrix    deltas;

    validation::tryAqSwapObjectDeltaLadder( headers, pillarNames, deltas, swapNamesAsAQLStringVector,
                                             curveCollectionNamesAsStringMatrix, fixingTableNamesAsStringMatrix,
                                             bumpSpreadInstruments, bumpSize, bumpModeAsAQLString, aggregateRisk,
                                             reportInLegCCY, riskCutOffTenor, xccyFXSpotRates );

    etrading::VariantMatrix result = buildLabeledDeltaMatrix( headers, pillarNames, deltas );

    return swig::fromVariantMatrixToMatrixOfString( result );

    AQ_API_END
}

/* @brief			swig interface for the aqSwapObjectDeltaLadderHorizontally function - as aqSwapObjectDeltaLadder,
*					with each curve's pillar/delta pair presented in its own two columns
*  @param [in]		swapNames				Column of cached swap handles
*  @param [in]		curveCollectionNames	Curve collection name(s) per swap leg
*  @param [in]		fixingTableNames		Fixing table name(s) per swap leg
*  @param [in]		bumpSpreadInstruments	Whether to bump the LIBOR-OIS spread instruments in the OIS curve
*  @param [in]		bumpSize				Bump size, e.g. 0.0001 for 1bp
*  @param [in]		bumpMode				Up, Down, or Central bumping
*  @param [in]		aggregateRisk			Whether to aggregate risk against the same instrument across curves
*  @param [in]		reportInLegCCY			Whether to report the risk in LegCCY (if true) or valuationCCY (if false)
*  @param [in]		riskCutOffTenor			The maximum tenor (e.g. 10Y) beyond which curves are no longer bumped
*  @param [in]		xccyFXSpotRates			Optional. Column of cross-currency FX spot rates
*  @return			A header row (each curve name repeated twice) followed by paired PillarName/Delta columns
*/
SWIG_STRINGMATRIX aqSwapObjectDeltaLadderHorizontally( const std::vector<std::string>& swapNames,
                                                    const SWIG_STRINGMATRIX & curveCollectionNames,
                                                    const SWIG_STRINGMATRIX & fixingTableNames,
                                                    bool bumpSpreadInstruments,
                                                    double bumpSize,
                                                    const std::string& bumpMode,
                                                    bool aggregateRisk,
                                                    bool reportInLegCCY,
                                                    const std::string& riskCutOffTenor,
                                                    const std::vector<double>& xccyFXSpotRates )
{
    AQ_API_START

    AQLStringVector swapNamesAsAQLStringVector;
    swig::buildStringVector( swapNamesAsAQLStringVector, swapNames );

    AQLStringMatrix curveCollectionNamesAsStringMatrix;
    swig::buildStringMatrix( curveCollectionNamesAsStringMatrix, curveCollectionNames );

    AQLStringMatrix fixingTableNamesAsStringMatrix;
    swig::buildStringMatrix( fixingTableNamesAsStringMatrix, fixingTableNames );

    AQLString bumpModeAsAQLString( bumpMode.c_str() );

    AQLStringVector                headers;
    std::vector<AQLStringVector>   pillarNames;
    std::vector<DoubleVector>      deltas;

    validation::tryAqSwapObjectDeltaLadderHorizontally( headers, pillarNames, deltas, swapNamesAsAQLStringVector,
                                                          curveCollectionNamesAsStringMatrix, fixingTableNamesAsStringMatrix,
                                                          bumpSpreadInstruments, bumpSize, bumpModeAsAQLString, aggregateRisk,
                                                          reportInLegCCY, riskCutOffTenor, xccyFXSpotRates );

    // Mirrors AQ_XLL's xllSwap.cpp aqSwapObjectDeltaLadderHorizontally body: each curve
    // contributes a (PillarName, Delta) pair of columns, all rows padded to the tallest column.
    std::size_t maxRows = 0;
    for ( const AQLStringVector& column : pillarNames )
    {
        maxRows = std::max( maxRows, column.size() );
    }

    etrading::VariantMatrix result;

    etrading::VariantVector headerRow;
    for ( std::size_t c = 0; c < headers.size(); ++c )
    {
        headerRow.push_back( etrading::Variant( headers[c].getCString() ) );
        headerRow.push_back( etrading::Variant( "" ) );
    }
    result.push_back( headerRow );

    for ( std::size_t r = 0; r < maxRows; ++r )
    {
        etrading::VariantVector row;
        for ( std::size_t c = 0; c < headers.size(); ++c )
        {
            if ( c < pillarNames.size() && r < pillarNames[c].size() )
            {
                row.push_back( etrading::Variant( pillarNames[c][r].getCString() ) );
                row.push_back( etrading::Variant( c < deltas.size() && r < deltas[c].size() ? deltas[c][r] : 0.0 ) );
            }
            else
            {
                row.push_back( etrading::Variant( "" ) );
                row.push_back( etrading::Variant( "" ) );
            }
        }
        result.push_back( row );
    }

    return swig::fromVariantMatrixToMatrixOfString( result );

    AQ_API_END
}
