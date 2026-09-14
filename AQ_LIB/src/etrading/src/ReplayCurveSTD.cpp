// replayCurveSTD.cpp

#include "ReplayCurveSTD.h"
#include "ReadDataFile.h"
#include "AQLUpdateStaticDataManager.h"
#include "InitializeETrading.h"

namespace etrading
{
    using etrading::InitializeETrading;

    const AQLString replayCurveSTD( const ReadDataFile::Load& inputFile )
    {
        // 1. Read the test file, tryMir/tryMe tests use diff key names
        // ---------------------
        AQLString        curveID             = inputFile.getOptional( "curveID", inputFile.getOptional( "curveCollection", AQLString() ) );
        AQLString        marketName          = inputFile.getOptional( "marketName", inputFile.getOptional( "staticDataTable", AQLString() ) );
        AQLStringMatrix    generalProps        = inputFile.getOptional( "generateProp", inputFile.getOptional( "generalProps", inputFile.getOptional( "curveConv", AQLStringMatrix() ) ) );
        AQLStringMatrix    moneyConv           = inputFile.getOptional( "moneyConv", inputFile.getOptional( "moneyMarketConv", AQLStringMatrix() ) );
        AQLStringMatrix    liborRates          = inputFile["liborRates"];
        AQLStringMatrix    liborConv           = inputFile["liborConv"];
        AQLStringMatrix    swapRates           = inputFile["swapRates"];
        AQLStringMatrix    swapConv            = inputFile["swapConv"];
        AQLStringMatrix    fra3mRates          = inputFile["fra3mRates"];
        AQLStringMatrix    fra6mRates          = inputFile["fra6mRates"];
        AQLStringMatrix    fraConv             = inputFile["fraConv"];
        AQLStringMatrix    futureRates         = inputFile["futureRates"];
        AQLStringMatrix    futureConv          = inputFile["futureConv"];
        AQLStringMatrix    adjustSwapConv      = inputFile.getOptional( "adjustSwapConv", inputFile.getOptional( "convexityAdjConv", AQLStringMatrix() ) );
        AQLStringMatrix    adjustSwapRates     = inputFile.getOptional( "adjustSwapRates", inputFile.getOptional( "convexityAdjRates", AQLStringMatrix() ) );
        AQLString        curveNames          = inputFile.getOptional( "curveNames", inputFile.getOptional( "curveIndex", AQLString() ) );

        // 2. Check Test Parameters
        // ------------------------
        AQ_THROW_IF( !generalProps.empty() && generalProps[0].size() < 2, "GeneralProps Matrix column size must be 2" );

        AQ_THROW_IF( !moneyConv.empty() && moneyConv[0].size() < 2, "MoneyConv Matrix column size must be 2" );

        AQ_THROW_IF( !liborConv.empty() && liborConv[0].size() < 2, "LiborConv Matrix column size must be 2" );

        AQ_THROW_IF( !liborRates.empty() && liborRates[0].size() < 2, "LiborRates Matrix column size must be 2" );

        AQ_THROW_IF( !swapConv.empty() && swapConv[0].size() < 2, "SwapConv Matrix column size must be 2" );

        AQ_THROW_IF( !swapRates.empty() && swapRates[0].size() < 2, "SwapRates Matrix column size must be 2" );

        AQ_THROW_IF( !fraConv.empty() && fraConv[0].size() < 2, "FraConv Matrix column size must be 2" );

        AQ_THROW_IF( !fra3mRates.empty() && fra3mRates[0].size() < 2, "Fra3mRates Matrix column size must be 2" );

        AQ_THROW_IF( !fra6mRates.empty() && fra6mRates[0].size() < 2, "Fra6mRates Matrix column size must be 2" );

        AQ_THROW_IF( !futureConv.empty() && futureConv[0].size() < 2, "FutureConv Matrix column size must be 2" );

        AQ_THROW_IF( !futureRates.empty() && futureRates[0].size() < 3, "FutureRates Matrix column size must be greater or equal to 3" );

        AQ_THROW_IF( !adjustSwapConv.empty() && adjustSwapConv[0].size() < 2, "AdjustSwapConv Matrix column size must be 2" );

        AQ_THROW_IF( !adjustSwapRates.empty() && adjustSwapRates[0].size() < 2, "AdjustSwapRates Matrix column size must be 2" );

        // 3. Build the STD Curve
        // ----------------------
        const bool checkStaticDataLoaded = true;
        AQLUpdateStaticDataManager::setUpSwapCurve( InitializeETrading::instance( checkStaticDataLoaded ).dataInstance(),
                                          curveID,
                                          marketName,
                                          generalProps,
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
                                          AQLString( "INVALID" ) ); // curveName_DF2: This parameter is deprecated

        // 4. Return the Result
        // --------------------
        AQLString result = curveID + " " + marketName + " Curve has been set";
        return result;
    }
}
